/*
*  Copyright (c) 2011 Fusion Project Team
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
#include "FusionGUI.h"

#include "FusionProfiling.h"
#include "FusionNetworkManager.h"
#include "FusionRakNetwork.h"
#include <RakNetStatistics.h>

#include "FusionPhysicsDebugDraw.h"

#include <Rocket/Core.h>

#include <tbb/parallel_sort.h>
#include <tbb/parallel_for.h>

#include <boost/thread/mutex.hpp>

namespace FusionEngine
{
	CLRenderSystem::CLRenderSystem(const CL_DisplayWindow& window, CameraSynchroniser* camera_sync)
		: m_DisplayWindow(window),
		m_CameraSynchroniser(camera_sync)
	{
	}

	void CLRenderSystem::RegisterScriptInterface(asIScriptEngine* engine)
	{
		CLRenderWorld::Register(engine);
	}

	std::shared_ptr<ISystemWorld> CLRenderSystem::CreateWorld()
	{
		return std::make_shared<CLRenderWorld>(this, m_DisplayWindow, m_CameraSynchroniser);
	}

	CLRenderWorld::CLRenderWorld(IComponentSystem* system, const CL_DisplayWindow& window, CameraSynchroniser* camera_sync)
		: ISystemWorld(system),
		m_CameraManager(camera_sync),
		m_PhysWorld(nullptr),
		m_PhysDebugDrawEnabled(false)
	{
		m_Renderer = new Renderer(window.get_gc());
		m_RenderTask = new CLRenderTask(this, m_Renderer);
		m_GUITask = new CLRenderGUITask(this, window, m_Renderer);

		Console::getSingleton().BindCommand("phys_debug_draw", [this](const std::vector<std::string>& params)->std::string
		{
			if (params.size() == 1 || params[1] == "on")
				this->m_PhysDebugDrawEnabled = true;
			else
				this->m_PhysDebugDrawEnabled = false;
			return "";
		});
	}

	CLRenderWorld::~CLRenderWorld()
	{
		Console::getSingleton().UnbindCommand("phys_debug_draw");

		delete m_GUITask;
		delete m_RenderTask;
		delete m_Renderer;
	}

	void CLRenderWorld::AddViewport(const ViewportPtr& viewport)
	{
		m_ViewportsToAdd.push(viewport);
	}

	void CLRenderWorld::RemoveViewport(const ViewportPtr& viewport)
	{
		boost::mutex::scoped_lock lock(m_ViewportMutex);
		auto _where = std::find(m_Viewports.begin(), m_Viewports.end(), viewport);
		if (_where != m_Viewports.end())
			m_Viewports.erase(_where);

		m_Extensions.erase(viewport);
	}

	void CLRenderWorld::AddQueuedViewports()
	{
		boost::mutex::scoped_lock lock(m_ViewportMutex);
		ViewportPtr viewport;
		while (m_ViewportsToAdd.try_pop(viewport))
		{
			//m_Viewports.clear();
			m_Viewports.push_back(viewport);
		}
	}

	void CLRenderWorld::AddRenderExtension(const std::weak_ptr<CLRenderExtension>& extension, const ViewportPtr& vp)
	{
		m_Extensions.insert(std::make_pair(vp, extension));
	}

	void CLRenderWorld::RunExtensions(const ViewportPtr& vp, const CL_GraphicContext& gc)
	{
		std::vector<ViewportPtr> removed;
		auto range = m_Extensions.equal_range(vp);
		for (auto it = range.first; it != range.second; ++it)
		{
			if (auto extension = it->second.lock())
				extension->Draw(gc);
			else
				removed.push_back(vp);
		}
		for (auto it = removed.begin(), end = removed.end(); it != end; ++it)
			m_Extensions.erase(*it);
	}

	std::vector<std::string> CLRenderWorld::GetTypes() const
	{
		//static const std::string types[] = { "CLSprite", "" };
		//return std::vector<std::string>(types, types + sizeof(types));
		std::vector<std::string> types;
		types.push_back("CLSprite");
		types.push_back("StreamingCamera");
		return types;
	}

	ComponentPtr CLRenderWorld::InstantiateComponent(const std::string& type)
	{
		ComponentPtr com;
		if (type == "CLSprite")
		{
			com = new CLSprite();
		}
		else if (type == "StreamingCamera")
		{
			com = new StreamingCamera();
		}
		return com;
	}

	void CLRenderWorld::OnActivation(const ComponentPtr& component)
	{
		if (component->GetType() == "CLSprite")
		{
			auto sprite = boost::dynamic_pointer_cast<CLSprite>(component);
			if (sprite)
			{
				FSN_ASSERT(std::find(m_Drawables.begin(), m_Drawables.end(), sprite) == m_Drawables.end());
				m_Drawables.push_back(sprite);
				m_Sprites.push_back(sprite);
			}
		}
		if (component->GetType() == "StreamingCamera")
		{
			if (auto camComponent = boost::dynamic_pointer_cast<StreamingCamera>(component))
			{
				const bool shared = camComponent->GetSyncType() == ICamera::Shared;
				const bool synced = camComponent->GetParent()->IsSyncedEntity();

				if (synced)
				{
					// Shared cameras have no owner
					const PlayerID owner = shared ? 0 : camComponent->GetParent()->GetOwnerID();
					camComponent->m_Camera = m_CameraManager->GetCamera(camComponent->GetParent()->GetID(), owner);
				}
				else
				{
					// Attached to a pseudo-entity: create an un-synchronised camera
					camComponent->m_Camera = std::make_shared<Camera>();
					// Unsynchronised cameras are only useful for creating viewports at the moment
					SendToConsole("Warning: unsynchronised camera created: cameras attached to unsynchronised entities won't cause the map to load.");
				}

				if (shared || PlayerRegistry::IsLocal(camComponent->GetParent()->GetOwnerID()) || !synced)
				{
					if (camComponent->m_ViewportEnabled)
					{
						camComponent->m_Viewport = std::make_shared<Viewport>(camComponent->m_ViewportRect, camComponent->m_Camera);
						AddViewport(camComponent->m_Viewport);
					}
				}
				m_Cameras.push_back(camComponent);
			}
		}
	}

	void CLRenderWorld::OnDeactivation(const ComponentPtr& component)
	{
		auto drawable = boost::dynamic_pointer_cast<IDrawable>(component);
		if (drawable)
		{
			auto _where = std::find(m_Drawables.begin(), m_Drawables.end(), drawable);
			if (_where != m_Drawables.end())
			{
				m_Drawables.erase(_where);
			}

			auto sprite = boost::dynamic_pointer_cast<CLSprite>(component);
			if (sprite)
			{
				auto _where = std::find(m_Sprites.begin(), m_Sprites.end(), sprite);
				if (_where != m_Sprites.end())
				{
					m_Sprites.erase(_where);
				}
			}
		}
		else if (component->GetType() == "StreamingCamera")
		{
			if (auto camComponent = boost::dynamic_pointer_cast<StreamingCamera>(component))
			{
				if (camComponent->m_ViewportEnabled && camComponent->m_Viewport)
				{
					RemoveViewport(camComponent->m_Viewport);
					camComponent->m_Viewport.reset();
				}

				auto _where = std::find(m_Cameras.begin(), m_Cameras.end(), camComponent);
				if (_where != m_Cameras.end())
				{
					m_Cameras.erase(_where);
				}

				// If the component being deactivated should be kept active by this camera then
				//  it must have been removed
				if (camComponent->GetParent()->IsSyncedEntity())
				{
					const bool shared = camComponent->GetSyncType() == ICamera::Shared;
					const PlayerID owner = camComponent->GetParent()->GetOwnerID();
					if (shared || PlayerRegistry::IsLocal(owner))
					{
						m_CameraManager->RemoveCamera(camComponent->GetParent()->GetID());
					}
				}
			}
		}
	}

	void CLRenderWorld::MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
	{
		if (type == "CLSprite")
		{
			CLSprite::DeltaSerialiser_t::copyChanges(result, current_data, new_data);
		}
	}

	std::vector<ISystemTask*> CLRenderWorld::GetTasks()
	{
		std::vector<ISystemTask*> tasks;
		tasks.push_back(m_RenderTask);
		//tasks.push_back(m_GUITask);
		return tasks;
	}

	CLRenderTask::CLRenderTask(CLRenderWorld* sysworld, Renderer* const renderer)
		: ISystemTask(sysworld),
		m_RenderWorld(sysworld),
		m_Renderer(renderer)
	{
		auto gc = m_Renderer->GetGraphicContext();
		m_DebugFont = CL_Font(gc, "Lucida Console", 14);
		m_DebugFont2 = CL_Font(gc, "Lucida Console", 10);
	}

	CLRenderTask::~CLRenderTask()
	{
	}

//#define FSN_CLRENDER_PARALLEL_UPDATE

	void CLRenderTask::Update(const float delta)
	{
		m_RenderWorld->AddQueuedViewports();

		auto& cameras = m_RenderWorld->GetCameras();
		auto& drawables = m_RenderWorld->GetDrawables();
		auto& sprites = m_RenderWorld->GetSprites();

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

		auto depthSort = [](const boost::intrusive_ptr<IDrawable>& first, const boost::intrusive_ptr<IDrawable>& second)->bool
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
		};

		{
			FSN_PROFILE("Animate");
			float animationDt = std::min(DeltaTime::GetDeltaTime(), DeltaTime::GetActualDeltaTime());
#define FSN_CLRENDER_PARALLEL_UPDATE

#ifdef FSN_CLRENDER_PARALLEL_UPDATE
			//tbb::atomic<bool> outOfOrder;
			//outOfOrder = false;
			tbb::parallel_for(tbb::blocked_range<size_t>(0, sprites.size()), [&](const tbb::blocked_range<size_t>& r)
			{
				for (auto i = r.begin(), end = r.end(); i != end; ++i)
				{
					auto& sprite = sprites[i];
#else
			//bool outOfOrder = false;
			for (auto it = sprites.begin(); it != sprites.end(); ++it)
			{
				auto& sprite = *it;
#endif
				sprite->Update(DeltaTime::GetTick(), animationDt, DeltaTime::GetInterpolationAlpha());

				//#ifdef FSN_CLRENDER_PARALLEL_UPDATE
				//			if (!outOfOrder && i != r.begin() && !depthSort(drawables[i - 1], drawable))
				//				outOfOrder = true;
				//#else
				//			if (!outOfOrder && it != drawables.begin() && !depthSort(*(it - 1), drawable))
				//				outOfOrder = true;
				//#endif
			}
#ifdef FSN_CLRENDER_PARALLEL_UPDATE
			});
#endif
		}

		//if (outOfOrder)
		{
			FSN_PROFILE("Depth Sort");
			tbb::parallel_sort(drawables.begin(), drawables.end(), depthSort);
		}

		Draw();
	}

	static void DrawSegment(Vector2 p1, Vector2 p2, CL_GraphicContext& gc)
	{
		CL_Colorf clcolor(1.f, 0.5f, 0.9f, 1.0f);

		CL_Vec2i positions[] =
		{
			CL_Vec2i((int)(p1.x * s_GameUnitsPerSimUnit), (int)(p1.y * s_GameUnitsPerSimUnit)),
			CL_Vec2i((int)(p2.x * s_GameUnitsPerSimUnit), (int)(p2.y * s_GameUnitsPerSimUnit))
		};

		CL_PrimitivesArray vertex_data(gc);
		vertex_data.set_attributes(0, positions);
		vertex_data.set_attribute(1, clcolor);
		gc.draw_primitives(cl_lines, 2, vertex_data);
	}

	void CLRenderTask::Draw()
	{
		FSN_PROFILE("Draw");
		auto& drawables = m_RenderWorld->GetDrawables();

		CL_GraphicContext gc = m_Renderer->GetGraphicContext();

		auto& viewports = m_RenderWorld->GetViewports();
		auto worldGUICtx = Rocket::Core::GetContext("world");
		for (auto it = viewports.begin(), end = viewports.end(); it != end; ++it)
		{
			const auto& vp = *it;

			const auto& camera = vp->GetCamera();

			CL_Rectf drawArea;
			m_Renderer->SetupDraw(vp, &drawArea);

			const auto& p = camera->GetPosition();
			//drawArea.translate(p);
			
			Vector2 camera_pos(p.x, p.y);
			for (auto dit = drawables.begin(), dend = drawables.end(); dit != dend; ++dit)
			{
				auto& drawable = *dit;
				if (!drawable->HasAABB() || drawArea.is_overlapped(drawable->GetAABB()))
				{
					drawable->Draw(gc, camera_pos);
				}
			}

			{
				FSN_PROFILE("GUI");
				worldGUICtx->Render();
			}

			m_RenderWorld->RunExtensions(vp, gc);

			m_Renderer->PostDraw();
		}

		{
			FSN_PROFILE("GUI");
			for (int i = 0; i < Rocket::Core::GetNumContexts(); ++i)
			{
				auto ctx = Rocket::Core::GetContext(i);
				if (ctx != worldGUICtx)
					ctx->Render();
			}
		}

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
			m_DebugFont.draw_text(gc, CL_Pointf(10.f, 40.f), debug_text);

			CL_Rectf bar(CL_Pointf(10.f, 4.f), CL_Sizef((float)(gc.get_width() - 20), 14.f));
			CL_Rectf fill = bar;
			fill.set_width(bar.get_width() * DeltaTime::GetInterpolationAlpha());
			CL_Colorf c1 = CL_Colorf::aqua;
			CL_Colorf c0 = c1;
			c0.set_alpha(0.25f);
			c1.set_alpha(DeltaTime::GetInterpolationAlpha());
			CL_Draw::box(gc, bar, CL_Colorf::silver);
			CL_Draw::gradient_fill(gc, fill, CL_Gradient(c0, c1, c0, c1));
		}

		{
			const auto pf = Profiling::getSingleton().GetTimes();
			CL_Pointf pfLoc(10.f, 110.f);
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

				m_DebugFont.draw_text(gc, pfLoc, line);

				pfLoc.y += m_DebugFont.get_text_size(gc, line).height;
			}
		}

		{
			auto network = NetworkManager::GetNetwork();

			if (network->IsConnected())
			{
				CL_Pointf pfLoc(400.f, 40.f);

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
						m_DebugFont2.draw_text(gc, pfLoc, *it);
						pfLoc.y += m_DebugFont2.get_text_size(gc, *it).height;
					}
				}
			}
		}


		if (!m_PhysDebugDraw)
		{
			m_PhysDebugDraw.reset(new B2DebugDraw(m_Renderer->GetGraphicContext()));
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
			CL_Rectf area;
			m_Renderer->CalculateScreenArea(area, viewports.front(), true);
			area.top *= s_SimUnitsPerGameUnit; area.right *= s_SimUnitsPerGameUnit; area.bottom *= s_SimUnitsPerGameUnit; area.left *= s_SimUnitsPerGameUnit;
			//auto center = area.get_center();
			//auto x = std::floor(area.left / 8) * 8, y = std::floor(area.top / 8) * 8;
			for (auto iy = std::floor(area.top / 8.f) * 8; iy < area.bottom; iy += 8)
			{
				for (auto ix = std::floor(area.left / 8.f) * 8; ix < area.right; ix += 8)
				{
					//auto x = std::floor(ix / 8) * 8, y = std::floor(iy / 8) * 8;
					DrawSegment(Vector2(ix, iy), Vector2(ix + 8, iy), gc);
					DrawSegment(Vector2(ix, iy), Vector2(ix, iy + 8), gc);
				}
			}
			const auto textHeight = m_DebugFont2.get_text_size(gc, "1").height;
			for (auto iy = std::floor(area.top / 8.f) * 8; iy < area.bottom; iy += 8)
			{
				for (auto ix = std::floor(area.left / 8.f) * 8; ix < area.right; ix += 8)
				{
					std::stringstream str; str << (ix / 8) << "," << (iy / 8);
					m_DebugFont2.draw_text(gc, ToRenderUnits(ix), ToRenderUnits(iy) + textHeight, str.str(), CL_Colorf::bisque);
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
		auto viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
		obj->AddViewport(viewport);
	}

	void CLRenderWorld::Register(asIScriptEngine* engine)
	{
		RegisterSingletonType<CLRenderSystem>("Renderer", engine);
		engine->RegisterObjectMethod("Renderer", "void addViewport(const Camera &in)", asFUNCTION(CLRenderWorld_AddViewport), asCALL_CDECL_OBJLAST);
	}


	CLRenderGUITask::CLRenderGUITask(CLRenderWorld* sysworld, const CL_DisplayWindow& window, Renderer* const renderer)
		: ISystemTask(sysworld),
		m_RenderWorld(sysworld),
		m_Renderer(renderer),
		m_DisplayWindow(window)
	{
		m_MouseMoveSlot = window.get_ic().get_mouse().sig_pointer_move().connect(this, &CLRenderGUITask::onMouseMove);
	}

	CLRenderGUITask::~CLRenderGUITask()
	{
	}

	inline int getRktModifierFlags(const CL_InputEvent &ev)
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

	void CLRenderGUITask::Update(const float delta)
	{
		auto context = Rocket::Core::GetContext(0);

		auto& viewports = m_RenderWorld->GetViewports();
		while (!m_BufferedEvents.empty())
		{
			CL_InputEvent ev = m_BufferedEvents.front();
			m_BufferedEvents.pop_front();

			auto mousePos = ev.mouse_pos;
			auto keyState = getRktModifierFlags(ev);

			for (auto it = viewports.begin(), end = viewports.end(); it != end; ++it)
			{
				const auto& camera = (*it)->GetCamera();

				// Calculate the area on-screen that the viewport takes up
				CL_Rectf screenArea;
				m_Renderer->CalculateScreenArea(screenArea, *it, false);

				if (screenArea.contains(CL_Vec2i(mousePos.x, mousePos.y)))
				{
					// Calculate the offset within the world of the viewport
					CL_Rectf worldArea;
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

	void CLRenderGUITask::onMouseMove(const CL_InputEvent &ev, const CL_InputState &state)
	{
		//if (m_ClickPause <= 0)
		//	m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, getRktModifierFlags(ev));
		m_BufferedEvents.push_back(ev);
	}

}
