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

#include "FusionStableHeaders.h"

#include "FusionCLRenderSystem.h"

#include "FusionDeltaTime.h"
#include "FusionCLRenderComponent.h"
#include "FusionRenderer.h"

// TODO: move into StreamingSystem?
#include "FusionStreamingCameraComponent.h"
#include "FusionCameraSynchroniser.h"

#include "FusionPlayerRegistry.h"

#include "FusionProfiling.h"
#include "FusionNetworkManager.h"
#include "FusionRakNetwork.h"
#include <RakNetStatistics.h>

#include "FusionPhysicsDebugDraw.h"

#include <tbb/parallel_sort.h>
#include <tbb/parallel_for.h>

namespace FusionEngine
{
	CLRenderSystem::CLRenderSystem(const CL_GraphicContext& gc, CameraSynchroniser* camera_sync)
		: m_GraphicContext(gc),
		m_CameraSynchroniser(camera_sync)
	{
	}

	std::shared_ptr<ISystemWorld> CLRenderSystem::CreateWorld()
	{
		return std::make_shared<CLRenderWorld>(this, m_GraphicContext, m_CameraSynchroniser);
	}

	CLRenderWorld::CLRenderWorld(IComponentSystem* system, const CL_GraphicContext& gc, CameraSynchroniser* camera_sync)
		: ISystemWorld(system),
		m_CameraManager(camera_sync)
	{
		m_Renderer = new Renderer(gc);
		m_RenderTask = new CLRenderTask(this, m_Renderer);
	}

	CLRenderWorld::~CLRenderWorld()
	{
		delete m_RenderTask;
		delete m_Renderer;
	}

	void CLRenderWorld::AddViewport(const ViewportPtr& viewport)
	{
		m_ViewportsToAdd.push(viewport);
	}

	void CLRenderWorld::RemoveViewport(const ViewportPtr& viewport)
	{
		// TODO: make threadsafe
		m_Viewports.erase(std::find(m_Viewports.begin(), m_Viewports.end(), viewport));
	}

	void CLRenderWorld::AddQueuedViewports()
	{
		ViewportPtr viewport;
		while (m_ViewportsToAdd.try_pop(viewport))
		{
			// TEMP:
			m_Viewports.clear();
			m_Viewports.push_back(viewport);
		}
	}

	std::vector<std::string> CLRenderWorld::GetTypes() const
	{
		//static const std::string types[] = { "CLSprite", "" };
		//return std::vector<std::string>(types, types + sizeof(types));
		std::vector<std::string> types;
		types.push_back("CLSprite");
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
			auto drawable = boost::dynamic_pointer_cast<IDrawable>(component);
			if (drawable)
			{
				FSN_ASSERT(std::find(m_Drawables.begin(), m_Drawables.end(), drawable) == m_Drawables.end());
				m_Drawables.push_back(drawable);
			}
		}
		if (component->GetType() == "StreamingCamera")
		{
			if (auto camComponent = boost::dynamic_pointer_cast<StreamingCamera>(component))
			{
				const bool shared = camComponent->GetSyncType() == ICamera::Shared;
				const PlayerID owner = shared ? 0 : camComponent->GetParent()->GetOwnerID();

				camComponent->m_Camera = m_CameraManager->GetCamera(component->GetParent()->GetID(), owner);

				if (shared || PlayerRegistry::IsLocal(camComponent->GetParent()->GetOwnerID()))
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

	ISystemTask* CLRenderWorld::GetTask()
	{
		return m_RenderTask;
	}

	CLRenderTask::CLRenderTask(CLRenderWorld* sysworld, Renderer* const renderer)
		: ISystemTask(sysworld),
		m_RenderWorld(sysworld),
		m_Renderer(renderer)
	{
		auto gc = m_Renderer->GetGraphicContext();
		m_DebugFont = CL_Font(gc, "Lucida Console", 22);
		m_DebugFont2 = CL_Font(gc, "Lucida Console", 14);
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

		for (auto it = cameras.cbegin(), end = cameras.cend(); it != end; ++it)
		{
			auto& camera = *it;
			camera->Update(DeltaTime::GetActualDeltaTime(), DeltaTime::GetInterpolationAlpha());
		}

		auto depthSort = [](boost::intrusive_ptr<IDrawable>& first, boost::intrusive_ptr<IDrawable>& second)->bool
		{
			if (first->GetParent() == second->GetParent())
			{
				if (first->GetLocalDepth() < second->GetLocalDepth())
					return true;
			}
			else if (first->GetEntityDepth() == second->GetEntityDepth())
			{
				if (first->GetParent() < second->GetParent())
					return true;
			}
			else if (first->GetEntityDepth() < second->GetEntityDepth())
				return true;

			return false;
		};

#ifdef FSN_CLRENDER_PARALLEL_UPDATE
		tbb::atomic<bool> outOfOrder;
		outOfOrder = false;
		tbb::parallel_for(tbb::blocked_range<size_t>(0, drawables.size()), [&](const tbb::blocked_range<size_t>& r)
		{
			for (auto i = r.begin(), end = r.end(); i != end; ++i)
			{
				auto& drawable = drawables[i];
#else
		bool outOfOrder = false;
		for (auto it = drawables.begin(); it != drawables.end(); ++it)
		{
			auto& drawable = *it;
#endif
			drawable->Update(DeltaTime::GetTick(), DeltaTime::GetActualDeltaTime(), DeltaTime::GetInterpolationAlpha());

#ifdef FSN_CLRENDER_PARALLEL_UPDATE
			if (!outOfOrder && i != r.begin() && !depthSort(drawables[i - 1], drawable))
				outOfOrder = true;
#else
			if (!outOfOrder && it != drawables.begin() && !depthSort(*(it - 1), drawable))
				outOfOrder = true;
#endif

			// Bubblesort
			//if (it != drawables.begin())
			//{
			//	auto& previous = *(it - 1);
			//	if (depthSort(previous, drawable))
			//		previous.swap(drawable);
			//}
		}
#ifdef FSN_CLRENDER_PARALLEL_UPDATE
		});
#endif

		if (outOfOrder)
			tbb::parallel_sort(drawables.begin(), drawables.end(), depthSort);

		Draw();
	}

	void CLRenderTask::Draw()
	{
		auto& drawables = m_RenderWorld->GetDrawables();

		CL_GraphicContext gc = m_Renderer->GetGraphicContext();

		auto& viewports = m_RenderWorld->GetViewports();
		for (auto it = viewports.begin(), end = viewports.end(); it != end; ++it)
		{
			const auto& camera = (*it)->GetCamera();

			CL_Rectf drawArea;
			m_Renderer->SetupDraw(*it, &drawArea);

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

			m_Renderer->PostDraw();
		}

		std::stringstream str;
		str << DeltaTime::GetFramesSkipped();
		std::string debug_text = "Frames Skipped: " + str.str();
		str.str("");
		str << DeltaTime::GetDeltaTime();
		debug_text += "\nDT: " + str.str();
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

		{
		auto pf = Profiling::getSingleton().GetTimes();
		CL_Pointf pfLoc(10.f, 90.f);
		for (auto it = pf.begin(), end = pf.end(); it != end; ++it)
		{
			std::stringstream str;
			str << it->second;
			std::string line = it->first + ": " + str.str() + "ms";
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
			m_RenderWorld->m_PhysWorld->SetDebugDraw(m_PhysDebugDraw.get());
			m_PhysDebugDraw->SetFlags(B2DebugDraw::e_centerOfMassBit | B2DebugDraw::e_jointBit | B2DebugDraw::e_pairBit | B2DebugDraw::e_shapeBit);
		}

		if (m_PhysDebugDraw && !viewports.empty() && m_RenderWorld->m_PhysDebugDrawEnabled)
		{
			m_PhysDebugDraw->SetViewport(viewports.front());
			m_PhysDebugDraw->SetupView();
			m_RenderWorld->m_PhysWorld->DrawDebugData();
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

}
