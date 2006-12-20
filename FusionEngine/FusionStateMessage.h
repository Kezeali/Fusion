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

#ifndef Header_FusionEngine_StateMessage
#define Header_FusionEngine_StateMessage

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Allows states to request actions from the StateManager.
	 */
	class StateMessage
	{
	public:
		//! Types of messages
		/*!
		 * Hopefully the names are self explanatory. :)
		 *
		 * \remarks
		 * Use of REMOVESTATE is preffered over QUIT if you want to quit the game,
		 * as this ensures other states can finish their processes before the game
		 * ends. Not that I adhear to that rule :P
		 */
		enum StateMessageType { ADDSTATE, REMOVESTATE, QUEUESTATE, UNQUEUESTATE, QUIT };

	public:
		//! Basic constructor
		StateMessage();
		//! Constructor +type +data
		StateMessage(StateMessageType type, FusionState *data)
			: m_Type(type)
		{
			m_Data = SharedState(data);
		}

	public:
		//! Retreives the state type
		StateMessageType GetType() const
		{
			return m_Type;
		}
		//! Retreives the custom data
		SharedState GetData() const
		{
			return m_Data;
		}

	protected:
		//! Stores the message type
		StateMessageType m_Type;
		//! Stores custom message data
		/*!
		 * \remarks MCS - Since all messages pass states ATM, this is typed as a
		 * SharedState, rather than a void*.
		 */
		SharedState m_Data;

	};

}

#endif