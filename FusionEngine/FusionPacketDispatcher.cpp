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

#include "Common.h"

#include "FusionPacketDispatcher.h"

#include "FusionNetwork.h"
#include "FusionPacketHandler.h"

namespace FusionEngine
{

	PacketDispatcher::PacketDispatcher()
		: m_Network(NULL),
		m_DefaultPacketHandler(NULL)
	{
	}

	PacketDispatcher::PacketDispatcher(Network *net)
		: m_Network(net),
		m_DefaultPacketHandler(NULL)
	{
	}

	PacketDispatcher::~PacketDispatcher()
	{
	}

	void PacketDispatcher::SetNetwork(Network *net)
	{
		m_Network = net;
	}

	void PacketDispatcher::SetDefaultPacketHandler(PacketHandler *handler)
	{
		m_DefaultPacketHandler = handler;
	}

	void PacketDispatcher::Run()
	{
		IPacket* packet = m_Network->Receive();
		while (packet != NULL)
		{
			// Find the handlers for this type
			HandlerRange range = m_ChannelHandlers.equal_range(packet->GetType());
			if (range.first != m_ChannelHandlers.end())
			{
				for (HandlerMultiMap::iterator it = range.first, end = range.second; it != end; ++it)
					it->second->HandlePacket(packet);
			}
			// If there are no handlers subscribed to the given type:
			else if (m_DefaultPacketHandler != NULL)
				m_DefaultPacketHandler->HandlePacket(packet);

			packet = m_Network->Receive();
		}
	}

	void PacketDispatcher::Subscribe(char type, PacketHandler *handler)
	{
		m_ChannelHandlers.insert( HandlerMultiMap::value_type(type, handler) );
	}

	void PacketDispatcher::Unsubscribe(char type, PacketHandler *handler)
	{
		HandlerRange range = m_ChannelHandlers.equal_range(type);
		if (range.first != m_ChannelHandlers.end())
		{
			for (HandlerMultiMap::iterator it = range.first, end = range.second; it != end; ++it)
			{
				if (it->second == handler)
				{
					m_ChannelHandlers.erase(it);
					break;
				}
			}
		}
	}



	ListPacketDispatcher::ListPacketDispatcher()
		: m_Network(NULL),
		m_DefaultPacketHandler(NULL)
	{
		memset(&m_ChannelLists[0], NULL, s_NumChannelTypes);
	}

	ListPacketDispatcher::ListPacketDispatcher(Network *net)
		: m_Network(net),
		m_DefaultPacketHandler(NULL)
	{
		memset(&m_ChannelLists[0], NULL, s_NumChannelTypes);
	}

	void ListPacketDispatcher::SetNetwork(FusionEngine::Network *net)
	{
		m_Network = net;
	}

	void ListPacketDispatcher::SetDefaultPacketHandler(PacketHandler *handler)
	{
		m_DefaultPacketHandler = handler;
	}

	void ListPacketDispatcher::Run()
	{
		IPacket* packet = m_Network->Receive();
		while (packet != NULL)
		{
			// Find the handler list for this channel
			PacketHandlerNode *node = m_ChannelLists[packet->GetType()-ID_USER_PACKET_ENUM];
			if (node != NULL)
			{
				node->ListHandlePacket(packet);
			}
			else if (m_DefaultPacketHandler != NULL)
				m_DefaultPacketHandler->HandlePacket(packet);

			packet = m_Network->Receive();
		}
	}

	void ListPacketDispatcher::Subscribe(char channel, PacketHandler *handler)
	{
		PacketHandlerNode *node = m_ChannelLists[channel-ID_USER_PACKET_ENUM];
		if (node == NULL)
			node = new PacketHandlerNode(handler);

		else
			node->push_back(new PacketHandlerNode(handler));
	}

	void ListPacketDispatcher::Unsubscribe(char channel, PacketHandler *handler)
	{
		PacketHandlerNode *node = m_ChannelLists[channel-ID_USER_PACKET_ENUM];
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
