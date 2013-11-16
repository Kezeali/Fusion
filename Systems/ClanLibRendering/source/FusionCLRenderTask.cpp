/*
*  Copyright (c) 2013 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "PrecompiledHeaders.h"

#include "FusionCLRenderTask.h"

#include "FusionCLRenderSystem.h"

#include "FusionDeltaTime.h"
#include "FusionCLRenderComponent.h"
#include "FusionCLRenderExtension.h"
#include "FusionRenderer.h"

#include "FusionEntity.h"

// TODO: move into StreamingSystem?
#include "FusionStreamingCameraComponent.h"
#include "FusionCameraSynchroniser.h"

#include "FusionPlayerRegistry.h"
#include "FusionInputHandler.h"

#include "FusionProfiling.h"
#include "FusionNetworkManager.h"
#include "FusionRakNetwork.h"
#include <RakNet/RakNetStatistics.h>

#include "FusionPhysicsDebugDraw.h"

#include <Rocket/Core.h>

#include <tbb/parallel_sort.h>
#include <tbb/parallel_for.h>

namespace FusionEngine
{

	CLRenderTask::CLRenderTask(CLRenderWorld* sysworld, Renderer* const renderer)
		: TaskBase(sysworld, "CLRenderTask"),
		m_RenderWorld(sysworld),
		m_Renderer(renderer)
	{
		auto canvas = m_Renderer->GetCanvas();
		m_DebugFont = clan::Font(canvas, "Lucida Console", 14);
		m_DebugFont2 = clan::Font(canvas, "Lucida Console", 10);
	}

	CLRenderTask::~CLRenderTask()
	{
	}

	void CLRenderTask::SetRenderAction(const RenderAction& action)
	{
		m_RenderActions[action.identifier] = action;
	}

	void CLRenderTask::RemoveRenderAction(const RenderAction& action)
	{
		m_RenderActions.erase(action.identifier);
	}

//#define FSN_CLRENDER_PARALLEL_UPDATE

	void CLRenderTask::Update()
	{
		m_RenderWorld->AddQueuedViewports();

		auto& cameras = m_RenderWorld->GetCameras();
		auto& drawables = m_RenderWorld->GetDrawables();
		auto& sprites = m_RenderWorld->GetSprites();

		{
		FSN_PROFILE("UpdateCameraPositions")
		for (auto it = cameras.cbegin(), end = cameras.cend(); it != end; ++it)
		{
			auto& camera = *it;
			camera->Update(DeltaTime::GetActualDeltaTime(), DeltaTime::GetInterpolationAlpha());

			if (camera->m_ViewportEnabled)
			{
				if (!camera->m_Viewport)
				{
					camera->m_Viewport = std::make_shared<Viewport>(camera->m_ViewportRect, camera->m_Camera);
					m_RenderWorld->AddViewport(camera->m_Viewport);
				}
			}
			else if (camera->m_Viewport)
			{
				m_RenderWorld->RemoveViewport(camera->m_Viewport);
				camera->m_Viewport.reset();
			}
		}
		}

		{
			FSN_PROFILE("Animate");
			float animationDt = std::min(DeltaTime::GetDeltaTime(), DeltaTime::GetActualDeltaTime());

			tbb::parallel_for(tbb::blocked_range<size_t>(0, sprites.size()), [&](const tbb::blocked_range<size_t>& r)
			{
				for (auto i = r.begin(), end = r.end(); i != end; ++i)
				{
					auto& sprite = sprites[i];

					sprite->Update(DeltaTime::GetTick(), animationDt, DeltaTime::GetInterpolationAlpha());
				}
			});
		}

		{
			FSN_PROFILE("Depth Sort");
			tbb::parallel_sort(drawables.begin(), drawables.end(), [](const boost::intrusive_ptr<IDrawable>& first, const boost::intrusive_ptr<IDrawable>& second)->bool
			{
				if (first->GetParent() == second->GetParent())
				{
					if (first->GetLocalDepth() < second->GetLocalDepth())
						return true;
				}
				else if (first->GetEntityDepth() == second->GetEntityDepth())
				{
					auto firstPos = first->GetPosition();
					auto secondPos = second->GetPosition();
					if (firstPos.y == secondPos.y)
					{
						if (first->GetParent() < second->GetParent())
							return true;
					}
					else if (firstPos.y < secondPos.y)
						return true;
				}
				else if (first->GetEntityDepth() < second->GetEntityDepth())
					return true;

				return false;
			});
		}

		Draw();
	}

	namespace
	{
		void DrawSegment(Vector2 p1, Vector2 p2, clan::Canvas& canvas)
		{
			clan::Colorf clcolor(1.f, 0.5f, 0.9f, 1.0f);

			canvas.draw_line(
				clan::Pointf((p1.x * s_GameUnitsPerSimUnit), (p1.y * s_GameUnitsPerSimUnit)),
				clan::Pointf((p2.x * s_GameUnitsPerSimUnit), (p2.y * s_GameUnitsPerSimUnit)),
				clcolor);
		}
	}

	void CLRenderTask::Draw()
	{
		FSN_PROFILE("Draw");
		auto& drawables = m_RenderWorld->GetDrawables();

		clan::Canvas canvas = m_Renderer->GetCanvas();

		auto& viewports = m_RenderWorld->GetViewports();
		auto worldGUICtx = Rocket::Core::GetContext("world");
		for (auto it = viewports.begin(), end = viewports.end(); it != end; ++it)
		{
			const auto& vp = *it;
			const auto& camera = vp->GetCamera();

			clan::Rectf drawArea;
			m_Renderer->SetupDraw(vp, &drawArea);

			const auto& p = camera->GetPosition();
			
			Vector2 camera_pos(p.x, p.y);
			for (auto dit = drawables.begin(), dend = drawables.end(); dit != dend; ++dit)
			{
				auto& drawable = *dit;
				if (!drawable->HasAABB() || drawArea.is_overlapped(drawable->GetAABB()))
				{
					drawable->Draw(canvas, camera_pos);
				}
			}

			//{
			//	FSN_PROFILE("GUI");
			//	worldGUICtx->Render();
			//}

			m_RenderWorld->RunExtensions(vp, canvas);

			m_Renderer->PostDraw();
		}

		for (auto it = viewports.begin(), end = viewports.end(); it != end; ++it)
		{
			const auto& vp = *it;
			const auto& camera = vp->GetCamera();

			const auto& p = camera->GetPosition();
			
			Vector2 camera_pos(p.x, p.y);

			for (const auto& action : m_RenderActions)
			{
				if (action.second.func)
					action.second.func(canvas, camera_pos);
			}
		}

		//{
		//	FSN_PROFILE("GUI");
		//	for (int i = 0; i < Rocket::Core::GetNumContexts(); ++i)
		//	{
		//		auto ctx = Rocket::Core::GetContext(i);
		//		if (ctx != worldGUICtx)
		//			ctx->Render();
		//	}
		//}

		if (m_RenderWorld->IsDebugTextEnabled())
		{
			{
				std::stringstream str;
				str << DeltaTime::GetFramesSkipped();
				std::string debug_text = "Frames Skipped: " + str.str();
				str.str("");
				str << DeltaTime::GetDeltaTime();
				debug_text += "\nDT: " + str.str() + "sec";
				str.str("");
				str << viewports.size();
				debug_text += "\nViewports: " + str.str();
				m_DebugFont.draw_text(canvas, clan::Pointf(10.f, 40.f), debug_text);

				clan::Rectf bar(clan::Pointf(10.f, 4.f), clan::Sizef((float)(canvas.get_width() - 20), 14.f));
				clan::Rectf fill = bar;
				fill.set_width(bar.get_width() * DeltaTime::GetInterpolationAlpha());
				clan::Colorf c1 = clan::Colorf::aqua;
				clan::Colorf c0 = c1;
				c0.set_alpha(0.25f);
				c1.set_alpha(DeltaTime::GetInterpolationAlpha());
				canvas.draw_box(bar, clan::Colorf::silver);
				canvas.fill_rect(fill, clan::Gradient(c0, c1, c0, c1));
			}

#ifdef FSN_PROFILING_ENABLED
			{
				const auto pf = Profiling::getSingleton().GetTimes();
				clan::Pointf pfLoc(10.f, 110.f);
				for (auto it = pf.begin(), end = pf.end(); it != end; ++it)
				{
					std::stringstream secStr, percentStr;
					secStr.setf(std::ios::fixed);
					secStr.precision(5);
					percentStr.precision(3);

					secStr << it->second;
					std::string line = it->first + ": " + secStr.str() + "sec";

					if (it->second > 0.0 && DeltaTime::GetDeltaTime() > 0.f)
					{
						percentStr << (it->second / DeltaTime::GetDeltaTime()) * 100.f;
						line += " (" + percentStr.str() + "%)";
					}

					m_DebugFont.draw_text(canvas, pfLoc, line);

					pfLoc.y += m_DebugFont.get_text_size(canvas.get_gc(), line).height;
				}
			}
#endif

			{
				auto network = NetworkManager::GetNetwork();

				if (network->IsConnected())
				{
					clan::Pointf pfLoc(400.f, 40.f);

					unsigned short numberSystems = network->GetPeerInterface()->GetMaximumNumberOfPeers();
					network->GetPeerInterface()->GetConnectionList(nullptr, &numberSystems);
					std::vector<RakNet::SystemAddress> remoteSystems(numberSystems);
					network->GetPeerInterface()->GetConnectionList(remoteSystems.data(), &numberSystems);

					RakNet::RakNetStatistics stats;
					for (unsigned short i = 0; i < numberSystems; ++i)
					{
						network->GetPeerInterface()->GetStatistics(remoteSystems[i], &stats);

						std::vector<std::string> lines;

						{
							char address[256];
							remoteSystems[i].ToString(true, address);
							lines.push_back(std::string(address) + ":");
						}

						std::stringstream str;

						int ping = network->GetPeerInterface()->GetAveragePing(remoteSystems[i]);
						str << ping;
						lines.push_back("Ping: " + str.str());
						str.str("");

						str << stats.BPSLimitByCongestionControl;
						lines.push_back(
							std::string("Congested: ") + std::string(stats.isLimitedByCongestionControl ? "yes" : "no") + std::string(stats.isLimitedByOutgoingBandwidthLimit ? " (arb)" : "") + std::string("  BPS limit: ") + str.str()
							);
						str.str("");
						for (size_t p = 0; p < NUMBER_OF_PRIORITIES; ++p)
						{
							str << "P" << p;
							str << "  Msg: " << stats.messageInSendBuffer[p] << " (r " << stats.messagesInResendBuffer << ")";
							str << "  Byt: " << stats.bytesInSendBuffer[p] << " (r " << stats.bytesInResendBuffer << ")";
							lines.push_back(str.str());
							str.str("");
						}
						str << "Packet loss: " << stats.packetlossLastSecond;
						lines.push_back(str.str());
						str.str("");

						lines.push_back("---");

						for (auto it = lines.begin(), end = lines.end(); it != end; ++it)
						{
							m_DebugFont2.draw_text(canvas, pfLoc, *it);
							pfLoc.y += m_DebugFont2.get_text_size(canvas.get_gc(), *it).height;
						}
					}
				}
			}
		}

		if (!m_PhysDebugDraw)
		{
			m_PhysDebugDraw.reset(new B2DebugDraw(m_Renderer->GetCanvas()));
			if (m_RenderWorld->m_PhysWorld)
				m_RenderWorld->m_PhysWorld->SetDebugDraw(m_PhysDebugDraw.get());
			m_PhysDebugDraw->SetFlags(B2DebugDraw::e_centerOfMassBit | B2DebugDraw::e_jointBit | B2DebugDraw::e_pairBit | B2DebugDraw::e_shapeBit);
		}

		if (m_PhysDebugDraw && m_RenderWorld->m_PhysDebugDrawEnabled && !viewports.empty())
		{
			FSN_PROFILE("PhysicsDebugDraw");
			//m_PhysDebugDraw->SetViewport(viewports.front());
			m_PhysDebugDraw->SetupView();

			m_Renderer->SetupDraw(viewports.front());

			// Draw cell division lines
			static const float cellSize = 5.0f;
			clan::Rectf area;
			m_Renderer->CalculateScreenArea(area, viewports.front(), true);
			area.top *= s_SimUnitsPerGameUnit; area.right *= s_SimUnitsPerGameUnit; area.bottom *= s_SimUnitsPerGameUnit; area.left *= s_SimUnitsPerGameUnit;
			//auto center = area.get_center();
			//auto x = std::floor(area.left / cellSize) * cellSize, y = std::floor(area.top / cellSize) * cellSize;
			for (auto iy = (int)std::floor(area.top / cellSize) * cellSize; iy < area.bottom; iy += cellSize)
			{
				for (auto ix = (int)std::floor(area.left / cellSize) * cellSize; ix < area.right; ix += cellSize)
				{
					//auto x = std::floor(ix / 8) * 8, y = std::floor(iy / 8) * 8;
					DrawSegment(Vector2(ix, iy), Vector2(ix + cellSize, iy), canvas);
					DrawSegment(Vector2(ix, iy), Vector2(ix, iy + cellSize), canvas);
				}
			}
			const auto textHeight = m_DebugFont2.get_text_size(canvas, "1").height;
			for (auto iy = (int)std::floor(area.top / cellSize) * cellSize; iy < area.bottom; iy += cellSize)
			{
				for (auto ix = (int)std::floor(area.left / cellSize) * cellSize; ix < area.right; ix += cellSize)
				{
					std::stringstream str; str << (ix / cellSize) << "," << (iy / cellSize);
					m_DebugFont2.draw_text(canvas, ToRenderUnits(ix), ToRenderUnits(iy) + textHeight, str.str(), clan::Colorf::bisque);
				}
			}

			if (m_RenderWorld->m_PhysWorld)
				m_RenderWorld->m_PhysWorld->DrawDebugData();

			m_Renderer->PostDraw();

			m_PhysDebugDraw->ResetView();
		}
	}

	void CLRenderWorld_AddViewport(const CameraPtr& camera, CLRenderWorld* obj)
	{
		auto viewport = std::make_shared<Viewport>(clan::Rectf(0.f, 0.f, 1.f, 1.f), camera);
		obj->AddViewport(viewport);
	}

	void CLRenderWorld::Register(asIScriptEngine* engine)
	{
		RegisterSingletonType<CLRenderSystem>("Renderer", engine);
		engine->RegisterObjectMethod("Renderer", "void addViewport(const Camera &in)", asFUNCTION(CLRenderWorld_AddViewport), asCALL_CDECL_OBJLAST);
	}


	CLRenderGUITask::CLRenderGUITask(CLRenderWorld* sysworld, const clan::Canvas& canvas, Renderer* const renderer)
		: TaskBase(sysworld, "CLRenderGUITask"),
		m_RenderWorld(sysworld),
		m_Renderer(renderer),
		m_Canvas(canvas)
	{
		m_MouseMoveSlot = canvas.get_window().get_ic().get_mouse().sig_pointer_move().connect(this, &CLRenderGUITask::onMouseMove);
	}

	CLRenderGUITask::~CLRenderGUITask()
	{
	}

	inline int getRktModifierFlags(const clan::InputEvent &ev)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;
		return modifier;
	}

	void CLRenderGUITask::Update()
	{
		auto context = Rocket::Core::GetContext(0);

		auto& viewports = m_RenderWorld->GetViewports();
		while (!m_BufferedEvents.empty())
		{
			clan::InputEvent ev = m_BufferedEvents.front();
			m_BufferedEvents.pop_front();

			auto mousePos = ev.mouse_pos;
			auto keyState = getRktModifierFlags(ev);

			for (auto it = viewports.begin(), end = viewports.end(); it != end; ++it)
			{
				const auto& camera = (*it)->GetCamera();

				// Calculate the area on-screen that the viewport takes up
				clan::Rectf screenArea;
				m_Renderer->CalculateScreenArea(screenArea, *it, false);

				if (screenArea.contains(clan::Vec2i(mousePos.x, mousePos.y)))
				{
					// Calculate the offset within the world of the viewport
					clan::Rectf worldArea;
					m_Renderer->CalculateScreenArea(worldArea, *it, true);

					mousePos.x += (int)worldArea.left;
					mousePos.y += (int)worldArea.top;

					//for (int i = 0; i < Rocket::Core::GetNumContexts(); ++i)
					{
						context->ProcessMouseMove(mousePos.x, mousePos.y, keyState);
					}
				}
			}
		}

		context->Update();
	}

	void CLRenderGUITask::onMouseMove(const clan::InputEvent &ev)
	{
		//if (m_ClickPause <= 0)
		//	m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, getRktModifierFlags(ev));
		m_BufferedEvents.push_back(ev);
	}

}
