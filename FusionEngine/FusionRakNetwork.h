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
		RakNetPacket(NetHandle from, Packet *originalPacket);

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
		//! Connects to a server
		virtual bool Connect(const std::string &host, unsigned short port);
		//! Disconnects cleanly
		virtual void Disconnect();

		//! Sends data as-is
		virtual bool SendRaw(char* data, unsigned int length,
			NetPriority priority, NetReliability reliability, char channel,
			const NetHandle& destination);
		/*!
		 * <p>Formats the packet as follows:</p>
		 *
		 * <b>Packet Format
		 * <ol>
		 *  <li> [bool]      Time stamp marker (indicates the packet is timestamped)
		 *  <li> [uint]      Time stamp (if the time stamp marker was included)
		 *  <li> [char]      Type ID
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

		inline const RakPeerInterface* GetRakNetPeer() const { return m_NetInterface; }

		//! Uses RakPeerInterface#ApplyNetworkSimulator to introduce fake lag
		/*!
		 * \param minLagMilis
		 * The minimum amount of lag time (milisecods) applied to communications
		 *
		 * \param variance
		 * Extra lag time (milisecods) which may be randomly applied
		 */
		virtual void SetDebugLag(unsigned int minLagMilis, unsigned int variance);
		//! Uses RakPeerInterface#ApplyNetworkSimulator to introduce fake packet loss
		/*!
		 * \param allowBps
		 * Maximum bits per second before packet loss
		 */
		virtual void SetDebugPacketLoss(double allowBps);

		//! Returns the current fake lag setting
		virtual unsigned int GetDebugLagMin() const;
		//! Returns the current fake lag variance setting
		virtual unsigned int GetDebugLagVariance() const;
		//! Returns the current fake packet loss setting
		virtual double GetDebugAllowBps() const;

		
	protected:
		RakPeerInterface* m_NetInterface;
		SystemAddressMap m_SystemAddresses;

		// Network Simulator settings
		unsigned int m_MinLagMilis;
		unsigned int m_LagVariance;
		double m_AllowBps;

	protected:
		//! Returns a RakNet enum for the given FusionEngine enum
		inline PacketPriority rakPriority(NetPriority priority);
		//! Returns a RakNet enum for the given FusionEngine enum
		inline PacketReliability rakReliability(NetReliability reliability);
	};

}

#endif