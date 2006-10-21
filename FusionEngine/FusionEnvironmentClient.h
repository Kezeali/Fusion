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
	 * FusionScene | FusionInput | FusionNetworkingHandler | GenericEnvironment.
	 */
	class ClientEnvironment : public GenericEnvironment
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

		//! Runs and maintains the statemanager and states
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
