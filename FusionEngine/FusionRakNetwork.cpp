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

		
	File Author(s):

		Elliot Hayward

*/

#include "Common.h"

#include "FusionRakNetwork.h"

// Core raknet stuff
#include <RakNetworkFactory.h>
#include <RakNetStatistics.h>
// Utilities
#include <BitStream.h>
#include <GetTime.h>

namespace FusionEngine
{

	///////////////
	// RakNetPacket
	RakNetPacket::RakNetPacket(Packet* internalPacket)
		: m_OriginalPacket(internalPacket),
		m_Handle(new RakNetHandleImpl(internalPacket->guid, internalPacket->systemAddress))
	{
		RakNet::BitStream bits(internalPacket->data, internalPacket->bitSize / 8, false);
		// Timestamp
		m_TimeStamped = false;
		m_Time = 0;
		if (internalPacket->data[0] == ID_TIMESTAMP)
		{
			unsigned char idTimestamp;
			bits.Read(idTimestamp);
			bits.Read(m_Time);
			m_TimeStamped = true;
		}
		// Type
		unsigned char type;
		bits.Read(type);
	}

	RakNetPacket::RakNetPacket(RakNetHandle from, Packet* internalPacket)
		: m_Handle(from),
		m_OriginalPacket(internalPacket)
	{
		RakNet::BitStream bits(internalPacket->data, internalPacket->bitSize / 8, false);
		// Timestamp
		m_TimeStamped = false;
		m_Time = 0;
		if (internalPacket->data[0] == ID_TIMESTAMP)
		{
			unsigned char idTimestamp;
			bits.Read(idTimestamp);
			bits.Read(m_Time);
			m_TimeStamped = true;
		}
		// Type
		unsigned char type;
		bits.Read(type);
	}

	RakNetPacket::~RakNetPacket()
	{
		assert(m_OriginalPacket == 0);
	}

	std::string RakNetPacket::GetDataString()
	{
		unsigned int header = (IsTimeStamped() ? g_HeaderLengthTimestamp : g_HeaderLength);
		unsigned int length = m_OriginalPacket->bitSize / 8;
		char* dataBegin;
		char* dataEnd;

		dataBegin = (char*)(m_OriginalPacket->data + header);
		dataEnd = (char*)(m_OriginalPacket->data + length);
		return std::string(dataBegin, dataEnd);
	}

	const char* RakNetPacket::GetData() const
	{
		return (char*)(m_OriginalPacket->data + (IsTimeStamped() ? g_HeaderLengthTimestamp : g_HeaderLength));
	}

	unsigned int RakNetPacket::GetLength() const
	{
		return m_OriginalPacket->bitSize / 8 - (IsTimeStamped() ? g_HeaderLengthTimestamp : g_HeaderLength);
	}

	unsigned char RakNetPacket::GetType() const
	{
		return m_OriginalPacket->data[IsTimeStamped() ? g_TimestampLength : 0];
	}

	bool RakNetPacket::IsTimeStamped() const
	{
		return m_TimeStamped;
	}

	NetTime RakNetPacket::GetTime() const
	{
		return m_Time;
	}

	NetHandle RakNetPacket::GetSystemHandle() const
	{
		return m_Handle;
	}

	/////////////
	// RakNetwork
	RakNetwork::RakNetwork()
		: m_MinLagMilis(0), m_LagVariance(0),
		m_AllowBps(0.0)
	{
		m_NetInterface = RakNetworkFactory::GetRakPeerInterface();
	}

	RakNetwork::~RakNetwork()
	{
		RakNetworkFactory::DestroyRakPeerInterface(m_NetInterface);
	}

	bool RakNetwork::Startup(unsigned short maxConnections, unsigned short incommingPort, unsigned short maxIncommingConnections)
	{
		SocketDescriptor socDesc(incommingPort, 0);
		if (m_NetInterface->Startup(maxConnections, 0, &socDesc, 1))
		{
			m_NetInterface->AttachPlugin(&connectionGraphPlugin);
			m_NetInterface->AttachPlugin(&fullyConnectedMeshPlugin);

			m_NetInterface->SetMaximumIncomingConnections(maxIncommingConnections);

			return true;
		}
		else
			return false;
	}

	bool RakNetwork::Connect(const std::string& host, unsigned short port)
	{
		if (m_NetInterface->Connect(host.c_str(), port, 0, 0))
		{
			m_NetInterface->SetOccasionalPing(true);
			return true;
		}
		else
			return false;
	}

	void RakNetwork::Disconnect()
	{
		m_NetInterface->Shutdown(100);
	}

	bool RakNetwork::IsConnected() const
	{
		return true;
	}

	NetHandle RakNetwork::GetLocalAddress() const
	{
		RakNetHandle handle(new RakNetHandleImpl(
			m_NetInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS),
			m_NetInterface->GetInternalID(UNASSIGNED_SYSTEM_ADDRESS))
			);
		return handle;
	}

	bool RakNetwork::Send(bool timestamped, unsigned char type, char* data, unsigned int length, NetPriority priority, NetReliability reliability, char channel, const NetHandle &destination, bool to_all)
	{
		RakNetHandleImpl* rakDestination = dynamic_cast<RakNetHandleImpl*>(destination.get());

		RakNet::BitStream bits;
		if (timestamped)
		{
			bits.Write((MessageID)ID_TIMESTAMP);
			bits.Write(RakNet::GetTime());
		}
		bits.Write((MessageID)type);
		bits.Write(data, length);

		return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, (rakDestination != NULL) ? rakDestination->Address : UNASSIGNED_SYSTEM_ADDRESS, to_all);
	}

	bool RakNetwork::Send(bool timestamped, unsigned char type, RakNet::BitStream *data, NetPriority priority, NetReliability reliability, char channel, const NetHandle &destination, bool to_all)
	{
		RakNetHandleImpl* rakDestination = dynamic_cast<RakNetHandleImpl*>(destination.get());

		RakNet::BitStream bits;
		if (timestamped)
		{
			bits.Write((MessageID)ID_TIMESTAMP);
			bits.Write(RakNet::GetTime());
		}
		bits.Write((MessageID)type);
		bits.Write(data);

		return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, (rakDestination != NULL) ? rakDestination->Address : UNASSIGNED_SYSTEM_ADDRESS, to_all);
	}

	bool RakNetwork::SendRaw(const char* data, unsigned int length, NetPriority priority, NetReliability reliability, char channel, const NetHandle& destination, bool to_all)
	{
		RakNetHandleImpl* rakDestination = dynamic_cast<RakNetHandleImpl*>(destination.get());

		return m_NetInterface->Send(data, length, rakPriority(priority), rakReliability(reliability), channel, (rakDestination != NULL) ? rakDestination->Address : UNASSIGNED_SYSTEM_ADDRESS, to_all);
	}

	IPacket* RakNetwork::Receive()
	{
		Packet* rakPacket = m_NetInterface->Receive();
		if (rakPacket == 0)
			return 0;

		//NetHandle handle(rakPacket->systemAddress);
		//m_SystemAddresses.insert( SystemAddressMap::value_type(handle, rakPacket->systemAddress) );

		return new RakNetPacket(rakPacket);
	}

	void RakNetwork::PushBackPacket(IPacket *packet, bool toHead)
	{
		RakNetPacket* nativePacket = dynamic_cast<RakNetPacket*>(packet);
		if (nativePacket != 0)
			m_NetInterface->PushBackPacket(nativePacket->m_OriginalPacket, toHead);
	}

	void RakNetwork::DeallocatePacket(IPacket *packet)
	{
		RakNetPacket* nativePacket = dynamic_cast<RakNetPacket*>(packet);
		if (nativePacket != 0)
			m_NetInterface->DeallocatePacket(nativePacket->m_OriginalPacket);

		nativePacket->m_OriginalPacket = 0;

		delete nativePacket;
	}

	RakNetStatistics *const RakNetwork::GetStatistics(const SystemAddress &system)
	{
		return m_NetInterface->GetStatistics(system);
	}

	int RakNetwork::GetPing(const NetHandle& handle)
	{
		RakNetHandleImpl *rakHandle = dynamic_cast<RakNetHandleImpl*>(handle.get());
		return m_NetInterface->GetAveragePing(rakHandle->Address);
	}

	int RakNetwork::GetLastPing(const NetHandle& handle)
	{
		RakNetHandleImpl *rakHandle = dynamic_cast<RakNetHandleImpl*>(handle.get());
		return m_NetInterface->GetLastPing(rakHandle->Address);
	}

	int RakNetwork::GetAveragePing(const NetHandle& handle)
	{
		RakNetHandleImpl *rakHandle = dynamic_cast<RakNetHandleImpl*>(handle.get());
		return m_NetInterface->GetAveragePing(rakHandle->Address);
	}

	int RakNetwork::GetLowestPing(const NetHandle& handle)
	{
		RakNetHandleImpl *rakHandle = dynamic_cast<RakNetHandleImpl*>(handle.get());
		return m_NetInterface->GetLowestPing(rakHandle->Address);
	}

	void RakNetwork::SetDebugLag(unsigned int minLagMilis, unsigned int variance)
	{
		m_MinLagMilis = minLagMilis;
		m_LagVariance = variance;
		m_NetInterface->ApplyNetworkSimulator(m_AllowBps, (unsigned short)minLagMilis, (unsigned short)variance);
	}

	void RakNetwork::SetDebugPacketLoss(float allowBps)
	{
		m_AllowBps = allowBps;
		m_NetInterface->ApplyNetworkSimulator(allowBps, (unsigned short)m_MinLagMilis, (unsigned short)m_LagVariance);
	}

	unsigned int RakNetwork::GetDebugLagMin() const
	{
		return m_MinLagMilis;
	}

	unsigned int RakNetwork::GetDebugLagVariance() const
	{
		return m_LagVariance;
	}

	float RakNetwork::GetDebugAllowBps() const
	{
		return m_AllowBps;
	}


	inline PacketPriority RakNetwork::rakPriority(NetPriority priority)
	{
		return (PacketPriority)priority;
	}

	inline PacketReliability RakNetwork::rakReliability(NetReliability reliability)
	{
		return (PacketReliability)reliability;
	}

}
