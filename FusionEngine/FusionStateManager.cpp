
#include "FusionStateManager.h"

using namespace FusionEngine;

StateManager::StateManager()
{
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

	// Add the new state if it managed to init
	SharedState state_spt(state);
	m_States.push_back(state_spt);

	return true;
}

bool StateManager::AddState(FusionState *state)
{
	// Try to initialise the state
	if (state->Initialise() == false)
		return false;

	// Add the state if it managed to init
	SharedState state_spt(state);
	m_States.push_back(state_spt);

	return true;
}

void StateManager::RemoveState(FusionState *state)
{
	StateList::iterator it;
	for (it = m_States.begin(); it != m_States.end(); ++it)
	{
		if (it->get() == state)
		{
			(*it)->CleanUp();
			it = m_States.erase(it);

			break;
		}
	}
}

void StateManager::Clear()
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

bool StateManager::Update(unsigned int split)
{
	// All states have encountered errors - nothing to do
	if (m_States.empty())
		return false;

	StateList::iterator it;
	for (it = m_States.begin(); it != m_States.end(); ++it)
	{
		// If this state thinks the game should end, tell the everything else.
		if (!(*it)->KeepGoing())
			m_KeepGoing = false;

		// Try to update the state
		if ((*it)->Update(split) == false)
		{
			// If the state fails to update:
			//  Record the error
			m_LastError = (*it)->GetLastError();
			//  Tell the state to clean up
			(*it)->CleanUp();
			//  Remove the state
			it = m_States.erase(it);
		}
	}
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
