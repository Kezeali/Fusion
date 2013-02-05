/*
*  Copyright (c) 2011-2012 Fusion Project Team
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
#include <RakNet/RakNetStatistics.h>

#include "FusionPhysicsDebugDraw.h"

#include <Rocket/Core.h>

#include <tbb/parallel_sort.h>
#include <tbb/parallel_for.h>

#include <boost/thread/mutex.hpp>

namespace FusionEngine
{
	CLRenderSystem::CLRenderSystem(const clan::Canvas& canvas, CameraSynchroniser* camera_sync)
		: m_Canvas(canvas),
		m_CameraSynchroniser(camera_sync)
	{
	}

	void CLRenderSystem::RegisterScriptInterface(asIScriptEngine* engine)
	{
		CLRenderWorld::Register(engine);
	}

	std::shared_ptr<ISystemWorld> CLRenderSystem::CreateWorld()
	{
		return std::make_shared<CLRenderWorld>(this, m_Canvas, m_CameraSynchroniser);
	}

	CLRenderWorld::CLRenderWorld(IComponentSystem* system, const clan::Canvas& canvas, CameraSynchroniser* camera_sync)
		: ISystemWorld(system),
		m_PhysWorld(nullptr),
		m_PhysDebugDrawEnabled(false),
		m_DebugTextEnabled(false),
		m_CameraManager(camera_sync)
	{
		m_Renderer = new Renderer(canvas);
		m_RenderTask = new CLRenderTask(this, m_Renderer);
		m_GUITask = new CLRenderGUITask(this, canvas, m_Renderer);

		m_SpriteDefinitionCache = std::make_shared<SpriteDefinitionCache>();

		Console::getSingleton().BindCommand("phys_debug_draw", [this](const std::vector<std::string>& params)->std::string
		{
			if (params.size() == 1 || params[1] == "on")
				this->m_PhysDebugDrawEnabled = true;
			else
				this->m_PhysDebugDrawEnabled = false;
			return "";
		});

		Console::getSingleton().BindCommand("r_debug_text", [this](const std::vector<std::string>& params)->std::string
		{
			if (params.size() == 1 || params[1] == "on")
				this->m_DebugTextEnabled = true;
			else
				this->m_DebugTextEnabled = false;
			return "";
		});
	}

	CLRenderWorld::~CLRenderWorld()
	{
		Console::getSingleton().UnbindCommand("r_debug_text");
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

	void CLRenderWorld::RunExtensions(const ViewportPtr& vp, clan::Canvas& canvas)
	{
		std::vector<ViewportPtr> removed;
		auto range = m_Extensions.equal_range(vp);
		for (auto it = range.first; it != range.second; ++it)
		{
			if (auto extension = it->second.lock())
				extension->Draw(canvas);
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
				if (camComponent->m_Viewport)
				{
					// Hack to remove viewport from the queue
					std::vector<ViewportPtr> keptViewports;
					ViewportPtr viewport;
					while (m_ViewportsToAdd.try_pop(viewport))
					{
						if (viewport != camComponent->m_Viewport)
							keptViewports.push_back(viewport);
					}
					for (auto it = keptViewports.begin(); it != keptViewports.end(); ++it)
					{
						m_ViewportsToAdd.push(*it);
					}
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
		auto canvas = m_Renderer->GetCanvas();
		m_DebugFont = clan::Font(canvas, "Lucida Console", 14);
		m_DebugFont2 = clan::Font(canvas, "Lucida Console", 10);
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

		//{
		//	FSN_PROFILE("DefineNewSprites");
		//	auto& spritesToDefine = m_RenderWorld->GetSpritesToDefine();
		//	for (auto it = sprites.begin(); it != sprites.end(); ++it)
		//	{
		//		const auto& sprite = *it;
		//		if (sprite->RequiresSpriteDefinition())
		//			m_RenderWorld->GetSpritesToDefine().push_back(sprite);
		//	}
		//	tbb::parallel_for(tbb::blocked_range<size_t>(0, std::min(spritesToDefine.size(), 10u)), [&spritesToDefine](const tbb::blocked_range<size_t>& r)
		//	{
		//		for (auto i = r.begin(), end = r.end(); i != end; ++i)
		//		{
		//			auto& sprite = spritesToDefine[i];
		//			
		//			sprite->DefineSpriteIfNecessary();
		//		}
		//	});

		//	//for (auto it = sprites.begin(); it != sprites.end(); ++it)
		//	//{
		//	//	const auto& sprite = *it;
		//	//	sprite->CreateSpriteIfNecessary(gc);
		//	//}
		//}

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

			//clan::Vec2i positions[] =
			//{
			//	clan::Vec2i((int)(p1.x * s_GameUnitsPerSimUnit), (int)(p1.y * s_GameUnitsPerSimUnit)),
			//	clan::Vec2i((int)(p2.x * s_GameUnitsPerSimUnit), (int)(p2.y * s_GameUnitsPerSimUnit))
			//};

			//clan::PrimitivesArray vertex_data(gc);
			//vertex_data.set_attributes(clan::attrib_position, clan::VertexArrayVector<clan::Vec2i>(gc, &positions[0], 2));
			//vertex_data.set_attributes(clan::attrib_color, clan::VertexArrayVector<clan::Vec4f>(gc, &clcolor, 1));
			//gc.draw_primitives(clan::type_lines, 2, vertex_data);

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
			//drawArea.translate(p);
			
			Vector2 camera_pos(p.x, p.y);
			for (auto dit = drawables.begin(), dend = drawables.end(); dit != dend; ++dit)
			{
				auto& drawable = *dit;
				if (!drawable->HasAABB() || drawArea.is_overlapped(drawable->GetAABB()))
				{
					drawable->Draw(canvas, camera_pos);
				}
			}

			{
				FSN_PROFILE("GUI");
				worldGUICtx->Render();
			}

			m_RenderWorld->RunExtensions(vp, canvas);

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
		: ISystemTask(sysworld),
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

	void CLRenderGUITask::Update(const float delta)
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
