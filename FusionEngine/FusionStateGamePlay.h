#ifndef Header_FusionEngine_FusionStateGamePlay
#define Header_FusionEngine_FusionStateGamePlay

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{

	/*!
	 * \brief
	 * Gameplay state. Dunno Why I made this (see GenericEnvironment - it's actually used.)
	 */
	class FusionStateGamePlay
	{
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

	protected:
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