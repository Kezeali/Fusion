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
#include "FusionError.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * The abstract base for FusionStates.
	 *
	 * Each state controls a specific task while the game runs. For example there is a
	 * LOADING state, which controls connecting to the server, downloading files, and
	 * finally loading the the requried data.
	 *
	 * \remarks
	 * Each state should require no knowlage of its StateManager, so they are never
	 * provided with access to it. This is because allowing states to modify the state
	 * manager could cause serious problems when running multiple states concurrently.
	 */
	class FusionState
	{
	public:
		//! Messages
		typedef std::deque<StateMessage*> MessageList;

	public:
		//! Destructor
		~FusionState();

	public:
		//! Pure virtual method
		virtual bool Initialise() = 0;
		//! Pure virtual method
		virtual bool Update(unsigned int split) = 0;
		//! Pure virtual method
		virtual void Draw() = 0;
		//! Pure virtual method
		virtual void CleanUp() = 0;

		//! Pulls the message from the front of the queue
		StateMessage *PopMessage();
		//! Adds a message to the back of the queue.
		/*!
		 * Messages should only be added internally, so this shouldn't be used (but it's here for exception circumstances.)
		 */
		void _pushMessage(StateMessage *m);

		//! Returns the most recent abort message encountered
		Error *GetLastError() const;

	protected:
		//! Messages to the state manager
		MessageList m_Messages;
		//! Last error
		Error *m_LastError;

	};

}

#endif