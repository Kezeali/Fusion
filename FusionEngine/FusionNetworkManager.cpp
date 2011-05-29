/*
*  Copyright (c) 2010 Fusion Project Team
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

#include "FusionNetworkManager.h"

#include "FusionNetDestinationHelpers.h"
#include "FusionPacketDispatcher.h"
#include "FusionRakNetwork.h"

#include <BitStream.h>

namespace FusionEngine
{

	void PeerIDManager::HandlePacket(Packet *packet)
	{
		RakNet::BitStream bs(packet->data, packet->length, false);
		MessageID mtid;
		bs.Read(mtid);

		switch (mtid)
		{
		case ID_NEW_INCOMING_CONNECTION:
			{
				RakNet::BitStream outgoing;
				// the host will always have an up-to-date id pool, since it is always the oldest peer (it will have
				//  received an MTID_SET_PEER_ID for each peer currently connected, if it didn't allocate them itself)
				uint8_t id = m_UnusedIDs.getFreeID(); 
				outgoing.Write(packet->guid);
				outgoing.Write(id);
				m_Network->Send(To::Populace(), false, MTID_SET_PEER_ID, &outgoing, MEDIUM_PRIORITY, RELIABLE, CID_SYSTEM);
			}
			break;
		case MTID_SET_PEER_ID:
			{
				RakNetGUID guid;
				bs.Read(guid);
				uint8_t id;
				bs.Read(id);
				m_UnusedIDs.takeID(id);
				if (guid == m_Network->GetLocalGUID())
					m_PeerID = id;
			}
			break;
		}
	}

	ElectionPacketHandler::ElectionPacketHandler()
	{
	}

	const RakNetGUID &ElectionPacketHandler::GetArbitratorGUID() const
	{
		return m_ArbitratorGUID;
	}

	void ElectionPacketHandler::HandlePacket(Packet *packet)
	{
		EasyPacket *easyPacket = (EasyPacket*)packet;
		FSN_ASSERT(easyPacket->GetType() == ID_FCM2_NEW_HOST);
		m_ArbitratorGUID = easyPacket->GetGUID();
	}

	NetworkManager::NetworkManager(RakNetwork *network, PacketDispatcher *dispatcher)
		: m_Network(network),
		m_Dispatcher(dispatcher),
		m_PeerIDManager(m_Network)
	{
		m_ArbitratorElector.m_ArbitratorGUID = network->GetLocalGUID();
		m_Dispatcher->Subscribe(ID_FCM2_NEW_HOST, &m_ArbitratorElector);

		m_Dispatcher->Subscribe(ID_NEW_INCOMING_CONNECTION, &m_PeerIDManager);
		m_Dispatcher->Subscribe(MTID_SET_PEER_ID, &m_PeerIDManager);
	}

	NetworkManager::~NetworkManager()
	{
		m_Dispatcher->Unsubscribe(ID_FCM2_NEW_HOST, &m_ArbitratorElector);

		m_Dispatcher->Unsubscribe(ID_NEW_INCOMING_CONNECTION, &m_PeerIDManager);
		m_Dispatcher->Unsubscribe(MTID_SET_PEER_ID, &m_PeerIDManager);
	}

	const RakNetGUID &NetworkManager::GetArbitratorGUID()
	{
		return getSingleton().m_ArbitratorElector.GetArbitratorGUID();
	}

	bool NetworkManager::ArbitratorIsLocal()
	{
		return getSingleton().m_Network->GetLocalGUID() == GetArbitratorGUID();
	}

	uint8_t NetworkManager::GetPeerSeniorityIndex()
	{
		return getSingleton().m_Network->GetPeerIndex();
	}

	bool NetworkManager::IsSenior(const RakNetGUID &peer)
	{
		return getSingleton().m_Network->IsSenior(peer);
	}

	uint8_t NetworkManager::GetPeerID()
	{
		return getSingleton().m_PeerIDManager.m_PeerID;
	}

	RakNetwork * const NetworkManager::GetNetwork()
	{
		return getSingleton().m_Network;
	}

	void NetworkManager::Subscribe(unsigned char type, PacketHandler *handler)
	{
		return m_Dispatcher->Subscribe(type, handler);
	}

	void NetworkManager::Unsubscribe(unsigned char type, PacketHandler *handler)
	{
		return m_Dispatcher->Unsubscribe(type, handler);
	}

	void NetworkManager::DispatchPackets()
	{
		FSN_ASSERT(m_Network != nullptr);
		FSN_ASSERT(m_Dispatcher != nullptr);
		// I like the following line for it's poetical qualities
		m_Dispatcher->Dispatch(m_Network);
	}

	void NetworkManager::RequestStepControl()
	{
		m_Network->Send(To::Arbiter(), !Timestamped, MTID_REQUESTSTEPCONTROL, nullptr, HIGH_PRIORITY, RELIABLE, 0);
	}

}
