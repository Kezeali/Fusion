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

#include "FusionTaskManager.h"

#include <functional>

#include <tbb/tbb.h>

namespace FusionEngine
{

	TaskManager::TaskManager()
	{
		m_PrimaryThreadID = tbb::this_tbb_thread::get_id();

		//m_Quit = false;
		//m_uRequestedNumberOfThreads = Singletons::EnvironmentManager.Variables().GetAsInt( "TaskManager::Threads", 0 );
		//if( m_uRequestedNumberOfThreads == 0 )
		//{
		//	// IMPLEMENTATION NOTE
		//	// The audio thread (which Thread Profiler shows as constantly working) 
		//	// has no negative impact on the performance when it causes apparent 
		//	// oversubscription. Shrinking the pool to avoid this apparent oversubscription
		//	// results in smaller FPS. So this is probably one of the cases when TP
		//	// misinterprets the behavior of threads created by some of the Windows 
		//	// subsystems (DirectX is historically its weak place).
		//	//
		//	m_uRequestedNumberOfThreads = tbb::task_scheduler_init::default_num_threads();
		//}

		//m_uMaxNumberOfThreads = 0;
		//m_uNumberOfThreads = 0;
		//m_uTargetNumberOfThreads = 0;

		//m_pStallPoolParent = NULL;
		//m_hStallPoolSemaphore = CreateSemaphore( NULL, 0, m_uRequestedNumberOfThreads, NULL );
		//SynchronizeTask::m_hAllCallbacksInvokedEvent = CreateEvent( NULL, True, False, NULL );

		//m_uMaxNumberOfThreads = m_uRequestedNumberOfThreads;
		//m_uTargetNumberOfThreads = m_uRequestedNumberOfThreads;
		//m_uNumberOfThreads = m_uRequestedNumberOfThreads;

		m_TbbScheduler = new tbb::task_scheduler_init(tbb::task_scheduler_init::default_num_threads());
		m_SystemTasksRoot = new (tbb::task::allocate_root()) tbb::empty_task;
	}

	TaskManager::~TaskManager()
	{
		// get the callback thread to exit
		//m_Quit = True;

		// trigger the release of the stall pool
		//ReleaseSemaphore( m_hStallPoolSemaphore, m_uMaxNumberOfThreads, NULL );

		m_SystemTasksRoot->destroy( *m_SystemTasksRoot );

		delete m_TbbScheduler;

		// now get rid of all the events
		//CloseHandle( m_hStallPoolSemaphore );
		//m_hStallPoolSemaphore = NULL;
		//CloseHandle( SynchronizeTask::m_hAllCallbacksInvokedEvent );
	}

	template <typename T>
	class FunctorTask : public tbb::task
	{
	public:
		FunctorTask(const std::function<T>& functor)
			: m_Function(functor)
		{
		}

		virtual tbb::task* execute()
		{
			FSN_ASSERT(m_Function);

			m_Function();

			return NULL;
		}

	protected:
		std::function<T> m_Function;
	};

	typedef FunctorTask<void (void)> SystemTask;

	void TaskManager::SpawnJobsForSystemTasks(const std::vector<ISystemTask*>& tasks, const float delta)
	{
		// Call this from the primary thread to schedule system work
		FSN_ASSERT(IsPrimaryThread());

		FSN_ASSERT(!tasks.empty());

		m_DeltaTime = delta;

		FSN_ASSERT(m_SystemTasksRoot != NULL);

		// Tasks will be added to the m_pSystemTasksRoot, and will eventually call wait_for_all on it.
		//  Support the eventual wait_for_all by setting reference count to 1 now
		m_SystemTasksRoot->set_ref_count(1);

		// now schedule the tasks, based upon their PerformanceHint order
		tbb::task_list taskList;

		auto affinityIterator = m_AffinityIDs.begin();
		for (auto it = tasks.begin(), end = tasks.end(); it != end; ++it)
		{
			auto task = *it;

			if (task->IsPrimaryThreadOnly())
			{
				m_PrimaryThreadSystemTasks.push_back(task);
			}
			else
			{
				SystemTask* systemTask = new( m_SystemTasksRoot->allocate_additional_child_of(*m_SystemTasksRoot) ) SystemTask(std::bind(&ISystemTask::Update, task, delta));
				FSN_ASSERT(systemTask != nullptr);

				// Affinity will increase the chances that each SystemTask will be assigned
				//  to a unique thread, regardless of PerformanceHint
				//systemTask->set_affinity(*(affinityIterator++));
				//if (affinityIterator == m_AffinityIDs.end())
				//	affinityIterator = m_AffinityIDs.begin();

				taskList.push_back(*systemTask);
			}
		}

		// Only system tasks spawn here. They in their turn will spawn descendant tasks.
		// Waiting for completion happens in WaitForSystemTasks.
		m_SystemTasksRoot->spawn(taskList);
	}

	void TaskManager::WaitForSystemTasks(const std::vector<ISystemTask*>& tasks)
	{
		FSN_ASSERT(IsPrimaryThread());

#ifdef _WIN32
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif

		std::vector<ISystemTask*> primaryThreadTasksNotRun;

		// If any of the given system tasks are primary-thread-only tasks, run them now
		for (auto it = m_PrimaryThreadSystemTasks.begin() ; it != m_PrimaryThreadSystemTasks.end(); ++it)
		{
			if (std::find(tasks.begin(), tasks.end(), *it) != tasks.end())
			{
				// We are, so execute it now on the primary thread
				//__ITT_EVENT_START( GetSupportForSystemTask( *it ).m_tpeSystemTask, PROFILE_TASKMANAGER );

				(*it)->Update(m_DeltaTime);

				//__ITT_EVENT_END( GetSupportForSystemTask( *it ).m_tpeSystemTask, PROFILE_TASKMANAGER );
			}
			else
			{
				// Save it for next time
				primaryThreadTasksNotRun.push_back(*it);
			}
		}

#ifdef _WIN32
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
#endif

		m_PrimaryThreadSystemTasks.clear();
		m_PrimaryThreadSystemTasks.swap(primaryThreadTasksNotRun);

		// Contribute to the parallel calculation, and when it completes, we're done
		m_SystemTasksRoot->wait_for_all();
	}

	bool TaskManager::IsPrimaryThread() const
	{
		return tbb::this_tbb_thread::get_id() == m_PrimaryThreadID;
	}

	bool TaskManager::IsTBBThread()
	{
		// This method is used to determine if the calling thread is an Intel Threading Building Blocks thread.
#ifdef _DEBUG
		// If called not from TBB thread task::self() will assert itself
		return &tbb::task::self() != NULL;
#else
		return true;
#endif
	}

}
