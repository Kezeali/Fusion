/*
*  Copyright (c) 2009-2010 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionEasyPacket.h"

#include "FusionAssert.h"

#include <MessageIdentifiers.h>

namespace FusionEngine
{

	//EasyPacket::EasyPacket(Packet* raknet_packet)
	//	: OriginalPacket(raknet_packet)
	//{
	//}

	//EasyPacket& EasyPacket::operator =(Packet *packet)
	//{
	//	OriginalPacket = packet;
	//	return *this;
	//}

	unsigned char* EasyPacket::GetData()
	{
		return OriginalPacket.data + (IsTimeStamped() ? s_TimestampedHeaderLength : s_BasicHeaderLength);
	}

	unsigned int EasyPacket::GetDataLength() const
	{
		return (OriginalPacket.bitSize / 8) - (IsTimeStamped() ? s_TimestampedHeaderLength : s_BasicHeaderLength);
	}

	unsigned char EasyPacket::GetType() const
	{
		return OriginalPacket.data[IsTimeStamped() ? s_TimestampLength : 0];
	}

	bool EasyPacket::IsTimeStamped() const
	{
		return OriginalPacket.data[0] == ID_TIMESTAMP;
	}

	RakNet::Time EasyPacket::GetTime() const
	{
		FSN_ASSERT(IsTimeStamped());
		if (IsTimeStamped())
			return RakNet::Time(*(OriginalPacket.data+1));
		else
			return (RakNet::Time)0;
	}

	const RakNet::RakNetGUID &EasyPacket::GetGUID() const
	{
		return OriginalPacket.guid;
	}

}