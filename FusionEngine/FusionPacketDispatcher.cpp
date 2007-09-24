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

#include "FusionPacketDispatcher.h"

#include "FusionNetwork.h"
#include "FusionPacketHandler.h"

namespace FusionEngine
{

	PacketDispatcher::PacketDispatcher()
		: m_Network(NULL)
	{
	}

	PacketDispatcher::PacketDispatcher(Network *net)
		: m_Network(net)
	{
	}

	void PacketDispatcher::Run()
	{
		IPacket* packet = m_Network->Receive();
		while (packet != NULL)
		{
			// Find the handler list for this channel
			HandlerMap::iterator handlerIter = m_ChannelHandlers.find(packet->m_Channel);
			if (handlerIter != m_ChannelHandlers.end())
				(*handlerIter).second->ListHandlePacket(packet);

			handlerIter = m_TypeHandlers.find(packet->m_Type);
			if (handlerIter != m_TypeHandlers.end())
				(*handlerIter).second->ListHandlePacket(packet);

			packet = m_Network->Receive();
		}
	}

	void PacketDispatcher::Subscribe(char channel, PacketHandler *handler)
	{
		// If this channel type hasn't been defined before we create a new list for it
		std::pair<HandlerMap::iterator, bool> listHead =
			m_ChannelHandlers.insert( HandlerMap::value_type(channel, handler) );
		// Here the list has already been created, so we just add the new handler to the front of the list
		if (!listHead.second)
			(*(listHead.first)).second->push_front(handler);
	}

	void PacketDispatcher::SubscribeToType(char type, PacketHandler *handler)
	{
		HandlerMap::iterator listHead = m_TypeHandlers.find(type);
		// If this channel type hasn't been defined before we create a new list for it
		if (listHead == m_TypeHandlers.end())
			m_TypeHandlers.insert( HandlerMap::value_type(channel, handler) );
		// Here the list has already been created, so we just add the new handler to the front of the list
		else
			(*listHead).second->push_front(handler);
	}

	void PacketDispatcher::Unsubscribe(char channel, PacketHandler *handler)
	{
		// Make sure the list isn't invalidated by this change
		HandlerMap::iterator listHead = m_ChannelHandlers.find(channel);
		if (listHead != m_ChannelHandlers.end())
			(*listHead).second = (*listHead).second->getNext();

		// Remove the handler from the list
		handler->remove();
	}



	ListPacketDispatcher::ListPacketDispatcher()
		: m_Network(NULL)
	{
	}

	ListPacketDispatcher::ListPacketDispatcher(Network *net)
		: m_Network(net)
	{
	}

	void ListPacketDispatcher::Run()
	{
		Packet* packet = m_Network->Receive();
		while (packet != NULL)
		{
			// Find the handler list for this channel
			HandlerMap::iterator handlerIter = m_ChannelHandlers.find(packet->m_Channel);
			if (handlerIter != m_ChannelHandlers.end())
				(*handlerIter).second->ListHandlePacket(packet);

			handlerIter = m_TypeHandlers.find(packet->m_Type);
			if (handlerIter != m_TypeHandlers.end())
				(*handlerIter).second->ListHandlePacket(packet);

			packet = m_Network->Receive();
		}
	}

	void ListPacketDispatcher::Subscribe(char channel, PacketHandler *handler)
	{
		// If this channel type hasn't been defined before we create a new list for it
		std::pair<HandlerMap::iterator, bool> listHead =
			m_ChannelHandlers.insert( HandlerMap::value_type(channel, handler) );
		// Here the list has already been created, so we just add the new handler to the front of the list
		if (!listHead.second)
			(*(listHead.first)).second->push_front(handler);
	}

	void ListPacketDispatcher::SubscribeToType(char type, PacketHandler *handler)
	{
		HandlerMap::iterator listHead = m_TypeHandlers.find(type);
		// If this channel type hasn't been defined before we create a new list for it
		if (listHead == m_TypeHandlers.end())
			m_TypeHandlers.insert( HandlerMap::value_type(channel, handler) );
		// Here the list has already been created, so we just add the new handler to the front of the list
		else
			(*listHead).second->push_front(handler);
	}

	void ListPacketDispatcher::Unsubscribe(char channel, PacketHandler *handler)
	{
		// Make sure the list isn't invalidated by this change
		HandlerMap::iterator listHead = m_ChannelHandlers.find(channel);
		if (listHead != m_ChannelHandlers.end())
			(*listHead).second = (*listHead).second->getNext();

		// Remove the handler from the list
		handler->remove();
	}

}
