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

#include "FusionProfiling.h"

#include <functional>

#include <tbb/tbb.h>

namespace FusionEngine
{

	namespace tasks
	{
		class SynchronisedTask: public tbb::task
		{
		public:
			SynchronisedTask() {}

			tbb::task *execute()
			{
				FSN_ASSERT(m_Callback);

				m_Callback();

				if (--m_CallbacksRemainingToCall == 0)
				{
					// This is the last callback to complete: allow all the others to return
					m_AllCallbacksInvokedEvent.set();
				}
				else
				{
					// Wait for everybody else to finish up
					CL_Event::wait(m_AllCallbacksInvokedEvent);
				}

				return NULL;
			}

			static void PrepareCallback(const std::function<void (void)>& fn, uint32_t count) 
			{
				m_Callback = fn;
				m_CallbacksRemainingToCall = count;
				m_AllCallbacksInvokedEvent.reset();
			}

		protected:
			friend class TaskManager;
			static CL_Event m_AllCallbacksInvokedEvent;
			static std::function<void (void)> m_Callback;
			static tbb::atomic<uint32_t> m_CallbacksRemainingToCall;
		}; // class SynchronizeTask

		CL_Event SynchronisedTask::m_AllCallbacksInvokedEvent;
		std::function<void (void)> SynchronisedTask::m_Callback;
		tbb::atomic<uint32_t> SynchronisedTask::m_CallbacksRemainingToCall;
	}

	using namespace tasks;

	TaskManager::TaskManager()
	{
		m_PrimaryThreadID = tbb::this_tbb_thread::get_id();

		SynchronisedTask::m_CallbacksRemainingToCall = 0;

		m_NumberOfThreads = tbb::task_scheduler_init::default_num_threads();

		m_TbbScheduler = new tbb::task_scheduler_init(m_NumberOfThreads);
		m_SystemTasksRoot = new (tbb::task::allocate_root()) tbb::empty_task;

		NonStandardPerThreadCallback([this]()
		{
			tbb::spin_mutex::scoped_lock lock(m_Mutex);
			m_AffinityIDs.push_back(tbb::task::self().affinity());
		});
	}

	TaskManager::~TaskManager()
	{
		NonStandardPerThreadCallback([](){ asThreadCleanup(); });

		m_SystemTasksRoot->destroy( *m_SystemTasksRoot );

		delete m_TbbScheduler;
	}

	class FunctorTask : public tbb::task
	{
	public:
		FunctorTask(const std::function<void (void)>& functor)
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
		std::function<void (void)> m_Function;
	};

	class FunctorSupertask : public tbb::task
	{
	public:
		FunctorSupertask(const std::vector<std::function<void (void)>>& functors)
			: m_Functors(functors)
		{
		}

		virtual tbb::task* execute()
		{
			FSN_ASSERT(!m_Functors.empty());

			for (auto it = m_Functors.begin(), end = m_Functors.end(); it != end; ++it)
			{
				auto& functor = *it;
				functor();
			}

			return NULL;
		}

	protected:
		std::vector<std::function<void (void)>> m_Functors;
	};

	template <class T>
	class FunctionTask : public tbb::task
	{
	public:
		FunctionTask(T fn)
			: m_Function(fn)
		{
		}

		virtual tbb::task* execute()
		{
			m_Function();

			return NULL;
		}

	protected:
		T m_Function;
	};

	class IntrusiveSystemTask : tbb::task
	{
	public:
		IntrusiveSystemTask(std::vector<ISystemTask*> subtasks, float delta)
			: m_Subtasks(subtasks),
			m_Delta(delta)
		{
		}

		virtual tbb::task* execute()
		{
			FSN_ASSERT(!m_Subtasks.empty());

			for (auto it = m_Subtasks.begin(), end = m_Subtasks.end(); it != end; ++it)
			{
				auto& subtask = *it;
				FSN_ASSERT(subtask != nullptr);
				subtask->Update(m_Delta);
			}

			return NULL;
		}

	protected:
		std::vector<ISystemTask*> m_Subtasks;
		const float m_Delta;
	};

	typedef FunctorTask SystemTask;
	typedef FunctorSupertask SystemSupertask;

	// Allows FunctionTask objects to be made for lambdas
	template <class T>
	FunctionTask<T>* MakeFunctionTask(tbb::task* root, T fn)
	{
		return new( root->allocate_additional_child_of(*root) ) FunctionTask<T>(fn);
	}

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

			FSN_ASSERT(task);

			if (task->IsPrimaryThreadOnly())
			{
				m_PrimaryThreadSystemTasks.push_back(task);
			}
			else
			{
				//auto systemTask = new( m_SystemTasksRoot->allocate_additional_child_of(*m_SystemTasksRoot) ) SystemTask(std::bind(&ISystemTask::Update, task, delta));
				//FSN_ASSERT(systemTask != nullptr);

				auto systemTask = MakeFunctionTask(m_SystemTasksRoot, [task, delta]()
				{
					//FSN_ASSERT(task->GetSystemWorld() && task->GetSystemWorld()->GetSystem(), "Invalid task");
					FSN_PROFILE(task->GetName());
					task->Update(delta);
				});
				FSN_ASSERT(systemTask != nullptr);

				// Affinity will increase the chances that each SystemTask will be assigned
				//  to a unique thread, regardless of PerformanceHint
				const auto affinityId = *(affinityIterator++);
				if (affinityIterator == m_AffinityIDs.end())
					affinityIterator = m_AffinityIDs.begin();
				systemTask->set_affinity(affinityId);

				taskList.push_back(*systemTask);
			}

		}

		// Only system tasks spawn here. They in their turn will spawn descendant tasks.
		// Waiting for completion happens in WaitForSystemTasks.
		m_SystemTasksRoot->spawn(taskList);
	}

	void TaskManager::WaitForSystemTasks()
	{
		FSN_ASSERT(IsPrimaryThread());

#ifdef _WIN32
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif

		std::vector<ISystemTask*> primaryThreadTasksNotRun;

		// Run primary-thread tasks
		for (auto it = m_PrimaryThreadSystemTasks.begin() ; it != m_PrimaryThreadSystemTasks.end(); ++it)
		{
			// We are, so execute it now on the primary thread
			//__ITT_EVENT_START( GetSupportForSystemTask( *it ).m_tpeSystemTask, PROFILE_TASKMANAGER );

			FSN_PROFILE((*it)->GetSystemWorld()->GetSystem()->GetName());

			(*it)->Update(m_DeltaTime);

			//__ITT_EVENT_END( GetSupportForSystemTask( *it ).m_tpeSystemTask, PROFILE_TASKMANAGER );
		}

#ifdef _WIN32
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
#endif

		m_PrimaryThreadSystemTasks.clear();

		// Contribute to the parallel execution, and when it completes, we're done
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

	void TaskManager::NonStandardPerThreadCallback(std::function<void (void)> fn)
	{
		// This method triggers a synchronized callback to be called once by each thread used
		// by the TaskManagerTBB.  This method waits until all callbacks have executed.

		// only one at a time here
		//SpinWait::Lock tLock( m_tSynchronizedCallbackMutex );

		//__ITT_EVENT_START( m_tSynchronizeTPEvent, PROFILE_TASKMANAGER );

		//u32 uNumberOfThreads = m_uNumberOfThreads;
		//if( uNumberOfThreads != m_uMaxNumberOfThreads )
		//{
		//	m_uTargetNumberOfThreads = m_uMaxNumberOfThreads;
		//	UpdateThreadPoolSize();
		//}

		SynchronisedTask::PrepareCallback(fn, m_NumberOfThreads);

		tbb::task* broadcastParent = new( tbb::task::allocate_root() ) tbb::empty_task;
		FSN_ASSERT(broadcastParent != NULL);

		// we have one reference for each thread, plus one for the wait_for_all below
		broadcastParent->set_ref_count( m_NumberOfThreads + 1 );

		tbb::task_list taskList;
		for( uint32_t i = 0; i < m_NumberOfThreads; i++ )
		{
			// Add a SynchronizedTasks for each thread in the TBB pool
			tbb::task *newTask = new( broadcastParent->allocate_child() ) SynchronisedTask;
			FSN_ASSERT(newTask != NULL);
			taskList.push_back(*newTask);
		}

		// get the synchronize tasks running
		broadcastParent->spawn_and_wait_for_all( taskList );
		broadcastParent->destroy(*broadcastParent);

		//if( uNumberOfThreads != m_uMaxNumberOfThreads )
		//{
		//	m_uTargetNumberOfThreads = uNumberOfThreads;
		//	UpdateThreadPoolSize();
		//}
		//__ITT_EVENT_END( m_tSynchronizeTPEvent, PROFILE_TASKMANAGER );
	}

}
