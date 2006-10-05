/*
 Copyright (c) 2006 Elliot Hayward
 
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

#ifndef Header_FusionEngine_FusionNetworkMessageQueue
#define Header_FusionEngine_FusionNetworkMessageQueue

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionMessage.h"

namespace FusionEngine
{

	//! Thread safe message storage
	/*!
	 * Possably [depreciated]... ATM seems useable, but all this stuff could be done in
	 * NetworkClient (at the expense of its simplicity.) Also, the thread safety is now
	 * redundant - FusionNetworkClient / FusionNetworkServer run in the main thread.
	 */
	class FusionNetworkMessageQueue
	{
	public:
		//! Constructor
		FusionNetworkMessageQueue();
		//! Destructor
		~FusionNetworkMessageQueue();

	public:
		//! A group of messages
		typedef std::deque<FusionMessage*> MessageQueue;
		//! A group of channels (each containing messages)
		typedef std::vector<MessageQueue> ChannelList;

		//! A group of net events.
		typedef std::vector<FusionMessage*> EventList;

		//! Use this to pre-allocate the required channels
		/*!
		 * This should be called before any retreival functions, to avoid
		 * accessing null space.
		 */
		void Resize(unsigned short channels);

		//! Used internally by FusionNetwork[Client|Server]. Threadsafe.
		MessageQueue *_getInMessages(int channel);
		//! Used internally... by nothing really :P Threadsafe.
		MessageQueue *_getOutMessages(int channel);

		//! Used internally by FusionNetworkReceiver. Threadsafe.
		void _addInMessage(FusionMessage *message, int channel);
		//! Used internally by FusionNetworkSender. Threadsafe.
		void _addOutMessage(FusionMessage *message, int channel);

		//! Used internally. Gets the next message. Null if none
		FusionMessage *_getInMessage(int channel);
		//! Used internally. Gets the next message. Null if none
		FusionMessage *_getOutMessage(int channel);

		//! Used internally. Allows the CE to act on network events
		/*!
		 * \param messageId The packet ID which CE should act on.
		 */
		void _addEvent(FusionMessage *message);
		//! Gets the next event. See #GetEvents()
		FusionMessage *GetEvent();
		//! Allows the ClientEnvironment, etc. to access the NetEvent queue
		const EventList GetEvents();

		//! Deletes all messages in the event list, and clears the list.
		void ClearEvents();
		//! Deletes messages in the in queue
		void ClearInMessages();
		//! Deletes messages in the out queue
		void ClearOutMessages();

	protected:
		//! Teh in queuez
		ChannelList m_InChannels;

		//! Teh out queuez
		ChannelList m_OutChannels;

		//! List of network events
		EventList m_EventList;

		/*!
		 * \brief
		 * [depreciated] Used for thread-safety.
		 *
		 * Ensures thread-safety by preventing concurrent data access via _getMessages().
		 */
		CL_Mutex *m_Mutex;
	};

}

#endif