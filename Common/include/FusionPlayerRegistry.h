/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#ifndef H_FusionPlayerRegistry
#define H_FusionPlayerRegistry

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSingleton.h"

#include "FusionTypes.h"

#include <RakNetTypes.h>
#include <boost/signals2.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <unordered_map>

namespace FusionEngine
{

	//! Max local (split-screen, hotseat, etc.) players per client
	/*
	* This is primarily used to define the size of constant size arrays but 
	* is also used for checking player numbers given by config files, etc.
	*/
	const unsigned int s_MaxLocalPlayers = 16;

	struct PlayerInfo
	{
		PlayerID NetID;
		unsigned int LocalIndex;
		RakNet::RakNetGUID GUID;
		PlayerInfo()
			: NetID(0),
			LocalIndex(s_MaxLocalPlayers)
		{}

		bool IsLocal() const { return LocalIndex < s_MaxLocalPlayers; }

		bool operator==(const PlayerInfo &other) const;
		bool operator!=(const PlayerInfo &other) const;
	};

	template <typename MapType>
	struct GetValue : public std::unary_function<typename MapType::value_type, typename MapType::mapped_type>
	{
		typedef typename std::unary_function<typename MapType::value_type, typename MapType::mapped_type>::result_type result_type;
		typedef typename std::unary_function<typename MapType::value_type, typename MapType::mapped_type>::argument_type argument_type;
		const result_type& operator() (const argument_type& p) const { return p.second; }
	};

	//! Singleton registry of player IDs and network addresses
	/*!
	* Only stores player info, maintenance is done primarily by
	* PlayerManager, which registers new players as they join the network
	* and allows the local peer to request new players.
	*/
	class PlayerRegistry : public Singleton<PlayerRegistry>
	{
	public:
		PlayerRegistry();

		typedef boost::signals2::signal<void (const PlayerInfo &)> RegistryChangedSigType;
		RegistryChangedSigType SignalPlayerAdded;
		RegistryChangedSigType SignalPlayerRemoved;
		//RegistryChangedSigType SignalPlayerRestored;

		static boost::signals2::connection ConnectToPlayerAdded(RegistryChangedSigType::slot_type callback);
		static boost::signals2::connection ConnectToPlayerRemoved(RegistryChangedSigType::slot_type callback);
		//static boost::signals2::connection ConnectToPlayerRestored(RegistryChangedSigType::slot_type &callback);

		//! Adds a new local player entry to the registry
		static void AddLocalPlayer(PlayerID net_id, unsigned int local_index);
		//! Adds a new remote player entry to the registry
		static void AddRemotePlayer(PlayerID net_id, const RakNet::RakNetGUID& guid);

		//! Removes the player with the given net ID - could be a local player, could be remote
		static void RemovePlayer(PlayerID id);
		//! Removes the local player with the given index
		static void RemoveLocalPlayer(unsigned int local_index);
		//! Removes all players who are from the system indicated by the given GUID
		static void RemovePlayersFrom(const RakNet::RakNetGUID& guid);

		static void Clear();

		//! Returns the number of players in the registry (including remote players)
		static unsigned int GetPlayerCount();
		//! Returns the number of players on the local peer
		static unsigned int GetLocalPlayerCount();

		static const PlayerInfo &GetPlayer(PlayerID id);
		static const PlayerInfo &GetPlayerByLocalIndex(unsigned int index);

		static std::vector<PlayerInfo> GetPlayersBySystem(const RakNet::RakNetGUID& system_address);

		static bool IsLocal(PlayerID net_index);

	private:
		typedef std::unordered_map<PlayerID, PlayerInfo> PlayersByNetIndexMap;
		typedef std::unordered_map<unsigned int, PlayerInfo> PlayersByLocalIndexMap;

	public:
		typedef GetValue<PlayersByNetIndexMap> ByNetIndexTransformFn;
		typedef boost::transform_iterator<ByNetIndexTransformFn, PlayersByNetIndexMap::const_iterator> const_player_iterator;
		typedef boost::transform_iterator<GetValue<PlayersByLocalIndexMap>, PlayersByLocalIndexMap::const_iterator> const_local_player_iterator;

		static const_player_iterator PlayersBegin();
		static const_player_iterator PlayersEnd();

		static const_local_player_iterator LocalPlayersBegin();
		static const_local_player_iterator LocalPlayersEnd();

	protected:
		PlayersByNetIndexMap m_ByNetID;
		PlayersByLocalIndexMap m_ByLocalIndex;

		//RakNet::RakNetGUID m_LocalGUID;

		PlayerInfo m_NoSuchPlayer;

		void addPlayer(PlayerID net_id, unsigned int local_index, const RakNet::RakNetGUID& guid);

		void removePlayer(PlayerID id);
		void removeLocalPlayer(unsigned int local_index);
		void removePlayersFrom(const RakNet::RakNetGUID& guid);

		void clear();

		unsigned int getPlayerCount() const;
		unsigned int getLocalPlayerCount() const;

		const PlayerInfo &getPlayerByNetID(PlayerID id) const;
		const PlayerInfo &getPlayerByLocalIndex(unsigned int index) const;
		std::vector<PlayerInfo> getPlayersBySystem(const RakNet::RakNetGUID& guid) const;
		
	};

}

#endif
