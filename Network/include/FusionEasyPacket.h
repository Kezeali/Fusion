/*
*  Copyright (c) 2010 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef H_FusionEasyPacket
#define H_FusionEasyPacket

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

// TODO: why is functional needed here?
#include <functional>
#include <memory>
#include <RakNet/RakNetTypes.h>

namespace FusionEngine
{

	//! The length of a header without a timestamp
	/*!
	* Consists of: <br>
	* <code>[MTID_x]</code>
	*/
	const size_t s_BasicHeaderLength = sizeof(unsigned char);

	//! Timestamp length
	const size_t s_TimestampLength = sizeof(unsigned char) + sizeof(RakNet::Time);

	//! The length of a header with a timestamp
	/*!
	* Consists of: <br>
	* <code>[ID_TIMESTAMP]+[RakNetTime]+[MTID_x]</code>
	*/
	const size_t s_TimestampedHeaderLength = s_TimestampLength + s_BasicHeaderLength;

	//! Makes it easier to get the data you want out of a packet
	/*!
	* <strong>Example usage:</strong>
	* <code>
	* typedef EasyPacket* Ezy;
	* ...
	* void readPacket(Packet *packet) {
	* unsigned char type = Ezy(packet)->GetType();
	* }
	* </code>
	*/
	struct EasyPacket
	{
		struct RakNet::Packet OriginalPacket;

		//! Constructor
		//EasyPacket() {}
		//! Constructor
		//EasyPacket(Packet *raknet_packet);

		//EasyPacket &operator =(Packet *raknet_packet);

		//! Returns the packet data after the header
		unsigned char* GetData();
		//! Returns the data length
		unsigned int GetDataLength() const;

		//! Returns the packet type
		unsigned char GetType() const;

		//! Returns true if this packet has a timestamp
		bool IsTimeStamped() const;
		RakNet::Time GetTime() const;

		//! Returns the system handle for the system that sent this packet
		const RakNet::RakNetGUID &GetGUID() const;
	};

	typedef std::shared_ptr<RakNet::Packet> PacketSpt;

}

#endif
