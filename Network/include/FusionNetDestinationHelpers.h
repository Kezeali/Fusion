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

#ifndef H_FusionNetDestinations
#define H_FusionNetDestinations

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"
#include "FusionCommon.h"

#include "FusionNetworkManager.h"
#include "FusionPlayerRegistry.h"
#include "FusionRakNetwork.h"

namespace FusionEngine
{

	//! Oh, I don't know.
	namespace Dear
	{

		//! Dearest Player, please create a new entity with the following qualities...
		static NetDestination Player(PlayerID id)
		{
			NetDestination destination;

			const PlayerInfo &playerInfo = PlayerRegistry::GetPlayer(id);
			destination.GUID = playerInfo.GUID;

			return destination;
		}

		//! Dearest Arbitrator, can you use your eternal wisdom to grant me a new player?
		static NetDestination Arbiter()
		{
			NetDestination destination;

			destination.GUID = NetworkManager::GetArbitratorGUID();

			return destination;
		}

		//! My fellow RakPeerInterfaces: it is time to address the state of our world...
		static NetDestination Populace()
		{
			NetDestination destination;

			destination.Broadcast = true;

			return destination;
		}

		//! Hey, don't tell player_id I told you this, but...
		static NetDestination OtherThan(PlayerID excluded_id)
		{
			NetDestination destination;

			const PlayerInfo &playerInfo = PlayerRegistry::GetPlayer(excluded_id);
			destination.GUID = playerInfo.GUID;
			destination.Broadcast = true;

			return destination;
		}

	}

	//! For better gram'ma - e.g. Send(To::Player(1), ...);
	namespace To = Dear;

}

#endif