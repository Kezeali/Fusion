/*
  Copyright (c) 2009 Fusion Project Team

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

#include "FusionCommon.h"

#include "FusionPlayerRegistry.h"


namespace FusionEngine
{

	void PlayerRegistry::AddPlayer(ObjectID net_index, unsigned int local_index, NetHandle system_address)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->addPlayer(net_index, local_index, system_address);
	}

	void PlayerRegistry::AddPlayer(ObjectID net_index, NetHandle system_address)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->addPlayer(net_index, g_MaxLocalPlayers, system_address);
	}

	void PlayerRegistry::RemovePlayer(ObjectID net_index)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->removePlayer(net_index);
	}

	void PlayerRegistry::RemovePlayer(unsigned int local_index)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->removePlayer(local_index);
	}

	void PlayerRegistry::RemovePlayersFrom(NetHandle system_address)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->removePlayersFrom(system_address);
	}

	const PlayerRegistry::PlayerInfo &PlayerRegistry::GetPlayerByNetIndex(ObjectID index)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getPlayerByNetIndex(index);
	}

	const PlayerRegistry::PlayerInfo &PlayerRegistry::GetPlayerByLocalIndex(unsigned int index)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getPlayerByLocalIndex(index);
	}

	std::vector<PlayerRegistry::PlayerInfo> PlayerRegistry::GetPlayersBySystem(NetHandle system_address)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getPlayersBySystem(system_address);
	}

	const void PlayerRegistry::SetArbitrator(ObjectID net_index)
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		registry->setArbitrator(net_index);
	}

	const PlayerRegistry::PlayerInfo &PlayerRegistry::GetArbitratingPlayer()
	{
		PlayerRegistry *registry = getSingletonPtr();
		FSN_ASSERT_MSG(registry != NULL, "Tried to use un-initialised PlayerRegistry");
		
		return registry->getPlayerByNetIndex(registry->m_Arbitrator);
	}

	PlayerRegistry::PlayerRegistry()
		: m_Arbitrator(0)
	{
		m_NoSuchPlayer.NetIndex = 0;
		m_NoSuchPlayer.LocalIndex = g_MaxLocalPlayers;
	}

	void PlayerRegistry::addPlayer(ObjectID net_index, unsigned int local_index, NetHandle system_address)
	{
		//PlayerInfoPtr playerInfo(new PlayerInfo);
		PlayerInfo playerInfo;
		playerInfo.NetIndex = net_index;
		playerInfo.LocalIndex = local_index;
		playerInfo.System = system_address;

		m_ByNetIndex[net_index] = playerInfo;
		m_ByLocalIndex[local_index] = playerInfo;
	}

	void PlayerRegistry::removePlayer(ObjectID net_index)
	{
		PlayersByNetIndexMap::iterator _where = m_ByNetIndex.find(net_index);
		m_ByLocalIndex.erase(_where->second.LocalIndex);
		m_ByNetIndex.erase(_where);
	}

	void PlayerRegistry::removePlayer(unsigned int local_index)
	{
		PlayersByLocalIndexMap::iterator _where = m_ByLocalIndex.find(local_index);
		m_ByNetIndex.erase(_where->second.NetIndex);
		m_ByLocalIndex.erase(_where);
	}

	void PlayerRegistry::removePlayersFrom(NetHandle system_address)
	{
		for (PlayersByNetIndexMap::iterator it = m_ByNetIndex.begin(), end = m_ByNetIndex.end(); it != end; ++it)
		{
			PlayerInfo &playerInfo = it->second;
			if (playerInfo.System == system_address)
			{
				if (playerInfo.LocalIndex == g_MaxLocalPlayers)
					m_ByLocalIndex.erase(playerInfo.LocalIndex);
				m_ByNetIndex.erase(playerInfo.NetIndex);
			}
		}
	}

	const PlayerRegistry::PlayerInfo &PlayerRegistry::getPlayerByNetIndex(ObjectID index) const
	{
		PlayersByNetIndexMap::const_iterator _where = m_ByNetIndex.find(index);
		if (_where != m_ByNetIndex.end())
			return _where->second;
		else
			return m_NoSuchPlayer;
	}

	const PlayerRegistry::PlayerInfo &PlayerRegistry::getPlayerByLocalIndex(unsigned int index) const
	{
		PlayersByLocalIndexMap::const_iterator _where = m_ByLocalIndex.find(index);
		if (_where != m_ByLocalIndex.end())
			return _where->second;
		else
			return m_NoSuchPlayer;
	}

	std::vector<PlayerRegistry::PlayerInfo> PlayerRegistry::getPlayersBySystem(NetHandle system_address) const
	{
		std::vector<PlayerRegistry::PlayerInfo> players;

		for (PlayersByNetIndexMap::const_iterator it = m_ByNetIndex.begin(), end = m_ByNetIndex.end(); it != end; ++it)
		{
			if (it->second.System == system_address)
				players.push_back(it->second);
		}
		
		return players;
	}

	void PlayerRegistry::setArbitrator(ObjectID net_index)
	{
		m_Arbitrator = net_index;
	}

	//const PlayerRegistry::PlayerInfo &PlayerRegistry::getArbitratingPlayer() const
	//{
	//	PlayersByNetIndexMap::const_iterator _where = m_ByNetIndex.find(m_Arbitrator);
	//	if (_where != m_ByNetIndex.end())
	//		return _where->second;
	//	else
	//		return m_NoSuchPlayer;
	//}

}