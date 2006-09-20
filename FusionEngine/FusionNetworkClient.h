#ifndef Header_FusionEngine_FusionNetworkClient
#define Header_FusionEngine_FusionNetworkClient

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "R

/// Fusion
#include "FusionClientOptions.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Handles network communicaion for the client in-game.
	 *
	 * This class gathers messages received from the host, sorts them, and allows the
	 * ClientEnvironment to access them.
	 */
	class FusionNetworkClient
	{
	public:
		/*!
		 * \brief
		 * Sets up a network client.
		 *
		 * \param host
		 * The hostname or ipaddress of the server.
		 *
		 * \param port
		 * The port of the server.
		 */
		FusionNetworkClient(const std::string &host, const std::string &port);
		/*!
		 * \brief
		 * Sets up a network client.
		 *
		 * \param address
		 * A CL_IPAddress pointing to the server.
		 */
		FusionNetworkClient(const CL_IPAddress &address);

		/*!
		 * \brief
		 * Sets up a network client. Gets settings from a ClientOptions object.
		 *
		 * \param host
		 * The hostname or ipaddress of the server.
		 *
		 * \param port
		 * The port of the server.
		 *
		 * \param options
		 * Object to load options from (max. rate, packet interval, etc.)
		 */
		FusionNetworkClient(const std::string &host, const std::string &port,
			ClientOptions *options);
		/*!
		 * \brief
		 * Sets up a network client. Gets settings from a ClientOptions object.
		 *
		 * \param address
		 * A CL_IPAddress pointing to the server.
		 *
		 * \param options
		 * Object to load options from (max. rate, packet interval, etc.)
		 */
		FusionNetworkClient(const CL_IPAddress &address,
			ClientOptions *options);

		InitialiseChannel();

		typedef std::queue<FusionMessage*> MessageQueue;

		//! Adds a message to the outgoing queue.
		void QueueMessage(const std::string &channel);
		//! Gets all messages from the incomming queue.
		MessageQueue GetAllMessages(const std::string &channel);
		//! Gets a message from the incomming queue.
		FusionMessage GetNextMessages(const std::string &channel);

	private:
		//! The hostname (or ip) and port to use.
		std::string m_Host, m_Port;

		//! Converts packets into messages and sorts them.
		onPacketReceive();

		/*!
		 * \brief
		 * Used for thread-safety.
		 *
		 * Ensures thread-safety by preventing concurrent data access via GetMessages().
		 */
		//CL_Mutex *m_Mutex;
	};

}

#endif