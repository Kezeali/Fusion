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

namespace FusionEngine
{
	// forward decl.
	class IDrawable;
	class CLRenderTask;
	class StreamingCamera;
	class CameraSynchroniser;

	class B2DebugDraw;

	class CLRenderSystem : public IComponentSystem
	{
	public:
		SystemType GetType() const { return SystemType::Rendering; }

		std::string GetName() const { return "CLRenderSystem"; }

		CLRenderSystem(const CL_GraphicContext& gc, CameraSynchroniser* camera_sync);

		std::shared_ptr<ISystemWorld> CreateWorld();

	private:
		CL_GraphicContext m_GraphicContext;
		CameraSynchroniser* m_CameraSynchroniser;
	};

	class CLRenderWorld : public ISystemWorld
	{
	public:
		CLRenderWorld(IComponentSystem* system, const CL_GraphicContext& gc, CameraSynchroniser* camera_sync);
		virtual ~CLRenderWorld();

		const std::vector<ViewportPtr>& GetViewports() const { return m_Viewports; }
		void AddViewport(const ViewportPtr& viewport);
		void RemoveViewport(const ViewportPtr& viewport);

		void AddQueuedViewports();

		// TEMP
		void SetPhysWorld(b2World* world) { m_PhysWorld = world; }
		b2World* m_PhysWorld;
		bool m_PhysDebugDrawEnabled;
		void SetDebugDraw(bool value) { m_PhysDebugDrawEnabled = value; }
		void ToggleDebugDraw() { m_PhysDebugDrawEnabled = !m_PhysDebugDrawEnabled; }

		const std::vector<boost::intrusive_ptr<IDrawable>>& GetDrawables() const { return m_Drawables; }
		std::vector<boost::intrusive_ptr<IDrawable>>& GetDrawables() { return m_Drawables; }

		std::vector<boost::intrusive_ptr<StreamingCamera>>& GetCameras() { return m_Cameras; }

		static void Register(asIScriptEngine* engine);

	private:
		std::vector<std::string> GetTypes() const;

		void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data);

		ComponentPtr InstantiateComponent(const std::string& type);
		void OnActivation(const ComponentPtr& component);
		void OnDeactivation(const ComponentPtr& component);

		ISystemTask* GetTask();

		CLRenderTask* m_RenderTask;

		std::vector<boost::intrusive_ptr<IDrawable>> m_Drawables;

		std::vector<boost::intrusive_ptr<StreamingCamera>> m_Cameras;

		boost::mutex m_ViewportMutex;
		std::vector<ViewportPtr> m_Viewports;
		tbb::concurrent_queue<ViewportPtr> m_ViewportsToAdd;

		CameraSynchroniser* m_CameraManager;

		Renderer* m_Renderer;
	};

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

}

#endif
