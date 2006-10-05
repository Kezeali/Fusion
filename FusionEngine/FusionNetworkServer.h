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

#ifndef Header_FusionEngine_FusionNetworkServer
#define Header_FusionEngine_FusionNetworkServer

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "../RakNet/RakServerInterface.h"
#include "../RakNet/RakNetworkFactory.h"
#include "../RakNet/PacketEnumerations.h"

/// Fusion
#include "FusionServerOptions.h"
#include "FusionMessage.h"
#include "FusionMessageBuilder.h"
#include "FusionNetworkMessageQueue.h"
#include "FusionNetworkTypes.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Handles network communicaion for the server in-game.
	 *
	 * This class gathers messages received from the clients, sorts them, and allows the
	 * ServerEnvironment to access them.
	 * <br>
	 * This impliments CL_Runnable; but the funny thing is, it isn't on-its-own thread safe -
	 * the storage class FusionNetworkMessageQueue is... I guess that just makes it tidier?
	 */
	class FusionNetworkServer : public CL_Runnable
	{
	public:
		/*!
		 * \brief
		 * Sets up a network server.
		 *
		 * \param port
		 * The port of the server.
		 */
		FusionNetworkServer(const std::string &port);
		/*!
		 * \brief
		 * Sets up a network server.
		 */
		//FusionNetworkClient();

		/*!
		 * \brief
		 * Sets up a network server. Gets settings from a ServerOptions object.
		 *
		 * \param port
		 * The port of the server.
		 *
		 * \param options
		 * Object to load options from (max. rate, packet interval, etc.)
		 */
		FusionNetworkServer(const std::string &port, ServerOptions *options);
		/*!
		 * \brief
		 * Sets up a network client. Gets settings from a ClientOptions object.
		 *
		 * \param options
		 * Object to load options from (max. rate, packet interval, etc.)
		 */
		//FusionNetworkServer(ServerOptions *options);

		//! Destructor
		~FusionNetworkServer();

	public:
		//! A group of messages
		typedef std::deque<FusionMessage*> MessageQueue;
		//! Maps Fusion player ids to RakNet player indexes
		typedef std::map<PlayerIndex, FusionEngine::PlayerInd> PlayerIDMap;

		//! Adds a message to the outgoing queue.
		void QueueMessage(FusionMessage *message, int channel);
		//! Gets the message from the front of the incomming queue.
		/*!
		 * Remember to delete the message when you're done!
		 */
		FusionMessage *GetNextMessage(int channel);

		//! [depreciated] Use GetNextMessage instead.
		//MessageQueue GetAllMessages(int channel);

		//! Updates the network
		void run();

		//! Allows the ServerEnvironment, etc. to access the NetEvent queue
		FusionMessage *GetNextEvent() const;

	protected:
		//! The underlying network interface (serverside, but it's really just a RakPeer...)
		RakServerInterface *m_RakServer;
		//! The hostname (or ip) and port to use.
		std::string m_Host, m_Port;

		//! Threadsafe, organised package storage
		FusionNetworkMessageQueue *m_Queue;

		//! Map of local player indexes, indexed by RakNet PlayerIDs
		PlayerIDMap m_PlayerIDMap;

		//! [depreciated] by Don'tRepeatYourself rules; see FusionMessageBuilder.
		/*!
		 * Extract the id from the packet.
		 */
		unsigned char getPacketIdentifier(Packet *p);
		//! Check if a packet can be handled by the network client
		bool handleRakPackets(Packet *p);

		//! Converts packets into messages and sorts them.
		//onPacketReceive();
	};

}

#endif