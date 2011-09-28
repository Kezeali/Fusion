/*
*  Copyright (c) 2007-2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "FusionStableHeaders.h"

#include "FusionRakNetwork.h"

// Core raknet stuff
#include <RakNetStatistics.h>
// Utilities
#include <BitStream.h>
#include <GetTime.h>

#include "FusionAssert.h"

#include <tbb/spin_mutex.h>

using namespace RakNet;

namespace FusionEngine
{
	
	typedef PacketPriority rakPriority;
	typedef PacketReliability rakReliability;

	PeerIndexPlugin::PeerIndexPlugin()
	{
		m_PeerIndex = 0;
	}

	uint8_t PeerIndexPlugin::GetPeerIndex() const
	{
		return m_PeerIndex;
	}

	bool PeerIndexPlugin::IsSenior(const RakNet::RakNetGUID &guid) const
	{
		Mutex_t::scoped_lock lock(m_PeersMutex);
		return m_SeniorPeers.find(guid) != m_SeniorPeers.end();
	}

	PluginReceiveResult PeerIndexPlugin::OnReceive(Packet *packet)
	{
		// When the ID_REMOTE_NEW_INCOMING_CONNECTION packet is being sent to a new connection
		// it contains a list of systems to connect to, rather than a single system - this can
		//  be used to generate a set of peers that connected before this peer. 
		if (packet->data[0] == ID_REMOTE_NEW_INCOMING_CONNECTION)
		{
			RakNet::BitStream received(packet->data, packet->length, false);
			received.IgnoreBytes(sizeof(MessageID));
			unsigned int count;
			received.Read(count);
			if (count > 0)
			{
				// Count is the number of peers currently connected, thus count == this peer's starting rank
				m_PeerIndex = count;

				// I don't know how many bytes SystemAddress takes in a packet (since it is passed
				//  using one of the template specializations when writing to a BitStream), so
				//  IgnoreBytes can't be safely used here
				SystemAddress this_is_ignored;
				RakNetGUID guid;
				Mutex_t::scoped_lock lock(m_PeersMutex);
				for (unsigned int i = 0; i < count; ++i)
				{
					received.Read(this_is_ignored);
					received.Read(guid);
					m_SeniorPeers.insert(guid);
				}
			}
		}

		return RR_CONTINUE_PROCESSING;
	}

	void PeerIndexPlugin::OnClosedConnection(SystemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason)
	{
		Mutex_t::scoped_lock lock(m_PeersMutex);
		auto _where = m_SeniorPeers.find(rakNetGUID);
		if (_where != m_SeniorPeers.end())
		{
			// A peer that connected before this one has left, so this peer gets to move up the ranks
			--m_PeerIndex;
			m_SeniorPeers.erase(_where);
		}
	}

	RakNetwork::RakNetwork()
		: m_MinLagMilis(0), m_LagVariance(0),
		m_AllowBps(0.0)
	{
		m_NetInterface = RakPeerInterface::GetInstance();

		m_FullyConnectedMeshPlugin.SetConnectOnNewRemoteConnection(true, "");
		m_FullyConnectedMeshPlugin.SetAutoparticipateConnections(true);

		m_NetInterface->AttachPlugin(&m_ConnectionGraphPlugin);
		m_NetInterface->AttachPlugin(&m_FullyConnectedMeshPlugin);
		m_NetInterface->AttachPlugin(&m_PeerIndexPlugin);

		m_NetInterface->SetMaximumIncomingConnections(s_MaxPeers);
	}

	RakNetwork::~RakNetwork()
	{
		RakPeerInterface::DestroyInstance(m_NetInterface);
	}

	bool RakNetwork::Startup(unsigned short incommingPort)
	{
		SocketDescriptor socDesc(incommingPort, 0);
		if (m_NetInterface->Startup(s_MaxPeers, &socDesc, 1))
			return true;
		else
			return false;
	}

	bool RakNetwork::Connect(const std::string& host, unsigned short port)
	{
		if (m_NetInterface->Connect(host.c_str(), port, 0, 0) == RakNet::CONNECTION_ATTEMPT_STARTED)
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

	uint8_t RakNetwork::GetPeerIndex() const
	{
		return m_PeerIndexPlugin.GetPeerIndex();
	}

	bool RakNetwork::IsSenior(const RakNetGUID &guid)
	{
		if (guid != UNASSIGNED_RAKNET_GUID)
			return !IsConnected() || guid == GetLocalGUID() || IsSenior(guid, GetLocalGUID());
		else
			return false;
	}

	bool RakNetwork::IsSenior(const RakNetGUID &guidSen, const RakNetGUID &guidJun)
	{
		if (guidSen != guidJun)
		{
			if (guidSen == GetHost())
				return true;
			else if (guidJun == GetHost())
				return false;

			DataStructures::List<RakNetGUID> participants;
			m_FullyConnectedMeshPlugin.GetParticipantList(participants); // This stupid method isn't const, so this overload of IsSenior can't be const :(
			for (size_t i = 0; i < participants.Size(); ++i)
			{
				if (participants.Get(i) == guidSen)
					return true;
				else if (participants.Get(i) == guidJun)
					return false;
			}
			FSN_EXCEPT(InvalidArgumentException, "Unknown participants");
		}
		else
			return true;
	}

	RakNetGUID RakNetwork::GetHost() const
	{
		return m_FullyConnectedMeshPlugin.GetConnectedHost();
	}

	bool RakNetwork::SendAsIs(const NetDestination& destination, const char* data, unsigned int length, PacketPriority priority, PacketReliability reliability, char channel)
	{
		return m_NetInterface->Send(data, length, rakPriority(priority), rakReliability(reliability), channel, destination.GUID, destination.Broadcast) != 0;
	}

	bool RakNetwork::SendAsIs(const NetDestination& destination, const RakNet::BitStream *data, PacketPriority priority, PacketReliability reliability, char channel)
	{
		return m_NetInterface->Send(data, rakPriority(priority), rakReliability(reliability), channel, destination.GUID, destination.Broadcast) != 0;
	}

	bool RakNetwork::Send(const NetDestination &destination, bool timestamped, unsigned char type, char* data, unsigned int length, PacketPriority priority, PacketReliability reliability, char channel)
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
			return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, destination.GUID, destination.Broadcast) != 0;
		else
			return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, GetLocalGUID(), destination.Broadcast) != 0;
	}

	bool RakNetwork::Send(const NetDestination &destination, bool timestamped, unsigned char type, RakNet::BitStream *data, PacketPriority priority, PacketReliability reliability, char channel)
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
			return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, destination.GUID, destination.Broadcast) != 0;
		else
			return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, GetLocalGUID(), destination.Broadcast) != 0;
	}

	Packet *RakNetwork::Receive()
	{
		return m_NetInterface->Receive();
	}

	PacketSpt RakNetwork::ReceivePacketSpt()
	{
		Packet *packet = m_NetInterface->Receive();
		if (packet == nullptr)
			return PacketSpt();

		using namespace std::placeholders;

		return PacketSpt(packet, std::bind(&RakNetwork::DeallocatePacket, this, _1));
	}

	void RakNetwork::PushBackPacket(Packet *packet, bool head)
	{
		m_NetInterface->PushBackPacket(packet, head);
	}

	void RakNetwork::DeallocatePacket(Packet *packet)
	{
		m_NetInterface->DeallocatePacket(packet);
	}

	std::shared_ptr<RakNet::RakNetStatistics> RakNetwork::GetStatistics(const RakNetGUID &guid)
	{
		auto stats = std::make_shared<RakNetStatistics>();
		SystemAddress system = m_NetInterface->GetSystemAddressFromGuid(guid);
		m_NetInterface->GetStatistics(system, stats.get());
		return stats;
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
