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
#include "FusionResourceLoader.h"
#include "FusionShipResource.h"
#include "FusionInputHandler.h"
#include "FusionNetworkServer.h"
#include "FusionPhysicsWorld.h"
#include "FusionError.h"

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
	 * \see
	 * FusionScene | FusionInput | FusionNetworkingHandler.
	 */
	class ServerEnvironment : public GenericEnvironment
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

	private:
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
		 * Gets the next available spawn position from the map.
		 *
		 * Depending on the map, this may be set for each player
		 * (thus based on the given PID) or it may be random.
		 *
		 * \remarks
		 * An exception will be thrown if the map has no available spawn positions.
		 * Whether adding another player will cause the max-players for this map
		 * to be exceeded should be checked before adding the player.
		 * Obviously this is a non-issue for maps in which the spawn is randomly
		 * selected.
		 */
		ShipState getSpawnState(ObjectID pid);

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
		 * MCS - don't ask me why this description is so different to the
		 * ClientEnvironment one :P it was C&P'ed from an IM convo between me
		 * and Bruski...
		 */
		//void updateAllPositions(unsigned int split);
	};

}

#endif
