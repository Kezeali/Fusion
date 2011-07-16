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

	TaskScheduler::TaskScheduler(TaskManager* task_manager)
		: m_TaskManager(task_manager),
		m_Accumulator(0),
		m_LastTime(0),
		m_Timer(1.f / 30.f),
		m_FramerateLimiterEnabled(false)
	{
		m_ThreadingEnabled = m_TaskManager != nullptr;
	}

	TaskScheduler::~TaskScheduler()
	{
	}

	void TaskScheduler::SetOntology(const std::vector<ISystemWorld*>& ontology)
	{
		m_ComponentWorlds = ontology;

		m_ResortTasks = true;

		m_SortedTasks.clear();
		for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
		{
			ISystemWorld* world = *it;
			m_SortedTasks.push_back(world->GetTask());

			if (world->GetSystemType() == SystemType::Rendering)
				m_SortedRenderTasks.push_back(world->GetTask());
		}
	}

	void TaskScheduler::Execute()
	{
		// Get the delta time; seconds since last Execute call.
		float deltaTime = 0.0f;

		//deltaTime = Singletons::PlatformManager.Timers().Wait( m_hExecutionTimer, !m_bBenchmarkingEnabled );

		auto currentTime = CL_System::get_time();
		if (m_LastTime == 0)
			m_LastTime = currentTime;
		auto timePassed = currentTime - m_LastTime;
		m_LastTime = currentTime;

		deltaTime = 1.f / 30.f;

		bool renderOnly = false;

		if (m_FramerateLimiterEnabled)
		{
			// Wait until it has been at least deltaTime since the last execution
#ifdef PROFILE_BUILD
			if (false/*m_Benchmark*/)
#endif
			m_Timer.Wait();
		}
		else
		{
			m_Accumulator += fe_min(timePassed, 33u);

			if (m_Accumulator >= (unsigned int)(deltaTime * 1000))
			{
				m_Accumulator -= (unsigned int)(deltaTime * 1000);
			}
			else
				renderOnly = true;
		}

		// Check if the execution is paused, and set delta time to 0
		//if ( Singletons::EnvironmentManager.Runtime().GetStatus() ==
		//	IEnvironment::IRuntime::Status::Paused )
		//{
		//	deltaTime = 0.0f;
		//}

		if (m_ThreadingEnabled)
		{
			if (m_ResortTasks)
			{
				std::sort(m_SortedTasks.begin(), m_SortedTasks.end(), [](ISystemTask* first, ISystemTask* second)->bool
				{
					return first->GetPerformanceHint() < second->GetPerformanceHint();
				});
				std::sort(m_SortedRenderTasks.begin(), m_SortedRenderTasks.end(), [](ISystemTask* first, ISystemTask* second)->bool
				{
					return first->GetPerformanceHint() < second->GetPerformanceHint();
				});
				m_ResortTasks = false;
			}

			// Schedule the tasks for component-worlds that are ready for execution
			if (!renderOnly)
			{
				m_TaskManager->SpawnJobsForSystemTasks(m_SortedTasks, deltaTime);

				m_TaskManager->WaitForSystemTasks(m_SortedTasks);
			}
			else
			{
				m_TaskManager->SpawnJobsForSystemTasks(m_SortedRenderTasks, deltaTime);

				m_TaskManager->WaitForSystemTasks(m_SortedRenderTasks);
			}
		}
		else // Not threading enabled
		{
			for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
			{
				ISystemWorld* world = *it;
				if (!renderOnly || world->GetSystemType() == SystemType::Rendering)
					world->GetTask()->Update(deltaTime);
			}
		}
	}

}
