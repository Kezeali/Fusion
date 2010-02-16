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

#include "FusionStableHeaders.h"

#include "FusionRakNetwork.h"

// Core raknet stuff
#include <RakNetworkFactory.h>
#include <RakNetStatistics.h>
// Utilities
#include <BitStream.h>
#include <GetTime.h>

#include "FusionAssert.h"

namespace FusionEngine
{
	
	typedef PacketPriority rakPriority;
	typedef PacketReliability rakReliability;

	RakNetwork::RakNetwork()
		: m_MinLagMilis(0), m_LagVariance(0),
		m_AllowBps(0.0)
	{
		m_NetInterface = RakNetworkFactory::GetRakPeerInterface();

		m_FullyConnectedMeshPlugin.SetConnectOnNewRemoteConnection(true, "");
		m_FullyConnectedMeshPlugin.SetAutoparticipateConnections(true);

		m_NetInterface->AttachPlugin(&m_ConnectionGraphPlugin);
		m_NetInterface->AttachPlugin(&m_FullyConnectedMeshPlugin);
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
		return m_FullyConnectedMeshPlugin.GetParticipantCount() > 0;
	}

	const RakNetGUID &RakNetwork::GetLocalGUID() const
	{
		return m_NetInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);
	}

	RakNetGUID RakNetwork::GetHost() const
	{
		return m_FullyConnectedMeshPlugin.GetConnectedHost();
	}

	bool RakNetwork::SendAsIs(const NetDestination& destination, const char* data, unsigned int length, NetPriority priority, NetReliability reliability, char channel)
	{
		return m_NetInterface->Send(data, length, rakPriority(priority), rakReliability(reliability), channel, destination.GUID, destination.Broadcast);
	}

	bool RakNetwork::SendAsIs(const NetDestination& destination, const RakNet::BitStream *data, NetPriority priority, NetReliability reliability, char channel)
	{
		return m_NetInterface->Send(data, rakPriority(priority), rakReliability(reliability), channel, destination.GUID, destination.Broadcast);
	}

	bool RakNetwork::Send(const NetDestination &destination, bool timestamped, unsigned char type, char* data, unsigned int length, NetPriority priority, NetReliability reliability, char channel)
	{
		RakNet::BitStream bits;
		if (timestamped)
		{
			bits.Write((MessageID)ID_TIMESTAMP);
			bits.Write(RakNet::GetTime());
		}
		bits.Write((MessageID)type);
		if (data != nullptr && length != 0)
			bits.Write(data, length);

		if (destination.GUID != RakNetGUID())
			return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, destination.GUID, destination.Broadcast);
		else
			return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, GetLocalGUID(), destination.Broadcast);
	}

	bool RakNetwork::Send(const NetDestination &destination, bool timestamped, unsigned char type, RakNet::BitStream *data, NetPriority priority, NetReliability reliability, char channel)
	{
		FSN_ASSERT(type >= ID_USER_PACKET_ENUM);

		RakNet::BitStream bits;
		if (timestamped)
		{
			bits.Write((MessageID)ID_TIMESTAMP);
			bits.Write(RakNet::GetTime());
		}
		bits.Write((MessageID)type);
		if (data != nullptr)
			bits.Write(data);

		if (destination.GUID != RakNetGUID())
			return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, destination.GUID, destination.Broadcast);
		else
			return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, GetLocalGUID(), destination.Broadcast);
	}

	Packet *RakNetwork::Receive()
	{
		return m_NetInterface->Receive();
	}

	AutoPacketPtr RakNetwork::ReceiveAutoPacket()
	{
		Packet *packet = m_NetInterface->Receive();
		if (packet == nullptr)
			return AutoPacketPtr();

		using namespace std::tr1::placeholders;

		return AutoPacketPtr( new AutoPacket(packet, std::bind(&RakNetwork::DeallocatePacket, this, _1)) );
	}

	void RakNetwork::PushBackPacket(const AutoPacketPtr &auto_packet, bool head)
	{
		if (auto_packet)
		{
			m_NetInterface->PushBackPacket(auto_packet->OriginalPacket, head);
			auto_packet->OriginalPacket = nullptr;
		}
	}

	void RakNetwork::PushBackPacket(Packet *packet, bool head)
	{
		m_NetInterface->PushBackPacket(packet, head);
	}

	void RakNetwork::DeallocatePacket(Packet *packet)
	{
		m_NetInterface->DeallocatePacket(packet);
	}

	RakNetStatistics *const RakNetwork::GetStatistics(const RakNetGUID &guid)
	{
		SystemAddress system = m_NetInterface->GetSystemAddressFromGuid(guid);
		return m_NetInterface->GetStatistics(system);
	}

	int RakNetwork::GetPing(const RakNetGUID &guid)
	{
		return m_NetInterface->GetAveragePing(guid);
	}

	int RakNetwork::GetLastPing(const RakNetGUID &guid)
	{
		return m_NetInterface->GetLastPing(guid);
	}

	int RakNetwork::GetAveragePing(const RakNetGUID &guid)
	{
		return m_NetInterface->GetAveragePing(guid);
	}

	int RakNetwork::GetLowestPing(const RakNetGUID &guid)
	{
		return m_NetInterface->GetLowestPing(guid);
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

}
