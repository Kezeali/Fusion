/*
 Copyright (c) 2006 Fusion Project Team
 
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

#include "FusionCommon.h"

#include "../RakNet/RakServerInterface.h"

/// Fusion
#include "FusionNetworkGeneric.h"
#include "FusionServerOptions.h"

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
	class FusionNetworkServer : public FusionNetworkGeneric
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
		//! Returns true if the mRate for a particular client hasn't been exceeded.		
		/*!
		 * First, the client in which the given player resides is found.
		 * If (GetStatistics()->bitsPerSecond) > (mRate) more data
		 * has been sent in the current second than the client's options
		 * allow, and thus SendAllowed() will return false.
		 *
		 * \param[in] player
		 * The player whose client should be checked.
		 */
		bool SendAllowed(ObjectID player) const;

		virtual void Receive() {}

		//! [depreciated] by Receive() and Send*() (and send()) methods. Updates the network
		void run();

	protected:
		//! The underlying network interface (serverside, but it's really just a RakPeer...)
		RakServerInterface *m_RakServer;

		//! Map of local player indexes, indexed by RakNet SystemAddresss
		SystemAddressMap m_SystemAddressMap;

	protected:
		// Send implementation
		virtual void send(char *message, PacketPriority priority, PacketReliability reliability, char channel) 
		{}

	};

}

#endif