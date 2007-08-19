/*
  Copyright (c) 2006-2007 Fusion Project Team

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

	StateManager::StateManager()
	{
	}

	StateManager::~StateManager()
	{
		Clear();
	}

	bool StateManager::SetExclusive(SharedState state)
	{
		// Try to initialise the new state
		if (!state->Initialise())
			return false;

		// Remove all the current states
		if (!m_States.empty())
		{
			StateList::iterator it;
			for (it = m_States.begin(); it != m_States.end(); ++it)
			{
				(*it)->CleanUp();
			}
			m_States.clear();
		}

		// Add the new state if it managed to init.
		//SharedState state_spt(state);
		m_States.push_back(state);

		return true;
	}

	bool StateManager::RunNextQueueState()
	{
		SharedState state = m_Queued.front();
		m_Queued.pop_front();
		return AddState(state);
	}

	bool StateManager::AddState(FusionState *state)
	{
		// Make a shared ptr out of the given pointer and add it
		//SharedState state_spt(state);
		return AddState(SharedState(state) );
	}

	bool StateManager::AddState(SharedState state)
	{
		// Try to initialise the state
		if (!state->Initialise())
			return false;

		// Add the state if it managed to init.
		m_States.push_back(state);

		return true;
	}


	void StateManager::AddStateToQueue(FusionState *state)
	{
		//SharedState spt_state(state);
		m_Queued.push_back( SharedState(state) );
	}

	void StateManager::AddStateToQueue(SharedState state)
	{
		m_Queued.push_back(state);
	}


	void StateManager::RemoveState(FusionState *state)
	{
		bool blocking = false;

		StateList::iterator it;
		for (it = m_States.begin(); it != m_States.end(); ++it)
		{
			if (it->get() == state)
			{
				(*it)->CleanUp();
				m_States.erase(it);
			}
			else
				blocking |= (*it)->IsBlocking();
		}

		// If none of the states left over are blocking, we can run the next queued state
		if (!blocking)
			RunNextQueueState();
	}

	void StateManager::RemoveState(SharedState state)
	{
		bool blocking = false;

		StateList::iterator it;
		for (it = m_States.begin(); it != m_States.end(); ++it)
		{
			// Compare pointers
			if ((*it) == state)
			{
				(*it)->CleanUp();
				m_States.erase(it);
			}
			else
				// Check if any of the states NOT removed ARE blocking
				blocking |= (*it)->IsBlocking();
		}

		// If none of the states left over are blocking, we can run the next queued state
		if (!blocking)
			RunNextQueueState();
	}


	void StateManager::RemoveStateFromQueue(FusionState *state)
	{
		StateQueue::iterator it;
		for (it = m_Queued.begin(); it != m_Queued.end(); ++it)
		{
			if (it->get() == state)
			{
				m_Queued.erase(it);
				break;
			}
		}
	}

	void StateManager::RemoveStateFromQueue(SharedState state)
	{
		StateQueue::iterator it;
		for (it = m_Queued.begin(); it != m_Queued.end(); ++it)
		{
			if ((*it).get() == state.get())
			{
				m_Queued.erase(it);
				break;
			}
		}
	}


	void StateManager::Clear()
	{
		ClearQueue();

		ClearActive();
	}

	void StateManager::ClearActive()
	{
		if (!m_States.empty())
		{
			StateList::iterator it;
			for (it = m_States.begin(); it != m_States.end(); ++it)
			{
				(*it)->CleanUp();
			}
			m_States.clear();
		}
	}

	void StateManager::ClearQueue()
	{
		m_Queued.clear();
	}

	bool StateManager::Update(unsigned int split)
	{
		// If game should have quit, but for some reason update is being called again...
		if (!m_KeepGoing)
			return true; // ... breakout!


		if (m_States.empty())
			// All states have encountered errors or completed
			if (m_Queued.empty())
				// Nothing in queue - nothing to do
				return false;
			else
				// Run the next queued state
				RunNextQueueState();


		StateList::iterator it;
		for (it = m_States.begin(); it != m_States.end(); ++it)
		{
			// Check state messages
			StateMessage *m = (*it)->PopMessage();
			while (m != 0)
			{
				switch (m->GetType())
				{
				case StateMessage::ADDSTATE:
					AddState(m->GetData());
					break;
				case StateMessage::REMOVESTATE:
					RemoveState(m->GetData());
					break;
				case StateMessage::QUEUESTATE:
					AddStateToQueue(m->GetData());
					break;
				case StateMessage::UNQUEUESTATE:
					RemoveStateFromQueue(m->GetData());
					break;
				case StateMessage::RUNNEXTSTATE:
					RunNextQueueState();
					break;
				case StateMessage::QUIT:
					Clear();
					m_KeepGoing = false;
					break;
				}

				// Clean up
				delete m;
				// Get the next message
				m = (*it)->PopMessage();
			} // while (m != 0)

			// Try to update the state
			(*it)->Update(split);
		}

		return true;
	}

	void StateManager::Draw()
	{
		StateList::iterator it;
		for (it = m_States.begin(); it != m_States.end(); ++it)
		{
			(*it)->Draw();
		}
	}

	bool StateManager::KeepGoing() const
	{
		return m_KeepGoing;
	}

}
