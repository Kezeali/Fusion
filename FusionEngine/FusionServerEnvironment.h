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
#include "FusionShip.h"
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
	class ServerEnvironment
	{
	public:
		ServerEnvironment(ServerOptions *options);

		int numPlayers;

		bool Initialise(ResourceLoader *resources);

		/*!
		 * \brief
		 * Does everything.
		 */
		bool Update(unsigned int split);

		void Draw();

	private:
		FusionScene *m_Scene;
		FusionInput *m_InputManager;
		FusionNetwork *m_NetworkManager;

		std::set<FusionShip> m_Ships;
		std::set<FusionProjectile> m_Projectiles;

		std::map<std::string, ShipResource*> m_ShipResources;

		//! Updates the state structures of all ships.
		void updateShipStates();
		/*!
		 * \brief
		 * Updates the positions of all syncronised objects.
		 *
		 * Provides predictive movement based on current velocity etc.
		 */
		void updateAllPositions();
	};

}

#endif
