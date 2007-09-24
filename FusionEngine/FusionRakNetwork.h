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
*/

#ifndef Header_FusionEngine_RakNetwork
#define Header_FusionEngine_RakNetwork

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionNetwork.h"

#include <RakNet/RakPeerInterface.h>
#include <RakNet/RakNetTypes.h>
#include <RakNet/MessageIdentifiers.h>

#include "FusionNetworkTypes.h"

namespace FusionEngine
{

	//! The length of a header without a timestamp
	/*!
	 * Consists of: <br>
	 * <code>[MTID_x]</code>
	 */
	const size_t g_HeaderLength = sizeof(unsigned char);

	//! Timestamp length
	const size_t g_TimestampLength = sizeof(unsigned char) + sizeof(RakNetTime);

	//! The length of a header with a timestamp
	/*!
	 * Consists of: <br>
	 * <code>[ID_TIMESTAMP]+[RakNetTime]+[MTID_x]</code>
	 */
	const size_t g_HeaderLengthTimestamp = g_TimestampLength + g_HeaderLength;

	//! RakNet implementation of of NetHandle
	//class RakNetHandle : public NetHandle
	//{
	//	SystemAddress m_SystemAddress;

	//public:
	//	RakNetHandle()
	//	{
	//	}

	//	RakNetHandle(SystemAddress systemAddress)
	//		: m_SystemAddress(systemAddress)
	//	{
	//	}

	//	const SystemAddress &GetSystemAddress() const
	//	{
	//		return m_SystemAddress;
	//	}
	//};

	//! RakNet packet specialization
	class RakNetPacket : public IPacket
	{
	public:
		// Cache members
		bool m_TimeStamped;
		NetTime m_Time;

		Packet* m_OriginalPacket;
		NetHandle m_Handle;

	public:
		//! Constructor
		RakNetPacket(Packet *originalPacket);

		//! Constructor
		RakNetPacket(NetHandle hash, Packet *originalPacket);

		//! Virtual destructor
		virtual ~RakNetPacket();

	public:
		//! Returns the packet data (after the header) as a string
		virtual std::string GetDataString();
		//! Returns the packet data after the header
		virtual const char* GetData() const;
		//! Returns the data length
		virtual unsigned int GetLength() const;
		//! Returns the packet type
		virtual unsigned char GetType() const;
		//! Returns true if this packet has a timestamp
		virtual bool IsTimeStamped() const;
		//! Returns the timestamp
		virtual NetTime GetTime() const;
		//! Returns the system handle for the system that sent this packet
		virtual const NetHandle& GetSystemHandle() const;
	};

	/*!
	 * \brief
	 * RakNet based implementation of FusionEngine#Network
	 */
	class RakNetwork : public Network
	{
	protected:
		//! System map
		typedef std::map<NetHandle, SystemAddress> SystemAddressMap;

	public:
		RakNetwork();
		~RakNetwork();

	protected:
		//! Starts the network
		virtual bool Startup(unsigned short maxConnections, unsigned short incommingPort, unsigned short maxIncommingConnections = 0);
		//! [dp] Starts listening
		virtual void StartListening(unsigned short incommingPort);
		//! Connects to a server
		virtual bool Connect(const std::string &host, unsigned short port);

		//! Sends data as-is
		virtual bool SendRaw(char* data, unsigned int length,
			NetPriority priority, NetReliability reliability, char channel,
			const NetHandle& destination);
		/*!
		 * <p>Formats the packet as follows:</p>
		 *
		 * <b>The Fusion Packet Format
		 * <ol>
		 *  <li> [char]      Time stamp marker (indicates the packet is timestamped)
		 *  <li> [uint]      Time stamp (if the time stamp marker was included)
		 *  <li> [char]      Type ID
		 *  <li> [char]      Sub ID (optional)
		 *  <li> [...]       Data
		 * </ol>
		 */
		virtual bool Send(bool timestamped, char type, char* data, unsigned int length,
			NetPriority priority, NetReliability reliability, char channel,
			const NetHandle& destination);
		//! Receives data
		virtual IPacket* Receive();
		//! Puts the given packet back on the receive buffer
		virtual void PushBackPacket(IPacket* packet, bool toHead = false);
		//! Deletes a packet
		virtual void DeallocatePacket(IPacket* packet);

	public:
		//! Returns RakNet stats
		RakNetStatistics* const GetStatistics(const SystemAddress system);

		//! Gets the ping to the given host
		virtual int GetPing(const NetHandle& handle);
		virtual int GetLastPing(const NetHandle& handle);
		virtual int GetAveragePing(const NetHandle& handle);
		virtual int GetLowestPing(const NetHandle& handle);

		RakPeerInterface* GetRakNetPeer() const;

		
	protected:
		RakPeerInterface* m_NetInterface;
		SystemAddressMap m_SystemAddresses;

	protected:
		//! Returns a RakNet enum for the given FusionEngine enum
		inline PacketPriority rakPriority(NetPriority priority);
		//! Returns a RakNet enum for the given FusionEngine enum
		inline PacketReliability rakReliability(NetReliability reliability);
		////! [dp] Adds a header to the given packet data
		///*!
		// * \param[out] buffer
		// * The mem to write to
		// *
		// * \param[in] buffer
		// * The data for the packet
		// *
		// * \param[in] length
		// * The current length of the data
		// *
		// * \param[in] timeStamp
		// * True if a timestamp should be added
		// *
		// * \param[in] mtid
		// * The type ID of this message
		// *
		// * \param[in] cid
		// * The channel ID for this message.
		// *
		// *
		// * \returns
		// * Returns the length of the data after the header is added.
		// */
		//unsigned int addHeader(unsigned char* buffer, const unsigned char* data, unsigned int length, bool timeStamp, unsigned char mtid);

		////! [dp] Removes the header from the given packet data
		///*!
		// * \param[out] buffer
		// * The mem to write to
		// *
		// * \param[in] data
		// * The packet data to remove the header form
		// *
		// * \param[in] length
		// * The current length of the data
		// *
		// * \returns
		// * Returns the length of the data after the header is added.
		// */
		//unsigned int removeHeader(unsigned char* buffer, const unsigned char* data, unsigned int length);

		////! [dp] Returns true if the packet passed is a RakNet system message.
		///*!
		// * If the packet is a system message, and a type of system message which the
		// * Environment should be aware of, its info will added to the event queue.
		// * Non-event system messages types will be handled or ignored without their 
		// * ID being added to the event queue.
		// * <br>
		// * The following ID's will be added to the event queue if they are found:
		// * <ul>
		// * <b>Client Remote events</b><br>
		// * <size="-1">(events which don't apply to this client, but
		// *  may be useful to know about)</size>
		// * <li>ID_REMOTE_DISCONNECTION_NOTIFICATION
		// * <li>ID_REMOTE_CONNECTION_LOST
		// * <li>ID_REMOTE_NEW_INCOMING_CONNECTION
		// * <li>ID_REMOTE_EXISTING_CONNECTION
		// * <b>Client events</b>
		// * <li>ID_CONNECTION_BANNED
		// * <li>ID_CONNECTION_REQUEST_ACCEPTED
		// * <li>ID_NO_FREE_INCOMING_CONNECTIONS
		// * <li>ID_INVALID_PASSWORD
		// * <b>Server events</b>
		// * <li>ID_NEW_INCOMING_CONNECTION
		// * <b>Client & Server events</b>
		// * <li>ID_DISCONNECTION_NOTIFICATION
		// * <li>ID_CONNECTION_LOST
		// * <li>ID_RECEIVED_STATIC_DATA
		// * <li>ID_MODIFIED_PACKET
		// * <li>ID_CONNECTION_ATTEMPT_FAILED
		// *  (I don't know how this message could arrive! Does it get sent to loopback?)
		// * </ul>
		// */
		//bool isChaff(Packet *p);
	};

}

#endif