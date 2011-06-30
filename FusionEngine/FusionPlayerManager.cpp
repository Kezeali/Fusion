/*
* Copyright (c) 2010 Fusion Project Team
*
* This software is provided 'as-is', without any express or implied warranty.
* In noevent will the authors be held liable for any damages arising from the
* use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not
*   claim that you wrote the original software. If you use this software in a
*   product, an acknowledgment in the product documentation would be
*   appreciated but is not required.
*
*   2. Altered source versions must be plainly marked as such, and must not
*   be misrepresented as being the original software.
*
*   3. This notice may not be removed or altered from any source distribution.
*
*
* File Author(s):
*
*   Elliot Hayward
*/

#include "FusionStableHeaders.h"

#include "FusionPlayerManager.h"

#include <BitStream.h>

#include "FusionNetDestinationHelpers.h"
#include "FusionNetworkManager.h"
#include "FusionPlayerRegistry.h"

using namespace RakNet;

namespace FusionEngine
{

	PlayerManager::PlayerManager()
		: m_LocalPlayerCount(0),
		m_NextNetId(1)
	{
		m_Network = NetworkManager::GetNetwork();
		FSN_ASSERT(m_Network != nullptr);
	}

	//PlayerManager::PlayerManager(RakNetwork* network)
	//	: m_Network(network),
	//	m_LocalPlayerCount(0),
	//	m_NextNetId(1)
	//{
	//}

	unsigned int PlayerManager::RequestNewPlayer()
	{
		unsigned int playerIndex = m_UnusedLocalIndicies.getFreeID();
		requestNewPlayer(playerIndex);
		return playerIndex;
	}

	bool PlayerManager::RequestNewPlayer(unsigned int player_index)
	{
		if (m_UnusedLocalIndicies.takeID(player_index))
		{
			requestNewPlayer(player_index);
			return true;
		}
		else
			return false;
	}

	void PlayerManager::RemovePlayer(unsigned int index)
	{
		RakNet::BitStream removePlayerNotification;
		PlayerID netId = PlayerRegistry::GetPlayerByLocalIndex(index).NetID;
		removePlayerNotification.Write(netId);

		m_Network->Send(
			To::Populace(), // Broadcast
			!Timestamped,
			MTID_REMOVEPLAYER, &removePlayerNotification,
			MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM);
	}

	void PlayerManager::HandlePacket(RakNet::Packet *packet)
	{
		RakNet::BitStream receivedData(packet->data, packet->length, false);

		RakNetwork *network = m_Network;
		RakNetGUID remotePeerGUID = packet->guid;

		unsigned char type;
		receivedData.Read(type);
		if (type == MTID_ADDPLAYER)
		{
			if (NetworkManager::ArbitratorIsLocal())
			{
				// So we can tell the remote peer what requested player we are adding:
				unsigned int remotePlayerIndex;
				receivedData.Read(remotePlayerIndex);

				PlayerID netId = m_NextNetId++;//m_UnusedNetIds.getFreeID();
				PlayerRegistry::AddRemotePlayer(netId, remotePeerGUID);

				{
					RakNet::BitStream response;
					response.Write1(); // Tell the peer that the player being added is on their system
					response.Write(netId);
					response.Write(remotePlayerIndex);
					network->Send(
						NetDestination(remotePeerGUID, false),
						!Timestamped,
						MTID_ADDPLAYER, &response,
						MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM);
				}
				{
					RakNet::BitStream newPlayerNotification;
					newPlayerNotification.Write0(); // Tell the peer that the player being added is on another system
					newPlayerNotification.Write(netId);
					newPlayerNotification.Write(remotePeerGUID);
					network->Send(
						NetDestination(remotePeerGUID, true), // Broadcast
						!Timestamped,
						MTID_ADDPLAYER, &newPlayerNotification,
						MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM);
				}
			}
			else
			{
				bool localPlayer = receivedData.ReadBit();
				// The net ID the arbiter has assigned to the new player:
				PlayerID netId;
				receivedData.Read(netId);

				if (localPlayer)
				{
					unsigned int localIndex;
					receivedData.Read(localIndex);
					PlayerRegistry::AddLocalPlayer(netId, localIndex);
				}
				else
				{
					RakNetGUID guid;
					receivedData.Read(guid);
					PlayerRegistry::AddRemotePlayer(netId, guid);
				}
			}
		}

		else if (type == MTID_REMOVEPLAYER)
		{
			PlayerID netId;
			receivedData.Read(netId);

			PlayerRegistry::RemovePlayer(netId);
		}
	}

	void PlayerManager::requestNewPlayer(unsigned int player_index)
	{
		RakNetwork *network = m_Network;
		if (NetworkManager::ArbitratorIsLocal())
		{
			createNewPlayer(player_index);
		}
		else
		{
			// Request a Net-Index if this peer isn't the arbitrator
			RakNet::BitStream bitStream;
			bitStream.Write(player_index);

			network->Send(
				To::Arbiter(),
				false,
				MTID_ADDPLAYER, &bitStream,
				MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM);
		}
	}

	void PlayerManager::createNewPlayer(unsigned int player_index)
	{
		PlayerID netId = m_NextNetId++;
		// Add the player
		PlayerRegistry::AddLocalPlayer(netId, player_index);

		RakNet::BitStream bitStream;
		bitStream.Write0();
		bitStream.Write(netId);
		bitStream.Write(m_Network->GetLocalGUID());

		// Notify other systems 
		m_Network->Send(
			To::Populace(),
			false,
			MTID_ADDPLAYER, &bitStream,
			MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM);
	}

}
