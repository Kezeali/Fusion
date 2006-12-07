
#include "FusionStateManager.h"

using namespace FusionEngine;

StateManager::StateManager()
{
}

StateManager::~StateManager()
{
	Clear();

	if (m_LastError)
		delete m_LastError;
}

bool StateManager::SetExclusive(FusionState *state)
{
	// Try to initialise the new state
	if (state->Initialise() == false)
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
	SharedState state_spt(state);
	m_States.push_back(state_spt);

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
	// Try to initialise the state
	if (state->Initialise() == false)
		return false;

	// Make a shared ptr out of the given pointer and store it
	SharedState state_spt(state);
	m_States.push_back(state_spt);

	return true;
}

bool StateManager::AddState(SharedState state)
{
	// Try to initialise the state
	if (state->Initialise() == false)
		return false;

	// Add the state if it managed to init.
	m_States.push_back(state_spt);

	return true;
}

void StateManager::AddStateToQueue(FusionEngine::FusionState *state)
{
	SharedState spt_state(state);
	m_Queued.push_back(spt_state);
}

void StateManager::AddStateToQueue(FusionEngine::SharedState state)
{
	m_Queued.push_back(state);
}

void StateManager::RemoveState(FusionState *state)
{
	StateList::iterator it;
	for (it = m_States.begin(); it != m_States.end(); ++it)
	{
		if (it->get() == state)
		{
			(*it)->CleanUp();
			m_States.erase(it);

			break;
		}
	}

	// Quit cleanly if the state removed was the last
	if (m_States.empty())
		m_KeepGoing = false;
}

void StateManager::RemoveStateFromQueue(FusionEngine::FusionState *state)
{
	StateQueue::iterator it;
	for (it = m_States.begin(); it != m_States.end(); ++it)
	{
		if (it->get() == state)
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
			case StateMessage::QUIT:
				Clear();
				m_KeepGoing = false;
				break;
			}
		}

		// Try to update the state
		if ((*it)->Update(split) == false)
		{
			// If the state fails to update:
			//  Record the error
			m_LastError = (*it)->GetLastError();
			//  Tell the state to clean up
			(*it)->CleanUp();
			//  Remove the state and jump to the next iteration
			it = m_States.erase(it);
		}
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
