/*
  Copyright (c) 2006-2007 Fusion Project Team

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

#ifndef Header_FusionEngine_ServerEnvironment
#define Header_FusionEngine_ServerEnvironment

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// STL

/// Fusion
#include "FusionEnvironmentGeneric.h"

#include "FusionServerOptions.h"
#include "FusionScene.h"
#include "FusionNode.h"
#include "FusionShip.h"
#include "FusionProjectile.h"
#include "FusionResourceManager.h"
#include "FusionShipResourceBundle.h"
#include "FusionInputHandler.h"
#include "FusionNetworkServer.h"
#include "FusionPhysicsWorld.h"
#include "FusionException.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * The server gameplay environment.
	 *
	 * This is where all the gameplay stuff goes on at the server-side.
	 * Moving ships, syncing clients, running AI, etc. are all controlled
	 * from here. That isn't to say just this one class does all thoes things, this
	 * class just controlls the activities of other, more specific classes which do.
	 *
	 * \remarks
	 * State is non-blocking
	 *
	 * \see
	 * FusionScene | FusionInput | FusionNetworkingHandler.
	 */
	class ServerEnvironment : public Environment
	{
	public:
		//! Basic Constructor
		ServerEnvironment() {}
		//! Starts the server
		ServerEnvironment(const std::string &port, ServerOptions *options);

		//! Destructor
		~ServerEnvironment();

	public:
		//! Init
		bool Initialise();

		/*!
		 * \brief
		 * Does everything.
		 * The main game loop calls this
     * before it draws each frame. Every
     * thing gets updated here, and data
     * gets read from the things that
     * update themselves (InputManager
     * and NetworkManager.)
		 */
		bool Update(unsigned int split);

		//! Draws
		void Draw();

		//! Cleans up, we assume
		void CleanUp();

		//! Creates a new ship defined by the given state
		void CreateShip(const ShipState &state);

	protected:
		//! The port to use
		std::string m_Port;

		//! NetMan
		FusionNetworkServer *m_NetworkManager;

		//! Options
		ServerOptions *m_Options;

		unsigned int m_MessageDelay;


		////! Ship List
		//ShipList m_Ships;
		////! Projectiles
		//ProjectileList m_Projectiles;

	protected:
		//! Send all packets
		void send();
		//! Receive all packets
		bool receive();

		/*!
		 * \brief
		 * Gets the next available PID.
		 *
		 * This gets a new player ID, which can be assigned to a new player.
		 */
		ObjectID getNextPID();

		/*!
		 * \brief
		 * Gets the next available OID.
		 *
		 * This gets a new object ID, which can be assigned to a new player.
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

		/*!
		 * \brief
		 * Updates the positions of all syncronised objects.
		 *
		 * This tells each ship to update its
     * node's position (the node is the thing
     * that handles drawing, it's seperate
     * form the rest of the ship 'cause that
     * makes it more generic.)
		 *
		 * \remarks
		 * Me - don't ask me why this description is so different to the
		 * ClientEnvironment one :P it was C&P'ed from an IM convo between me
		 * and Bruski...
		 */
		//void updateAllPositions(unsigned int split);
	};

}

#endif