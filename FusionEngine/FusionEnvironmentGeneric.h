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

#ifndef Header_FusionEngine_Environment
#define Header_FusionEngine_Environment

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionState.h"
#include "FusionSingleton.h"

/// Fusion
#include "FusionScene.h"
#include "FusionNode.h"

#include "FusionShip.h"
#include "FusionProjectile.h"
#include "FusionLevel.h"

#include "FusionResourceManager.h"
#include "FusionLevelResourceBundle.h"
#include "FusionShipResourceBundle.h"
#include "FusionWeaponResourceBundle.h"

#include "FusionScriptingEngine.h"
#include "FusionNetworkClient.h"
#include "FusionPhysicsWorld.h"

#include "FusionException.h"


namespace FusionEngine
{

	//! Default milis per frame
	const unsigned int g_DefaultFrameTime = 33;

	//! Lowest OID (OIDs below this are reserved)
	const ObjectID g_BaseOID = 1;
	//! [depreciated] Lowest PID
	const ObjectID g_BasePID = 1;

	//! Max OID assignable
	const ObjectID g_MaxOID = 65535;

	//! The virtual environment! (pun?)
	class Environment : public FusionState, public Singleton<Environment>
	{
	public:
		//! Basic Constructor
		Environment()
			: m_NumPlayers(0),
			m_FrameTime(g_DefaultFrameTime),
			m_NextOID(g_BaseOID),
			m_NextPID(g_BasePID)
		{}

		//! Virtual destructor
		virtual ~Environment() {}

	public:
		//! A list of ships
		typedef std::map<ObjectID, FusionShip*> ShipList;
		//! A list of projectiles
		typedef std::map<ObjectID, FusionProjectile*> ProjectileList;

		//! A list of FusionPhysicsBodys 
		typedef std::map<ObjectID, FusionPhysicsBody*> BodyList;

		//! A list of inputs WHY IS THIS HERE? WHAT IS THIS FOR?
		typedef std::vector<ShipInput> ShipInputList;

		//! A list of player ObjectIDs (player IDs) mapped to resourceID's
		typedef std::map<ObjectID, std::string> ShipResMap;
		//! A list of projectile ObjectIDs mapped to resourceID's
		typedef std::map<ObjectID, std::string> ProjectileResMap;

	public:
		//! Returns the index of the newly created ship
		virtual void CreateShip(const ShipState &state) =0;

		//! I don't think this is used.
		//virtual ShipResource *GetShipResourceByID(const std::string &id);

		//! I don't think this is used.
		//virtual ShipResource *GetProjectileResourceByID(const std::string &id);

		//! Detonate the given projectile
		virtual void DetonateProjectile(ObjectID index);

		//! Returns the ship corresponding to the given ObjectID
		virtual const FusionShip* GetShip(ObjectID index);

		//! Returns the projectile corresponding to the given ObjectID
		virtual const FusionProjectile* GetProjectile(ObjectID index);

		virtual const FusionPhysicsBody* GetBody(ObjectID index);

		//! Returns a list of projectiles
		virtual const ProjectileList& GetProjectileList() const;

		//! Returns a list of ships
		virtual const ShipList& GetShipList() const;

	protected:		
		//! Number of players in the game (total, ClientOptions#NumPlayers is local only)
		unsigned int m_NumPlayers;

		//! Next assignable PID
		ObjectID m_NextPID;
		//! Next assignable OID
		ObjectID m_NextOID;

		//! Miliseconds per frame
		unsigned int m_FrameTime;

		//! SceneGraph
		FusionScene *m_Scene;
		//! High level physics manager
		FusionPhysicsWorld *m_PhysicsWorld;

		//! List of ships currently in the environment
		ShipList m_Ships;
		//! List of Projectiles currently in the environment
		ProjectileList m_Projectiles;

		//! List of ship resources in use.
		/*!
		 * Maps ObjectIDs to Resource Tags
		 */
		ShipResMap m_ShipResources;
		//! List of weapon resources in use.
		/*!
		 * Maps ObjectIDs to Resource Tags
		 */
		ProjectileResMap m_WeaponResources;

	protected:
		//! Send all packets
		virtual void send() = 0;
		//! Receive all packets
		/*!
		 * "Receiving" can be done here - of course, FusionNetwork will have done the real
		 * receiving, this just handles the data from it.
		 */
		virtual bool receive() = 0;

		//! Waits until the frame rate set in Options is reached
		void limitFrames(unsigned int split);

		//! Builds a message from a ShipState (usually outgoing)
		//! \todo Put all this stuff (message building and parsing) into the
		//!  relavant classes - e.g. Ship state should build and parse MTID_SHIPFRAME packets
		FusionMessage *BuildMessage(const ShipState &input);
		//! Builds a message from a ProjectileState (usually outgoing)
		FusionMessage *BuildMessage(const ProjectileState &input);
		//! Builds a message from a InputState (usually outgoing)
		FusionMessage *BuildMessage(const ShipInput &input);

		//! Takes a received message, extracts the ShipState, and puts it into the relavant ship
		//! \todo Get rid of these methods, just use the parsing built into the relavant
		//!  classes; call said parsing methods from within the environments receive() method.
		void installShipFrameFromMessage(FusionMessage *m);
		//! Extracts the InputState, and puts it into the relavant ship.
		void installShipInputFromMessage(FusionMessage *m);

		//! Takes a received message, extracts the ProjectileState, and puts it into the relavant proj.
		void installProjectileFrameFromMessage(FusionMessage *m);

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
