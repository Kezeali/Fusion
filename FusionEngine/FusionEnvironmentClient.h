/*
  Copyright (c) 2006 FusionTeam

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
*/

#ifndef Header_FusionEngine_ClientEnvironment
#define Header_FusionEngine_ClientEnvironment

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// STL

/// Inherited
#include "FusionEnvironmentGeneric.h"

/// Fusion
#include "FusionClientOptions.h"

#include "FusionScene.h"
#include "FusionNode.h"

#include "FusionShip.h"
#include "FusionProjectile.h"
#include "FusionLevel.h"

#include "FusionResourceLoader.h"
#include "FusionLevelResource.h"
#include "FusionShipResource.h"
#include "FusionWeaponResource.h"

#include "FusionScriptingEngine.h"
#include "FusionInputHandler.h"
#include "FusionNetworkClient.h"
#include "FusionPhysicsWorld.h"

#include "FusionError.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * The client gameplay environment.
	 *
	 * \todo ClientEnvironment and ServerEnvironment should probably inherit from
	 * a common parent.
	 *
	 * This is where all the gameplay stuff goes on at the client-side.
	 * Moving / drawing ships, syncing with server, prediction, etc. is all controlled
	 * from here. That isn't to say just this one class does all thoes things, this
	 * class just controlls the activities of other, more specific classes which do.
	 *
	 * \see
	 * FusionScene | FusionInput | FusionNetworkingHandler | GenericEnvironment.
	 */
	class ClientEnvironment : public GenericEnvironment
	{
	public:
		//! A list of local player numbers (0 to g_MaxPlayers) 
		typedef std::vector<PlayerInd> LocalPlayerIdList;

	public:
		//! Basic constructor
		ClientEnvironment();
		//! Constructor
		ClientEnvironment(const std::string &hostname, const std::string &port, ClientOptions *options);

		//! Destructor
		~ClientEnvironment();

	public:
		//! Pulls the resources from the ResourceLoader and some other stuff
		bool Initialise();

		//! Runs and maintains the statemanager and states
		bool Update(unsigned int split);

		//! Draws stuff
		void Draw();

		//! Cleans up, we assume
		void CleanUp();

		//! Creates a new ship defined by the given state
		void CreateShip(const ShipState &state);

	private:
		//! The address of the server
		std::string m_Hostname;
		//! The port to use
		std::string m_Port;

		//! Options
		ClientOptions *m_Options;
		//! NetMan
		FusionNetworkClient *m_NetworkManager;

		//! The maximum health as set by the server
		int m_MaxHealth;

		//! Player IDs of the local players
		LocalPlayerIdList m_PlayerIDs;

		//! Implimentation of GeneircEnvironment#Send
		void send();
		//! Implimentation of GeneircEnvironment#Receive
		bool receive();

		//! Updates the input structures of all local ships.
		void gatherLocalInput();

	};

}

#endif
