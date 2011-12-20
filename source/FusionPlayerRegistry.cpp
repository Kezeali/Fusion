/*
*  Copyright (c) 2009-2010 Fusion Project Team
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

#include "FusionPlayerRegistry.h"

using namespace RakNet;

namespace FusionEngine
{

	bool PlayerInfo::operator==(const PlayerInfo &other) const
	{
		return NetID == other.NetID;
	}

	bool PlayerInfo::operator!=(const PlayerInfo &other) const
	{
		return !(*this == other);
	}

	boost::signals2::connection PlayerRegistry::ConnectToPlayerAdded(RegistryChangedSigType::slot_type slot)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");

		return registry->SignalPlayerAdded.connect(slot);
	}

	boost::signals2::connection PlayerRegistry::ConnectToPlayerRemoved(RegistryChangedSigType::slot_type slot)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");

		return registry->SignalPlayerRemoved.connect(slot);
	}

	//boost::signals2::connection PlayerRegistry::ConnectToPlayerRestored(RegistryChangedSigType::slot_type &slot)
	//{
	//	PlayerRegistry *registry = getSingletonPtr();
	//	FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");

	//	return registry->SignalPlayerRestored.connect(slot);
	//}

	void PlayerRegistry::AddLocalPlayer(PlayerID id, unsigned int local_index)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->addPlayer(id, local_index, UNASSIGNED_RAKNET_GUID);
	}

	void PlayerRegistry::AddRemotePlayer(PlayerID id, const RakNetGUID& guid)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->addPlayer(id, s_MaxLocalPlayers, guid);
	}

	void PlayerRegistry::RemovePlayer(PlayerID id)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->removePlayer(id);
	}

	void PlayerRegistry::RemoveLocalPlayer(unsigned int local_index)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->removePlayer(local_index);
	}

	void PlayerRegistry::RemovePlayersFrom(const RakNetGUID& guid)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->removePlayersFrom(guid);
	}

	void PlayerRegistry::Clear()
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->clear();
	}

	unsigned int PlayerRegistry::GetPlayerCount()
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getPlayerCount();
	}

	unsigned int PlayerRegistry::GetLocalPlayerCount()
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getLocalPlayerCount();
	}

	const PlayerInfo &PlayerRegistry::GetPlayer(PlayerID id)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getPlayerByNetID(id);
	}

	const PlayerInfo &PlayerRegistry::GetPlayerByLocalIndex(unsigned int index)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getPlayerByLocalIndex(index);
	}

	std::vector<PlayerInfo> PlayerRegistry::GetPlayersBySystem(const RakNetGUID& guid)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getPlayersBySystem(guid);
	}

	bool PlayerRegistry::IsLocal(PlayerID net_index)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getPlayerByNetID(net_index).LocalIndex < s_MaxLocalPlayers;
	}

	PlayerRegistry::const_player_iterator PlayerRegistry::PlayersBegin()
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return const_player_iterator(registry->m_ByNetID.begin());
	}

	PlayerRegistry::const_player_iterator PlayerRegistry::PlayersEnd()
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return const_player_iterator(registry->m_ByNetID.end());
	}

	PlayerRegistry::const_local_player_iterator PlayerRegistry::LocalPlayersBegin()
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return const_local_player_iterator(registry->m_ByLocalIndex.begin());
	}

	PlayerRegistry::const_local_player_iterator PlayerRegistry::LocalPlayersEnd()
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return const_local_player_iterator(registry->m_ByLocalIndex.end());
	}

	PlayerRegistry::PlayerRegistry()
	{
		m_NoSuchPlayer.NetID = 0;
		m_NoSuchPlayer.LocalIndex = s_MaxLocalPlayers;
	}

	void PlayerRegistry::addPlayer(PlayerID net_id, unsigned int local_index, const RakNetGUID& guid)
	{
		//PlayerInfoPtr playerInfo(new PlayerInfo);
		PlayerInfo playerInfo;
		playerInfo.NetID = net_id;
		playerInfo.LocalIndex = local_index;
		playerInfo.GUID = guid;

		m_ByNetID[net_id] = playerInfo;
		if (local_index < s_MaxLocalPlayers)
			m_ByLocalIndex[local_index] = playerInfo;

		SignalPlayerAdded(playerInfo);
	}

	void PlayerRegistry::removePlayer(PlayerID net_index)
	{
		PlayersByNetIndexMap::iterator _where = m_ByNetID.find(net_index);
		m_ByLocalIndex.erase(_where->second.LocalIndex);
		m_ByNetID.erase(_where);
	}

	void PlayerRegistry::removeLocalPlayer(unsigned int local_index)
	{
		PlayersByLocalIndexMap::iterator _where = m_ByLocalIndex.find(local_index);
		m_ByNetID.erase(_where->second.NetID);
		m_ByLocalIndex.erase(_where);
	}

	void PlayerRegistry::removePlayersFrom(const RakNetGUID& guid)
	{
		for (PlayersByNetIndexMap::iterator it = m_ByNetID.begin(), end = m_ByNetID.end(); it != end; ++it)
		{
			PlayerInfo &playerInfo = it->second;
			if (playerInfo.GUID == guid)
			{
				if (playerInfo.LocalIndex < s_MaxLocalPlayers)
					m_ByLocalIndex.erase(playerInfo.LocalIndex);
				m_ByNetID.erase(playerInfo.NetID);
			}
		}
	}

	void PlayerRegistry::clear()
	{
		m_ByLocalIndex.clear();
		m_ByNetID.clear();
	}

	unsigned int PlayerRegistry::getPlayerCount() const
	{
		return m_ByNetID.size();
	}

	unsigned int PlayerRegistry::getLocalPlayerCount() const
	{
		return m_ByLocalIndex.size();
	}

	const PlayerInfo &PlayerRegistry::getPlayerByNetID(PlayerID index) const
	{
		PlayersByNetIndexMap::const_iterator _where = m_ByNetID.find(index);
		if (_where != m_ByNetID.end())
			return _where->second;
		else
			return m_NoSuchPlayer;
	}

	const PlayerInfo &PlayerRegistry::getPlayerByLocalIndex(unsigned int index) const
	{
		PlayersByLocalIndexMap::const_iterator _where = m_ByLocalIndex.find(index);
		if (_where != m_ByLocalIndex.end())
			return _where->second;
		else
			return m_NoSuchPlayer;
	}

	std::vector<PlayerInfo> PlayerRegistry::getPlayersBySystem(const RakNetGUID& guid) const
	{
		std::vector<PlayerInfo> players;

		for (PlayersByNetIndexMap::const_iterator it = m_ByNetID.begin(), end = m_ByNetID.end(); it != end; ++it)
		{
			if (it->second.GUID == guid)
				players.push_back(it->second);
		}
		
		return players;
	}

}
