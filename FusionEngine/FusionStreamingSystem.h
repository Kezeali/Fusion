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

#ifndef H_FusionStreamingSystem
#define H_FusionStreamingSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentSystem.h"

#include "FusionEntityManager.h"

namespace FusionEngine
{
	
	/*class StreamingTask;

	class StreamingSystem : public IComponentSystem
	{
	public:
		StreamingSystem(const std::shared_ptr<StreamingManager>& manager);
		virtual ~StreamingSystem()
		{}

		ISystemWorld* CreateWorld();

	private:
		SystemType GetType() const { return SystemType::Simulation; }

		std::string GetName() const { return "StreamingSystem"; }

		std::shared_ptr<StreamingManager> m_StreamingManager;
	};

	class StreamingWorld : public ISystemWorld
	{
	public:
		StreamingWorld(IComponentSystem* system, const std::shared_ptr<StreamingManager>& manager);
		~StreamingWorld();

	private:
		std::vector<std::string> GetTypes() const { return std::vector<std::string>(); }

		ComponentPtr InstantiateComponent(const std::string& type);
		ComponentPtr InstantiateComponent(const std::string& type, const Vector2& pos, float angle, RakNet::BitStream* continious_data, RakNet::BitStream* occasional_data);

		void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta);

		void Prepare(const ComponentPtr& component);
		void OnActivation(const ComponentPtr& component);
		void OnDeactivation(const ComponentPtr& component);

		ISystemTask* GetTask();

	private:
		std::shared_ptr<StreamingManager> m_StreamingManager;
		StreamingTask* m_StreamingTask;
	};*/

	class StreamingTask : public ISystemTask
	{
	public:
		StreamingTask(EntityManager* streaming_manager)
			: ISystemTask(nullptr), m_StreamingManager(streaming_manager),
			newSlowness(0u)
		{}
		~StreamingTask() {}

		void Update(const float delta);

		std::string GetName() const { return "Streaming"; }

		SystemType GetTaskType() const { return SystemType::Simulation; }

		PerformanceHint GetPerformanceHint() const { return ISystemTask::LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		EntityManager* m_StreamingManager;

		cl_uint64 newSlowness;
	};

	class StreamingTaskB : public ISystemTask
	{
	public:
		StreamingTaskB(EntityManager* streaming_manager)
			: ISystemTask(nullptr), m_StreamingManager(streaming_manager)
		{}
		~StreamingTaskB() {}

		void Update(const float delta);

		SystemType GetTaskType() const { return SystemType::Rendering; }

		PerformanceHint GetPerformanceHint() const { return ISystemTask::LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		EntityManager* m_StreamingManager;
	};
	
	inline void StreamingTask::Update(const float delta)
	{
		m_StreamingManager->UpdateActiveRegions();
		m_StreamingManager->ProcessActiveEntities(delta);
	}
	
	inline void StreamingTaskB::Update(const float delta)
	{
		m_StreamingManager->UpdateActiveRegions();
	}

}

#endif
