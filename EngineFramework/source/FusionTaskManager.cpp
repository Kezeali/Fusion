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

#include "FusionTaskManager.h"

#include "FusionProfiling.h"

#include "FusionLogger.h"

#include "FusionSystemTask.h"

#include <algorithm>
#include <functional>

#include <tbb/tbb.h>

#include <EASTL/hash_map.h>

namespace FusionEngine
{

#if defined(FSN_ALLOW_PRIMARY_THREAD_TASK_DEPENDENCIES)
	class PrimaryThreadTask
	{
	public:
		PrimaryThreadTask(SystemTaskBase* implementation);

		void IncrementDependencyCount() { ++m_UnexecutedDependencies; }

		int DecrementDependencyCount() { return --m_UnexecutedDependencies; }

		int UnexecutedDependencies() const { return m_UnexecutedDependencies; }

		void Execute();

	private:
		SystemTaskBase* m_Implementation;

		tbb::atomic<int> m_UnexecutedDependencies;
	};

	class PrimaryThreadTaskScheduler : public Singleton<PrimaryThreadTaskScheduler>
	{
	public:
		PrimaryThreadTaskScheduler();

		void AddTask(std::unique_ptr<PrimaryThreadTask> task);

		void Enqueue(PrimaryThreadTask* ready_task);

		void Process();

		bool IsDone() const;

	private:
		std::vector<std::unique_ptr<PrimaryThreadTask>> m_Tasks;

		tbb::concurrent_queue<PrimaryThreadTask*> m_ReadyTaskQueue;
	};

	PrimaryThreadTask::PrimaryThreadTask(SystemTaskBase* implementation)
		: m_Implementation(implementation)
	{
	}

	void PrimaryThreadTask::Execute()
	{
		FSN_ASSERT(m_UnexecutedDependencies == 0);
		ExecuteTaskImplementation(m_Implementation);
	}

	PrimaryThreadTaskScheduler::PrimaryThreadTaskScheduler()
	{
	}

	void PrimaryThreadTaskScheduler::AddTask(std::unique_ptr<PrimaryThreadTask> task)
	{
		m_Tasks.push_back(std::move(task));
	}
	
	void PrimaryThreadTaskScheduler::Enqueue(PrimaryThreadTask* ready_task)
	{
		m_ReadyTaskQueue.push(ready_task);
	}

	void PrimaryThreadTaskScheduler::Process()
	{
		PrimaryThreadTask* readyTask;
		while (m_ReadyTaskQueue.try_pop(readyTask))
		{
			FSN_ASSERT(!m_Tasks.empty());
			FSN_ASSERT(std::count(m_Tasks.cbegin(), m_Tasks.cend(), readyTask) == 1);
			FSN_ASSERT(readyTask->UnexecutedDependencies() == 0);
			readyTask->Execute();
			m_Tasks.erase(std::remove(m_Tasks.begin(), m_Tasks.end(), readyTask));
		}
	}

	bool PrimaryThreadTaskScheduler::IsDone() const
	{
		FSN_ASSERT(m_ReadyTaskQueue.empty() || !m_Tasks.empty());
		return m_Tasks.empty();
	}
#endif

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
					clan::Event::wait(m_AllCallbacksInvokedEvent);
				}

				return NULL;
			}

			static void PrepareCallback(const std::function<void (void)>& fn, uint32_t count) 
			{
				m_Callback = fn;
				m_CallbacksRemainingToCall = count;
				m_AllCallbacksInvokedEvent.reset();
			}

#ifdef _WIN32
		protected:
#endif
			friend class TaskManager;
			static clan::Event m_AllCallbacksInvokedEvent;
			static std::function<void (void)> m_Callback;
			static tbb::atomic<uint32_t> m_CallbacksRemainingToCall;
		}; // class SynchronizeTask

		clan::Event SynchronisedTask::m_AllCallbacksInvokedEvent;
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

	template <class T>
	class FunctionTask : public tbb::task
	{
	public:
		FunctionTask(T fn)
			: m_Function(fn)
		{
		}

		virtual tbb::task* execute() override
		{
			m_Function();

			return NULL;
		}

	protected:
		T m_Function;
	};

	namespace
	{
		void ExecuteTaskImplementation(SystemTaskBase* task_implementation)
		{
			FSN_PROFILE(task_implementation->GetName().c_str());
			try
			{
				task_implementation->Update();
			}
			catch (std::exception& e)
			{
				AddLogEntry(e.what(), LOG_CRITICAL);
				SendToConsole(e.what());
			}
		}
	}

	class DepTask : public tbb::task
	{
	public:
		DepTask(SystemTaskBase* implementation)
			: m_Implementation(implementation)
		{
		}

		void AddDependant(tbb::task* dependant)
		{
			FSN_ASSERT(this->state() != executing);
			dependant->increment_ref_count();
			m_Successors.push_back(dependant);
		}

#if defined(FSN_ALLOW_PRIMARY_THREAD_TASK_DEPENDENCIES)
		void AddDependant(PrimaryThreadTask* dependant)
		{
			FSN_ASSERT(this->state() != executing);
			dependant->IncrementDependencyCount();
			m_PrimaryThreadSuccessors.push_back(dependant);
		}
#endif

		SystemTaskBase* GetImplementation() const { return m_Implementation; }

		virtual tbb::task* execute() override
		{
			ExecuteTaskImplementation(m_Implementation);
			for (auto successor : m_Successors)
			{
				if (successor->decrement_ref_count() == 0) // if this was the last prerequisite that the next task was waiting on
					task::spawn(*successor);
			}
			m_Successors.clear();
#if defined(FSN_ALLOW_PRIMARY_THREAD_TASK_DEPENDENCIES)
			for (auto successorPrim : m_PrimaryThreadSuccessors)
			{
				if (successorPrim->DecrementDependencyCount() == 0)
					PrimaryThreadTaskScheduler::getSingleton().Enqueue(successorPrim);
			}
			m_PrimaryThreadSuccessors.clear();
#endif
			return NULL;
		}

	protected:
		SystemTaskBase* m_Implementation;
		std::vector<tbb::task*> m_Successors;
#if defined(FSN_ALLOW_PRIMARY_THREAD_TASK_DEPENDENCIES)
		std::vector<PrimaryThreadTask*> m_PrimaryThreadSuccessors;
#endif
	};

	// Allows FunctionTask objects to be made for lambdas
	template <class T>
	FunctionTask<T>* MakeFunctionTask(tbb::task* root, T fn)
	{
		return new( root->allocate_additional_child_of(*root) ) FunctionTask<T>(fn);
	}

	void TaskManager::SpawnJobsForSystemTasks(const std::vector<SystemTaskBase*>& tasks)
	{
		// Call this from the primary thread to schedule system work
		FSN_ASSERT(IsPrimaryThread());

		FSN_ASSERT(!tasks.empty());

		FSN_ASSERT(m_SystemTasksRoot != NULL);

		// Tasks will be added to the m_pSystemTasksRoot, and will eventually call wait_for_all on it.
		//  Support the eventual wait_for_all by setting reference count to 1 now
		m_SystemTasksRoot->set_ref_count(1);

		std::vector<DepTask*> spawnedTasks;
		eastl::hash_map<eastl::string, DepTask*> namedTasks;

		// now schedule the tasks, based upon their PerformanceHint order
		tbb::task_list taskList;

		auto affinityIterator = m_AffinityIDs.begin();
		for (auto it = tasks.begin(), end = tasks.end(); it != end; ++it)
		{
			auto taskImplementation = *it;

			FSN_ASSERT(taskImplementation);

			if (taskImplementation->IsPrimaryThreadOnly())
			{
#if defined(FSN_ALLOW_PRIMARY_THREAD_TASK_DEPENDENCIES)
				m_PrimaryThreadTaskScheduler->AddTask(std::unique_ptr<PrimaryThreadTask>(new PrimaryThreadTask(taskImplementation)));
#else
				m_TasksToExecuteInPrimaryThread.push_back(taskImplementation);
#endif
			}
			else
			{
				// Affinity will increase the chances that each SystemTask will be assigned
				//  to a unique thread
				const auto affinityId = *(affinityIterator++);
				if (affinityIterator == m_AffinityIDs.end())
					affinityIterator = m_AffinityIDs.begin();

				auto tbbTask = new DepTask(taskImplementation);
				tbbTask->set_affinity(affinityId);

				spawnedTasks.push_back(tbbTask);
				namedTasks.insert(eastl::make_pair(taskImplementation->GetName(), tbbTask));

				taskList.push_back(*tbbTask);
			}
		}

		for (auto task : spawnedTasks)
		{
			auto deps = task->GetImplementation()->GetDependencies();
			for (auto dep : deps)
			{
				auto entry = namedTasks.find(dep);
				if (entry != namedTasks.end())
					entry->second->AddDependant(task);
			}
		}

		// Only system tasks spawn here. They in their turn will spawn descendant tasks.
		// Waiting for completion happens in WaitForSystemTasks.
		m_SystemTasksRoot->spawn(taskList);
	}

	void TaskManager::WaitForSystemTasks()
	{
		FSN_ASSERT(IsPrimaryThread());

//#ifdef _WIN32
//		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
//#endif

#if !defined(FSN_ALLOW_PRIMARY_THREAD_TASK_DEPENDENCIES)
		for (auto taskImplementation : m_TasksToExecuteInPrimaryThread)
		{
			ExecuteTaskImplementation(taskImplementation);
		}
		m_TasksToExecuteInPrimaryThread.clear();
#else
		m_PrimaryThreadTaskScheduler->Process();
#endif

//#ifdef _WIN32
//		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
//#endif

		// Contribute to the parallel execution & capture task errors
		try
		{
			m_SystemTasksRoot->wait_for_all();
		}
		catch (tbb::captured_exception& e)
		{
			AddLogEntry(std::string(e.name()) + e.what(), LOG_CRITICAL);
			SendToConsole(e.what());
		}
		catch (tbb::bad_last_alloc& e)
		{
			AddLogEntry(e.what(), LOG_CRITICAL);
			throw e;
		}
		catch (tbb::improper_lock& e)
		{
			AddLogEntry(e.what(), LOG_CRITICAL);
			throw e;
		}
		catch (tbb::invalid_multiple_scheduling& e)
		{
			AddLogEntry(e.what(), LOG_CRITICAL);
			throw e;
		}
		catch (tbb::missing_wait& e)
		{
			AddLogEntry(e.what(), LOG_CRITICAL);
			throw e;
		}
		catch (std::exception e)
		{
			AddLogEntry(e.what(), LOG_CRITICAL);
			SendToConsole(e.what());
		}
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
