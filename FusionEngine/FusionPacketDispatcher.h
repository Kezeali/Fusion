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

#include "FusionNetwork.h"

namespace FusionEngine
{

	//class IPacketDispatcher
	//{
	//public:
	//	//! Establishes a packet subscription
	//	virtual void SubscribeToChannel(char channel, PacketHandler* handler) = 0;
	//	//! Establishes a packet subscrption for a specific type
	//	/*!
	//	 * Allows a subscriber to receive all packets of a specific type
	//	 * (reguardless of chanel)
	//	 */
	//	virtual void Subscribe(char type, PacketHandler* handler) = 0;

	//	//! Removes the given subscription
	//	virtual void UnsubscribeFromChannel(char channel, PacketHandler* handler) = 0;
	//	//! Removes the given subscription
	//	virtual void Unsubscribe(char type, PacketHandler* handler) = 0;

	//	//! Reads packets from the network and dispatches them accordingly
	//	virtual void Run() = 0;

	//};

	//! Packet dispatcher
	/*!
	 * Default (signal based) implementation
	 */
	class PacketDispatcher
	{
	public:
		//! List of handlers
		typedef std::list<PacketHandler*> HandlerList;
		//! Map of handler lists
		typedef std::map<char, HandlerList> HandlerListMap;

	public:
		//! Constructor
		PacketDispatcher();
		//! Constructor
		PacketDispatcher(Network* net);
		//! Destructor
		~PacketDispatcher();

	public:
		//! Establishes a packet subscription
		void EstablishChannel(char channel);
		//! Establishes a packet subscrption for a specific type
		/*!
		 * Allows a subscriber to receive all packets of a specific type
		 * (reguardless of chanel)
		 */
		void Subscribe(char channel, PacketHandler* handler);

		//! Removes the given subscription
		void CloseChannel(char channel);
		//! Removes the given subscription
		void Unsubscribe(char channel, PacketHandler* handler);

		//! Reads packets from the network and dispatches them accordingly
		void Run();

	protected:
		Network* m_Network;
		HandlerListMap m_ChannelHandlers;
		PacketHandler* m_DefaultPacketHandler;

	};


	//! (List based) Network packet routing
	/*!
	 * An alternative version of PacketDispatcher which uses the ListNode based ListPacketHandler
	 * classes for (possibly) greater effeciancy.
	 *
	 * \sa PacketDispatcher | PacketHandler | ListPacketHandler.
	 */
	class ListPacketDispatcher
	{
	public:
		//! Map of handlers
		typedef std::map<char, ListPacketHandler*> HandlerMap;

	public:
		//! Constructor?
		ListPacketDispatcher();
		//! Constructor
		ListPacketDispatcher(Network* net);
		//! Destructor
		~ListPacketDispatcher();

	public:

		//! Establishes a packet subscription
		void SubscribeToChannel(char channel, PacketHandler* handler);
		//! Establishes a packet subscrption for a specific type
		/*!
		 * Allows a subscriber to receive all packets of a specific type
		 * (reguardless of chanel)
		 */
		void Subscribe(char type, PacketHandler* handler);

		//! Removes the given subscription
		void UnsubscribeFromChannel(char channel, PacketHandler* handler);
		//! Removes the given subscription
		void Unsubscribe(char type, PacketHandler* handler);

		//! Reads packets from the network and dispatches them accordingly
		void Run();

	protected:
		Network* m_Network;
		//! Handlers
		HandlerMap m_ChannelHandlers;
		// Type handlers
		HandlerMap m_TypeHandlers;
	};

}

#endif
