/*
  Copyright (c) 2006 FusionTeam

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

#include "FusionEngineCommon.h"

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
	 */
	class StateManager
	{
	public:
		//! Self managing state pointer
		typedef CL_SharedPtr<FusionState> SharedState;
		//! List of states
		typedef std::vector<SharedState> StateList;

	public:
		//! Basic constructor
		StateManager();

	public:
		//! Removes all other states and adds the state specified.
		/*!
		 * \retval True if the state initialised successfully
		 * \retval False otherwise
		 */
		bool SetExclusive(FusionState *state);

		//! Adds the state specified
		/*!
		 * \retval True if the state initialised successfully
		 * \retval False otherwise
		 */
		bool AddState(FusionState *state);

		//! Removes the state specified
		void RemoveState(FusionState *state);

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
		//! Set to false if when FusionState#KeepGoing() returns false.
		bool m_KeepGoing;
		//! Last error
		Error *m_LastError;

	};

}

#endif