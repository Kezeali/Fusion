/*
  Copyright (c) 2009 Fusion Project Team

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

#include "Common.h"

#include "FusionEntityManager.h"

#include <boost/lexical_cast.hpp>
#include <BitStream.h>

#include "FusionClientOptions.h"
#include "FusionEntityFactory.h"
#include "FusionExceptionFactory.h"
#include "FusionNetworkSystem.h"
#include "FusionNetworkTypes.h"
#include "FusionPlayerRegistry.h"
#include "FusionRakNetwork.h"
#include "FusionScriptTypeRegistrationUtils.h"
// For script registration (the script method EntityManager::instance() returns a script object)
#include "FusionScriptedEntity.h"


namespace FusionEngine
{

	ConsolidatedInput::ConsolidatedInput(InputManager *input_manager)
		: m_LocalManager(input_manager),
		m_ChangedCount(0)
	{
		m_InputChangedConnection = m_LocalManager->SignalInputChanged.connect( boost::bind(&ConsolidatedInput::onInputChanged, this, _1) );
	}

	ConsolidatedInput::~ConsolidatedInput()
	{
		m_InputChangedConnection.disconnect();
	}

	void ConsolidatedInput::SetState(ObjectID player, const std::string input, bool active, float position)
	{
		PlayerInputsMap::iterator _where = m_PlayerInputs.find(player);
		if (_where != m_PlayerInputs.end())
		{
			_where->second->SetState(input, active, position);
		}
	}

	PlayerInputPtr ConsolidatedInput::GetInputsForPlayer(ObjectID player)
	{
		// TODO: unmuddle this method

		PlayerInputsMap::iterator _where = m_PlayerInputs.find(player);
		if (_where != m_PlayerInputs.end())
		{
			return _where->second;
		}
		else
		{
			if (PlayerRegistry::GetPlayerByNetIndex(player).IsInGame)
				return m_PlayerInputs[player] = PlayerInputPtr( new PlayerInput(m_LocalManager->GetDefinitionLoader()->GetInputDefinitions()) );
			else
				return PlayerInputPtr();
		}
	}

	const ConsolidatedInput::PlayerInputsMap &ConsolidatedInput::GetPlayerInputs() const
	{
		return m_PlayerInputs;
	}

	unsigned short ConsolidatedInput::ChangedCount() const
	{
		return m_ChangedCount;
	}

	void ConsolidatedInput::ChangesRecorded()
	{
		m_ChangedCount = 0;
	}

	ObjectID ConsolidatedInput::LocalToNetPlayer(unsigned int local)
	{
		return PlayerRegistry::GetPlayerByLocalIndex(local).NetIndex;
	}

	void ConsolidatedInput::onInputChanged(const InputEvent &ev)
	{
		ObjectID player = LocalToNetPlayer(ev.Player);
		PlayerInputsMap::iterator _where = m_PlayerInputs.find(player);
		if (_where != m_PlayerInputs.end())
		{
			PlayerInputPtr &playerInput = _where->second;

			bool changed = playerInput->HasChanged();
			
			if (ev.Type == InputEvent::Binary)
				playerInput->SetActive(ev.Input, ev.Down);
			else
				playerInput->SetPosition(ev.Input, (float)ev.Value);

			if (changed != playerInput->HasChanged())
				++m_ChangedCount;
		}
	}

	EntitySynchroniser::EntitySynchroniser(InputManager *input_manager, NetworkSystem *network_system)
		: m_InputManager(input_manager),
		m_NetworkSystem(network_system),
		m_Network(network_system->GetNetwork()),
		m_PlayerInputs(new ConsolidatedInput(input_manager))
	{
		m_NetworkSystem->AddPacketHandler(MTID_IMPORTANTMOVE, this);
		m_NetworkSystem->AddPacketHandler(MTID_ENTITYMOVE, this);
	}

	EntitySynchroniser::~EntitySynchroniser()
	{
		m_NetworkSystem->RemovePacketHandler(MTID_IMPORTANTMOVE, this);
		m_NetworkSystem->RemovePacketHandler(MTID_ENTITYMOVE, this);
	}

	const EntitySynchroniser::InstanceDefinitionArray &EntitySynchroniser::GetReceivedEntities() const
	{
		return m_ReceivedEntities;
	}

	void EntitySynchroniser::BeginPacket()
	{
		m_ImportantMove = false;

		const ConsolidatedInput::PlayerInputsMap &inputs = m_PlayerInputs->GetPlayerInputs();
		for (ConsolidatedInput::PlayerInputsMap::const_iterator it = inputs.begin(), end = inputs.end(); it != end; ++it)
		{
			ObjectID playerId = it->first;
			const PlayerInputPtr &playerInput = it->second;

			if (playerInput->HasChanged())
			{
				if (!m_ImportantMove)
				{
					m_PacketData.Write((MessageID)MTID_IMPORTANTMOVE);
					m_PacketData.Write(m_PlayerInputs->ChangedCount());
					m_ImportantMove = true;
				}

				m_PacketData.Write(playerId);

				playerInput->Serialise(&m_PacketData);
			}
		}

		m_PlayerInputs->ChangesRecorded();

		if (!m_ImportantMove)
		{
			m_PacketData.Write((MessageID)MTID_ENTITYMOVE);
		}
	}

	void EntitySynchroniser::EndPacket()
	{
		//RakNet::BitStream packetData;
		m_PacketData.Write((unsigned int)m_EntityPacketData.size());
		// Fill packet (from priority lists)
		for (EntityPriorityMap::iterator it = m_EntityPacketData.begin(), end = m_EntityPacketData.end(); it != end; ++it)
		{
			const EntityPacketData &packetData = it->second;

			m_PacketData.Write(packetData.ID);

			m_PacketData.Write(packetData.State.mask);
			m_PacketData.Write(packetData.State.data.length());
			m_PacketData.Write(packetData.State.data.c_str(), packetData.State.data.length());

			// Note the state that was sent (so that the state wont be sent again till it changes)
			m_SentStates[packetData.ID] = packetData.State;;
		}
	}

	void EntitySynchroniser::Send()
	{
		// TODO: different packets for each system - only send entities that that system can see
		// Note that this sends to the destinations selected in the call to EndPacket
		//for (SystemArray::const_iterator it = m_PacketDestinations.begin(), end = m_PacketDestinations.end(); it != end; ++it)
		//{
		//	const NetHandle &address = *it->System;
		//	if (m_ImportantMove)
		//	{
		//		m_Network->SendRaw((const char*)m_PacketData.GetData(), m_PacketData.GetNumberOfBytesUsed(), MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_INPUTUPDATE, address, false);
		//	}
		//	else
		//	{
		//		m_Network->SendRaw((const char*)m_PacketData.GetData(), m_PacketData.GetNumberOfBytesUsed(), MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, CID_ENTITYSYNC, address, false);
		//	}
		//}

		if (m_ImportantMove)
			{
				m_Network->SendRaw((const char*)m_PacketData.GetData(), m_PacketData.GetNumberOfBytesUsed(), MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_INPUTUPDATE, NetHandle(), true);
			}
			else
			{
				m_Network->SendRaw((const char*)m_PacketData.GetData(), m_PacketData.GetNumberOfBytesUsed(), MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, CID_ENTITYSYNC, NetHandle(), true);
			}

		m_PacketData.Reset();
	}

	void EntitySynchroniser::OnEntityInstanced(const EntityPtr &entity)
	{
		if (PlayerRegistry::ArbitratorIsLocal())
			m_Network->Send(false, MTID_INSTANCEENTITY, "", 0, LOW_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER, NetHandle(), true);
	}

	void EntitySynchroniser::OnEntityAdded(EntityPtr &entity)
	{
		//for (unsigned int i = 0; i < s_MaxLocalPlayers; ++i)
		//{
		//	const PlayerRegistry::PlayerInfo &playerInfo = ;
		//	if (playerInfo.LocalIndex != s_MaxLocalPlayers && playerInfo.NetIndex == entity->GetOwnerID())
		//}

		const PlayerRegistry::PlayerInfo &playerInfo = PlayerRegistry::GetPlayerByNetIndex(entity->GetOwnerID());
		if (playerInfo.IsInGame)
		{
			PlayerInputPtr playerInput = m_PlayerInputs->GetInputsForPlayer(playerInfo.NetIndex);
			if (playerInput)
				entity->_setPlayerInput(playerInput);
		}
	}

	bool EntitySynchroniser::ReceiveSync(EntityPtr &entity, const EntityDeserialiser &entity_deserialiser)
	{
		ObjectStatesMap::const_iterator _where = m_ReceivedStates.find(entity->GetID());
		if (_where != m_ReceivedStates.end())
			entity->DeserialiseState(_where->second, false, entity_deserialiser);

		return true; // Return false to not update this entity
	}

	bool EntitySynchroniser::AddToPacket(EntityPtr &entity)
	{
		bool arbitor = PlayerRegistry::ArbitratorIsLocal();
		bool isOwnedLocally = PlayerRegistry::IsLocal(entity->GetOwnerID());
		// Only send if: 1) the entity is owned locally, or 2) the entity is under default authroity
		//  and this system is the arbitor
		if ((entity->GetOwnerID() == 0 && arbitor) || isOwnedLocally)
		{
			// Calculate priority
			unsigned int priority;
			if (arbitor)
				priority = (isOwnedLocally ? 2 : 1) * entity->GetSkippedPacketsCount();
			else
				priority = entity->GetSkippedPacketsCount();

			// Obviously the packet might not be skipped, but if it is actually sent
			//  it's skipped-count gets reset to zero, so the following operation will
			//  over-ruled
			entity->PacketSkipped();

			// TODO:
			// for each system
			//  if system can see this entity
			//  continue as below: (already implemented)

			// If the Entity quota hasn't been filled, always insert
			if (m_EntityPacketData.size() < s_EntitiesPerPacket)
			{
				SerialisedData state;
				entity->SerialiseState(state, false);
				if (state.data != m_SentStates[entity->GetID()].data)
				{
					m_EntityPacketData[priority].ID = entity->GetID();
					m_EntityPacketData[priority].State = state;

					//m_EntityDataUsed += state.data.length() + sizeof(unsigned int) + sizeof(ObjectID); // TODO: function to calc packet-data-size for SerialisedData
					return true;
				}
			}
			// If the Entity quota has been filled, insert if the new entity has a higher priority
			else
			{
				FSN_ASSERT(m_EntityPacketData.size() > 1);
				EntityPriorityMap::iterator back = --m_EntityPacketData.end();
				if (priority > back->first)
				{
					// Check that the Entity has changed since it was last sent
					SerialisedData state;
					entity->SerialiseState(state, false);
					if (state.data != m_SentStates[entity->GetID()].data)
					{
						// Remove the entity with the lowest priority
						m_EntityPacketData.erase(back);

						m_EntityPacketData[priority].ID = entity->GetID();
						m_EntityPacketData[priority].State = state;
					}

					return true;
				}
			}
		}

		return false;
	}

	//bool EntitySynchroniser::AddToPacket(PhysicsBodyPtr &body)
	//{
	//	size_t lengthAfterWrite = m_PacketData.GetNumberOfBytesUsed() + state.data.length() + sizeof(unsigned int) * 2;
	//	if (lengthAfterWrite > s_MaxEntityPacketSize)
	//	{
	//		return false;
	//	}

	//	return true;
	//}

	void EntitySynchroniser::HandlePacket(IPacket *packet)
	{
		RakNet::BitStream bitStream((unsigned char*)packet->GetData(), packet->GetLength(), true);

		switch (packet->GetType())
		{
		case MTID_IMPORTANTMOVE:
			{
				// Get Input data
				unsigned short playerCount;
				bitStream.Read(playerCount);
				for (unsigned short pi = 0; pi < playerCount; pi++)
				{
					ObjectID player;
					bitStream.Read(player);

					unsigned short count; // number of inputs that changed
					bitStream.Read(count);

					const InputDefinitionLoader *inputDefinitions = m_InputManager->GetDefinitionLoader();

					for (unsigned short i = 0; i < count; ++i)
					{
						bool active;
						float position;

						unsigned short inputIndex;
						bitStream.Read(inputIndex);

						bitStream.Read(active);
						bitStream.Read(position);

						const InputDefinition &definition = inputDefinitions->GetInputDefinition((size_t)inputIndex);

						m_PlayerInputs->SetState(player, definition.Name, active, position);
					}
				}
			}
		case MTID_ENTITYMOVE:
			{
				unsigned short entityCount;
				bitStream.Read(entityCount);
				for (unsigned short i = 0; i < entityCount; i++)
				{
					ObjectID entityID;
					bitStream.Read(entityID);

					SerialisedData &state = m_ReceivedStates[entityID];

					bitStream.Read(state.mask);
					size_t dataLength;
					bitStream.Read(dataLength);
					state.data.resize(dataLength);
					bitStream.Read(&state.data[0], dataLength);
				}
			}
		}
	}

	EntityManager::EntityManager(EntityFactory *factory, Renderer *renderer, InputManager *input_manager, EntitySynchroniser *entity_synchroniser, StreamingManager *streaming)
		: m_EntityFactory(factory),
		m_Renderer(renderer),
		m_InputManager(input_manager),
		m_EntitySynchroniser(entity_synchroniser),
		m_Streaming(streaming),
		m_UpdateBlockedFlags(0),
		m_DrawBlockedFlags(0),
		m_EntitiesLocked(false),
		m_ClearWhenAble(false),
		m_NextId(1)
	{
		for (size_t i = 0; i < s_EntityDomainCount; ++i)
			m_DomainState[i] = DS_ALL;
	}

	EntityManager::~EntityManager()
	{
	}

	EntityPtr EntityManager::InstanceEntity(const std::string &type, const std::string &name, ObjectID owner_id)
	{
		EntityPtr entity = m_EntityFactory->InstanceEntity(type, name);
		if (entity)
		{
			entity->SetOwnerID(owner_id);
			AddEntity(entity);
		}
		return entity;
	}

	EntityFactory *EntityManager::GetFactory() const
	{
		return m_EntityFactory;
	}

	IDTranslator EntityManager::MakeIDTranslator() const
	{
		return IDTranslator(m_NextId-1);
	}

	void EntityManager::CompressIDs()
	{
		if (m_EntitiesLocked)
			return;

		{
			IDEntityMap::iterator it = m_Entities.begin();
			ObjectID nextSequentialId = 1;
			for (IDEntityMap::iterator end = m_Entities.end(); it != end; ++it)
			{
				if (it->first != nextSequentialId)
				{
					m_Entities[nextSequentialId] = it->second;
					it->second->SetID(nextSequentialId);
				}

				++nextSequentialId;
			}

			m_Entities.erase(it, m_Entities.end());
		}
	}

	void EntityManager::AddEntity(EntityPtr &entity)
	{
		if (m_EntitiesLocked)
			m_EntitiesToAdd.push_back( EntityToAdd(entity, false) );

		else
		{
			if (entity->GetID() == 0) // Get a free ID if one hasn't been prescribed
				entity->SetID(getFreeID());

			if (entity->GetName() == "default")
				entity->_notifyDefaultName(generateName(entity));

			IDEntityMap::iterator _where = m_Entities.find(entity->GetID());
			if (_where != m_Entities.end())
				FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::AddEntity", "An entity with the ID " + boost::lexical_cast<std::string>(entity->GetID()) + " already exists");

			m_Entities.insert(_where, std::pair<ObjectID, EntityPtr>( entity->GetID(), entity ));

			m_EntitiesByName[entity->GetName()] = entity;

			m_EntitiesToUpdate[entity->GetDomain()].push_back(entity);

			m_EntitySynchroniser->OnEntityAdded(entity);
		}
	}

	void EntityManager::AddPseudoEntity(EntityPtr &pseudo_entity)
	{
		if (m_EntitiesLocked)
			m_EntitiesToAdd.push_back( EntityToAdd(pseudo_entity, true) );

		else
		{
			if (pseudo_entity->GetName() == "default")
				pseudo_entity->_notifyDefaultName(generateName(pseudo_entity));

			NameEntityMap::iterator _where = m_EntitiesByName.find(pseudo_entity->GetName());
			if (_where != m_EntitiesByName.end())
				FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::AddEntity", "An entity with the name " + pseudo_entity->GetName() + " already exists");
			
			m_PseudoEntities.insert(pseudo_entity);
			m_EntitiesByName.insert(_where, NameEntityMap::value_type(pseudo_entity->GetName(), pseudo_entity) );

			m_EntitiesToUpdate[pseudo_entity->GetDomain()].push_back(pseudo_entity);
		}
	}

	void EntityManager::RemoveEntity(const EntityPtr &entity)
	{
		// Make sure the entity is removed from it's update domain
		entity->MarkToRemove();

		if (m_EntitiesLocked)
		{
			m_EntitiesToRemove.push_back(entity);
		}
		else
		{
			if (!entity->IsPseudoEntity())
			{
				if (entity->GetID() < m_NextId-1)
					m_UnusedIds.push_back(entity->GetID()); // record unused ID
				else
					--m_NextId;
				m_Entities.erase(entity->GetID());
			}
			else
				m_PseudoEntities.erase(entity);

			m_EntitiesByName.erase(entity->GetName());

			// Note that entity will be removed from the Domain list during the next Update() call
		}
	}

	void EntityManager::RemoveEntityNamed(const std::string &name)
	{
		RemoveEntity(GetEntity(name));
	}

	void EntityManager::RemoveEntityById(ObjectID id)
	{
		RemoveEntity(GetEntity(id));
	}

	void EntityManager::ReplaceEntity(ObjectID id, EntityPtr &entity)
	{
		if (m_EntitiesLocked)
			FSN_EXCEPT(ExCode::NotImplemented, "EntityManager::InsertEntity", "EntityManager is currently updating: Can't replace Entities while updating");
		
		// Make sure the entity to be inserted has the given ID
		entity->SetID(id);

		// Generate a name if necessary
		if (entity->GetName() == "default")
			entity->_notifyDefaultName(generateName(entity));

		// Try to insert the entity - overwrite if another is present
		EntityPtr &value = m_Entities[id];
		if (value.get() != NULL)
		{
			// Erase the existing Entity from the name map
			m_EntitiesByName.erase(value->GetName());
			// Mark the entity so it will be removed from it's update domain
			value->MarkToRemove();
		}
		// Replace the entry in the ID map (referenced by 'value') with the new entity
		value = entity;
		// Add the entity to the other indexes
		m_EntitiesByName[entity->GetName()] = entity;
		m_EntitiesToUpdate[entity->GetDomain()].push_back(entity);

		m_EntitySynchroniser->OnEntityAdded(entity);
	}

	void EntityManager::RenameEntity(EntityPtr &entity, const std::string &new_name)
	{
		NameEntityMap::iterator _where = m_EntitiesByName.find(entity->GetName());
		if (_where != m_EntitiesByName.end())
		{
			m_EntitiesByName.erase(_where);

			entity->_setName(new_name);
			m_EntitiesByName.insert( NameEntityMap::value_type(new_name, entity) );
		}
	}

	void EntityManager::RenameEntity(const std::string &current_name, const std::string &new_name)
	{
		NameEntityMap::iterator _where = m_EntitiesByName.find(current_name);
		if (_where != m_EntitiesByName.end())
		{
			EntityPtr entity = _where->second;
			m_EntitiesByName.erase(_where);

			entity->_setName(new_name);
			m_EntitiesByName.insert( NameEntityMap::value_type(new_name, entity) );
		}
	}

	bool isNamed(EntityManager::IDEntityMap::value_type &element, const std::string &name)
	{
		return element.second->GetName() == name;
	}

	EntityPtr EntityManager::GetEntity(const std::string &name, bool throwIfNotFound)
	{
		//IDEntityMap::iterator _where = std::find_if(m_Entities.begin(), m_Entities.end(), boost::bind(&isNamed, _1, name));
		NameEntityMap::iterator _where = m_EntitiesByName.find(name);
		if (_where == m_EntitiesByName.end())
			if (throwIfNotFound)
				FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::GetEntity", std::string("There is no entity called: ") + name);
			else
				return EntityPtr();
		return _where->second;
	}

	EntityPtr EntityManager::GetEntity(ObjectID id, bool throwIfNotFound)
	{
		IDEntityMap::iterator _where = m_Entities.find(id);
		if (_where == m_Entities.end())
			if (throwIfNotFound)
				FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::GetEntity", std::string("There is no entity with the ID: ") + boost::lexical_cast<std::string>(id));
			else
				return EntityPtr();
		return _where->second;
	}

	const EntityManager::IDEntityMap &EntityManager::GetEntities() const
	{
		return m_Entities;
	}

	const EntityManager::EntitySet &EntityManager::GetPseudoEntities() const
	{
		return m_PseudoEntities;
	}

	EntityArray &EntityManager::GetDomain(EntityDomain idx)
	{
		return m_EntitiesToUpdate[idx];
	}

	bool EntityManager::AddTag(const std::string &entity_name, const std::string &tag)
	{
		try
		{
			EntityPtr entity = GetEntity(entity_name);
			return AddTag(entity, tag);
		}
		catch (InvalidArgumentException &)
		{
			return false;
		}
	}

	bool EntityManager::AddTag(EntityPtr entity, const std::string &tag)
	{
		entity->AddTag(tag);
		return true;
	}

	void EntityManager::RemoveTag(const std::string &entity_name, const std::string &tag)
	{
		try
		{
			EntityPtr entity = GetEntity(entity_name);
			RemoveTag(entity, tag);
		}
		catch (InvalidArgumentException &)
		{
		}
	}

	void EntityManager::RemoveTag(EntityPtr entity, const std::string &tag)
	{
		try
		{
			entity->RemoveTag(tag);
		}
		catch (InvalidArgumentException &) // The given tag doesn't exist
		{
		}
	}

	bool EntityManager::CheckTag(const std::string &entity_name, const std::string &tag)
	{
		EntityPtr entity = GetEntity(entity_name);
		if (entity != NULL)
			return entity->CheckTag(tag);
		return false;
	}

	inline bool EntityManager::IsBlocked(EntityPtr entity, const BlockingChangeMap &tags)
	{
		for (BlockingChangeMap::const_iterator it = tags.begin(), end = tags.end(); it != end; ++it)
		{
			if (!it->second && entity->CheckTag(it->first))
				return true;
		}
		return false;
	}

	void EntityManager::PauseEntitiesWithTag(const std::string &tag)
	{
		m_ChangedUpdateStateTags[tag] = false;
	}

	void EntityManager::ResumeEntitiesWithTag(const std::string &tag)
	{
		m_ChangedUpdateStateTags[tag] = true;
	}

	void EntityManager::HideEntitiesWithTag(const std::string &tag)
	{
		m_ChangedDrawStateTags[tag] = false;
	}

	void EntityManager::ShowEntitiesWithTag(const std::string &tag)
	{
		m_ChangedDrawStateTags[tag] = true;
	}

	void EntityManager::RemoveEntitiesWithTag(const std::string &tag)
	{
		for (IDEntityMap::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			if (it->second->CheckTag(tag))
				RemoveEntity(it->second);
		}
	}

	void EntityManager::clearEntities(bool real_only)
	{
		if (m_EntitiesLocked)
		{
			for (size_t i = 0; i < s_EntityDomainCount; i++)
			{
				for (EntityArray::iterator it = m_EntitiesToUpdate[i].begin(), end = m_EntitiesToUpdate[i].end(); it != end; ++it)
				{
					EntityPtr &entity = *it;
					entity->MarkToRemove();
				}
			}
			m_EntitiesToAdd.clear();
			m_EntitiesToRemove.clear();
			m_EntitiesByName.clear();
			m_Entities.clear();
			m_PseudoEntities.clear();
			
			m_NextId = 1;
			m_UnusedIds.clear();

			m_ChangedUpdateStateTags.clear();
			m_ChangedDrawStateTags.clear();
		}
		else
		{
			m_EntitiesToAdd.clear();
			m_EntitiesToRemove.clear();
			for (size_t i = 0; i < s_EntityDomainCount; i++) // Clear all domains
				m_EntitiesToUpdate[i].clear();
			m_EntitiesByName.clear();
			m_Entities.clear();
			m_PseudoEntities.clear();

			m_NextId = 1;
			m_UnusedIds.clear();

			m_ChangedUpdateStateTags.clear();
			m_ChangedDrawStateTags.clear();
		}
	}

	void EntityManager::Clear()
	{
		clearEntities(false);
	}

	void EntityManager::ClearRealEntities()
	{
		clearEntities(true);
	}

	void EntityManager::ClearDomain(EntityDomain idx)
	{
		EntityArray &domain = m_EntitiesToUpdate[idx];
		for (EntityArray::iterator it = domain.begin(), end = domain.end(); it != end; ++it)
		{
			EntityPtr &entity = *it;

			entity->MarkToRemove();
			if (entity->IsPseudoEntity())
				m_PseudoEntities.erase(entity);
			else
			{
				if (entity->GetID() < m_NextId-1)
					m_UnusedIds.push_back(entity->GetID()); // record unused ID
				else
					--m_NextId;
				m_Entities.erase(entity->GetID());
			}
			m_EntitiesByName.erase(entity->GetName());
		}
		// If the manager isn't updating, we can immeadiately clear the
		//  domain (otherwise it will be cleared next time it is updated
		//  due to the MarkToRemove() call above)
		if (!m_EntitiesLocked)
			domain.clear();
	}

	typedef std::set<uintptr_t> ptr_set;
	void updateRenderables(EntityPtr &entity, float split, ptr_set &updated_sprites)
	{
		RenderableArray &renderables = entity->GetRenderables();
		for (RenderableArray::iterator it = renderables.begin(), end = renderables.end(); it != end; ++it)
		{
			RenderablePtr &renderable = *it;
			if (!renderable->IsPaused() && renderable->GetSpriteResource().IsLoaded())
			{
				std::pair<ptr_set::iterator, bool> result = updated_sprites.insert((uintptr_t)renderable->GetSpriteResource().Get());
				if (result.second)
					renderable->GetSpriteResource()->update((int)(split * 1000.f));
			}
			renderable->UpdateAABB();
		}
	}

	void EntityManager::Update(EntityDomain idx, float split)
	{
		EntityDeserialiser entityDeserialiser(this);

		bool entityRemoved = false;

		ptr_set updatedSprites;

		EntityArray::iterator it = m_EntitiesToUpdate[idx].begin(),
			end = m_EntitiesToUpdate[idx].end();
		while (it != end)
		{
			EntityPtr &entity = *it;

			// Check for reasons to remove the
			//  entity from the update domain
			if (entity->IsMarkedToRemove())
			{
				it = m_EntitiesToUpdate[idx].erase(it);
				end = m_EntitiesToUpdate[idx].end();

				entityRemoved = true;
			}
			else if (entity->GetTagFlags() & m_ToDeleteFlags)
			{
				RemoveEntity(entity);
				it = m_EntitiesToUpdate[idx].erase(it);
				end = m_EntitiesToUpdate[idx].end();
			}

			// Also make sure the entity isn't blocked by a flag
			else if ((entity->GetTagFlags() & m_UpdateBlockedFlags) == 0)
			{
				if (idx != SYSTEM_DOMAIN && CheckState(idx, DS_STREAMING))
					m_Streaming->ProcessEntity(entity); // stream the entity in if it is within range of any cameras

				if (entity->Wait())
				{
					if (CheckState(idx, DS_SYNCH)) m_EntitySynchroniser->ReceiveSync(entity, entityDeserialiser);
					if (CheckState(idx, DS_ENTITYUPDATE)) entity->Update(split);
					if (CheckState(idx, DS_SYNCH)) m_EntitySynchroniser->AddToPacket(entity);

					updateRenderables(entity, split, updatedSprites);

					if (entity->IsStreamedOut())
						entity->SetWait(2);
				}
				else
					entity->PacketSkipped();

				// Next entity
				++it;
			}
		}

		if (entityRemoved) // Do a full GC cycle if entities were removed
		{
			ScriptManager::getSingleton().GetEnginePtr()->GarbageCollect();
		}
	}

	void EntityManager::Update(float split)
	{
		m_EntitiesLocked = true;

		for (size_t i = 0; i < s_EntityDomainCount; i++)
		{
			if (m_DomainState[i])
				Update(i, split);
		}

		m_EntitiesLocked = false;

		// Clear the ToDeleteFlags
		m_ToDeleteFlags = 0;

		// Actually remove entities which were marked for removal during update
		for (EntityArray::iterator it = m_EntitiesToRemove.begin(), end = m_EntitiesToRemove.end(); it != end; ++it)
		{
			m_Entities.erase((*it)->GetID());
			m_EntitiesByName.erase((*it)->GetName());
		}
		m_EntitiesToRemove.clear();

		// Actually add entities which were 'added' during the update
		for (EntityToAddArray::iterator it = m_EntitiesToAdd.begin(), end = m_EntitiesToAdd.end(); it != end; ++it)
		{
			if (it->second)
				AddPseudoEntity(it->first);
			else
				AddEntity(it->first);
		}
		m_EntitiesToAdd.clear();
	}

	void EntityManager::Draw(Renderer *renderer, const ViewportPtr &viewport, size_t layer)
	{
		renderer->Draw(m_EntitiesToUpdate[GAME_DOMAIN], viewport, layer);
	}

	void EntityManager::SetDomainState(EntityDomain domain_index, char modes)
	{
		m_DomainState[domain_index] = modes;
	}

	bool EntityManager::CheckState(EntityDomain domain_index, DomainState mode) const
	{
		return (m_DomainState[domain_index] & mode) == mode;
	}

	char EntityManager::GetDomainState(EntityDomain domain_index) const
	{
		return m_DomainState[domain_index];
	}

	void EntityManager::SetModule(ModulePtr module)
	{
		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( boost::bind(&EntityManager::OnModuleRebuild, this, _1) );
	}

	void EntityManager::OnModuleRebuild(BuildModuleEvent& ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
		{
			ev.manager->RegisterGlobalObject("EntityManager entity_manager", this);
		}
	}

	asIScriptObject* EntityManager_InstanceEntity(const std::string &type, EntityManager *obj)
	{
		Entity *entity = obj->InstanceEntity(type).get();
		//entity->Spawn();
		return ScriptedEntity::GetScriptObject( entity );
	}

	asIScriptObject* EntityManager_InstanceEntity(const std::string &type, ObjectID owner, EntityManager *obj)
	{
		Entity *entity = obj->InstanceEntity(type, "default", owner).get();
		//entity->Spawn();
		return ScriptedEntity::GetScriptObject( entity );
	}

	asIScriptObject* EntityManager_InstanceEntity(const std::string &type, const std::string &name, EntityManager *obj)
	{
		Entity *entity = obj->InstanceEntity(type, name).get();
		//entity->Spawn();
		return ScriptedEntity::GetScriptObject( entity );
	}

	asIScriptObject* EntityManager_InstanceEntity(const std::string &type, const std::string &name, ObjectID owner, EntityManager *obj)
	{
		Entity *entity = obj->InstanceEntity(type, name, owner).get();
		//entity->Spawn();
		return ScriptedEntity::GetScriptObject( entity );
	}

	void EntityManager_RemoveEntity(const std::string &name, EntityManager *obj)
	{
		obj->RemoveEntityNamed(name);
	}

	void EntityManager_RemoveEntity(ObjectID id, EntityManager *obj)
	{
		obj->RemoveEntityById(id);
	}

	void EntityManager_RemoveEntity(asIScriptObject *script_entity, EntityManager *obj)
	{
		obj->RemoveEntity( EntityPtr(ScriptedEntity::GetAppObject(script_entity)) );
		script_entity->Release();
	}

	void EntityManager::Register(asIScriptEngine *engine)
	{
		int r;
		// TODO: some way to set the domain - either more instance() methods (with domain param.s) or
		//  a way to instance entities without adding them to the EntityManager (factory access or 
		//  more instance() methods that work like EntityFactory's instance method)
		RegisterSingletonType<EntityManager>("EntityManager", engine);
		r = engine->RegisterObjectMethod("EntityManager",
			"IEntity@ instance(const string &in)",
			asFUNCTIONPR(EntityManager_InstanceEntity, (const std::string &, EntityManager*), asIScriptObject*), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"IEntity@ instance(const string &in, uint16)",
			asFUNCTIONPR(EntityManager_InstanceEntity, (const std::string &, ObjectID, EntityManager*), asIScriptObject*), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"IEntity@ instance(const string &in, const string &in)",
			asFUNCTIONPR(EntityManager_InstanceEntity, (const std::string &, const std::string &, EntityManager*), asIScriptObject*), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"IEntity@ instance(const string &in, const string &in, uint16)",
			asFUNCTIONPR(EntityManager_InstanceEntity, (const std::string &, const std::string &, ObjectID, EntityManager*), asIScriptObject*), asCALL_CDECL_OBJLAST);
		
		r = engine->RegisterObjectMethod("EntityManager",
			"void remove(const string &in)",
			asFUNCTIONPR(EntityManager_RemoveEntity, (const std::string &, EntityManager*), void), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"void remove(uint16)",
			asFUNCTIONPR(EntityManager_RemoveEntity, (ObjectID, EntityManager*), void), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"void remove(IEntity@)",
			asFUNCTIONPR(EntityManager_RemoveEntity, (asIScriptObject*, EntityManager*), void), asCALL_CDECL_OBJLAST);
	}

	ObjectID EntityManager::getFreeID()
	{
		if (m_UnusedIds.empty())
			return m_NextId++;
		else
		{
			ObjectID id = m_UnusedIds.back();
			m_UnusedIds.pop_back();
			return id;
		}
	}

	std::string EntityManager::generateName(const EntityPtr &entity)
	{
		std::stringstream stream;
		if (entity->IsPseudoEntity())
			stream << "__entity_pseudo_" << (unsigned int)entity.get();
		else
			stream << "__entity_id_" << entity->GetID();
		return stream.str();
	}

	void EntityManager::updateTags(EntityPtr &entity) const
	{
		for (BlockingChangeMap::const_iterator it = m_ChangedUpdateStateTags.begin(), end = m_ChangedUpdateStateTags.end(); it != end; ++it)
		{
			if (it->second)
				entity->_notifyResumedTag(it->first);
			else
				entity->_notifyHiddenTag(it->first);
		}

		for (BlockingChangeMap::const_iterator it = m_ChangedDrawStateTags.begin(), end = m_ChangedDrawStateTags.end(); it != end; ++it)
		{
			if (it->second)
				entity->_notifyShownTag(it->first);
			else
				entity->_notifyHiddenTag(it->first);
		}
	}

}
