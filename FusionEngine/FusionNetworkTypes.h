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

#ifndef Header_FusionEngine_FusionNetworkTypes
#define Header_FusionEngine_FusionNetworkTypes

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{
	//! The amount of ordering channels defined
	const unsigned short g_ChannelNum = 4;
	//! Channels
	enum
	{
		//@{
		//! Channels

		//! ignore these -> (0000 0001) it only applied when I was using binary flags
		CID_SYSTEM = 1,
		//! (0000 0010)
		CID_FILESYS = 2,
		//! (0000 0100)
		CID_GAME = 3,
		//! (0000 1000)
		CID_CHAT = 4,
		//@}

		//@{
		//! System channel message types

		//! (0001 0000)
		MTID_NEWPLAYER = 1,
		//@}

		//@{
		//! File transfer channel message types

		//! (0001 0000)
		MTID_STARTTRANSFER = 1,
		//@}

		//@{
		//! Gameplay channel message types

		//! (0001 0000)
		MTID_SHIPFRAME = 1,
		//! (0010 0000)
		MTID_PROJECTILEFRAME = 2,
		//! (0100 0000) Low priority
		MTID_TERRAINBITMASK = 3,
		//@}

		//@{
		//! Chat channel message types

		//! (0001 0000)
		MTID_CHALL = 1,
		//! (0010 0000)
		MTID_CHTEAM = 2,
		//! (0100 0000)
		MTID_CHONE = 3
		//@}
	};

}

#endif
