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

#ifndef Header_FusionEngine_FusionNetworkClient
#define Header_FusionEngine_FusionNetworkClient

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "../RakNet/RakClientInterface.h"
#include "../RakNet/RakNetworkFactory.h"
#include "../RakNet/PacketEnumerations.h"

/// Fusion
#include "FusionClientOptions.h"
#include "FusionMessage.h"
#include "FusionNetworkMessageQueue.h"
#include "FusionNetworkTypes.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Handles network communicaion for the client in-game.
	 *
	 * This class gathers messages received from the host, sorts them, and allows the
	 * ClientEnvironment to access them.
	 * <br>
	 * This impliments CL_Runnable; but the funny thing is, it isn't on-its-own thread safe -
	 * the storage class FusionNetworkMessageQueue is... I guess that just makes it tidier?
	 */
	class FusionNetworkClient : public CL_Runnable
	{
	public:
		/*!
		 * \\brief
		 * Sets up a network client.
		 *
		 * \\param host
		 * The hostname or ipaddress of the server.
		 *
		 * \\param port
		 * The port of the server.
		 */
		FusionNetworkClient(const std::string &host, const std::string &port);
		/*!
		 * \\brief
		 * Sets up a network client.
		 *
		 * \\param address
		 * A CL_IPAddress pointing to the server.
		 */
		//FusionNetworkClient(const CL_IPAddress &address);

		/*!
		 * \\brief
		 * Sets up a network client. Gets settings from a ClientOptions object.
		 *
		 * \\param host
		 * The hostname or ipaddress of the server.
		 *
		 * \\param port
		 * The port of the server.
		 *
		 * \\param options
		 * Object to load options from (max. rate, packet interval, etc.)
		 */
		FusionNetworkClient(const std::string &host, const std::string &port,
			ClientOptions *options);
		/*!
		 * \\brief
		 * Sets up a network client. Gets settings from a ClientOptions object.
		 *
		 * \\param address
		 * A CL_IPAddress pointing to the server.
		 *
		 * \\param options
		 * Object to load options from (max. rate, packet interval, etc.)
		 */
		//FusionNetworkClient(const CL_IPAddress &address, ClientOptions *options);

		//! Destructor
		~FusionNetworkClient();

	public:
		//! A group of messages
		typedef std::deque<FusionMessage*> MessageQueue;

		//! Adds a message to the outgoing queue.
		void QueueMessage(FusionMessage *message, int channel);
		//! Gets all messages from the incomming queue.
		const MessageQueue &GetAllMessages(int channel);
		//! Gets the message from the front of the incomming queue.
		const FusionMessage &GetNextMessage(int channel);

		//! Updates the network
		void run();

		//! Allows the ClientEnvironment, etc. to access the NetEvent queue
		EventList GetEvents() const;

	protected:
		//! The underlying network interface (clientside, but it's really just a RakPeer...)
		RakClientInterface *m_RakClient;
		//! The hostname (or ip) and port to use.
		std::string m_Host, m_Port;

		//! Threadsafe, organised package storage
		FusionNetworkMessageQueue *m_Queue;

		//! Extract the id from the packet
		unsigned char getPacketIdentifier(Packet *p);
		//! Check if a packet can be handled by the network client
		bool handleRakPackets(Packet *p);

		//! Converts packets into messages and sorts them.
		//onPacketReceive();
	};

}

#endif