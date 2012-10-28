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

#ifndef H_FusionCLRenderSystem
#define H_FusionCLRenderSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionCommon.h"

#include "FusionComponentSystem.h"
#include "FusionViewport.h"

#include <ClanLib/display.h>
#include <tbb/concurrent_queue.h>
#include <boost/thread/mutex.hpp>
#include <map>
#include <vector>

namespace FusionEngine
{
	// forward decl.
	class IDrawable;
	class CLSprite;
	class CLRenderTask;
	class CLRenderGUITask;
	class StreamingCamera;
	class CameraSynchroniser;

	class CLRenderExtension;

	class B2DebugDraw;

	class CLRenderSystem : public IComponentSystem
	{
	public:
		CLRenderSystem(const CL_DisplayWindow& window, CameraSynchroniser* camera_sync);

		std::shared_ptr<ISystemWorld> CreateWorld();

	private:
		SystemType GetType() const { return SystemType::Rendering; }

		std::string GetName() const { return "CLRenderSystem"; }

		void RegisterScriptInterface(asIScriptEngine* engine);

		CL_DisplayWindow m_DisplayWindow;
		CameraSynchroniser* m_CameraSynchroniser;
	};
	
	class SpriteDefinitionCache;

	//! ClanLib Renderer
	class CLRenderWorld : public ISystemWorld
	{
	public:
		CLRenderWorld(IComponentSystem* system, const CL_DisplayWindow& window, CameraSynchroniser* camera_sync);
		virtual ~CLRenderWorld();

		const std::vector<ViewportPtr>& GetViewports() const { return m_Viewports; }
		void AddViewport(const ViewportPtr& viewport);
		void RemoveViewport(const ViewportPtr& viewport);

		void AddRenderExtension(const std::weak_ptr<CLRenderExtension>& extension, const ViewportPtr& viewport);
		void RunExtensions(const ViewportPtr& vp, const CL_GraphicContext& gc);

		void AddQueuedViewports();

		// TEMP
		void SetPhysWorld(b2World* world) { m_PhysWorld = world; }
		b2World* m_PhysWorld;
		bool m_PhysDebugDrawEnabled;
		void SetDebugDraw(bool value) { m_PhysDebugDrawEnabled = value; }
		void ToggleDebugDraw() { m_PhysDebugDrawEnabled = !m_PhysDebugDrawEnabled; }

		bool IsDebugTextEnabled() const { return m_DebugTextEnabled; }

		const std::vector<boost::intrusive_ptr<IDrawable>>& GetDrawables() const { return m_Drawables; }
		std::vector<boost::intrusive_ptr<IDrawable>>& GetDrawables() { return m_Drawables; }
		std::vector<boost::intrusive_ptr<CLSprite>>& GetSprites() { return m_Sprites; }

		std::vector<boost::intrusive_ptr<CLSprite>>& GetSpritesToDefine() { return m_SpritesToDefine; }

		std::vector<boost::intrusive_ptr<StreamingCamera>>& GetCameras() { return m_Cameras; }

		static void Register(asIScriptEngine* engine);

	private:
		std::vector<std::string> GetTypes() const;

		ComponentPtr InstantiateComponent(const std::string& type);
		void OnActivation(const ComponentPtr& component);
		void OnDeactivation(const ComponentPtr& component);

		std::vector<ISystemTask*> GetTasks();

		CLRenderTask* m_RenderTask;
		CLRenderGUITask* m_GUITask;

		// Drawables contains all drawable components (sprites, etc.) sorted for rendering
		std::vector<boost::intrusive_ptr<IDrawable>> m_Drawables;

		std::vector<boost::intrusive_ptr<CLSprite>> m_Sprites;

		std::vector<boost::intrusive_ptr<StreamingCamera>> m_Cameras;

		std::vector<boost::intrusive_ptr<CLSprite>> m_SpritesToDefine;

		boost::mutex m_ViewportMutex;
		std::vector<ViewportPtr> m_Viewports;
		tbb::concurrent_queue<ViewportPtr> m_ViewportsToAdd;

		std::multimap<ViewportPtr, std::weak_ptr<CLRenderExtension>> m_Extensions;

		std::shared_ptr<SpriteDefinitionCache> m_SpriteDefinitionCache;

		bool m_DebugTextEnabled;

		CameraSynchroniser* m_CameraManager;

		Renderer* m_Renderer;
	};

	//! ClanLib render task
	class CLRenderTask : public ISystemTask
	{
	public:
		CLRenderTask(CLRenderWorld* sysworld, Renderer* const renderer);
		~CLRenderTask();

		void Update(const float delta);

		void Draw();

		SystemType GetTaskType() const { return SystemType::Rendering; }

		PerformanceHint GetPerformanceHint() const { return LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return true;
		}

	private:
		CLRenderWorld* m_RenderWorld;
		Renderer* const m_Renderer;

		float m_Accumulator;

		std::unique_ptr<B2DebugDraw> m_PhysDebugDraw;

		CL_Font m_DebugFont;
		CL_Font m_DebugFont2;
	};

	class CLRenderGUITask : public ISystemTask
	{
	public:
		CLRenderGUITask(CLRenderWorld* sysworld, const CL_DisplayWindow& window, Renderer* const renderer);
		~CLRenderGUITask();

		void Update(const float delta);

		SystemType GetTaskType() const { return SystemType::Simulation; }

		PerformanceHint GetPerformanceHint() const { return Short; }

		bool IsPrimaryThreadOnly() const
		{
			return true;
		}

		void onMouseMove(const CL_InputEvent &ev, const CL_InputState &state);

	private:
		CLRenderWorld* m_RenderWorld;
		Renderer* const m_Renderer;

		CL_DisplayWindow m_DisplayWindow;

		CL_Slot m_MouseMoveSlot;
		std::deque<CL_InputEvent> m_BufferedEvents;
		//Vector2i m_LastPosition;
	};

}

#endif
