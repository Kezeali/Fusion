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
		: m_TaskManager(task_manager)
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
	}

	void TaskScheduler::Execute()
	{
		// Get the delta time; seconds since last Execute call.
		float deltaTime = 0.0f;

		//deltaTime = Singletons::PlatformManager.Timers().Wait( m_hExecutionTimer, !m_bBenchmarkingEnabled );

		unsigned int currentTime = CL_System::get_time();
		unsigned int timePassed = currentTime - m_LastTime;
		m_LastTime = currentTime;

		// TODO: use timePassed to wait until the next frame

		deltaTime = 1.f / 30.f;

		// Update instrumentation for this frame.
		// If we do this here, there's no thread sync to worry about since we're single-threaded here.
		//Singletons::ServiceManager.Instrumentation().UpdatePeriodicData(deltaTime);

		// Check if the execution is paused, and set delta time to 0 if so.
		//if ( Singletons::EnvironmentManager.Runtime().GetStatus() ==
		//	IEnvironment::IRuntime::Status::Paused )
		//{
		//	deltaTime = 0.0f;
		//}

		if (m_ThreadingEnabled)
		{
			// Schedule the tasks for component-worlds that are ready for execution
			if (m_ResortTasks)
			{
				m_SortedTasks.clear();
				for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
				{
					ISystemWorld* world = *it;
					m_SortedTasks.push_back(world->GetTask());
				}
				std::sort(m_SortedTasks.begin(), m_SortedTasks.end(), [](ISystemTask* first, ISystemTask* second)->bool
				{
					return first->GetPerformanceHint() < second->GetPerformanceHint();
				});
			}

			m_TaskManager->SpawnJobsForSystemTasks(m_SortedTasks, deltaTime);

			// Wait for the tasks to complete
			m_TaskManager->WaitForSystemTasks(m_SortedTasks);
		}
		else // Not threading enabled
		{
			for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
			{
				ISystemWorld* world = *it;
				world->GetTask()->Update(deltaTime);
			}
		}
	}

}
