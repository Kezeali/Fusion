/*
  Copyright (c) 2006 Fusion Project Team

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
*/

#ifndef Header_FusionEngine_StateManager
#define Header_FusionEngine_StateManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionState.h"

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
	 * <br>
	 * When adding states, the SharedPtr. forms of the methods are preffered, as
	 * that way I can trust you to not delete them! 
	 *
	 * \remarks
	 * I use shared pointers here in the hopes that it will prevent states from 
	 * causing memory leaks if they are 'lost' (added incorrectly for example.) and so 
	 * that they can be removed from the list without being deleted immeadiately.
	 *
	 */
	class StateManager
	{
	public:
		//! List of states
		typedef std::vector<SharedState> StateList;
		//! Queue of states
		typedef std::deque<SharedState> StateQueue;

	public:
		//! Basic constructor
		StateManager();
		//! Destructor
		~StateManager();

	public:
		//! Removes all other states and adds the state specified.
		/*!
		 * \retval True if the state initialised successfully
		 * \retval False otherwise
		 */
		bool SetExclusive(FusionState *state);

		//! Adds the next state in the queue to the execution list
		/*!
		 * \retval True if the state initialised successfully
		 * \retval False otherwise
		 */
		bool RunNextQueueState();

		//! Adds the state specified
		/*!
		 * \retval True if the state initialised successfully
		 * \retval False otherwise
		 */
		bool AddState(FusionState *state);

		//! Adds the specified shared ptr. to a state
		/*!
		 * \retval True if the state initialised successfully
		 * \retval False otherwise
		 */
		bool AddState(SharedState state);

		//! Adds the state specified to the queue
		/*!
		 * When all currently running states are complete, this state will run.
		 */
		void AddStateToQueue(FusionState *state);

		//! Adds the specified shared ptr. to a state, to the queue
		/*!
		 * When all currently running states are complete, this state will run.
		 */
		void AddStateToQueue(SharedState state);

		//! Removes the state specified
		void RemoveState(FusionState *state);

		//! Removes the state specified
		void RemoveState(SharedState state);

		//! Removes the state specified from the queue
		void RemoveStateFromQueue(FusionState *state);

		//! Removes the state specified from the queue
		void RemoveStateFromQueue(SharedState state);

		//! Removes all states (including queued)
		void Clear();

		//! Removes active states
		void ClearActive();

		//! Removes queued states
		void ClearQueue();

		//! Updates all states
		bool Update(unsigned int split);
		//! Draws all states
		void Draw();

		//! Returns false after a state requests quit
		bool KeepGoing() const;

		//! Retreives the last error reported by a state
		Error *GetLastError() const;

	protected:
		//! List of all running states
		StateList m_States;
		//! List of all not-running states
		StateQueue m_Queued;

		//! Set to false if when FusionState#KeepGoing() returns false.
		bool m_KeepGoing;
		//! Last error
		Error *m_LastError;

	};

}

#endif