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

/// STL

/// Fusion
#include "FusionScene.h"
#include "FusionNode.h"
#include "FusionResourceLoader.h"
#include "FusionShipResource.h"
#include "FusionNetworkClient.h"
#include "FusionPhysicsWorld.h"
#include "FusionErrorTypes.h"

namespace FusionEngine
{

	//! The virtual environment! (pun?)
	//! Now this' a FusionState; whatchu gonna do about it?
	class GenericEnvironment : public FusionState
	{
	public:
		//! Basic Constructor
		GenericEnvironment();
		//! Virtual destructor
		virtual ~GenericEnvironment();

	public:
		//! A list of ships
		typedef std::vector<FusionClientShip*> ShipList;
		//! A list of projectiles
		typedef std::vector<FusionProjectile*> ProjectileList;
		//! A list of resources
		typedef std::map<std::string, ShipResource*> ShipResourceMap;

		//! A list of inputs
		typedef std::vector<ShipInput> ShipInputList;

	public:
		//! Pulls the resources from the ResourceLoader
		bool Initialise(ResourceLoader *resources);

		/*!
		 * \brief
		 * Runs the statemanager.
		 */
		bool Update(unsigned int split);

		//! Draws stuff
		void Draw();

		//! Called by FusionShipDrawable#Draw() to get the sprite, etc.
		ShipResource *GetShipResourceByID(const std::string &id);

		//! Leaves the client environment cleanly.
		/*!
		 * \param message The explaination to give to the user
		 */
		void _quit(ErrorType type);

	protected:
		//! If this is set to true, the update command will return false next time it runs
		//!  (thus quitting the gameplay.)
		bool m_Quit;

		//! Number of players in the env
		unsigned int m_NumPlayers;

		//! SceneGraph
		FusionScene *m_Scene;
		//! Options (controlls, etc.)
		ClientOptions *m_Options;
		//! High level input manager
		FusionInput *m_InputManager;
		//! High level network manager
		FusionNetworkClient *m_NetworkManager;
		//! High level physics manager
		FusionPhysicsWorld *m_PhysicsWorld;

		//! Thread from which the network manager works
		/*!
		 * \remarks
		 * Having the FusionNetworkClient object running in another
		 * thread removes the nessescity to limit it's working time
		 * per step (that is to say, it can take as long as it needs
		 * process every packet, as it won't hold up the redraw.)
		 * This may turn out to be more of a performance hit than just
		 * limiting it's working time, but it's easyer to remove code
		 * than write it in later :P
		 */
		CL_Thread *m_NetManThread;

		//! List of ships currently in the environment
		ShipList m_Ships;
		//! List of Projectiles currently in the environment
		ProjectileList m_Projectiles;

		//! List of ship resources in loaded.
		/*!
		 * Maps ships to shipnames
		 */
		ShipResourceMap m_ShipResources;

		//! Send all packets
		/*!
		 * "Sending" can be done here (ofcourse, FusionNetwork will have done the real
		 * receiving, this just handles the data from it.)
		 */
		void send();
		//! Receive all packets
		bool receive();

		//! Takes a received message, extracts the ShipState, and puts it into the relavant ship
		//! \todo Maybe this should be in a helper class? meh, I think that will
		//! overcomplicate things, especially as my current goal is "just make it compile"!
		void installShipFrameFromMessage(FusionMessage *m);
		//! Extracts the InputState, and puts it into the relavant ship.
		void installShipInputFromMessage(FusionMessage *m);

		//! Takes a received message, extracts the ProjectileState, and puts it into the relavant proj.
		void installProjectileFrameFromMessage(FusionMessage *m);

		//! Updates the input structures of all local ships.
		void gatherLocalInput();
		/*!
		 * [depreciated] by installShipFrameFromMessage()
		 * Updates the state structures of local and remote ships.
		 */
		void updateShipStates();
		/*!
		 * \brief
		 * Updates the positions of all syncronised objects.
		 *
		 * FusionPhysics methods should be called here.
		 * Provides predictive movement based on current velocity etc.
		 */
		void updateAllPositions(unsigned int split);
	};

}

#endif
