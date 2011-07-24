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

#include "FusionTaskScheduler.h"

#include <functional>

#include <tbb/task.h>

namespace FusionEngine
{

	float DeltaTime::m_DT = 0.f;
	unsigned int DeltaTime::m_ActualDTMS = 0;
	float DeltaTime::m_Alpha = 0.f;
	unsigned int DeltaTime::m_FramesSkipped = 0;
	unsigned int DeltaTime::m_Tick = 0;

	TaskScheduler::TaskScheduler(TaskManager* task_manager)
		: m_TaskManager(task_manager),
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

	void TaskScheduler::SetOntology(const std::vector<ISystemWorld*>& ontology)
	{
		m_ComponentWorlds = ontology;

		//m_ResortTasks = true;

		m_SortedTasks.clear();
		m_SortedSimulationTasks.clear();
		m_SortedRenderTasks.clear();
		for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
		{
			ISystemWorld* world = *it;
			m_SortedTasks.push_back(world->GetTask());

			if (world->GetSystemType() == SystemType::Rendering)
			{
				auto renderTask = dynamic_cast<ISystemRenderingTask*>(world->GetTask());
				FSN_ASSERT_MSG(renderTask, "The given task is marked as a Rendering task, but doesn't implement ISystemRenderTask");
				m_SortedRenderTasks.push_back(renderTask);
			}
			else
				m_SortedSimulationTasks.push_back(world->GetTask());
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

	uint8_t TaskScheduler::Execute()
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

				m_TaskManager->WaitForSystemTasks(m_SortedTasks);
			}
			else if (taskFilter & SystemType::Rendering)
			{
				m_TaskManager->SpawnJobsForSystemTasks(m_SortedRenderTasks, deltaTime);

				m_TaskManager->WaitForSystemTasks(m_SortedRenderTasks);
			}
			else if (taskFilter & SystemType::Simulation)
			{
				m_TaskManager->SpawnJobsForSystemTasks(m_SortedSimulationTasks, deltaTime);

				m_TaskManager->WaitForSystemTasks(m_SortedSimulationTasks);
			}
		}
		else // ... not threading enabled:
		{
			for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
			{
				ISystemWorld* world = *it;
				if (world->GetSystemType() & taskFilter)
					world->GetTask()->Update(deltaTime);
			}
		}

		return taskFilter;
	}

}
