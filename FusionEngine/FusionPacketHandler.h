/*
  Copyright (c) 2006-2007 Fusion Project Team

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


	File Author(s):

		Elliot Hayward

*/
#ifndef Header_FusionEngine_PacketDispatcher
#define Header_FusionEngine_PacketDispatcher

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionPacket.h"
#include "FusionLinkedNode.h"

namespace FusionEngine
{

	class PacketHandler
	{
	public:
		//! Constructor
		PacketHandler();
		//! Destructor
		virtual ~PacketHandler();

	public:
		//! Callback
		virtual void HandlePacket(Packet* packet) = 0;
	};


	//! Network packet routing
	/*!
	 * Linked list based (semi) implementation
	 *
	 * \sa ListPacketDispatcher
	 */
	class ListPacketHandler : public PacketHandler, public LinkedNode
	{
	public:
		//! Constructor
		PacketHandler();
		//! Destructor
		virtual ~PacketHandler();

	public:
		//! Callback
		void HandlePacket(Packet* packet) = 0;

		//! Passes the packet to all subsequent list members
		void ListHandlePacket(Packet* packet)
		{
			if (m_Next != NULL)
				m_Next->ListHandlePacket(packet);

			HandlePacket(packet);
		}
	};

}

#endif
