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

#include "FusionPlayerRegistry.h"

namespace FusionEngine
{

	PlayerManager::PlayerManager()
		: m_LocalPlayerCount(0)
	{
	}

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

	void PlayerManager::HandlePacket(IPacket *packet)
	{
		RakNet::BitStream bitStream(packet->GetData(), packet->GetLength(), true);

		RakNetwork *network = m_Network;
		NetHandle originHandle = packet->GetSystemHandle();

		if (packet->GetType() == MTID_ADDPLAYER)
		{
			if (PlayerRegistry::ArbitratorIsLocal())
			{
				unsigned int playerIndex;
				{
					RakNet::BitStream bitStream(packet->GetData(), packet->GetLength(), false);
					bitStream.Read(playerIndex);
				}

				ObjectID netIndex = m_UnusedNetIds.getFreeID();
				PlayerRegistry::AddPlayer(netIndex, originHandle);

				RakNetHandle rakHandle = std::tr1::dynamic_pointer_cast<RakNetHandleImpl>(originHandle);
				{
					RakNet::BitStream bitStream;
					bitStream.Write1();
					bitStream.Write(netIndex);
					bitStream.Write(playerIndex);
					network->Send(
						false,
						MTID_ADDPLAYER, &bitStream,
						MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM,
						originHandle);
				}
				{
					SystemAddress systemAddress = rakHandle->Address;
					RakNetGUID guid = rakHandle->SystemIdent;

					RakNet::BitStream bitStream;
					bitStream.Write0();
					bitStream.Write(netIndex);
					bitStream.Write(systemAddress);
					bitStream.Write(guid);
					network->Send(
						false,
						MTID_ADDPLAYER, &bitStream,
						MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM,
						originHandle, true);
				}
			}
			else
			{
				RakNet::BitStream bitStream((unsigned char*)packet->GetData(), packet->GetLength(), false);

				bool localPlayer = bitStream.ReadBit();
				ObjectID netIndex;
				bitStream.Read(netIndex);

				if (localPlayer)
				{
					unsigned int localIndex;
					bitStream.Read(localIndex);
					PlayerRegistry::AddPlayer(netIndex, localIndex);
				}
				else
				{
					RakNetGUID guid;
					SystemAddress address;
					bitStream.Read(address);
					bitStream.Read(guid);

					RakNetHandle handle(new RakNetHandleImpl(guid, address));
					PlayerRegistry::AddPlayer(netIndex, handle);
				}
			}
		}

		else if (packet->GetType() == MTID_REMOVEPLAYER)
		{
			RakNet::BitStream bitStream((unsigned char*)packet->GetData(), packet->GetLength(), false);

			ObjectID netIndex;
			bitStream.Read(netIndex);

			if (PlayerRegistry::ArbitratorIsLocal())
			{
				PlayerRegistry::RemovePlayer(netIndex);
				releasePlayerIndex(netIndex);

				//network->Send(
				//	false,
				//	MTID_REMOVEPLAYER, bitStream.GetData(), bitStream.GetNumberOfBytesUsed(),
				//	MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM,
				//	originHandle, true);
			}
			else
			{
				PlayerRegistry::RemovePlayer(netIndex);
			}
		}
	}

	void PlayerManager::requestNewPlayer(unsigned int player_index)
	{
		RakNetwork *network = m_Network;
		if (PlayerRegistry::ArbitratorIsLocal())
		{
			createNewPlayer(player_index);
		}
		else
		{
			// Request a Net-Index if this peer isn't the arbitrator
			RakNet::BitStream bitStream;
			bitStream.Write(player_index);

			network->Send(
				false,
				MTID_ADDPLAYER, &bitStream,
				MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM,
				PlayerRegistry::GetArbitratingPlayer().System);
		}
	}

	void PlayerManager::createNewPlayer(unsigned int player_index)
	{
		ObjectID netId = m_UnusedNetIds.getFreeID();

		// Add the player
		PlayerRegistry::AddPlayer(netId, player_index);

		RakNet::BitStream bitStream;
		bitStream.Write0();
		bitStream.Write(netId);

		// Notify other systems 
		m_Network->Send(
			false,
			MTID_ADDPLAYER, &bitStream,
			MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM, NetHandle(), true);
	}

}
