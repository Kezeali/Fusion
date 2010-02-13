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

#ifndef Header_FusionEngine_PlayerRegistry
#define Header_FusionEngine_PlayerRegistry

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionSingleton.h"

#include "FusionNetwork.h"

#include <boost/signals2.hpp>


namespace FusionEngine
{

	//! Singleton registry of player IDs and network addresses
	class PlayerRegistry : public Singleton<PlayerRegistry>
	{
	public:
		struct PlayerInfo
		{
			ObjectID NetIndex;
			unsigned int LocalIndex;
			NetHandle System;
			bool IsInGame;
			PlayerInfo()
				: NetIndex(0),
				LocalIndex(s_MaxLocalPlayers),
				IsInGame(true)
			{}

			bool operator==(const PlayerInfo &other) const;
			bool operator!=(const PlayerInfo &other) const;
		};

		typedef std::tr1::shared_ptr<PlayerInfo> PlayerInfoPtr;

		PlayerRegistry();

		typedef boost::signals2::signal<void (const PlayerInfo &)> RegistryChangedSigType;
		RegistryChangedSigType SignalPlayerAdded;
		RegistryChangedSigType SignalPlayerRemoved;
		RegistryChangedSigType SignalPlayerRestored;

		static boost::signals2::connection ConnectToPlayerAdded(RegistryChangedSigType::slot_type &callback);
		static boost::signals2::connection ConnectToPlayerRemoved(RegistryChangedSigType::slot_type &callback);
		static boost::signals2::connection ConnectToPlayerRestored(RegistryChangedSigType::slot_type &callback);

		static void AddPlayer(ObjectID net_index, unsigned int local_index, NetHandle system_address);
		static void AddPlayer(ObjectID net_index, unsigned int local_index);
		static void AddPlayer(ObjectID net_index, NetHandle system_address);

		static void RemovePlayer(ObjectID net_index);
		static void RemovePlayer(unsigned int local_index);
		static void RemovePlayersFrom(NetHandle system_address);

		static void Clear();

		static const PlayerInfo &GetPlayerByNetIndex(ObjectID index);
		static const PlayerInfo &GetPlayerByLocalIndex(unsigned int index);

		static std::vector<PlayerInfo> GetPlayersBySystem(NetHandle system_address);

		static const void SetArbitrator(ObjectID net_index);

		static const PlayerInfo &GetArbitratingPlayer();

		static bool ArbitratorIsLocal();

		static bool IsLocal(ObjectID net_index);
	protected:
		void addPlayer(ObjectID net_index, unsigned int local_index, NetHandle system_address);

		void removePlayer(ObjectID net_index);
		void removePlayer(unsigned int local_index);
		void removePlayersFrom(NetHandle system_address);

		void clear();

		const PlayerInfo &getPlayerByNetIndex(ObjectID index) const;
		const PlayerInfo &getPlayerByLocalIndex(unsigned int index) const;
		std::vector<PlayerInfo> getPlayersBySystem(NetHandle system_address) const;

		void setArbitrator(ObjectID net_index);
		//const PlayerInfo &getArbitratingPlayer() const;

		typedef std::tr1::unordered_map<ObjectID, PlayerInfo> PlayersByNetIndexMap;
		typedef std::tr1::unordered_map<unsigned int, PlayerInfo> PlayersByLocalIndexMap;
		typedef std::tr1::unordered_multimap<NetHandle, PlayerInfo> PlayersBySystemAddressMap;

		PlayersByNetIndexMap m_ByNetIndex;
		PlayersByLocalIndexMap m_ByLocalIndex;
		PlayersBySystemAddressMap m_BySystem;

		ObjectID m_Arbitrator;

		PlayerInfo m_NoSuchPlayer;
	};

}

#endif