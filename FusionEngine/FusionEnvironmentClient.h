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

#ifndef Header_FusionEngine_ClientEnvironment
#define Header_FusionEngine_ClientEnvironment

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionClientOptions.h"
#include "FusionScene.h"
#include "FusionNode.h"
#include "FusionClientShip.h"
#include "FusionProjectile.h"
#include "FusionResourceLoader.h"
#include "FusionShipResource.h"
#include "FusionInputHandler.h"
#include "FusionNetworkClient.h"
#include "FusionPhysicsWorld.h"
#include "FusionErrorTypes.h"

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
	 * FusionScene | FusionInput | FusionNetworkingHandler.
	 */
	class ClientEnvironment
	{
	public:
		//! Constructor
		ClientEnvironment();
		//! Constructor
		ClientEnvironment(const std::string &hostname, const std::string &port, ClientOptions *options);

		//! Destructor
		~ClientEnvironment();

	public:
		//! Pulls the resources from the ResourceLoader
		bool Initialise(ResourceLoader *resources);

		/*!
		 * \brief
		 * Does everything.
		 *
		 * Actually, this function just does the following:
		 * -# Queries FusionInput for input data
		 * -# Queries FusionNetwork for new messages.
		 * -# Updates the state of all objects based on any frames received. It does this by
		 *    finding the time of the received frame in the history, then interpolating
		 *    between the closest stored frames to that time, to check if the local movements
		 *    match up with the remote ones. If they don't, the gameplay is run through
		 *    again from that point (without drawing of course!), using the new frame and any
		 *    input from subsiquent keyframes (where the input changed) in that time period,
		 *    to place the object at the correct position.
		 * -# Updates the positions of all objects based on their current state / inputs. This
		 *    provides pridictive movement if no new frames were received.
		 * -# Using their current state and inputs in the prevoius steps, it builds a new
		 *    frame for each local ship.
		 * -# Decides whether enough time has passed to allow an update, and adds the current
		 *    frame of each ship to the message queue and history if it has.
		 * -# Sends everything in the message queue (this may include non-gameplay messages
		 *    added to the queue by other threads (eg from the console and chat) too, but we
		 *    don't care about them - I may even give them their own queue and/or socket).
		 * -# Sets the positions of the ship nodes by calling UpdateNode on each ship.
		 * -# Updates the positions of all non-synced objects (this doesn't include weapons,
		 *    engines, etc. because those are child nodes and moved with their parents.)
		 *
		 * \param split The amount of time since ClientEnvironment#Update() was lass called.
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

	private:
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
		/*!
		 * \brief
		 * Updates the scene graph. ie. tells all ships to call UpdateNode();
		 * so they draw in the right place.
		 *
		 * \remarks
		 * This is now depreciated, as FusionClientShip#SetPosition does this.
		 * BTW, set position and set accel. / force should remain seperate
		 * as you might only have a few of them changing sometimes (efficiancy.)
		 */
		//void updateSceneGraph();

		// Deprecated - level and ships are now nodes so the scene draws them.
		/*void drawLevel();
		void drawShip(FusionShip ship);*/
	};

}

#endif
