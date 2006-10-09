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

#ifndef Header_FusionEngine_FusionMessageBuilder
#define Header_FusionEngine_FusionMessageBuilder

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionMessage.h"
#include "FusionShipState.h"
#include "FusionProjectileState.h"

#include "../RakNet/NetworkTypes.h"

namespace FusionEngine
{

	//! Not quite a factory.
	/*!
	 * Creates FusionMessage objects from either game structs or RakNet Packets.
	 */
	class FusionMessageBuilder
	{
	public:
		//! Builds a message from a ShipState (usually outgoing)
		static FusionMessage *BuildMessage(const ShipState &input, PlayerInd playerid);
		//! Builds a message from a ProjectileState (usually outgoing)
		static FusionMessage *BuildMessage(const ProjectileState &input, PlayerInd playerid);

		//! Builds a message from a network packet (usually incoming)
		static FusionMessage *BuildMessage(Packet *packet, PlayerInd playerid);

		//! Builds a message from a RakNet engine message (I call these 'events')
		static FusionMessage *BuildEventMessage(Packet *packet, PlayerInd playerind);

		//! Extract the id from the packet. Internal use only
		static unsigned char _getPacketIdentifier(Packet *p);

		//! Extract the timestamp from the packet. Internal use only
		//! \returns 0 if no timestamp is  present
		static RakNetTime _getPacketTime(Packet *p);

		//! Get the length of the packet header (ID and Timestamp). Internal use only
		static int _getHeaderLength(Packet *p);

	};

}

#endif
