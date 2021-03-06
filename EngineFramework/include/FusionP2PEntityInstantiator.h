/*
*  Copyright (c) 2010 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef H_FusionP2PEntityInstantiator
#define H_FusionP2PEntityInstantiator

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityInstantiator.h"

#include "FusionComponentFactory.h"
#include "FusionEntity.h"
#include "FusionIDStack.h"
#include "FusionPacketHandler.h"
#include "FusionRakNetwork.h"

#include <array>
#include <iostream>

namespace FusionEngine
{

	//! Thrown when an entity can't be synchronised
	class InstanceSyncException : public Exception
	{
	public:
		//! Constructor
		InstanceSyncException(const std::string& description, const std::string& origin, const char* file, long line)
			: Exception(description, origin, file, line) {}
	};

	class SaveableObjectIDSet : public ObjectIDSet
	{
	public:
		void Save(std::ostream& stream);
		void Load(std::istream& stream);
	};

	//! Synchronises the creation of new instances
	class P2PEntityInstantiator : public EntityInstantiator, public PacketHandler
	{
	public:
		P2PEntityInstantiator(ComponentFactory *factory, EntityManager *manager);
		~P2PEntityInstantiator();

		//! Resets the used ID lists, setting the min world id to the one given.
		void Reset(ObjectID min_unused = 0);

		void SaveState(std::ostream& stream);
		void LoadState(std::istream& stream);

		//! Removes the given ID from the available pool
		/*!
		* This should be called when loading maps/saved games for each loaded ID
		* and when receiving new instances from other peers.
		*/
		void TakeID(ObjectID id);
		//! Puts the given ID back in the pool
		void FreeID(ObjectID id);

		ObjectID GetFreeGlobalID();

		//! Tries to create a new entity (only succeeds if this peer has authority to do so)
		/*!
		* \param[in] requester
		* The Entity calling this function. The EntityAdded callback will
		* be called on this entity when the new entity is created.
		* Also used to validate the call - GetOwnerID() must be a valid 
		* player ID unless this peer is the arbitrator.
		* \param[in] synced
		* True if the new Entity should be synchronised (not pseudo-)
		* \param[in] type
		* The type of the Entity to create.
		* \param[in] instance_owner
		* The player who will own the new instance. 0 for none
		*/
		EntityPtr RequestInstance(EntityPtr &requester, bool synced, const std::string &type, const std::string &name, Vector2 pos, float angle, PlayerID instance_owner = 0);

		//! Tries to create a new entity (only succeeds if this peer has authority to do so)
		/*!
		* \param[in] requester
		* The Entity calling this function. The EntityAdded callback will
		* be called on this entity when the new entity is created.
		* Also used to validate the call - GetOwnerID() must be a valid 
		* player ID unless this peer is the arbitrator.
		* \param[in] synced
		* True if the new Entity should be synchronised (not pseudo-)
		* \param[in] type
		* The type of the Entity to create.
		* \param[in] instance_owner
		* The player who will own the new instance. 0 for none
		*/
		EntityPtr RequestInstance(EntityPtr &requester, bool synced, const std::string &type, Vector2 pos, float angle, PlayerID instance_owner = 0);

		//! Returns the given Entity's ID to the pool
		void RemoveInstance(EntityPtr& entity);

		ComponentPtr AddComponent(const EntityPtr& entity, const std::string& type, const std::string& identifier);
		bool RemoveComponent(const EntityPtr& entity, const ComponentPtr& component);

		//! Pick up entity creation packets
		void HandlePacket(RakNet::Packet *packet);

		static void Register(asIScriptEngine* engine);

	protected:
		ComponentFactory *m_Factory;
		EntityManager *m_EntityManager;

		RakNetwork *m_Network;

		std::array<SaveableObjectIDSet, s_MaxPeers> m_LocalIdGenerators;
		SaveableObjectIDSet m_WorldIdGenerator;

		//boost::signals2::connection m_EntityInstancedCnx;

		//! Gets a new ID for local-authroity instancing
		ObjectID generateLocalId();
		//! Notifies other clients of new entities
		void sendInstancingMessage(ObjectID requester_id, ObjectID id, const std::string &transformType, const Vector2& pos, float angle, const std::string &name, PlayerID owner_id);
		//! Sends full entity data to clients
		void sendFullSynch(NetDestination& destination, const EntityPtr& entity);
	};

}

#endif
