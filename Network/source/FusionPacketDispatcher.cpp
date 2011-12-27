/*
  Copyright (c) 2007 Fusion Project Team

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

#include "PrecompiledHeaders.h"

#include "FusionPacketDispatcher.h"

#include <RakNetTypes.h>

#include "FusionRakNetwork.h"
#include "FusionPacketHandler.h"

using namespace RakNet;

namespace FusionEngine
{

	PacketDispatcher::PacketDispatcher()
		: m_DefaultPacketHandler(NULL)
	{
	}

	PacketDispatcher::~PacketDispatcher()
	{
	}

	void PacketDispatcher::SetDefaultPacketHandler(PacketHandler *handler)
	{
		m_DefaultPacketHandler = handler;
	}

	typedef EasyPacket* Ezy;

	void PacketDispatcher::Dispatch(RakNetwork *network)
	{
		Packet* packet = network->Receive();
		while (packet != nullptr)
		{
			// Find the handlers for this type
			auto range = m_TypeHandlers.equal_range(Ezy(packet)->GetType());
			if (range.first != m_TypeHandlers.end())
			{
				std::for_each(range.first, range.second, [&](HandlerMultiMap::value_type &it)
				{
					it.second->HandlePacket(packet);
				});
				//for (HandlerMultiMap::iterator it = range.first, end = range.second; it != end; ++it)
				//	it->second->HandlePacket(packet);
			}
			// If there are no handlers subscribed to the given type:
			else if (m_DefaultPacketHandler != nullptr)
				m_DefaultPacketHandler->HandlePacket(packet);

			network->DeallocatePacket((Packet*)packet);
			packet = network->Receive();
		}
	}

	void PacketDispatcher::Subscribe(unsigned char type, PacketHandler *handler)
	{
#ifdef FSN_ASSERTS_ENABLED
		// Fails if this is a duplicate:
		auto range = m_TypeHandlers.equal_range(type);
		auto existingEntry = std::find_if(range.first, range.second, [handler](const HandlerMultiMap::value_type& entry) { return entry.second == handler; });
		FSN_ASSERT(existingEntry == range.second);
#endif
		m_TypeHandlers.insert( HandlerMultiMap::value_type(type, handler) );
	}

	void PacketDispatcher::Unsubscribe(unsigned char type, PacketHandler *handler)
	{
		HandlerRange range = m_TypeHandlers.equal_range(type);
		if (range.first != m_TypeHandlers.end())
		{
			for (HandlerMultiMap::iterator it = range.first, end = range.second; it != end; ++it)
			{
				if (it->second == handler)
				{
					m_TypeHandlers.erase(it);
					break;
				}
			}
		}
	}



	ListPacketDispatcher::ListPacketDispatcher()
		: m_DefaultPacketHandler(NULL)
	{
		memset(&m_TypeLists[0], NULL, s_NumPacketTypes);
	}

	void ListPacketDispatcher::SetDefaultPacketHandler(PacketHandler *handler)
	{
		m_DefaultPacketHandler = handler;
	}

	void ListPacketDispatcher::Dispatch(RakNetwork *network)
	{
		Packet* packet = network->Receive();
		while (packet != NULL)
		{
			// Find the handler list for this channel
			PacketHandlerNode *node = m_TypeLists[Ezy(packet)->GetType()-ID_USER_PACKET_ENUM];
			if (node != NULL)
			{
				node->ListHandlePacket(packet);
			}
			else if (m_DefaultPacketHandler != NULL)
				m_DefaultPacketHandler->HandlePacket(packet);

			packet = network->Receive();
		}
	}

	void ListPacketDispatcher::Subscribe(unsigned char channel, PacketHandler *handler)
	{
		PacketHandlerNode *node = m_TypeLists[channel-ID_USER_PACKET_ENUM];
		if (node == NULL)
			node = new PacketHandlerNode(handler);

		else
			node->push_back(new PacketHandlerNode(handler));
	}

	void ListPacketDispatcher::Unsubscribe(unsigned char channel, PacketHandler *handler)
	{
		PacketHandlerNode *node = m_TypeLists[channel-ID_USER_PACKET_ENUM];
		while (node != NULL)
		{
			if (node->m_Handler == handler)
			{
				// Remove the node from this list
				delete node;
				break;
			}
			node = dynamic_cast<PacketHandlerNode*>( node->getNext() );
		}
	}

}
