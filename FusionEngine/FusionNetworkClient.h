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

#ifndef Header_FusionEngine_FusionNetworkClient
#define Header_FusionEngine_FusionNetworkClient

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <RakNet/RakClientInterface.h>

/// Fusion
#include "FusionNetworkGeneric.h"
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
	class FusionNetworkClient : public FusionNetworkGeneric
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
		//FusionNetworkClient(const CL_IPAddress &address);

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
		//FusionNetworkClient(const CL_IPAddress &address, ClientOptions *options);

		//! Destructor
		~FusionNetworkClient();

	public:
		//! Returns true if the mRate for this client hasn't been exceeded.		
		/*!
		 * If (GetStatistics()->bitsPerSecond) > (mRate) more data
		 * has been sent in the current second than the client's options
		 * allow, and thus SendAllowed() will return false.
		 */
		bool SendAllowed() const;

		//! Updates the network
		/*!
		 * Acts as Fusion Network's sorting office. Game packets are checks and sorted
		 * into catagory queues (channels) while critical infrastructure packets are added
		 * to the 'Events' list.
		 */
		void Receive();

	protected:
		//! \todo Work out whether NetworkClient/Server needs a options object.
		ClientOptions *m_Options;

		//! The underlying network interface (clientside, but it's really just a RakPeer...)
		RakClientInterface *m_RakClient;

	protected:
		//! Implementation of FusionNetworkGeneric#send()
		bool send(char *message, int length, PacketPriority priority, PacketReliability reliability, ChannelID channel);

	};

}

#endif
