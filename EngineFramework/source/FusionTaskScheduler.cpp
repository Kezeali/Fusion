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

#include "FusionTaskScheduler.h"

#include "FusionDeltaTime.h"
#include "FusionStreamingSystem.h"

#include <functional>

#include <tbb/task.h>

namespace FusionEngine
{

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

	class SystemTaskExecutor : public ISystemTask
	{
	public:
		SystemTaskExecutor(ISystemWorld* world, const std::vector<ISystemTask*>& sub_tasks);

	private:
		void Update(const float delta);

		SystemType GetTaskType() const { return m_TaskType; }

		PerformanceHint GetPerformanceHint() const { return m_PerfHint; }

		bool IsPrimaryThreadOnly() const { return m_PrimaryThreadOnly; }

		std::vector<ISystemTask*> m_SubTasks;
		SystemType m_TaskType;
		PerformanceHint m_PerfHint;
		bool m_PrimaryThreadOnly;
	};

	SystemTaskExecutor::SystemTaskExecutor(ISystemWorld* world, const std::vector<ISystemTask*>& sub_tasks)
		: ISystemTask(world),
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

	void SystemTaskExecutor::Update(const float delta)
	{
		for (auto it = m_SubTasks.begin(), end = m_SubTasks.end(); it != end; ++it)
		{
			(*it)->Update(delta);
		}
	}

	void TaskScheduler::SetUniverse(const std::vector<std::shared_ptr<ISystemWorld>>& universe)
	{
		m_ComponentWorlds = universe;

		//m_ResortTasks = true;

		m_SortedTasks.clear();
		m_SortedSimulationTasks.clear();
		m_SortedRenderTasks.clear();
		for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
		{
			auto world = *it;
			auto tasks = world->GetTasks();

			FSN_ASSERT(!tasks.empty());

			if (tasks.size() > 1)
			{
				// Create a proxy-task to execute all the tasks for this system if there is more than one
				ISystemTask* task = new SystemTaskExecutor(world.get(), tasks);
				m_SortedTasks.push_back(task);
				m_ProxyTasks.push_back(std::unique_ptr<ISystemTask>(task));
			}
			else
				m_SortedTasks.push_back(tasks.front());

			// Grab all the render tasks
			std::vector<ISystemTask*> renderTasks;
			std::copy_if(tasks.begin(), tasks.end(), std::back_inserter(renderTasks),
				[](ISystemTask* task) { return task->GetTaskType() == SystemType::Rendering; });

			std::vector<ISystemTask*> simulationTasks;
			std::copy_if(tasks.begin(), tasks.end(), std::back_inserter(simulationTasks),
				[](ISystemTask* task) { return task->GetTaskType() == SystemType::Simulation; });

			if (renderTasks.size() > 1)
			{
				auto task = new SystemTaskExecutor(world.get(), renderTasks);
				m_SortedRenderTasks.push_back(task);
				m_ProxyTasks.push_back(std::unique_ptr<ISystemTask>(task));
			}
			else if (!renderTasks.empty())
				m_SortedRenderTasks.push_back(renderTasks.front());

			if (simulationTasks.size() > 1)
			{
				auto task = new SystemTaskExecutor(world.get(), simulationTasks);
				m_SortedSimulationTasks.push_back(task);
				m_ProxyTasks.push_back(std::unique_ptr<ISystemTask>(task));
			}
			else if (!simulationTasks.empty())
				m_SortedSimulationTasks.push_back(simulationTasks.front());
		}

		if (m_EntityManager)
		{
			m_StreamingTask.reset(new StreamingTask(m_EntityManager, m_Archivist));
			m_SortedTasks.push_back(m_StreamingTask.get());
			m_SortedSimulationTasks.push_back(m_StreamingTask.get());
			m_SortedRenderTasks.push_back(m_StreamingTask.get());
		}

		SortTasks();
	}

	void TaskScheduler::SortTasks()
	{
		auto pred = [](ISystemTask* first, ISystemTask* second)->bool
		{
			return first->GetPerformanceHint() < second->GetPerformanceHint();
		};

		std::sort(m_SortedTasks.begin(), m_SortedTasks.end(), pred);
		std::sort(m_SortedSimulationTasks.begin(), m_SortedSimulationTasks.end(), pred);
		std::sort(m_SortedRenderTasks.begin(), m_SortedRenderTasks.end(), pred);
	}

	uint8_t TaskScheduler::Execute(uint8_t what)
	{
		auto currentTime = CL_System::get_time();
		if (m_LastTime == 0)
			m_LastTime = currentTime;
		auto timePassed = currentTime - m_LastTime;
		m_LastTime = currentTime;

		uint8_t taskFilter = 0xFF;
		const uint8_t simAndRender = (SystemType::Simulation | SystemType::Rendering);

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
						taskFilter = SystemType::Simulation;
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

		// TODO: remove this
		taskFilter &= what;
		if (taskFilter == 0)
			return taskFilter;

		// Simulating another step, update the frame count
		if (taskFilter & SystemType::Simulation)
			++DeltaTime::m_Tick; // (this is just stored here until I implement networking again)

		if (m_ThreadingEnabled)
		{
			if (m_ResortTasks)
			{
				SortTasks();
				m_ResortTasks = false;
			}

			// Schedule the tasks for component-worlds that are ready for execution
			if ((taskFilter & simAndRender) == simAndRender)
			{
				m_TaskManager->SpawnJobsForSystemTasks(m_SortedTasks, deltaTime);
			}
			else if (taskFilter & SystemType::Rendering)
			{
				m_TaskManager->SpawnJobsForSystemTasks(m_SortedRenderTasks, deltaTime);
			}
			else if (taskFilter & SystemType::Simulation)
			{
				m_TaskManager->SpawnJobsForSystemTasks(m_SortedSimulationTasks, deltaTime);
			}

			m_TaskManager->WaitForSystemTasks();
		}
		else // ... not threading enabled:
		{
			for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
			{
				auto world = *it;
				if (world->GetSystemType() & taskFilter)
					world->GetTask()->Update(deltaTime);
			}

			m_EntityManager->UpdateActiveRegions();
			m_EntityManager->ProcessActiveEntities(deltaTime);
		}

		return taskFilter;
	}

}
