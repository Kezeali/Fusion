/*
*  Copyright (c) 2011-2013 Fusion Project Team
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
#include "FusionRenderer.h"

#include "FusionCLRenderComponent.h"
#include "FusionCLRenderExtension.h"
#include "FusionCLRenderTask.h"

#include "FusionGraphicalProfilerTask.h"

#include "FusionEntity.h"
#include "FusionPlayerRegistry.h"

// TODO: move into StreamingSystem?
#include "FusionStreamingCameraComponent.h"
#include "FusionCameraSynchroniser.h"

#include "FusionProfiling.h"

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
		m_GraphicalProfilerTask = new GraphicalProfilerTask(this, canvas);

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

		delete m_GraphicalProfilerTask;
		delete m_GUITask;
		delete m_RenderTask;
		delete m_Renderer;
	}

	void CLRenderWorld::AddViewport(const ViewportPtr& viewport)
	{
		m_ViewportsToAddOrRemove.push(std::make_pair(viewport, true));
	}

	void CLRenderWorld::RemoveViewport(const ViewportPtr& viewport)
	{
		m_ViewportsToAddOrRemove.push(std::make_pair(viewport, false));
	}

	void CLRenderWorld::AddQueuedViewports()
	{
		std::pair<ViewportPtr, bool> viewportOperation;
		while (m_ViewportsToAddOrRemove.try_pop(viewportOperation))
		{
			if (viewportOperation.second)
				m_Viewports.push_back(viewportOperation.first);
			else
			{
				auto entry = std::find(m_Viewports.begin(), m_Viewports.end(), viewportOperation.first);
				if (entry != m_Viewports.end())
					m_Viewports.erase(entry);
			}
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

	void CLRenderWorld::EnqueueViewportRenderAction(const RenderAction& action)
	{
		m_RenderTask->EnqueueViewportRenderAction(action);
	}

	void CLRenderWorld::EnqueueViewportRenderAction(const ViewportPtr& vp, const RenderAction& action)
	{
		m_RenderTask->EnqueueViewportRenderAction(vp, action);
	}

	void CLRenderWorld::EnqueueWorldRenderAction(const RenderAction& action)
	{
		m_RenderTask->EnqueueWorldRenderAction(action);
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
					// Hack to remove viewport add operations from the queue
					std::vector<std::pair<ViewportPtr, bool>> keptViewportOps;
					std::pair<ViewportPtr, bool> viewportOperation;
					while (m_ViewportsToAddOrRemove.try_pop(viewportOperation))
					{
						if (viewportOperation.first != camComponent->m_Viewport)
							keptViewportOps.push_back(viewportOperation);
					}
					for (auto it = keptViewportOps.begin(); it != keptViewportOps.end(); ++it)
					{
						m_ViewportsToAddOrRemove.push(*it);
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
		tasks.push_back(m_GraphicalProfilerTask);
		return tasks;
	}

}
