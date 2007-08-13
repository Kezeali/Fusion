/*
  Copyright (c) 2006 Fusion Project Team

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

#include "FusionCommon.h"

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

#include "FusionResourceManager.h"
#include "FusionLevelResourceBundle.h"
#include "FusionShipResourceBundle.h"
#include "FusionWeaponResourceBundle.h"

#include "FusionScriptingEngine.h"
#include "FusionInputHandler.h"
#include "FusionNetworkClient.h"
#include "FusionPhysicsWorld.h"

#include "FusionException.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * The client gameplay environment.
	 *
	 * This is where all the gameplay stuff goes on at the client-side.
	 * Moving / drawing ships, syncing with server, prediction, etc. is all controlled
	 * from here. That isn't to say just this one class does all thoes things, this
	 * class just controlls the activities of other, more specific classes which do.
	 *
	 * \remarks
	 * State is non-blocking
	 *
	 * \see
	 * FusionScene | FusionInput | ResourceLoader | FusionNetworkGeneral | GenericEnvironment | ClientOptions.
	 */
	class ClientEnvironment : public GenericEnvironment
	{
	public:
		//! A list of local player IDs (0 to g_MaxPlayers) 
		typedef std::vector<ObjectID> LocalSystemAddressList;

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
		LocalSystemAddressList m_SystemAddresses;

	private:
		//! Implimentation of GeneircEnvironment#Send
		void send();
		//! Implimentation of GeneircEnvironment#Receive
		bool receive();

		//! Updates the input structures of all local ships.
		void gatherLocalInput();

		/*!
		 * \brief
		 * Gets a local PID
		 *
		 * This gets a new player ID, which should be replaced by a valid one from the
		 * server.
		 */
		ObjectID getNextPID();

		/*!
		 * \brief
		 * Gets a local OID
		 *
		 * This gets a new object ID, which should be replaced by a valid one from the
		 * server.
		 */
		ObjectID getNextOID();

		/*!
		 * \brief
		 * Gets the next available spawn position from the map.
		 *
		 * This will attempt to find a spawn position with no players nearby. If none
		 * can be found, the "next" spawn will be used (next ID in the list).
		 */
		ShipState getSpawnState();

	};

}

#endif
