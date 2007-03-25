/*
  Copyright (c) 2006-2007 Fusion Project Team

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


	File Author(s):

		Elliot Hayward

*/
#ifndef Header_FusionEngine_FusionNetworkPacketQueue
#define Header_FusionEngine_FusionNetworkPacketQueue

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <RakNet/NetworkTypes.h>

namespace FusionEngine
{

	//! Network message (packet) storage and access
	/*!
	 * <b> This class is Thread-safe </b>
	 */
	class PacketQueue
	{
	public:
		//! Constructor
		PacketQueue();
		//! Destructor
		~PacketQueue();

	public:
		//! A group of messages
		typedef std::deque<Packet*> MessageQueue;
		//! A group of channels (each containing messages)
		typedef std::vector<MessageQueue> ChannelList;

		//! Use this to pre-allocate the required channels
		/*!
		 * <p>
		 * Creates the given number of channel queues.
		 * </p>
		 *
		 * This should be called before any retreival functions, to avoid
		 * accessing null space.
		 */
		void Resize(unsigned int channels);

		//! Used internally by FusionNetworkGeneric#Receive(). Threadsafe.
		void PushMessage(Packet *message, char channel);

		//! Used internally. Gets the next message. Null if none
		Packet *PopMessage(char channel);

		//! Deletes messages in the queue
		void ClearMessages();

	protected:
		//! Channels for received packets
		ChannelList m_Channels;

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
