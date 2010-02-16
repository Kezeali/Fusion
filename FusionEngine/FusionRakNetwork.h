/*
 Copyright (c) 2007-2009 Fusion Project Team

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

#include "FusionPrerequisites.h"

// RakNet
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <MessageIdentifiers.h>
// Plugins
#include <FullyConnectedMesh2.h>
#include <ConnectionGraph2.h>

#include "FusionEasyPacket.h"
#include "FusionNetworkTypes.h"

namespace FusionEngine
{

	//! Fusion Packet priority enumeration
	enum NetPriority
	{
		HIGH_PRIORITY = ::HIGH_PRIORITY,
		MEDIUM_PRIORITY = ::MEDIUM_PRIORITY,
		LOW_PRIORITY = ::LOW_PRIORITY
	};

	//! Fusion Packet reliability enumeration
	enum NetReliability
	{
		UNRELIABLE = ::UNRELIABLE,
		UNRELIABLE_SEQUENCED = ::UNRELIABLE_SEQUENCED,
		RELIABLE = ::RELIABLE,
		RELIABLE_ORDERED = ::RELIABLE_ORDERED,
		RELIABLE_SEQUENCED = ::RELIABLE_SEQUENCED
	};

	//inline PacketPriority rakPriority(NetPriority priority)
	//{
	//	return (PacketPriority)priority;
	//}
	//inline PacketReliability rakReliability(NetReliability reliability)
	//{
	//	return (PacketReliability)reliability;
	//}

	//! Somewhere to send a packet
	class NetDestination
	{
	public:
		// Leaving GUID default will instruct the Send method to set it to localhost (UNASSIGNED_SYSTEM_ADDRESS)
		RakNetGUID GUID;
		bool Broadcast;

		//! Default ctor
		NetDestination()
			: Broadcast(false)
		{}
		//! Destination with data initialisation
		NetDestination(const RakNetGUID &guid, bool broadcast)
			: GUID(guid),
			Broadcast(broadcast)
		{}

		//! Copy constructor
		NetDestination(const NetDestination &copy)
			: GUID(copy.GUID),
			Broadcast(copy.Broadcast)
		{}
		//! Move constructor
		NetDestination(NetDestination&& rvalue)
			: GUID(std::move(rvalue.GUID)),
			Broadcast(std::move(rvalue.Broadcast))
		{}
	};

	/*!
	 * \brief
	 * RakNet based network interface
	 */
	class RakNetwork
	{
	public:
		RakNetwork();
		~RakNetwork();

		//! Starts the network
		bool Startup(unsigned short maxConnections, unsigned short incommingPort, unsigned short maxIncommingConnections = 0);
		//! Connects to a server
		bool Connect(const std::string &host, unsigned short port);
		//! Disconnects cleanly
		void Disconnect();

		bool IsConnected() const;

		//! Returnst he GUID of this machine
		const RakNetGUID &GetLocalGUID() const;

		//! Gets the host GUID from the FullyConnectedMesh plugin
		RakNetGUID GetHost() const;

		//! Sends data as-is (without adding a header)
		/*!
		* Make sure you include an ID as the first byte, or RakNet may eat the message! :S
		*/
		bool SendAsIs(const NetDestination &dest,
			const char* data, unsigned int length,
			NetPriority priority, NetReliability reliability, char channel);

		//! Sends a RakNet#BitStream as-is (without adding a header)
		/*!
		* Make sure you include an ID as the first byte, or RakNet may eat the message! :O
		*/
		bool SendAsIs(const NetDestination &dest,
			const RakNet::BitStream *bitStream,
			NetPriority priority, NetReliability reliability, char channel);

		//! Prepends the given data with a header and sends it
		/*!
		 * <b>Packet Format
		 * <ol>
		 *  <li> [bool]          Time stamp marker (indicates the packet is timestamped)
		 *  <li> [unsigned int]  Time stamp (if the time stamp marker was 'true')
		 *  <li> [unsigned char] Type ID
		 *  <li> [...]           The given data
		 * </ol>
		 */
		virtual bool Send(const NetDestination &destination,
			bool timestamped, unsigned char type, char* data, unsigned int length,
			NetPriority priority, NetReliability reliability, char channel);

		//! Prepends the given RakNet#BitStream with a header and sends it
		bool Send(const NetDestination &destination,
			bool timestamped, unsigned char type, RakNet::BitStream *data,
			NetPriority priority, NetReliability reliability, char channel);
		
		//! Receives data
		Packet *Receive();

		//! Receives data, returning an AutoPacket
		AutoPacketPtr ReceiveAutoPacket();

		//! Puts the given packet back on the receive buffer
		void PushBackPacket(Packet *packet, bool to_head = false);

		//! Puts the given packet back on the receive buffer
		void PushBackPacket(const AutoPacketPtr &auto_packet, bool to_head = false);

		//! Deletes a packet
		void DeallocatePacket(Packet* packet);

		//! Returns RakNet stats
		RakNetStatistics *const GetStatistics(const RakNetGUID &guid);

		//! Gets the ping to the given host
		int GetPing(const RakNetGUID& guid);
		int GetLastPing(const RakNetGUID& guid);
		int GetAveragePing(const RakNetGUID& guid);
		int GetLowestPing(const RakNetGUID& guid);

		const RakPeerInterface* GetPeerInterface() const { return m_NetInterface; }

		//! Uses RakPeerInterface#ApplyNetworkSimulator to introduce fake lag
		/*!
		 * \param minLagMilis
		 * The minimum amount of lag time (milisecods) applied to communications
		 *
		 * \param variance
		 * Extra lag time (milisecods) which may be randomly applied
		 */
		void SetDebugLag(unsigned int minLagMilis, unsigned int variance);
		//! Uses RakPeerInterface#ApplyNetworkSimulator to introduce fake packet loss
		/*!
		 * \param allowBps
		 * Maximum bits per second before packet loss
		 */
		void SetDebugPacketLoss(float allowBps);

		//! Returns the current fake lag setting
		unsigned int GetDebugLagMin() const;
		//! Returns the current fake lag variance setting
		unsigned int GetDebugLagVariance() const;
		//! Returns the current fake packet loss setting
		float GetDebugAllowBps() const;

		
	protected:
		RakPeerInterface* m_NetInterface;
		FullyConnectedMesh2 m_FullyConnectedMeshPlugin;
		ConnectionGraph2 m_ConnectionGraphPlugin;

		// Network Simulator settings
		unsigned int m_MinLagMilis;
		unsigned int m_LagVariance;
		float m_AllowBps;
	};

}

#endif