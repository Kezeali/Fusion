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
	class SystemMessage
	{
	public:
		//! Types of messages
		/*!
		 * Hopefully the names are self explanatory. :)
		 */
		enum MessageType {
			//! Add a new system
			ADDSYSTEM,
			//! Remove a system
			REMOVESYSTEM,

			//! No message (safe default)
			NONE,

			//! Quit the state manager
			QUIT,

			//! Pauses the target states
			PAUSE,
			//! Resumes the target states
			RESUME,
			//! Steps the target states if they are paused
			STEP,
			//! Steps paused states
			STEPALL,
			//! Pauses states other than the origin and target states
			PAUSEOTHERS,
			//! Resumes states other than the origin and target states
			RESUMEOTHERS,
			//! Hides the target states
			HIDE,
			//! Shows the target states
			SHOW,
			//! Hides states other than the origin and target states
			HIDEOTHERS,
			//! Shows states other than the origin and target states
			SHOWOTHERS
		};

	public:
		//! Basic constructor
		SystemMessage()
			: m_Type(NONE)
		{}
		//! Constructor +type
		SystemMessage(MessageType type)
			: m_Type(type),
			m_IncludeSender(true)
		{}
		//! Constructor +type +target
		SystemMessage(MessageType type, const std::string &target_system, bool include_sender = false)
			: m_Type(type),
			m_IncludeSender(include_sender)
		{
			m_Targets.push_back(target_system);
		}
		//! CTOR +type +targets
		SystemMessage(MessageType type, const StringVector &systems, bool include_sender = false)
			: m_Type(type),
			m_Targets(systems),
			m_IncludeSender(include_sender)
		{
		}

		//! Constructor +system
		SystemMessage(const SystemPtr &system_to_add)
			: m_Type(SystemMessage::ADDSYSTEM),
			m_System(system_to_add),
			m_IncludeSender(false)
		{}

		virtual ~SystemMessage()
		{
		}

	public:
		//! Retreives the state type
		MessageType GetType() const
		{
			return m_Type;
		}

		const StringVector &GetTargets() const
		{
			return m_Targets;
		}

		bool IncludeSender() const
		{
			return m_IncludeSender;
		}

		const SystemPtr &GetSystem() const
		{
			return m_System;
		}

	protected:
		//! Stores the message type
		MessageType m_Type;
		//! Stores custom message data
		StringVector m_Targets;
		bool m_IncludeSender;

		// A System object to be added or removed (note that systems can also be removed by name)
		SystemPtr m_System;

	};

}

#endif