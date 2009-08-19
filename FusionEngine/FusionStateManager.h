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

	//! System shared_ptr
	typedef std::tr1::shared_ptr<System> SystemPtr;

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
	 */
	class SystemsManager
	{
	public:
		typedef std::vector<SystemPtr> SystemArray;

	public:
		//! Basic constructor
		SystemsManager();
		//! Destructor
		~SystemsManager();

	public:
		//! Removes all other states and adds the state specified.
		/*!
		 * \retval True if the state initialised successfully
		 * \retval False otherwise
		 */
		bool SetExclusive(const SystemPtr &state);

		bool AddSystem(const SystemPtr &system);

		void RemoveSystem(const SystemPtr &system);

		//! Removes all states (including queued)
		void Clear();

		//! Updates all states
		bool Update(float split);
		//! Draws all states
		void Draw();

		//! Returns false after a state requests quit
		bool KeepGoing() const;

	protected:
		void setFlagsAll(int flags);

		void addFlagAll(System::StateFlags flag);
		void removeFlagAll(System::StateFlags flag);

		void addFlagFor(const StringVector &target, System::StateFlags flag);
		void removeFlagFor(const StringVector &target, System::StateFlags flag);

		//! Adds the given flag to systems other than those listed
		/*!
		* \param[in] excluded_system
		* The system that sent the message which initiated this call.
		*
		* \param[in] excluded_targets
		* Names of systems which shouldn't be flagged (from message->GetTargets()).
		*/
		void addFlagForOthers(const SystemPtr &excluded_system, const StringVector &excluded_targets, System::StateFlags flag);
		void removeFlagForOthers(const SystemPtr &excluded_system, const StringVector &excluded_targets, System::StateFlags flag);

	protected:
		SystemArray m_Systems;

		//! Set to false if when FusionState#KeepGoing() returns false.
		bool m_KeepGoing;

	};

}

#endif