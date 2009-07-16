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

#ifndef Header_FusionEngine_MessageIdentifiers
#define Header_FusionEngine_MessageIdentifiers

#if _MSC_VER > 1000
#pragma once
#endif

#include <RakNet/MessageIdentifiers.h>

namespace FusionEngine
{
	//static const int CHANNEL_SIZE = 20;

	//! Channel IDs
	/*!
	 * These are used to sort messages (e.g. per subsystem). In the actual
	 * packet, they come in the char after the Type ID (MTID_...)
	 */
	enum ChannelType
	{
		//@{
		//! Channels

		//! System messages
		CID_SYSTEM = 1,
		//! File channel
		CID_FILESYNC,
		//! NetworkedEntityManager channel
		CID_ENTITYMANAGER,
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
		//! [client|server] When new players join (clients can have more than one player)
		/*!
		 * Server-side Structure:<br>
		 * <ol>
		 * <li> [ObjectID]     The prelim. ID the client gave this ship
		 * <li> [bool]         True if the client wants ownership of this entity
		 * <li> [string]       The class name of this players chosen entity
		 * <li> [string]       The nickname for this player
		 * </ol>
		 *
		 * <br>
		 * Client-side Structure:<br>
		 * <ol>
		 * <li> [ObjectID]     The ID given to this ship by the server
		 * <li> [bool]         True if the remote client has ownership of this entity
		 * <li> [ObjectID]     If the previous item is 'true': the authorityID of the client that owns the entity
		 * <li> [string]       The class name of this players chosen entity
		 * <li> [string]       The nickname for this player
		 * </ol>
		 */
		MTID_ADDENTITY = ID_USER_PACKET_ENUM,
		//! [client] The owned entity request was allowed by the server
		/*!
		 * Structure:<br>
		 * <ol>
		 * <li> [ObjectID] The prelim. ID the client gave this ship
		 * <li> [ObjectID] The 'official' ID gaven to this ship by the server
		 * </ol>
		 */
		MTID_ADDALLOWED,
		//! [client|server]
		MTID_REMOVEENTITY,
		//! [client|server] Updates the gamemode class entity name
		MTID_CHANGEMODE,
		//! [server|client]
		MTID_CHANGENAME,

		//! [client|server]
		/*!
		 * Client-side Structure:<br>
		 * <ol>
		 * <li> TODO: stuff?
		 * </ol>
		 */
		MTID_STARTSYNC,
		//! [client]
		MTID_ENDSYNC,
		//! [client] If a package fails to verify, the client simply disconnects (thus no VERIFYFAILED is necessary)
		MTID_VERIFYPACKAGE,

		//! Gives the client a hint about what it's starting tick should be
		MTID_JOINSETUP,
		//! [client|server] Low priority
		MTID_ENTITYMOVE,
		//! [client] High Priority
		MTID_CORRECTION,
		//! [server] High priority
		MTID_IMPORTANTMOVE,

		MTID_REQUESTSTEPCONTROL,

		//! [client|server] On client, this is just used to choose the heading in the chatlog
		MTID_CHALL,
		//! [client|server] On client, this is just used to choose the heading in the chatlog
		MTID_CHTEAM,
		//! [client|server] On client, this is just used to choose the heading in the chatlog
		MTID_CHONE,

		//! Max type ID
		MTID_MAX
	};

}

#endif
