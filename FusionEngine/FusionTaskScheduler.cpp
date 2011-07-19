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
		m_FramerateLimiterEnabled(false),
		m_Unlimited(false)
	{
		m_DeltaTime = 1.f / 30.f;
		m_DeltaTimeMS = (unsigned int)(m_DeltaTime * 1000);

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
		auto currentTime = CL_System::get_time();
		if (m_LastTime == 0)
			m_LastTime = currentTime;
		auto timePassed = currentTime - m_LastTime;
		m_LastTime = currentTime;

		bool renderOnly = false;

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
			m_Accumulator += fe_min(timePassed, m_DeltaTimeMS);

			if (m_Accumulator >= m_DeltaTimeMS)
			{
				m_Accumulator -= m_DeltaTimeMS;
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
				m_TaskManager->SpawnJobsForSystemTasks(m_SortedTasks, m_DeltaTime);

				m_TaskManager->WaitForSystemTasks(m_SortedTasks);
			}
			else
			{
				m_TaskManager->SpawnJobsForSystemTasks(m_SortedRenderTasks, m_DeltaTime);

				m_TaskManager->WaitForSystemTasks(m_SortedRenderTasks);
			}
		}
		else // Not threading enabled
		{
			for (auto it = m_ComponentWorlds.begin(); it != m_ComponentWorlds.end(); ++it)
			{
				ISystemWorld* world = *it;
				if (!renderOnly || world->GetSystemType() == SystemType::Rendering)
					world->GetTask()->Update(m_DeltaTime);
			}
		}
	}

}
