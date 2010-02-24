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

#include "FusionStableHeaders.h"

#include "FusionInstanceSynchroniser.h"

#include <BitStream.h>
#include <RakNetTypes.h>

#include "FusionEntityFactory.h"
#include "FusionEntityManager.h"
#include "FusionExceptionFactory.h"
#include "FusionLogger.h"
#include "FusionNetworkManager.h"
#include "FusionNetDestinationHelpers.h"
#include "FusionPlayerRegistry.h"

namespace FusionEngine
{
	
	InstanceSynchroniser::InstanceSynchroniser(EntityFactory *factory, EntityManager *manager)
		: m_Factory(factory),
		m_EntityManager(manager)
	{
		//m_EntityInstancedCnx = factory->SignalEntityInstanced.connect(boost::bind(&InstanceSynchroniser::OnEntityInstanced, this, _1));
	}

	ObjectID InstanceSynchroniser::generateLocalId()
	{
		ObjectID id = 0;

		uint8_t peerIndex = NetworkManager::GetLocalPeerIndex();

		if (peerIndex < s_MaxPeers)
		{
			// Generate the part of the id that makes it unique to this peer
			// Shift the peer index to the left of the id - it takes up the higher bits
			id = ObjectID(peerIndex) << (sizeof(ObjectID) * 8 - 5);
			id &= 0xF800;
			// id will now be 1----00000000000, where ---- is the peer index
		}
		// If the current peer index is >= s_MaxPeers a peer has recently
		//  left and the peer indicies are being re-assigned - all entity
		//  creation must be delegated to the arbiter during this period
		//  (leaving the object id 0 will indicate to the arbiter that
		//  this is the case when it receives the message.)

		// Check that the next ID wont overwrite the peer index
		if ((m_LocalIdGenerators[peerIndex].peekNextID() & 0xF800) == 0)
			id |= m_LocalIdGenerators[peerIndex].getFreeID();
		else
			Logger::getSingleton().Add("Can't fulfil requests to instance any more Entities: out of IDs");

		return id;
	}

	void InstanceSynchroniser::sendInstancingMessage(ObjectID requester_id, ObjectID id, const std::string &type, const std::string &name, PlayerID owner_id)
	{
		RakNet::BitStream newEntityData;
		newEntityData.Write(requester_id);
		newEntityData.Write(type.length());
		newEntityData.Write(type.c_str());
		newEntityData.Write(id);
		newEntityData.Write(owner_id);
		if (name.empty())
			newEntityData.Write0();
		else
		{
			newEntityData.Write1();
			newEntityData.Write(name.length());
			newEntityData.Write(name.c_str());
		}
		if (id != 0)
			m_Network->Send(To::Populace(), !Timestamped, MTID_INSTANCEENTITY, &newEntityData, LOW_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER);
		else // This peer failed to generate an ID, so the job of instancing now goes to the arbiter
			m_Network->Send(To::Arbiter(), !Timestamped, MTID_INSTANCEENTITY, &newEntityData, LOW_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER);

	}

	void InstanceSynchroniser::RequestInstance(EntityPtr &requester, bool syncable, const std::string &type, const std::string &name, PlayerID owner_id)
	{
		if (requester)
			FSN_EXCEPT(ExCode::InvalidArgument, "InstanceSynchroniser::RequestInstance", "You must pass a valid requester instance");

		bool localAuthority = PlayerRegistry::IsLocal(requester->GetOwnerID());
		uint8_t peerIndex = m_Network->GetLocalPeerIndex(); // Used to generate an ID if this entity is being created under local authority

		// Syncable entities can only be instanced by a call from either an owned entity or the arbiter
		//  pseudo-entities (!syncable) are local-only so peers can spawn them willy-nilly
		if (!syncable || localAuthority || NetworkManager::ArbitratorIsLocal())
		{
			ObjectID id = 0;

			if (syncable)
			{
				if (localAuthority)
					id = generateLocalId();
				else
				{
					if ((m_WorldIdGenerator.peekNextID() & 0x8000) == 0)
						id = m_WorldIdGenerator.getFreeID();
					else
					{
						Logger::getSingleton().Add("Can't fulfil requests to instance any more Entities: out of IDs");
						// If the arbiter has run out of IDs, we have no recourse
						//if (NetworkManager::ArbitratorIsLocal())
						return;
					}
				}
				sendInstancingMessage(requester->GetID(), id, type, name, owner_id);
			}

			EntityPtr entity = m_Factory->InstanceEntity(type);

			entity->SetID(id);
			entity->SetOwnerID(owner_id);

			m_EntityManager->AddEntity(entity);

			if (requester->IsSyncedEntity()) // If the entity isn't synced this call can't be synced, so it isn't made in that case
				requester->OnRequestInstanceFulfilled(entity);
		}
	}

	void InstanceSynchroniser::RequestInstance(EntityPtr &requester, bool syncable, const std::string &type, PlayerID owner)
	{
		RequestInstance(requester, syncable, type, "", owner);
	}

	void InstanceSynchroniser::HandlePacket(Packet *packet)
	{
		RakNet::BitStream newEntityData(packet->data, packet->length, false);

		ObjectID requesterId; // the entity that requested this instance (the OnRequestFulfilled method must be called on this object)
		newEntityData.Read(requesterId);

		// Read the entity type-name
		std::string::size_type length;
		std::string entityType;
		newEntityData.Read(length);
		entityType.resize(length);
		newEntityData.Read(&entityType[0]);

		ObjectID id, ownerId;
		newEntityData.Read(id);
		newEntityData.Read(ownerId);

		// Entity name (an alternative ID represented as a human-readable string)
		std::string name;
		if (newEntityData.ReadBit())
		{
			newEntityData.Read(length);
			name.resize(length);
			newEntityData.Read(&name[0]);
		}

		// id == 0 indicates that a peer is requesting an id for this entity
		if (id == 0)
		{
			//if ((m_WorldIdGenerator.peekNextID() & 0x8000) == 0)
			//	id = m_WorldIdGenerator.getFreeID();
			// I've decided that peers should just use their own IDs; using up the
			//  world/arbiter id's when a peer runs out seems foolish
			return;
		}

		EntityPtr entity = m_Factory->InstanceEntity(entityType);

		entity->SetID(id);
		entity->SetOwnerID(ownerId);
		if (!name.empty())
			entity->_setName(name);

		m_EntityManager->AddEntity(entity);

		EntityPtr requester = m_EntityManager->GetEntity(requesterId, false);
		if (requester)
			requester->OnRequestInstanceFulfilled(entity);
	}

}
