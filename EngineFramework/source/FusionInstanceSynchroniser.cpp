/*
*  Copyright (c) 2010-2011 Fusion Project Team
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
#include <StringCompressor.h>
#include <StringTable.h>
#include <climits>

#include "FusionComponentFactory.h"
#include "FusionEntityManager.h"
#include "FusionExceptionFactory.h"
#include "FusionLogger.h"
#include "FusionNetworkManager.h"
#include "FusionNetDestinationHelpers.h"
#include "FusionPlayerRegistry.h"

#include "FusionTransformComponent.h"

#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionScriptManager.h"
#include "FusionAngelScriptComponent.h"

#include "FusionEntitySerialisationUtils.h"

using namespace RakNet;

namespace FusionEngine
{
	
	InstancingSynchroniser::InstancingSynchroniser(EntityFactory *factory, EntityManager *manager)
		: m_Factory(factory),
		m_EntityManager(manager)
	{
		m_Network = NetworkManager::getSingleton().GetNetwork(); // TODO: make this a param of the ctor

		NetworkManager::getSingleton().Subscribe(MTID_INSTANCEENTITY, this);
		NetworkManager::getSingleton().Subscribe(MTID_REMOVEENTITY, this);
		NetworkManager::getSingleton().Subscribe(MTID_STARTSYNC, this);
		NetworkManager::getSingleton().Subscribe(ID_NEW_INCOMING_CONNECTION, this);

		ScriptManager::getSingleton().RegisterGlobalObject("Ontology ontology", this);
	}

	InstancingSynchroniser::~InstancingSynchroniser()
	{
		NetworkManager::getSingleton().Unsubscribe(MTID_INSTANCEENTITY, this);
		NetworkManager::getSingleton().Unsubscribe(MTID_REMOVEENTITY, this);
		NetworkManager::getSingleton().Unsubscribe(ID_NEW_INCOMING_CONNECTION, this);
	}

	void InstancingSynchroniser::Reset(ObjectID next)
	{
		for (int i = 0; i < s_MaxPeers; ++i)
			m_LocalIdGenerators[i].freeAll();
		m_WorldIdGenerator.takeAll(next);
	}

	static const ObjectID localFlag = 0x1 << (sizeof(ObjectID) * 8 - 1); // - 1 because the flag is 1 bit
	static const ObjectID peerIndexMask = 0x78 << (sizeof(ObjectID) * 8 - 8); // - 8 because the mask overlaps 2 bytes
	static const ObjectID localIdMask = ~(localFlag | peerIndexMask);

	void InstancingSynchroniser::TakeID(ObjectID id)
	{
		if ((id & localFlag) == localFlag) // if the first bit is set (this is a local-authority ID)
		{
			uint8_t peerIndex = (id & peerIndexMask) >> (sizeof(ObjectID) * 8 - 5);
			if (peerIndex < s_MaxPeers)
			{
				m_LocalIdGenerators[peerIndex].takeID(id & localIdMask);
			}
		}
		else
		{
			m_WorldIdGenerator.takeID(id);
		}
	}

	void InstancingSynchroniser::FreeID(ObjectID id)
	{
		if ((id & localFlag) == localFlag) // if the first bit is set this is a local-authority ID
		{
			uint8_t peerIndex = (id & peerIndexMask) >> (sizeof(ObjectID) * 8 - 5);
			if (peerIndex < s_MaxPeers)
			{
				m_LocalIdGenerators[peerIndex].freeID(id & localIdMask);
			}
		}
		else
		{
			m_WorldIdGenerator.freeID(id);
		}
	}

	ObjectID InstancingSynchroniser::generateLocalId()
	{
		static_assert(CHAR_BIT == 8, "Why are your bytes weird? :(");

		ObjectID id = 0;

		uint8_t peerId = NetworkManager::GetPeerID();

		if (peerId < s_MaxPeers)
		{
			// Generate the part of the id that makes it unique to this peer:
			// Shift the peer index to the left of the id - it takes up the higher
			//  bits after the first bit
			id = ObjectID(peerId) << (sizeof(ObjectID) * 8 - 5); // assumes peerIndex fits into 4 bits (s_MaxPeers is 16) TODO: remove this assumption?
			id |= localFlag; // set the left-most bit to 1
			// id will now be 1----00000000000, where ---- is the peer index
		}

		// Check that the next ID wont overwrite the peer index
		if ((m_LocalIdGenerators[peerId].peekNextID() & peerIndexMask) == 0)
			id |= m_LocalIdGenerators[peerId].getFreeID();
		else
			AddLogEntry(g_LogGeneral, "Out of IDs: can't fulfil requests to instanciate Entities anymore");

		return id;
	}

	void InstancingSynchroniser::sendInstancingMessage(ObjectID requester_id, ObjectID id, const std::string &type, const Vector2& pos, float angle, const std::string &name, PlayerID owner_id)
	{
		RakNet::BitStream newEntityData;
		//newEntityData.Write(requester_id);
		RakNet::StringTable::Instance()->EncodeString(type.c_str(), s_NetCompressedStringTrunc, &newEntityData);
		newEntityData.Write(pos.x);
		newEntityData.Write(pos.y);
		newEntityData.Write(angle);

		newEntityData.Write(id);
		newEntityData.Write(owner_id);
		if (name.empty())
			newEntityData.Write0();
		else
		{
			newEntityData.Write1();
			newEntityData.Write(name.length());
			newEntityData.Write(name.c_str(), name.length());
		}
		if (id != 0)
			m_Network->Send(To::Populace(), !Timestamped, MTID_INSTANCEENTITY, &newEntityData, MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER);
		else // This peer failed to generate an ID, so the job of instancing now goes to the arbiter
			m_Network->Send(To::Arbiter(), !Timestamped, MTID_INSTANCEENTITY, &newEntityData, MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER);
	}

	void InstancingSynchroniser::sendFullSynch(NetDestination& destination, const EntityPtr& entity)
	{
		RakNet::BitStream entityData;

		entityData.Write(entity->GetID());
		entityData.Write(entity->GetOwnerID());
		RakNet::StringCompressor::Instance()->EncodeString(entity->GetName().c_str(), 256, &entityData);
		EntitySerialisationUtils::SerialiseEntity(entityData, entity, IComponent::All);

		m_Network->Send(destination, !Timestamped, MTID_STARTSYNC, &entityData, MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER);
	}

	EntityPtr InstancingSynchroniser::RequestInstance(EntityPtr &requester, bool syncable, const std::string &type, const std::string &name, Vector2 pos, float angle, PlayerID owner_id)
	{
		bool localAuthority = requester && PlayerRegistry::IsLocal(requester->GetOwnerID());

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
					if ((m_WorldIdGenerator.peekNextID() & localFlag) == 0) // make sure the next ID won't clobber the local flag
						id = m_WorldIdGenerator.getFreeID();
					else
					{
						AddLogEntry(g_LogGeneral, "Out of IDs: can't fulfil requests to instanciate Entities anymore");
						return EntityPtr();
					}
				}
				sendInstancingMessage(requester->GetID(), id, type, pos, angle, name, owner_id);
			}

			auto transform = m_Factory->InstanceComponent(type, pos, angle);
			if (!transform)
				FSN_EXCEPT(InvalidArgumentException, type + " doesn't exist, so you can't instantiate an entity with it.");
			if (dynamic_cast<ITransform*>(transform.get()) == nullptr)
				FSN_EXCEPT(InvalidArgumentException, type + " doesn't implement ITransform, so you can't instantiate an entity with it.");

			// TODO: make m_PropChangedQueue a member of this class?
			EntityPtr entity = std::make_shared<Entity>(m_EntityManager, &m_EntityManager->m_PropChangedQueue, transform);

			if (entity)
			{
				entity->SetID(id);
				entity->SetOwnerID(owner_id);
				entity->SetName(name);

				transform->SynchronisePropertiesNow();

				m_EntityManager->AddEntity(entity);

				// TODO: set this entity to a property, rather than calling this callback
				//if (requester->IsSyncedEntity()) // If the entity isn't synced this call can't be synced, so it isn't made in that case
				//	requester->OnInstanceRequestFulfilled(entity);
			}

			return entity;
		}

		return EntityPtr();
	}

	EntityPtr InstancingSynchroniser::RequestInstance(EntityPtr &requester, bool syncable, const std::string &type, Vector2 pos, float angle, PlayerID owner)
	{
		return RequestInstance(requester, syncable, type, "", Vector2::zero(), 0.f, owner);
	}

	void InstancingSynchroniser::RemoveInstance(EntityPtr& entity)
	{
		// TODO: reconsider the logic of who should be able to instance / delete entities (WRT owner ID)
		if (entity->IsSyncedEntity())
		{
			if (PlayerRegistry::IsLocal(entity->GetOwnerID()) || NetworkManager::ArbitratorIsLocal())
			{
				FreeID(entity->GetID());

				RakNet::BitStream removeEntityNotification;
				removeEntityNotification.Write(entity->GetID());

				m_Network->Send(
					To::Populace(), // Broadcast
					!Timestamped,
					MTID_REMOVEENTITY, &removeEntityNotification,
					MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM);

				m_EntityManager->RemoveEntity(entity);
			}
		}
		else
			m_EntityManager->RemoveEntity(entity);
	}

	void InstancingSynchroniser::AddComponent(EntityPtr& entity, const std::string& type, const std::string& identifier)
	{
		if (entity)
		{
			auto com = m_Factory->InstanceComponent(type);
			if (com)
			{
				entity->AddComponent(com, identifier);
				m_EntityManager->OnComponentAdded(entity, com);

				//sendComponent(To::Populace(), entity, com);
			}
		}
		// TODO: throw exception
	}

	void InstancingSynchroniser::HandlePacket(Packet *packet)
	{
		RakNet::BitStream receivedData(packet->data, packet->length, false);

		unsigned char type;
		receivedData.Read(type);
		switch (type)
		{
		case MTID_INSTANCEENTITY:
			{
				//ObjectID requesterId; // the entity that requested this instance (the OnRequestFulfilled method must be called on this object)
				//receivedData.Read(requesterId);

				std::string::size_type length;

				// Read the entity transform type-name
				char table_string[s_NetCompressedStringTrunc];
				RakNet::StringTable::Instance()->DecodeString(table_string, s_NetCompressedStringTrunc, &receivedData);
				std::string transformTypeName(table_string);
				//receivedData.Read(length);
				//entityType.resize(length);
				//receivedData.Read(&entityType[0], length);
				Vector2 position;
				receivedData.Read(position.x);
				receivedData.Read(position.y);
				float angle;
				receivedData.Read(angle);

				ObjectID id;
				receivedData.Read(id);
				PlayerID ownerId;
				receivedData.Read(ownerId);

				TakeID(id);

				// Entity name
				std::string name;
				if (receivedData.ReadBit())
				{
					receivedData.Read(length);
					name.resize(length);
					receivedData.Read(&name[0], length);
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

				auto transform = m_Factory->InstanceComponent(transformTypeName, position, angle);
				if (!transform)
					FSN_EXCEPT(InstanceSyncException, type + " doesn't exist, so you can't instantiate an entity with it.");
				if (dynamic_cast<ITransform*>(transform.get()) == nullptr)
					FSN_EXCEPT(InstanceSyncException, type + " doesn't implement ITransform, so you can't instantiate an entity with it.");

				EntityPtr entity = std::make_shared<Entity>(m_EntityManager, &m_EntityManager->m_PropChangedQueue, transform);
				if (!entity)
					FSN_EXCEPT(InstanceSyncException, "Failed to create entity");

				entity->SetID(id);
				entity->SetOwnerID(ownerId);
				if (!name.empty())
					entity->SetName(name);

				transform->SynchronisePropertiesNow();

				m_EntityManager->AddEntity(entity);

				//EntityPtr requester = m_EntityManager->GetEntity(requesterId, false);
				//if (requester)
				//	requester->OnInstanceRequestFulfilled(entity);
			}
			break;
		case MTID_REMOVEENTITY:
			{
				ObjectID removedId;
				receivedData.Read(removedId);

				m_EntityManager->RemoveEntityById(removedId);
			}
			break;
		case MTID_STARTSYNC:
			{
				ObjectID id;
				receivedData.Read(id);
				PlayerID ownerId;
				receivedData.Read(ownerId);
				std::string name;
				{
					char nameBuf[256];
					RakNet::StringCompressor::Instance()->DecodeString(nameBuf, 256, &receivedData);
					name = nameBuf;
				}
				EntityPtr entity = EntitySerialisationUtils::DeserialiseEntity(receivedData, m_Factory, m_EntityManager);
				entity->SetID(id);
				entity->SetOwnerID(ownerId);
				entity->SetName(name);
				m_EntityManager->AddEntity(entity);

				std::stringstream str; str << id;
				SendToConsole("Received full synch for " + str.str());
			}
			break;
		case ID_NEW_INCOMING_CONNECTION:
			if (NetworkManager::ArbitratorIsLocal())
			{
				SendToConsole("Sending full sync to " + std::string(packet->guid.ToString()));
				const auto& entities = m_EntityManager->GetEntities();
				for (auto it = entities.begin(), end = entities.end(); it != end; ++it)
				{
					sendFullSynch(NetDestination(packet->guid, false), it->second);
				}
			}
			break;
		} // end switch (type)
	}

	static EntityPtr InstantiationSynchroniser_Instantiate(ASScript* app_obj, const std::string& transform_component, bool synch, Vector2 pos, float angle, PlayerID owner_id, const std::string& name, InstancingSynchroniser* obj)
	{
		auto entity = app_obj->GetParent()->shared_from_this();

		return obj->RequestInstance(entity, synch, transform_component, name, pos, angle, owner_id);
	}

	static EntityPtr InstantiationSynchroniser_InstantiateAuto(const std::string& transform_component, bool synch, Vector2 pos, float angle, PlayerID owner_id, const std::string& name, InstancingSynchroniser* obj)
	{
		ASScript* nativeCom = ASScript::GetActiveScript();

		//ScriptUtils::Calling::Caller(com, "ASScript@ _getAppObj()"
		auto entity = nativeCom->GetParent()->shared_from_this();

		auto newEntity = obj->RequestInstance(entity, synch, transform_component, name, pos, angle, owner_id);

		//newEntity->addRef();
		return newEntity;//.get();
	}

	static void InstantiationSynchroniser_AddComponent(EntityPtr entity, const std::string& type, const std::string& identifier, InstancingSynchroniser* obj)
	{
		obj->AddComponent(entity, type, identifier);
	}

	void InstancingSynchroniser::Register(asIScriptEngine* engine)
	{
		RegisterSingletonType<InstancingSynchroniser>("Ontology", engine);

		engine->RegisterObjectMethod("Ontology", "Entity instantiate(ASScript @, const string &in, bool, Vector, float, PlayerID owner_id = 0, const string &in name = string())",
			asFUNCTION(InstantiationSynchroniser_Instantiate), asCALL_CDECL_OBJLAST);

		engine->RegisterObjectMethod("Ontology", "Entity instantiate(const string &in, bool, Vector, float, PlayerID owner_id = 0, const string &in name = string())",
			asFUNCTION(InstantiationSynchroniser_InstantiateAuto), asCALL_CDECL_OBJLAST);

		engine->RegisterObjectMethod("Ontology", "void addComponent(Entity, const string &in, const string &in)",
			asFUNCTIONPR(InstantiationSynchroniser_AddComponent, (EntityPtr, const std::string&, const std::string&, InstancingSynchroniser*), void), asCALL_CDECL_OBJLAST);
	}

}
