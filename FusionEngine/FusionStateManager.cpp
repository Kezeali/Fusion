/*
  Copyright (c) 2006-2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
		
		
	File Author(s):

		Elliot Hayward

*/

#include "FusionStateManager.h"

namespace FusionEngine
{

	SystemsManager::SystemsManager()
	{
	}

	SystemsManager::~SystemsManager()
	{
		Clear();
	}

	bool SystemsManager::SetExclusive(System *system)
	{
		// Try to initialise the new state
		if (!system->Initialise())
			return false;

		// Remove all the current states
		Clear();

		// Add the new state if it managed to init.
		//SharedState state_spt(state);
		m_Systems.push_back(system);

		return true;
	}

	//bool StateManager::RunNextQueueState()
	//{
	//	SharedState state = m_Queued.front();
	//	m_Queued.pop_front();
	//	return AddState(state);
	//}

	//bool StateManager::AddState(FusionState *state)
	//{
	//	// Make a shared ptr out of the given pointer and add it
	//	//SharedState state_spt(state);
	//	return AddState(SharedState(state) );
	//}

	//bool StateManager::AddState(SharedState state)
	//{
	//	// Try to initialise the state
	//	if (!state->Initialise())
	//		return false;

	//	// Add the state if it managed to init.
	//	m_States.push_back(state);

	//	return true;
	//}


	//void StateManager::AddStateToQueue(FusionState *state)
	//{
	//	//SharedState spt_state(state);
	//	m_Queued.push_back( SharedState(state) );
	//}

	//void StateManager::AddStateToQueue(SharedState state)
	//{
	//	m_Queued.push_back(state);
	//}


	//void StateManager::RemoveState(FusionState *state)
	//{
	//	bool blocking = false;

	//	StateList::iterator it;
	//	for (it = m_States.begin(); it != m_States.end(); ++it)
	//	{
	//		if (it->get() == state)
	//		{
	//			(*it)->CleanUp();
	//			m_States.erase(it);
	//		}
	//		else
	//			blocking |= (*it)->IsBlocking();
	//	}

	//	// If none of the states left over are blocking, we can run the next queued state
	//	if (!blocking)
	//		RunNextQueueState();
	//}

	//void StateManager::RemoveState(SharedState state)
	//{
	//	bool blocking = false;

	//	StateList::iterator it;
	//	for (it = m_States.begin(); it != m_States.end(); ++it)
	//	{
	//		// Compare pointers
	//		if ((*it) == state)
	//		{
	//			(*it)->CleanUp();
	//			m_States.erase(it);
	//		}
	//		else
	//			// Check if any of the states NOT removed ARE blocking
	//			blocking |= (*it)->IsBlocking();
	//	}

	//	// If none of the states left over are blocking, we can run the next queued state
	//	if (!blocking)
	//		RunNextQueueState();
	//}


	//void StateManager::RemoveStateFromQueue(FusionState *state)
	//{
	//	StateQueue::iterator it;
	//	for (it = m_Queued.begin(); it != m_Queued.end(); ++it)
	//	{
	//		if (it->get() == state)
	//		{
	//			m_Queued.erase(it);
	//			break;
	//		}
	//	}
	//}

	//void StateManager::RemoveStateFromQueue(SharedState state)
	//{
	//	StateQueue::iterator it;
	//	for (it = m_Queued.begin(); it != m_Queued.end(); ++it)
	//	{
	//		if ((*it).get() == state.get())
	//		{
	//			m_Queued.erase(it);
	//			break;
	//		}
	//	}
	//}

	bool SystemsManager::AddSystem(System *system)
	{
		m_Systems.push_back(system);
		return system->Initialise();
	}

	void SystemsManager::RemoveSystem(System *system)
	{
		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			if ((*it) == system)
			{
				m_Systems.erase(it);
				system->CleanUp();
				delete system;
				break;
			}
		}
	}

	void SystemsManager::Clear()
	{
		if (!m_Systems.empty())
		{
			for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
			{
				System *system = *it;
				system->CleanUp();
				delete system;
			}
			m_Systems.clear();
		}
		//ClearQueue();

		//ClearActive();
	}

	//void StateManager::ClearActive()
	//{
	//	if (!m_States.empty())
	//	{
	//		StateList::iterator it;
	//		for (it = m_States.begin(); it != m_States.end(); ++it)
	//		{
	//			(*it)->CleanUp();
	//		}
	//		m_States.clear();
	//	}
	//}

	//void StateManager::ClearQueue()
	//{
	//	m_Queued.clear();
	//}

	bool SystemsManager::Update(float split)
	{
		// If game should have quit, but for some reason update is being called again...
		if (!m_KeepGoing)
			return true; // ... breakout!

		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			System *system = *it;

			// Check state messages
			SystemMessage *m = system->PopMessage();
			while (m != NULL)
			{
				switch (m->GetType())
				{
				//case SystemMessage::ADDSTATE:
				//	AddState(m->GetData());
				//	break;
				//case SystemMessage::REMOVESTATE:
				//	RemoveState(m->GetData());
				//	break;
				//case SystemMessage::QUEUESTATE:
				//	AddStateToQueue(m->GetData());
				//	break;
				//case SystemMessage::UNQUEUESTATE:
				//	RemoveStateFromQueue(m->GetData());
				//	break;
				//case SystemMessage::RUNNEXTSTATE:
				//	RunNextQueueState();
				//	break;

				case SystemMessage::PAUSE:
					if (m->IncludeSender())
						system->AddFlag(System::PAUSE);
					addFlagFor(m->GetTargets(), System::PAUSE);
					break;
				case SystemMessage::RESUME:
					if (m->IncludeSender())
						system->RemoveFlag(System::PAUSE);
					removeFlagFor(m->GetTargets(), System::PAUSE);
					break;
				case SystemMessage::STEP:
					if (m->IncludeSender())
						system->AddFlag(System::STEP);
					addFlagFor(m->GetTargets(), System::STEP);
					break;
				case SystemMessage::STEPALL:
					addFlagAll(System::STEP);
					break;
				case SystemMessage::PAUSEOTHERS:
					addFlagForOthers(system, m->GetTargets(), System::PAUSE);
					break;
				case SystemMessage::RESUMEOTHERS:
					removeFlagForOthers(system, m->GetTargets(), System::PAUSE);
					break;

				case SystemMessage::HIDE:
					removeFlagForOthers(system, m->GetTargets(), System::PAUSE);
					break;

				case SystemMessage::QUIT:
					Clear();
					m_KeepGoing = false;
					break;
				}

				// Clean up
				delete m;
				// Get the next message
				m = system->PopMessage();
			}

			// If the system isn't paused or a step message was just received:
			if (!system->CheckFlag(System::PAUSE) || system->CheckFlag(System::STEP))
			{
				system->Update(split);
				// Clear step flag (in case it has been added, since we have now done the requested 'step')
				system->RemoveFlag(System::STEP);
			}
		}

		return true;
	}

	void SystemsManager::Draw()
	{
		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			System *system = *it;
			if (!system->CheckFlag(System::HIDE))
				system->Draw();
		}
	}

	bool SystemsManager::KeepGoing() const
	{
		return m_KeepGoing;
	}

	void SystemsManager::setFlagsAll(int flags)
	{
		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			(*it)->SetFlags(flags);
		}
	}

	void SystemsManager::addFlagAll(System::StateFlags flag)
	{
		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			(*it)->AddFlag(flag);
		}
	}

	void SystemsManager::removeFlagAll(System::StateFlags flag)
	{
		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			(*it)->RemoveFlag(flag);
		}
	}

	void SystemsManager::addFlagFor(const StringVector &targets, System::StateFlags flag)
	{
		for (StringVector::const_iterator target_it = targets.begin(), targets_end = targets.end(); target_it != targets_end; ++target_it)
		{
			for (SystemArray::iterator system_it = m_Systems.begin(), systems_end = m_Systems.end();
				system_it != systems_end; ++system_it)
			{
				if (*target_it == (*system_it)->GetName())
					(*system_it)->AddFlag(flag);
			}
		}
	}

	void SystemsManager::removeFlagFor(const StringVector &targets, System::StateFlags flag)
	{
		for (StringVector::const_iterator target_it = targets.begin(), targets_end = targets.end(); target_it != targets_end; ++target_it)
		{
			for (SystemArray::iterator system_it = m_Systems.begin(), systems_end = m_Systems.end();
				system_it != systems_end; ++system_it)
			{
				if (*target_it == (*system_it)->GetName())
					(*system_it)->RemoveFlag(flag);
			}
		}
	}

	void SystemsManager::addFlagForOthers(System *excluded_system, const StringVector &excluded_targets, System::StateFlags flag)
	{
		for (SystemArray::iterator system_it = m_Systems.begin(), systems_end = m_Systems.end();
			system_it != systems_end; ++system_it)
		{
			System *system = *system_it;
			if (system != excluded_system)
			{
				// Check for the current systems's name in the exclusions
				bool excluded = false;
				for (StringVector::const_iterator exclude_it = excluded_targets.begin(), targets_end = excluded_targets.end();
					exclude_it != targets_end; ++exclude_it)
				{
					if (system->GetName() == *exclude_it)
						excluded = true;
				}
				if (!excluded)
					system->AddFlag(flag);
			}
		}
	}

	void SystemsManager::removeFlagForOthers(System *excluded_system, const StringVector &excluded_targets, System::StateFlags flag)
	{
		for (SystemArray::iterator system_it = m_Systems.begin(), systems_end = m_Systems.end();
			system_it != systems_end; ++system_it)
		{
			System *system = *system_it;
			if (system != excluded_system)
			{
				// Check for the current systems's name in the exclusions
				bool excluded = false;
				for (StringVector::const_iterator exclude_it = excluded_targets.begin(), targets_end = excluded_targets.end();
					exclude_it != targets_end; ++exclude_it)
				{
					if (system->GetName() == *exclude_it)
						excluded = true;
				}
				if (!excluded)
					system->RemoveFlag(flag);
			}
		}
	}

}
