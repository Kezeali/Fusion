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

	//! Worker thread for receiving messages
	/*!
	 * Possably [depreciated] by RakNet... ATM seems useable.
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
		typedef std::queue<FusionMessage*> MessageQueue;
		//! A group of channels (containing messages)
		typedef std::vector<MessageQueue> ChannelList;

		//! Used internally by FusionNetwork[Client|Server]. Threadsafe.
		const MessageQueue &_getInMessages(int channel);
		//! Used internally by FusionNetworkReceiver. Threadsafe.
		void _addInMessage(FusionMessage *message, int channel);

		//! Used internally... by nothing really :P Threadsafe.
		const MessageQueue &_getOutMessages(int channel);
		//! Used internally by FusionNetworkSender. Threadsafe.
		void _addOutMessage(FusionMessage *message, int channel);

	protected:
		//! Teh in queuez
		ChannelList m_InChannels;

		//! Teh out queuez
		ChannelList m_OutChannels;

		/*!
		 * \brief
		 * Used for thread-safety.
		 *
		 * Ensures thread-safety by preventing concurrent data access via _getMessages().
		 */
		CL_Mutex *m_Mutex;
	};

}

#endif