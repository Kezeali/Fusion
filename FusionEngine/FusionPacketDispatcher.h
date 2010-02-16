/*
  Copyright (c) 2006-2010 Fusion Project Team

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

#ifndef Header_FusionPacketDispatcher
#define Header_FusionPacketDispatcher

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionNetworkTypes.h"

namespace FusionEngine
{

	//! Packet dispatcher
	/*!
	 * Default implementation
	 */
	class PacketDispatcher
	{
	public:
		//! List of handlers
		//typedef std::list<PacketHandler*> HandlerList;
		//! Map of handler lists
		typedef std::tr1::unordered_multimap<unsigned char, PacketHandler*> HandlerMultiMap;
		typedef std::pair<HandlerMultiMap::iterator, HandlerMultiMap::iterator> HandlerRange;

	public:
		//! Constructor
		PacketDispatcher();
		//! Destructor
		~PacketDispatcher();

	public:
		void SetDefaultPacketHandler(PacketHandler* handler);

		//! Establishes a packet subscrption for a specific type
		void Subscribe(unsigned char type, PacketHandler* handler);

		//! Removes the given subscription
		void Unsubscribe(unsigned char type, PacketHandler* handler);

		//! Reads packets from the network and dispatches them accordingly
		void Dispatch(RakNetwork *net);

	protected:
		HandlerMultiMap m_TypeHandlers;
		PacketHandler* m_DefaultPacketHandler;

	};

	const unsigned char s_NumPacketTypes = MTID_MAX - ID_USER_PACKET_ENUM;

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
		typedef std::map<unsigned char, PacketHandlerNode*> HandlerMap;

	public:
		//! Constructor?
		ListPacketDispatcher();
		//! Destructor
		~ListPacketDispatcher();

	public:
		void SetDefaultPacketHandler(PacketHandler* handler);

		//! Establishes a packet subscrption for a specific type
		void Subscribe(unsigned char type, PacketHandler* handler);

		//! Removes the given subscription
		void Unsubscribe(unsigned char type, PacketHandler* handler);

		//! Reads packets from the network and dispatches them accordingly
		void Dispatch(RakNetwork *network);

	protected:
		PacketHandlerNode *m_TypeLists[s_NumPacketTypes];
		PacketHandler* m_DefaultPacketHandler;

	};

}

#endif
