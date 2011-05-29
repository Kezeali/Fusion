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

#ifndef H_FusionPlayerManager
#define H_FusionPlayerManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <boost/signals2.hpp>

#include "FusionPacketHandler.h"

#include "FusionIDStack.h"
#include "FusionRakNetwork.h"

namespace FusionEngine
{

	//! Synchronises player IDs across the network
	class PlayerManager : public PacketHandler
	{
	public:
		PlayerManager();
		// TODO: Use this CTOR instead?
		//PlayerManager(RakNetwork* network);

		//! Sends a new player request to the arbitor
		unsigned int RequestNewPlayer();
		//! Sends a new player request to the arbitor
		bool RequestNewPlayer(unsigned int index);

		//! Drops the given player from the game
		void RemovePlayer(unsigned int index);
		//! Drops the given player from the game and destroys its session
		void RemovePlayerPermanently(unsigned int index);

		unsigned int GetLocalPlayerCount() const { return m_LocalPlayerCount; }

		void HandlePacket(Packet *packet);

		boost::signals2::signal<void (unsigned int)> SignalRequestRejected;

	protected:
		unsigned int m_LocalPlayerCount;

		// Used by 'unsigned int RequestNewPlayer()' to get a free index
		IDSet<unsigned int> m_UnusedLocalIndicies;
		IDSet<PlayerID> m_UnusedNetIds;
		PlayerID m_NextNetId;

		RakNetwork *m_Network;

		//! General method used by both public methods of the same name
		/*!
		* This does the actual requesting, but doesn't modify m_UnusedLocalIndicies
		* (that is taken care of by the public RequestNewPlayer methods.)
		*/
		void requestNewPlayer(unsigned int index);
		//! Used by the arbitor to create players
		void createNewPlayer(unsigned int index);
	};

}

#endif
