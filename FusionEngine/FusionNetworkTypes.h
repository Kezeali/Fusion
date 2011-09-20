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

/*!
 * \file FusionNetworkTypes.h
 * Previously I had used a binary flag setup for message ID's, but
 * that is stupidly complicated and limiting.
 */

#ifndef H_FusionEngine_MessageIdentifiers
#define H_FusionEngine_MessageIdentifiers

#if _MSC_VER > 1000
#pragma once
#endif

#include <MessageIdentifiers.h>

namespace FusionEngine
{
	//static const int CHANNEL_SIZE = 20;

	//! Channel IDs
	/*!
	 * These are used to sort messages (e.g. per subsystem). In the actual
	 * packet, they come in the char after the Type ID (MTID_...)
	 */
	enum ChannelType : char
	{
		//@{
		//! Channels

		//! System messages
		CID_SYSTEM = 1,
		//! File channel
		CID_FILESYNC,
		//! EntityManager channel
		CID_ENTITYMANAGER,
		//! EntitySynchroniser channel
		CID_ENTITYSYNC,
		//! Entity updates that contain input
		CID_INPUTUPDATE,
		//! No channel
		CID_MAXCID
		//@}
	};


	//! FusionMessage types
	/*!
	 * [client|server] indicates what type of peer can receive
	 * each type - i.e. [client] means only clients can receive it.
	 */
	enum MessageType
	{
		MTID_SET_PEER_ID = ID_USER_PACKET_ENUM,

		MTID_INSTANCEENTITY,
		MTID_ADDENTITY,
		MTID_ADDALLOWED,
		MTID_REMOVEENTITY,

		MTID_ADDPLAYER,
		MTID_REMOVEPLAYER,
		MTID_CHANGENAME,

		MTID_LOADMAP,

		MTID_STARTSYNC,
		MTID_ENDSYNC,
		MTID_VERIFY,

		MTID_JOINSETUP,
		MTID_ENTITYMOVE,
		MTID_CORRECTION,
		MTID_IMPORTANTMOVE,

		MTID_REQUESTSTEPCONTROL,

		MTID_CHALL,
		MTID_CHTEAM,
		MTID_CHONE,

		//! Max type ID
		MTID_MAX
	};

}

#endif
