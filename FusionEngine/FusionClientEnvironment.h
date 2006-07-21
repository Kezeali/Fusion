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
#include "FusionProjectile.h"
#include "FusionResourceLoader.h"
#include "FusionShipResource.h"
#include "FusionInputHandler.h"

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
	class ClientEnvironment
	{
	public:
		ClientEnvironment(ClientOptions *options);

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

		void Draw();

		//! Called by FusionShipDrawable#Draw() to get the sprite, etc.
		const ShipResource &GetShipResourceByID(std::string id) const;

	private:
		FusionScene *m_Scene;
		FusionInput *m_InputManager;
		FusionNetwork *m_NetworkManager;

		std::set<FusionShip> m_Ships;
		std::set<FusionProjectile> m_Projectiles;

		typedef std::map<std::string, ShipResource*> ShipResourceMap;
		ShipResourceMap m_ShipResources;

		//! Updates the input structures of all local ships.
		void gatherLocalInput();
		/*!
		 * Updates the state structures of local and remote ships.
		 * "Sending" and "receiving" can be done here (ofcourse, FusionNetwork
		 * will have done the real receiving, this just handles the data from it.)
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
