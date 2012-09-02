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

#ifndef H_FusionTaskManager
#define H_FusionTaskManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentSystem.h"

#include <tbb/task.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/tbb_thread.h>

namespace FusionEngine
{

	class TaskManager
	{
	public:
		TaskManager();

		~TaskManager();

		void SpawnJobsForSystemTasks(const std::vector<ISystemTask*>& tasks, const float delta);
		void WaitForSystemTasks();

    /// This method triggers a synchronized callback to be called once by each thread used by the <c>TaskManagerTBB</c>.
    /// This method waits until all callbacks have executed.
    void NonStandardPerThreadCallback(std::function<void(void)> fn);

		bool IsPrimaryThread() const;

	private:
		tbb::tbb_thread::id m_PrimaryThreadID;

		tbb::task_scheduler_init* m_TbbScheduler;

		tbb::task* m_SystemTasksRoot;

		unsigned int m_NumberOfThreads;

		tbb::spin_mutex m_Mutex;

		std::vector<ISystemTask*> m_PrimaryThreadSystemTasks;

		float m_DeltaTime;

		std::vector<tbb::task::affinity_id> m_AffinityIDs;

		static bool IsTBBThread();
	};

}

#endif