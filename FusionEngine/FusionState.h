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

#ifndef Header_FusionEngine_FusionState
#define Header_FusionEngine_FusionState

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Fusion
#include "FusionStateMessage.h"
#include "FusionException.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * The abstract base for Systems.
	 * 
	 * Each system controls a specific task while the game runs. For example there is a
	 * LOADING state, which controls connecting to the server, downloading files, and
	 * finally loading the the requried data.
	 *
	 * \remarks
	 * <p>
	 * Each state should require no knowlage of its StateManager, so they are never
	 * provided with access to it. This is because allowing states to modify the state
	 * manager could cause serious problems when running multiple states concurrently.
	 * </p>
	 */
	class System
	{
	public:
		//! Messages
		typedef std::deque<SystemMessage*> MessageList;

		enum StateFlags
		{
			PAUSE = 0x1,
			STEP  = 0x2,
			HIDE  = 0x4
		};

	public:
		//! CTOR
		System()
			: m_Blocking(false),
			m_StateFlags(0)
		{}
		//! CTOR
		System(bool blocking)
			: m_Blocking(blocking),
			m_StateFlags(0)
		{}
		//! Destructor
		virtual ~System()
		{}

	public:
		//! Should initialise the system
		virtual bool Initialise() = 0;
		//! Should free all resources used by the system
		virtual void CleanUp() = 0;

		//! Should do whatever the system does while it isn't paused
		virtual void Update(unsigned int split) = 0;
		//! Should draw stuff
		virtual void Draw() = 0;

		//! Should return the name of the system
		virtual const std::string &GetName() const = 0;

		//! Pulls the message from the front of the queue
		SystemMessage *PopMessage();
		//! Adds a message to the back of the queue.
		void PushMessage(SystemMessage *m);

		//! Returns true if this system currently depends on the system with the given name
		bool IsDependency(const std::string &system_name);
		//! Adds a name for a system that depends on this on
		void AddDependency(const std::string &system_name);
		//! Removes a dependency
		void RemoveDependency(const std::string &system_name);

		void SetFlags(unsigned char flags);

		void AddFlag(StateFlags flag);
		void RemoveFlag(StateFlags flag);

		bool CheckFlag(StateFlags flag);

		//! Sets the blocking mode for this state
		void SetBlocking(bool blocking);

		//! Checks the blocking mode for this state
		bool IsBlocking() const;

	private:
		//! Messages to the state manager
		MessageList m_Messages;

		unsigned char m_StateFlags;

		typedef std::tr1::unordered_set<std::string> DependencySet;
		DependencySet m_Dependencies;

		//! If this is true, queued states won't become active (automaticaly, that is) until this state is removed
		bool m_Blocking;

	};

}

#endif