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

#include "FusionTaskScheduler.h"

#include "FusionComponentSystem.h"
#include "FusionDeltaTime.h"
#include "FusionStreamingSystem.h"
#include "FusionSystemWorld.h"
#include "FusionSystemTask.h"
#include "FusionSystemType.h"
#include "FusionProfiling.h"

#include <functional>

#include <tbb/task.h>

namespace FusionEngine
{
	
	using namespace System;

	TaskScheduler::TaskScheduler(TaskManager* task_manager, EntityManager* entity_manager, RegionCellArchivist* archivist)
		: m_TaskManager(task_manager),
		m_EntityManager(entity_manager),
		m_Archivist(archivist),
		m_Accumulator(0),
		m_LastTime(0),
		m_Timer(1.f / 30.f),
		m_FramerateLimiterEnabled(false),
		m_Unlimited(false),
		m_FramesSkipped(0),
		m_MaxFrameskip(2)
	{
		m_DeltaTime = 1.f / 30.f;
		m_DeltaTimeMS = (unsigned int)(m_DeltaTime * 1000);

		DeltaTime::m_DT = m_DeltaTime;

		m_ThreadingEnabled = m_TaskManager != nullptr;
	}

	TaskScheduler::~TaskScheduler()
	{
	}

	void TaskScheduler::SetDT(float dt)
	{
		FSN_ASSERT(dt > 0.001f);
		if (dt <= 0.001f)
			return;

		m_DeltaTime = dt;
		m_DeltaTimeMS = (unsigned int)(m_DeltaTime * 1000);

		DeltaTime::m_DT = m_DeltaTime;

		m_Timer.SetInterval(m_DeltaTime);
	}

	float TaskScheduler::GetDT() const
	{
		return m_DeltaTime;
	}

	class SystemTaskExecutor : public TaskBase
	{
	public:
		SystemTaskExecutor(WorldBase* world, const std::vector<TaskBase*>& sub_tasks);

	private:
		void Update() override;

		SystemType GetTaskType() const { return m_TaskType; }

		PerformanceHint GetPerformanceHint() const { return m_PerfHint; }

		bool IsPrimaryThreadOnly() const { return m_PrimaryThreadOnly; }

		std::vector<TaskBase*> m_SubTasks;
		SystemType m_TaskType;
		PerformanceHint m_PerfHint;
		bool m_PrimaryThreadOnly;
	};

	SystemTaskExecutor::SystemTaskExecutor(WorldBase* world, const std::vector<TaskBase*>& sub_tasks)
		: TaskBase(world, world->GetSystem()->GetName().c_str()),
		m_SubTasks(sub_tasks)
	{
		FSN_ASSERT(!sub_tasks.empty());

		{
			auto firstSubTask = m_SubTasks.front();
			m_TaskType = firstSubTask->GetTaskType();
			m_PrimaryThreadOnly = firstSubTask->IsPrimaryThreadOnly();
		}

		for (auto it = m_SubTasks.begin(), end = m_SubTasks.end(); it != end; ++it)
		{
			FSN_ASSERT(*it);
			auto& subTask = *it;

			// Make sure that the sub-set of sub-tasks given is correct
			FSN_ASSERT(subTask->IsPrimaryThreadOnly() == m_PrimaryThreadOnly);

			// The perf-hint for this super-task is the slowest perf-hint from the sub-tasks
			//  (this is the min, since the enum is ordered slowest-fastest)
			m_PerfHint = std::min(m_PerfHint, subTask->GetPerformanceHint());
		}
	}

	void SystemTaskExecutor::Update()
	{
		for (auto it = m_SubTasks.begin(), end = m_SubTasks.end(); it != end; ++it)
		{
			(*it)->Update();
		}
	}

	const size_t s_NumSystemTypeCombinations = 6;
	const std::uint8_t s_SystemTypeCombinations[s_NumSystemTypeCombinations] =
	{
		SystemType::Simulation,
		SystemType::Rendering,
		SystemType::Streaming,
		SystemType::Simulation | SystemType::Rendering,
		SystemType::Rendering | SystemType::Streaming,
		SystemType::Simulation | SystemType::Streaming
	};

	void TaskScheduler::SetUniverse(const std::vector<std::shared_ptr<WorldBase>>& universe)
	{
		m_ComponentWorlds = universe;

		//m_ResortTasks = true;

		//m_SortedTasks.clear();
		//for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
		//{
		//	auto world = *it;
		//	auto tasks = world->GetTasks();

		//	FSN_ASSERT(!tasks.empty());

		//	for (int i = 0; i < 2; ++i)
		//	{
		//		const bool primaryThread = i == 0;
		//		// Grab the tasks that do / don't need to run in the primary thread
		//		std::vector<TaskBase*> threadRestrictionTasks;
		//		std::copy_if(tasks.begin(), tasks.end(), std::back_inserter(threadRestrictionTasks),
		//			[primaryThread](TaskBase* task) { return (task->IsPrimaryThreadOnly() == primaryThread); });
		//		if (threadRestrictionTasks.size() > 1)
		//		{
		//			// Create a proxy-task to execute all the tasks for this system if there is more than one
		//			TaskBase* task = new SystemTaskExecutor(world.get(), threadRestrictionTasks);
		//			m_SortedTasks.push_back(task);
		//			m_ProxyTasks.push_back(std::unique_ptr<TaskBase>(task));
		//		}
		//		else if (!threadRestrictionTasks.empty())
		//			m_SortedTasks.push_back(threadRestrictionTasks.front());

		//		// Grab all the tasks for each combination of SystemTypes
		//		for (int i = 0; i < s_NumSystemTypeCombinations; ++i)
		//		{
		//			const uint8_t systemTypeCombination = s_SystemTypeCombinations[i];

		//			std::vector<TaskBase*> systemTypeTasks;
		//			std::copy_if(tasks.begin(), tasks.end(), std::back_inserter(systemTypeTasks),
		//				[systemTypeCombination, primaryThread](TaskBase* task) { return (task->GetTaskType() & systemTypeCombination) != 0 && (task->IsPrimaryThreadOnly() == primaryThread); });

		//			if (systemTypeTasks.size() > 1)
		//			{
		//				auto task = new SystemTaskExecutor(world.get(), systemTypeTasks);
		//				m_GroupedSortedTasks[systemTypeCombination].push_back(task);
		//				m_ProxyTasks.push_back(std::unique_ptr<TaskBase>(task));
		//			}
		//			else if (!systemTypeTasks.empty())
		//				m_GroupedSortedTasks[systemTypeCombination].push_back(systemTypeTasks.front());
		//		}
		//	}
		//}

		if (m_EntityManager)
		{
			m_StreamingTask.reset(new StreamingTask(m_EntityManager, m_Archivist));

			//m_SortedTasks.push_back(m_StreamingTask.get());

			//m_GroupedSortedTasks[SystemType::Streaming].push_back(m_StreamingTask.get());
			//m_GroupedSortedTasks[SystemType::Streaming | SystemType::Rendering].push_back(m_StreamingTask.get());
			//m_GroupedSortedTasks[SystemType::Streaming | SystemType::Simulation].push_back(m_StreamingTask.get());
		}

		//SortTasks();
	}

	namespace
	{
		template <typename Pred>
		void sortTaskVector(std::vector<TaskBase*>& vec, Pred pred)
		{
			std::sort(vec.begin(), vec.end(), pred);
		}
	}

	void TaskScheduler::SortTasks()
	{
		auto pred = [](TaskBase* first, TaskBase* second)->bool
		{
			return first->GetPerformanceHint() < second->GetPerformanceHint();
		};

		std::sort(m_SortedTasks.begin(), m_SortedTasks.end(), pred);

		for (int i = 0; i < s_NumSystemTypeCombinations; ++i)
			sortTaskVector(m_GroupedSortedTasks[s_SystemTypeCombinations[i]], pred);
	}

	uint8_t TaskScheduler::Execute(uint8_t what)
	{
		FSN_PROFILE("Executing Tasks");

		auto currentTime = clan::System::get_time();
		if (m_LastTime == 0)
			m_LastTime = currentTime;
		auto timePassed = currentTime - m_LastTime;
		m_LastTime = currentTime;

		uint8_t taskFilter = 0xFF;
		static const uint8_t everySystemType = (SystemType::Simulation | SystemType::Rendering | SystemType::Streaming | SystemType::Messaging | SystemType::Editor);

		float deltaTime = m_DeltaTime;

		DeltaTime::m_ActualDTMS = timePassed;
		DeltaTime::m_Alpha = 1.0f;

		if (m_FramerateLimiterEnabled)
		{
			// Wait until it has been at least deltaTime since the last execution
			m_Timer.Wait();
		}
		else
#ifdef PROFILE_BUILD
		if (!m_Unlimited)
#endif
		{
			m_Accumulator += fe_min(timePassed, m_DeltaTimeMS * (m_MaxFrameskip + 1));

			DeltaTime::m_ActualDTMS += m_DeltaTimeMS * m_FramesSkipped;
			DeltaTime::m_FramesSkipped = m_FramesSkipped;

			if (m_Accumulator >= m_DeltaTimeMS)
			{
				m_Accumulator -= m_DeltaTimeMS;

				// If last frame took > 1 DT, frameskip to catch up (if allowed):
				if (m_FramesSkipped < m_MaxFrameskip)
				{
					if (m_Accumulator >= m_DeltaTimeMS)
					{
						taskFilter = SystemType::Simulation | SystemType::Streaming;
						++m_FramesSkipped;
					}
					else
						m_FramesSkipped = 0;
				}
				else // Max-frameskip reached: give up and flush the accumulator (simulation will run slower than real-world time)
				{
					while (m_Accumulator >= m_DeltaTimeMS)
						m_Accumulator -= m_DeltaTimeMS;
					m_FramesSkipped = 0;
				}
			}
			else // Render while waiting for enough time to accumulate
			{
				taskFilter = SystemType::Rendering;
				//deltaTime = timePassed * 0.001f;
				m_FramesSkipped = 0;
			}

			DeltaTime::m_Alpha = m_Accumulator / (float)m_DeltaTimeMS;
		}

		FSN_ASSERT(DeltaTime::m_Alpha >= 0.0f && (m_FramesSkipped > 0 || DeltaTime::m_Alpha <= 1.0f));

		taskFilter &= what;
		if (taskFilter == 0)
			return taskFilter;

		// Simulating another step, update the frame count
		if (taskFilter & SystemType::Simulation)
			++DeltaTime::m_Tick; // (this is just stored here until I implement networking again)

		if (m_ThreadingEnabled)
		{
			//if (m_ResortTasks)
			//{
			//	SortTasks();
			//	m_ResortTasks = false;
			//}

			std::vector<std::vector<TaskBase*>> taskLists;
			taskLists.reserve(m_ComponentWorlds.size());

			// Schedule the tasks for component-worlds that are ready for execution
			if ((taskFilter & everySystemType) == everySystemType)
			{
				for (const auto& world : m_ComponentWorlds)
				{
					auto t = world->GetTasks();
					std::vector<TaskBase*> tasks;
					for (auto it = t.begin(); it != t.end(); ++it)
						tasks.push_back(&(*it));
					if (!tasks.empty())
						taskLists.push_back(tasks);
				}
			}
			else
			{
				for (const auto& world : m_ComponentWorlds)
				{
					auto t = world->GetTasks();
					std::vector<TaskBase*> tasks;
					for (auto it = t.begin(); it != t.end(); ++it)
						tasks.push_back(&(*it));
					auto newEnd = std::remove_if(tasks.begin(), tasks.end(), [taskFilter](TaskBase* task) { return (task->GetTaskType() & taskFilter) == 0; });
					if (newEnd != tasks.end())
						tasks.erase(newEnd, tasks.end());
					if (!tasks.empty())
						taskLists.push_back(std::move(tasks));
				}
			}

			if ((taskFilter & System::Streaming) != 0)
			{
				//TaskList_t tasks;
				//tasks.push_back(*m_StreamingTask.get());
				std::vector<TaskBase*> tasks;
				tasks.push_back(m_StreamingTask.get());
				taskLists.push_back(std::move(tasks));
			}

			m_TaskManager->SpawnJobsForSystemTasks(taskLists);
			m_TaskManager->WaitForSystemTasks();
		}
		else // ... not threading enabled:
		{
			for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
			{
				auto world = *it;
				for (auto& task : world->GetTasks())
					if ((task.GetTaskType() & taskFilter) != 0)
						task.Update();
			}

			if (taskFilter & SystemType::Streaming)
			{
				m_EntityManager->UpdateActiveRegions();
				m_EntityManager->ProcessActiveEntities(deltaTime);
			}
		}

		return taskFilter;
	}

}
