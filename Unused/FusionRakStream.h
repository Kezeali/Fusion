/*
 Copyright (c) 2008 Fusion Project Team

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

#ifndef Header_FusionEngine_RakStream
#define Header_FusionEngine_RakStream

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionNetwork.h"

#include <RakNet/BitStream.h>

namespace FusionEngine
{

	typedef std::tr1::shared_ptr<RakNet::BitStream> BitsPtr;
	//! Alternative to using RakStream
	static BitsPtr DataStream(IPacket *packet)
	{
		return BitsPtr( new RakNet::BitStream((unsigned char*)packet->GetData(), packet->GetLength(), false) );
	}

	//! Simple extension with Fusion specific functions
	class RakStream : public RakNet::BitStream
	{
	public:
		RakStream();
		RakStream(IPacket *packet);
		RakStream(IPacket *packet, bool copyData);

		char* GetDataC() const;
	};

	RakStream::RakStream()
		: RakNet::BitStream()
	{
	}

	RakStream::RakStream(IPacket *packet)
		: RakNet::BitStream((unsigned char*)packet->GetData(), packet->GetLength(), false)
	{
	}

	RakStream::RakStream(IPacket *packet, bool copyData)
		: RakNet::BitStream((unsigned char*)packet->GetData(), packet->GetLength(), copyData)
	{
	}

	char* RakStream::GetDataC() const
	{
		return (char*)GetData();
	}
}

#endif