#ifndef Header_FusionEngine_StateManager
#define Header_FusionEngine_StateManager

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{

	/*!
	 * \brief
	 * This class manages what can happen and when, while the game is running.
	 *
	 * To be more specific, this class manages the switching of states - such as moving from
	 * LOADING to GAMEPLAY - and also running multiple concurrent states - such as GAMEPLAY
	 * + MENU. Some states also have their own state managers, for instance GAMEPLAY has a
	 * state manager for switching between NORMAL, PAUSED, and RESULT states.
	 */
	class StateManager
	{
	public:
		typedef CL_SharedPtr<UCThing> SharedState;
		typedef vector<SharedState> SharedStateList;

	private:
		SharedStateList m_States;

	public:
		StateManager() {}

		void SetExclusive(UCThing *type)
		{
			if (!m_States.empty())
			{
				SharedStateList::iterator it;
				for (it = m_States.begin(); it != m_States.end(); ++it)
				{
					// no cleanup function, just print some trash
					cout << "Cleaning up: " << (*it)->x << endl;
				}
				m_States.clear();
			}

			SharedState state(type);
			m_States.push_back(state);
		}

		void AddState(UCThing *type)
		{
			SharedState state(type);
			m_States.push_back(state);
		}

		void RemoveState(UCThing *state)
		{
			SharedStateList::iterator it;
			for (it = m_States.begin(); it != m_States.end();)
			{
				if (it->get() == state)
				{
					// no cleanup function, just print some trash
					cout << "Cleaning up: " << (*it)->x << endl;
					it = m_States.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	};

}

#endif