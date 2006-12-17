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

#ifndef Header_FusionEngine_NetUtils
#define Header_FusionEngine_NetUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "../RakNet/NetworkTypes.h"
#include "../RakNet/Bitstream.h"

namespace FusionEngine
{
	//! Network Utils
	class NetUtils
	{
	public:
		//! Checks if this packet has a non-RakNet type.
		static bool IsFusionPacket(Packet *p);
		//! Checks if this packet data has a non-RakNet type.
		static bool IsFusionPacket(unsigned char *data);


		//! Extracts the id from the packet.
		static unsigned char GetPacketIdentifier(Packet *p);
		//! Extracts the id from the packet from data.
		static unsigned char GetPacketIdentifier(unsigned char *data, unsigned int length);

		//! Extracts the channel id from the packet.
		/*!
		 * This doesn't check whether the given packet is actually a Fusion packet,
		 * so it may just return the first character of the message. Use
		 * NetUtils#IsFusionPacket first to check the packet, if necessary.
		 */
		static unsigned char GetPacketChannel(Packet *p);
		//! Extracts the channel id from packet data.
		/*!
		 * This doesn't check whether the given packet is actually a Fusion packet,
		 * so it may just return the first character of the message. Use
		 * NetUtils#IsFusionPacket first to check the packet, if necessary.
		 */
		static unsigned char GetPacketChannel(unsigned char *data, unsigned int length);

		//! Extracts the timestamp from the packet.
		//! \retval 0 if no timestamp is present
		static RakNetTime GetPacketTime(Packet *p);
		//! Extracts the timestamp from the packet from data.
		//! \retval 0 if no timestamp is present
		static RakNetTime GetPacketTime(unsigned char *data, unsigned int length);


		//! Get the length of the packet header (ID and Timestamp).
		static int GetHeaderLength(Packet *p);
		//! Get the length of the packet header (ID and Timestamp) from data.
		static int GetHeaderLength(unsigned char *data);
	};

}

#endif
