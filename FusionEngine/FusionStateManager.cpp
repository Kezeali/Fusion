
#include "FusionStateManager.h"

using namespace FusionEngine;

StateManager::StateManager()
{
}

void StateManager::SetExclusive(FusionState *state)
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

	SharedState state(type);
	m_States.push_back(state);

	// Finally, initialise the state
	state->Initialise();
}

void StateManager::AddState(FusionState *state)
{
	SharedState state(type);
	m_States.push_back(state);

	// Finally, initialise the state
	state->Initialise();
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

		if ((*it)->Update(split) == false)
		{
			// Record the error
			m_LastError = (*it)->GetLastError();
			// Remove the state if it falied to update
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
