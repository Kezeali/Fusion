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

/*!
 * \file FusionNetworkTypes.h
 * Previously I had used a binary flag setup for message ID's, but
 * that is stupidly complicated and limiting.
 */

#ifndef Header_FusionEngine_NetworkTypes
#define Header_FusionEngine_NetworkTypes

#if _MSC_VER > 1000
#pragma once
#endif

#include <RakNet/NetworkTypes.h>

namespace FusionEngine
{

	//! The length of a header with a timestamp
	/*!
	 * Consists of: <br>
	 * <code>[ID_TIMESTAMP]+[RakNetTime]+[MTID_x]+[CID_x]</code>
	 */
	const size_t g_HeaderLengthTimestamp =
		sizeof(unsigned char) + sizeof(RakNetTime) +
		sizeof(unsigned char) + sizeof(unsigned char);

	//! The length of a header without a timestamp
	/*!
	 * Consists of: <br>
	 * <code>[MTID_x]+[CID_x]</code>
	 */
	const size_t g_HeaderLength = sizeof(unsigned char) + sizeof(unsigned char);

	//! The amount of ordering channels defined
	const unsigned short g_ChannelNum = 4;

	//! Channel IDs
	/*!
	 * These are used to sort messages in the MessageQueue. In the actual
	 * packet, they come in the char after the Type ID (MTID_...)
	 */
	enum ChannelID
	{
		//@{
		//! Channels

		//! System messages
		CID_SYSTEM = 0,
		//! File messages
		CID_FILESYS,
		//! Gameplay messages
		CID_GAME,
		//! Chat messages
		CID_CHAT,
		//! No channel
		CID_NONE = 255
		//@}
	};


	//! FusionMessage types
	/*!
	 * [client|server] indicates what type of peer can receive
	 * each type - i.e. [client] means only clients can receive it.
	 */
	enum MessageType
	{
		//@{
		//! System channel message types

		//! [client|server] When new players join (clients can have more than one player)
		/*!
		 * Server-side Structure:<br>
		 * <ol>
		 * <li> [char]         MTID_ADDPLAYER
		 * <li> [ObjectID]     The prelim. ID the client gave this ship
		 * <li> [unsigned int] The team this player wants to join
		 * <li> [string]       The ship resource tag of this players chosen ship
		 * <li> [string]       The nickname for this player
		 * </ol>
		 *
		 * <p>NB: The Resource ID and name can also arrive via MTID_PLAYERCONFIG</p>
		 *
		 * <br>
		 * Client-side Structure:<br>
		 * <ol>
		 * <li> [char]         MTID_ADDPLAYER
		 * <li> [ObjectID]     The ID given to this ship by the server
		 * <li> [unsigned int] The team the player is on
		 * <li> [string]       The ship resource tag of this players chosen ship
		 * <li> [string]       The nickname for this player
		 * </ol>
		 */
		MTID_ADDPLAYER = ID_USER_PACKET_ENUM,
		//! [client] The server has allowed a local player to join
		/*!
		 * Structure:<br>
		 * <ol>
		 * <li> [char]     MTID_ADDALLOWED
		 * <li> [ObjectID] The prelim. ID the client gave this ship
		 * <li> [ObjectID] The 'official' ID gaven to this ship by the server
		 * <li> The rest is the same format as MTID_SHIPFRAME
		 * </ol>
		 */
		MTID_ADDALLOWED,
		//! [client|server]
		MTID_REMOVEPLAYER,
		//! [client|server] ShipPackageID and Name for the player
		/*!
		 * Like AddPlayer, but for ships that already exist
		 */
		MTID_PLAYERCONFIG,
		//! [client]
		MTID_CHANGEMAP,
		//! [server]
		/*!
		 * If the server receives this message during a game, it will send the client the
		 * current gamestate, in the form of reliable ADDPLAYER and SHIPFRAME packets, etc.
		 * The client sends this message at the begining of the ClientEnvironment process,
		 * so the first thing the ClientEnvrionment (ClientNetworkManager to be precise)
		 * receives should be those all important game state messages.
		 */
		MTID_NEEDGAMESTATE,
		//! [server|client]
		MTID_CHANGETEAM,
		//! [server|client]
		MTID_CHANGENAME,
		//@}

		//@{
		//! File transfer channel message types

		//! [client|server]
		/*!
		 * Client-side Structure:<br>
		 * <ol>
		 * <li> [char]     MTID_STARTSYNC
		 * <li> TODO: other stuff?
		 * </ol>
		 */
		MTID_STARTSYNC,
		//! [client]
		MTID_ENDSYNC,
		//! [client] If a package fails to verify, the client simply disconnects (thus no VERIFYFAILED is necessary)
		MTID_VERIFYPACKAGE,
		//@}

		//@{
		//! Gameplay channel message types

		//! [client|server] High priority
		MTID_CHANGEWEAPON,
		//! [client|server] High priority
		/*!
		 * Structure:<br>
		 * <ol>
		 * <li> [char]     ID_TIMESTAMP
		 * <li> [long]     Time
		 * <li> [char]     MTID_FIREWEAPON
		 * <li> [ObjectID] Ship to which this applies
		 * <li> [float]    x origin (of the ship? I think so...)
		 * <li> [float]    y origin
		 * <li> [float]    direction at origin
		 * </ol>
		 */
		MTID_FIREWEAPON,
		//! [client] High priority
		MTID_MAKEHOLE,
		//! [client|server] Low priority
		MTID_SHIPFRAME,
		//! [client|server] Low Priority
		MTID_PROJECTILEFRAME,
		//! [client] Low priority
		MTID_TERRAINBITMASK,
		//@}

		//@{
		//! Chat channel message types

		//! [client|server] On client, this is just used to choose the heading in the console
		MTID_CHALL,
		//! [client|server] On client, this is just used to choose the heading in the console
		MTID_CHTEAM,
		//! [client|server] On client, this is just used to choose the heading in the console
		MTID_CHONE
		//@}
	};

}

#endif
