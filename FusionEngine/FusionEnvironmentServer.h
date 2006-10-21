/*
  Copyright (c) 2006 Elliot Hayward

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

#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionServerOptions.h"
#include "FusionScene.h"
#include "FusionNode.h"
#include "FusionClientShip.h"
#include "FusionProjectile.h"
#include "FusionResourceLoader.h"
#include "FusionShipResource.h"
#include "FusionInputHandler.h"
#include "FusionNetworkServer.h"
#include "FusionPhysicsWorld.h"
#include "FusionErrorTypes.h"

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
	class ServerEnvironment
	{
	public:
		//! Starts the server
		ServerEnvironment(const std::string &port, ServerOptions *options);

	public:
		//! A list of ships
		typedef std::vector<FusionClientShip*> ShipList;
		//! A list of projectiles
		typedef std::vector<FusionProjectile*> ProjectileList;
		//! A list of resources
		typedef std::map<std::string, ShipResource*> ShipResourceMap;

		//! A list of inputs
		typedef std::vector<ShipInput> ShipInputList;

		//! What the hell is this? I don't remember adding this :P Bah, messy
		int numPlayers;

		//! Init
		bool Initialise(ResourceLoader *resources);

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

	private:
		//! Phys
		FusionPhysicsWorld *m_PhysicsWorld;
		//! Scene
		FusionScene *m_Scene;
		//! Options
		ServerOptions *m_Options;
		//! InputManager
		FusionInput *m_InputManager;
		//! NetMan
		FusionNetworkServer *m_NetworkManager;

		//! Ship List
		ShipList m_Ships;
		//! Projectiles
		ProjectileList m_Projectiles;

		//! ResMap
		ShipResourceMap m_ShipResources;

		/*!
		 * Updates the state structures of all ships.
		 *
		 * This is a method called by Update,
     * it takes the data received from the
     * clients (or for the ClientEnv version,
     * it takes the data from the keyboard)
     * and puts it into FusionShip.shipstate .
		 *
		 * REMEMBER, this method simply takes data received by FusionImput
		 * and FusionNetwork and inserts it into the correct state objects for
		 * each ship; updateAllPositions does all the real work!
		 *
		 * \remarks
		 * MCS - don't ask me why this description is so different to the
		 * ClientEnvironment one :P it was C&P'ed from an IM convo between me
		 * and Bruski...
		 */
		void updateShipStates();
		/*!
		 * \brief
		 * Updates the positions of all syncronised objects.
		 *
		 * This tells each ship to update its
     * node's position (the node is the thing
     * that handles drawing, it's seperate
     * form the rest of the ship 'cause that
     * makes it more generic.)
		 */
		void updateAllPositions(unsigned int split);
	};

}

#endif
