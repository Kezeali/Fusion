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

#ifndef Header_FusionEngine_FusionNetworkGeneric
#define Header_FusionEngine_FusionNetworkGeneric

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

//#include <RakNet/RakNetworkFactory.h>
//#include <RakNet/NetworkTypes.h>
//#include <RakNet/PacketEnumerations.h>
//#include <RakNet/RakNetStatistics.h>

/// Fusion
#include "FusionNetworkPacketQueue.h"
#include "FusionNetworkTypes.h"

namespace FusionEngine
{

	//! Stores event packet information.
	struct Event
	{
		//! The ID for the event packet.
		unsigned char eventID;
		//! The player ID from the event data, if applicable.
		SystemAddress systemAddress;
	};

	/*!
	 * \brief
	 * Generic methods for FusionNetworkServer and FusionNetworkClient
	 */
	class Network
	{
	public:
		//! Basic constructor. Don't use.
		Network() {}

		/*!
		 * \brief
		 * Sets up a generic network. Probably used by the server.
		 *
		 * \param port
		 * The port of the server.
		 */
		Network(const std::string &port);

		/*!
		 * \brief
		 * Sets up a generic network. Probably used by the client (has host)
		 *
		 * \param host
		 * The hostname of the server.
		 *
		 * \param port
		 * The port of the server.
		 */
		Network(const std::string &host, const std::string &port);

		//! Virtual destructor
		virtual ~Network();

	public:
		//! Maps Fusion player ids to RakNet SystemAddresss
		typedef std::map<FusionEngine::ObjectID, SystemAddress> SystemAddressMap;
		//! Type for storing events
		typedef std::deque<Event*> EventQueue;

	public:
		//! Sends a message of undefined type.
		void SendRaw(char *data, char channel);
		//! Sends a Add player message.
		/*!
		 * This will optimise sending for high reliablility, which is required
		 * for AddPlayer messages.
		 *
		 * THIS SHOULDN'T BE IMPLEMENTED IN GENERIC, CLIENT AND SERVER USE ADDPLAYER DIFFERENTLY
		 */
		virtual void SendAddPlayer()=0;
		//! Sends a Remove player message.
		void SendRemovePlayer(ObjectID player);
		//! Sends a ShipState message.
		void SendShipState(const ShipState &state);
		//! Sends a Chat message.
		void SendChatter(const std::string &message);


		//! Gets packets from the network. Implimented by specialisations.
		virtual void Receive() =0;

		//! Gets the message from the front of the incomming queue, in the given channel.
		/*!
		 * <b> This method is Thread-safe </b>
		 * <br>
		 * Remember to delete the message when you're done!
		 */
		Packet *PopNextMessage(char channel);
		//! Gets the next item in the event queue.
		/*!
		 * When a RakNet system packet, such as ID_CONNECTION_LOST, is received 
		 * its ID will be extracted and stored in the event queue. 
		 */
		unsigned char PopNextEvent() const;
		//! Destroys all Events in the event queue.
		void ClearEvents();

	protected:
		//! The hostname (or ip) and port to use.
		std::string m_Host, m_Port;

		//! Threadsafe, organised packet storage
		PacketQueue *m_Queue;

		//! Event storage
		EventQueue m_Events;

	protected:
		//! Sends data. Implemented by specialisations.
		/*!
		 * \param[out] data
		 * The data to write to
		 *
		 * \param[in] length
		 * The current length of the data
		 *
		 * \param[in] timeStamp
		 * True if a timestamp should be added
		 *
		 * \param[in] mtid
		 * The type ID of this message
		 *
		 * \param[in] cid
		 * The channel ID for this message.
		 *
		 *
		 * \returns
		 * Returns the length of the data after the header is added.
		 */
		virtual bool send(char *message, int length, PacketPriority priority, PacketReliability reliability, ChannelID channel) =0;

		//! Adds a header to the given packet data
		/*!
		 * \param[out] buffer
		 * The mem to write to
		 *
		 * \param[in] buffer
		 * The data for the packet
		 *
		 * \param[in] length
		 * The current length of the data
		 *
		 * \param[in] timeStamp
		 * True if a timestamp should be added
		 *
		 * \param[in] mtid
		 * The type ID of this message
		 *
		 * \param[in] cid
		 * The channel ID for this message.
		 *
		 *
		 * \returns
		 * Returns the length of the data after the header is added.
		 */
		int addHeader(unsigned char* buffer, const unsigned char* data, int length, bool timeStamp, MessageType mtid, ChannelID cid);

		//! Removes the header from the given packet data
		/*!
		 * \param[out] buffer
		 * The mem to write to
		 *
		 * \param[in] data
		 * The packet data to remove the header form
		 *
		 * \param[in] length
		 * The current length of the data
		 *
		 * \returns
		 * Returns the length of the data after the header is added.
		 */
		int removeHeader(unsigned char* buffer, const unsigned char* data, int length);

		//! Returns true if the packet passed is a RakNet system message.
		/*!
		 * If the packet is a system message, and a type of system message which the
		 * Environment should be aware of, its info will added to the event queue.
		 * Non-event system messages types will be handled or ignored without their 
		 * ID being added to the event queue.
		 * <br>
		 * The following ID's will be added to the event queue if they are found:
		 * <ul>
		 * <b>Client Remote events</b><br>
		 * <size="-1">(events which don't apply to this client, but
		 *  may be useful to know about)</size>
		 * <li>ID_REMOTE_DISCONNECTION_NOTIFICATION
		 * <li>ID_REMOTE_CONNECTION_LOST
		 * <li>ID_REMOTE_NEW_INCOMING_CONNECTION
		 * <li>ID_REMOTE_EXISTING_CONNECTION
		 * <b>Client events</b>
		 * <li>ID_CONNECTION_BANNED
		 * <li>ID_CONNECTION_REQUEST_ACCEPTED
		 * <li>ID_NO_FREE_INCOMING_CONNECTIONS
		 * <li>ID_INVALID_PASSWORD
		 * <b>Server events</b>
		 * <li>ID_NEW_INCOMING_CONNECTION
		 * <b>Client & Server events</b>
		 * <li>ID_DISCONNECTION_NOTIFICATION
		 * <li>ID_CONNECTION_LOST
		 * <li>ID_RECEIVED_STATIC_DATA
		 * <li>ID_MODIFIED_PACKET
		 * <li>ID_CONNECTION_ATTEMPT_FAILED
		 *  (I don't know how this message could arrive! Does it get sent to loopback?)
		 * </ul>
		 */
		bool grabEvents(Packet *p);

	};

}

#endif
