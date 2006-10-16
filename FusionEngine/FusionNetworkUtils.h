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

#ifndef Header_FusionEngine_NetUtils
#define Header_FusionEngine_NetUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "../RakNet/NetworkTypes.h"
#include "../RakNet/Bitstream.h"

namespace FusionEngine
{
	//! Network Utils
	class NetUtils
	{
	public:
		//! Extract the id from the packet.
		static unsigned char GetPacketIdentifier(Packet *p);

		//! Extract the id from the packet from data.
		static unsigned char GetPacketIdentifier(unsigned char *data, unsigned int length);


		//! Extract the timestamp from the packet.
		//! \returns 0 if no timestamp is present
		static RakNetTime GetPacketTime(Packet *p);

		//! Extract the timestamp from the packet from data.
		//! \returns 0 if no timestamp is present
		static RakNetTime GetPacketTime(unsigned char *data, unsigned int length);


		//! Get the length of the packet header (ID and Timestamp).
		static int GetHeaderLength(Packet *p);

		//! Get the length of the packet header (ID and Timestamp) from data.
		static int GetHeaderLength(unsigned char *data);
	};

}

#endif