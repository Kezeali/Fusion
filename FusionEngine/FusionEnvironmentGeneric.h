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

#ifndef Header_FusionEngine_GenericEnvironment
#define Header_FusionEngine_GenericEnvironment

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Inherited
#include "FusionState.h"

/// Fusion
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
#include "FusionNetworkClient.h"
#include "FusionPhysicsWorld.h"

#include "FusionError.h"

namespace FusionEngine
{

	//! The virtual environment! (pun?)
	//! Now this' a FusionState; whatchu gonna do about it?
	class GenericEnvironment : public FusionState
	{
	public:
		//! Basic Constructor
		GenericEnvironment() : m_Abort(false), m_NumPlayers(0) {}
		//! Virtual destructor
		virtual ~GenericEnvironment() {}

	public:
		//! A list of ships
		typedef std::vector<FusionShip*> ShipList;
		//! A list of projectiles
		typedef std::vector<FusionProjectile*> ProjectileList;

		//! Map of ship res. names to ship resources
		typedef std::map<std::string, ShipResource*> ShipResourceMap;
		//! \see ShipResourceMap
		typedef std::map<std::string, WeaponResource*> WeaponResourceMap;

		//! A list of playerID's mapped to resourceID's
		typedef std::map<PlayerInd, std::string> PlayerShipResMap;

		//! A list of inputs
		typedef std::vector<ShipInput> ShipInputList;

	public:
		//! Pulls the resources from the ResourceLoader and some other stuff
		//virtual bool Initialise() = 0;

		/*!
		 * \brief
		 * Runs the statemanager.
		 */
		//virtual bool Update(unsigned int split) = 0;

		//! Draws stuff
		//virtual void Draw() = 0;

		//! Returns the index of the newly created ship
		virtual void CreateShip(const ShipState &state) =0;

		//! Called by FusionShipDrawable#Draw() to get the sprite, etc.
		virtual ShipResource *GetShipResourceByID(const std::string &id);

		//! Leaves the environment cleanly after an error.
		/*!
		 * \remarks
		 * If you wan't to leave the env without an error, use the state
		 * message system to remove the env state.
		 *
		 * \param[in] e The explaination to give to the user
		 */
		void _error(Error *e);

	protected:
		//! If this is set to true, the update command will return false next time it runs
		//!  (thus quitting the gameplay.)
		bool m_Abort;
		
		//! Number of players in the game (total, ClientOptions#NumPlayers is local only)
		unsigned int m_NumPlayers;

		//! SceneGraph
		FusionScene *m_Scene;
		//! High level network manager
		FusionNetworkClient *m_NetworkManager;
		//! High level physics manager
		FusionPhysicsWorld *m_PhysicsWorld;

		//! List of ships currently in the environment
		ShipList m_Ships;
		//! List of Projectiles currently in the environment
		ProjectileList m_Projectiles;

		//! List of ship resources in loaded.
		/*!
		 * Maps ships to shipnames
		 */
		ShipResourceMap m_ShipResources;

		//! Map of players to their associated ship resource
		PlayerShipResMap m_PlayerResourceIds;

		//! Send all packets
		virtual void send() = 0;
		//! Receive all packets
		/*!
		 * "Receiving" can be done here - of-course, FusionNetwork will have done the real
		 * receiving, this just handles the data from it.
		 */
		virtual bool receive() = 0;

		//! Takes a received message, extracts the ShipState, and puts it into the relavant ship
		//! \todo Maybe this should be in a helper class? meh, I think that will
		//! overcomplicate things, especially as my current goal is "just make it compile"!
		virtual void installShipFrameFromMessage(FusionMessage *m);
		//! Extracts the InputState, and puts it into the relavant ship.
		virtual void installShipInputFromMessage(FusionMessage *m);

		//! Takes a received message, extracts the ProjectileState, and puts it into the relavant proj.
		virtual void installProjectileFrameFromMessage(FusionMessage *m);

		/*!
		 * \brief
		 * Updates the positions of all syncronised objects.
		 *
		 * FusionPhysics methods should be called here.
		 * Provides predictive movement based on current velocity etc.
		 */
		void updateAllPositions(unsigned int split);

		/*!
		 * \brief
		 * Updates the scene graph. ie. tells all ships to call UpdateNode();
		 * so they draw in the right place.
		 *
		 * \remarks
		 * This is now depreciated, as FusionShip#SetPosition does this.
		 * BTW, set position and set accel. / force should remain seperate
		 * as you might only have a few of them changing sometimes (efficiancy.)
		 */
		//void updateSceneGraph();

	};

}

#endif
