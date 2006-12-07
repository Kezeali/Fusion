/*
 Copyright (c) 2006 FusionTeam

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

namespace FusionEngine
{
	//! The amount of ordering channels defined
	const unsigned short g_ChannelNum = 4;

	//! Channel IDs
	/*!
	 * These are used to sort messages in the MessageQueue. In the actual
	 * packet, they come in the char after the Type ID (MTID_...)
	 */
	enum ChannelIDs
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
		CID_CHAT
		//@}
	};


	//! Fusion message types
	/*!
	 * [client|server] indicates what type of peer can receive
	 * this type - [client] means client can receive it.
	 */
	enum NetworkTypes
	{
		//@{
		//! System channel message types

		//! [client|server] On new player
		MTID_NEWPLAYER = ID_USER_PACKET_ENUM,
		//@}

		//@{
		//! File transfer channel message types

		//! [client|server]
		MTID_STARTSYNC,
		//! [client]
		MTID_VERIFYPACKAGE,
		//@}

		//@{
		//! Gameplay channel message types

		//! [client|server]
		MTID_SHIPFRAME,
		//! [client|server]
		MTID_PROJECTILEFRAME,
		//! [client]
		MTID_MAKEHOLE,
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
