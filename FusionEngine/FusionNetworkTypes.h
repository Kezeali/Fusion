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

#ifndef Header_FusionEngine_FusionNetworkTypes
#define Header_FusionEngine_FusionNetworkTypes

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{
	const unsigned short g_ChannelNum = 4;
	//! Channels
	enum
	{
		//@{
		//! Channels

		//! (0000 0001)
		CID_SYSTEM = 1,
		//! (0000 0010)
		CID_FILETRANSFER = 2,
		//! (0000 0100)
		CID_GAME = 4,
		//! (0000 1000)
		CID_CHAT = 8,
		//@}

		//@{
		//! System channel message types

		//! (0001 0000)
		MTID_NEWPLAYER = 16,
		//@}

		//@{
		//! File transfer channel message types

		//! (0001 0000)
		MTID_STARTTRANSFER = 16,
		//@}

		//@{
		//! Gameplay channel message types

		//! (0001 0000)
		MTID_SHIPFRAME = 16,
		//! (0010 0000)
		MTID_PROJECTILEFRAME = 32,
		//! (0100 0000) Low priority
		MTID_TERRAINBITMASK = 64
		//@}

		//@{
		//! Chat channel message types

		//! (0001 0000)
		MTID_CHALL = 16,
		//! (0010 0000)
		MTID_CHTEAM = 32,
		//! (0100 0000)
		MTID_CHONE = 64
		//@}
	};

}

#endif