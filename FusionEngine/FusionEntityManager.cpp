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

#include "FusionEntityFactory.h"
#include "FusionClientOptions.h"
#include "FusionPlayerRegistry.h"

#include "FusionNetworkTypes.h"
#include "FusionNetworkSystem.h"

#include <boost/lexical_cast.hpp>
#include <BitStream.h>

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
		//! \todo TODO: unmuddle this method

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
				playerInput->SetPosition(ev.Input, ev.Value);

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
		m_PacketData.Reset();

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
	}

	void EntitySynchroniser::Send(ObjectID player)
	{
		const PlayerRegistry::PlayerInfo &info = PlayerRegistry::GetPlayerByNetIndex(player);
		const NetHandle &address = info.System;

		//RakNetHandleImpl* rakAddress = dynamic_cast<RakNetHandleImpl*>(address.get());

		//RakNetwork *network = dynamic_cast<RakNetwork*>( m_Network );
		//const RakPeerInterface *peer = network->GetRakNetPeer();

		if (m_ImportantMove)
		{
			m_Network->SendRaw((const char*)m_PacketData.GetData(), m_PacketData.GetNumberOfBytesUsed(), MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_INPUTUPDATE, address);
		}
		else
		{
			m_Network->SendRaw((const char*)m_PacketData.GetData(), m_PacketData.GetNumberOfBytesUsed(), MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, CID_ENTITYMANAGER, address);
		}
	}

	void EntitySynchroniser::OnEntityAdded(EntityPtr &entity)
	{
		//for (unsigned int i = 0; i < g_MaxLocalPlayers; ++i)
		//{
		//	const PlayerRegistry::PlayerInfo &playerInfo = ;
		//	if (playerInfo.LocalIndex != g_MaxLocalPlayers && playerInfo.NetIndex == entity->GetOwnerID())
		//}

		const PlayerRegistry::PlayerInfo &playerInfo = PlayerRegistry::GetPlayerByNetIndex(entity->GetOwnerID());
		if (playerInfo.LocalIndex != g_MaxLocalPlayers)
		{
			PlayerInputPtr playerInput = m_PlayerInputs->GetInputsForPlayer(playerInfo.NetIndex);
			if (playerInput)
				entity->_setPlayerInput(playerInput);
		}
	}

	bool EntitySynchroniser::ReceiveSync(EntityPtr &entity, const EntityDeserialiser &entity_deserialiser)
	{
		ReceivedStatesMap::const_iterator _where = m_ReceivedStates.find(entity->GetID());
		if (_where != m_ReceivedStates.end())
			entity->DeserialiseState(_where->second, false, entity_deserialiser);

		return true; // Return false to not update this entity
	}

	bool EntitySynchroniser::SendSync(EntityPtr &entity)
	{
		SerialisedData state;
		entity->SerialiseState(state, false);

		size_t lengthAfterWrite = m_PacketData.GetNumberOfBytesUsed() + state.data.length() + sizeof(unsigned int) * 2;
		if (lengthAfterWrite > s_MaxEntityPacketSize)
		{
			return false;
		}
		
		//m_PacketData.AddBitsAndReallocate(state.data.length() + sizeof(unsigned int) * 2);
		m_PacketData.Write(state.mask);
		m_PacketData.Write(state.data.length());
		m_PacketData.Write(state.data.c_str(), state.data.length());

		//m_PacketData.append((char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed());

		return true;
	}

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

	EntityManager::EntityManager(Renderer *renderer, InputManager *input_manager, EntitySynchroniser *entity_synchroniser)
		: m_Renderer(renderer),
		m_InputManager(input_manager),
		m_EntitySynchroniser(entity_synchroniser),
		m_UpdateBlockedFlags(0),
		m_DrawBlockedFlags(0),
		m_EntitiesLocked(false),
		m_NextId(1)
	{
		m_EntityFactory = new EntityFactory();
	}

	EntityManager::~EntityManager()
	{
		delete m_EntityFactory;
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

	void EntityManager::AddEntity(EntityPtr entity)
	{
		if (m_EntitiesLocked)
			m_EntitiesToAdd.push_back( EntityToAdd(entity, false) );

		else
		{
			if (entity->GetID() == 0) // Get a free ID if one hasn't been prescribed
				entity->SetID(getFreeID());

			if (entity->GetName() == "default")
				entity->_setName(generateName(entity));

			IDEntityMap::iterator _where = m_Entities.find(entity->GetID());
			if (_where != m_Entities.end())
				FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::AddEntity", "An entity with the ID " + boost::lexical_cast<std::string>(entity->GetID()) + " already exists");

			m_Entities.insert(_where, std::pair<ObjectID, EntityPtr>( entity->GetID(), entity ));

			m_EntitiesByName[entity->GetName()] = entity;

			m_Renderer->Add(entity);
			m_EntitySynchroniser->OnEntityAdded(entity);

			entity->Spawn();

			m_EntitiesToUpdate[entity->GetDomain()].push_back(entity);
		}
	}

	void EntityManager::AddPseudoEntity(EntityPtr pseudo_entity)
	{
		if (m_EntitiesLocked)
			m_EntitiesToAdd.push_back( EntityToAdd(pseudo_entity, true) );

		else
		{
			if (pseudo_entity->GetName() == "default")
				pseudo_entity->_setName(generateName(pseudo_entity));

			NameEntityMap::iterator _where = m_EntitiesByName.find(pseudo_entity->GetName());
			if (_where != m_EntitiesByName.end())
				FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::AddEntity", "An entity with the name " + pseudo_entity->GetName() + " already exists");
			
			m_PseudoEntities.insert(pseudo_entity);
			m_EntitiesByName.insert(_where, NameEntityMap::value_type(pseudo_entity->GetName(), pseudo_entity) );

			pseudo_entity->Spawn();

			m_EntitiesToUpdate[pseudo_entity->GetDomain()].push_back(pseudo_entity);
		}
	}

	void EntityManager::RemoveEntity(EntityPtr entity)
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
				m_Entities.erase(entity->GetID());
			}
			else
				m_PseudoEntities.erase(entity);

			m_EntitiesByName.erase(entity->GetName());

			m_Renderer->Remove(entity);
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

	void EntityManager::ReplaceEntity(ObjectID id, EntityPtr entity)
	{
		if (m_EntitiesLocked)
			FSN_EXCEPT(ExCode::NotImplemented, "EntityManager::InsertEntity", "EntityManager is currently updating: Can't replace Entities while updating");
		
		// Make sure the entity to be inserted has the given ID
		entity->SetID(id);

		// Generate a name if necessary
		if (entity->GetName() == "default")
			entity->_setName(generateName(entity));

		// Try to insert the entity - overwrite if another is present
		EntityPtr &value = m_Entities[id];
		if (value.get() != NULL)
		{
			// Erase the existing Entity from the name map
			m_EntitiesByName.erase(value->GetName());
			m_Renderer->Remove(value);
			// Mark the entity so it will be removed from it's update domain
			value->MarkToRemove();
		}
		// Replace the map-entry (referenced by 'value') with the new entity
		value = entity;

		//std::pair<IDEntityMap::iterator, bool> check = m_Entities.insert( IDEntityMap::value_type(entity->GetID(), entity) );
		//if (!check.second)
		//{
		//	// Erase the existing entity from both maps
		//	m_EntitiesByName.erase(check.first->second->GetName());
		//	m_Entities.erase(check.first);
		//	// Insert the new Entity
		//	m_Entities.insert(check.first, IDEntityMap::value_type( entity->GetID(), entity ));
		//}

		m_EntitiesByName[entity->GetName()] = entity;

		m_Renderer->Add(entity);
		m_EntitySynchroniser->OnEntityAdded(entity);
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

		m_Renderer->HideTag(tag);
	}

	void EntityManager::ShowEntitiesWithTag(const std::string &tag)
	{
		m_ChangedDrawStateTags[tag] = true;

		m_Renderer->ShowTag(tag);
	}

	void EntityManager::RemoveEntitiesWithTag(const std::string &tag)
	{
		for (IDEntityMap::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			if (it->second->CheckTag(tag))
				RemoveEntity(it->second);
		}
	}

	void EntityManager::Clear()
	{
		m_EntitiesToAdd.clear();
		m_EntitiesToRemove.clear();
		for (size_t i = 0; i < s_EntityDomainCount; i++) // Clear all domains
				m_EntitiesToUpdate[i].clear();
		m_EntitiesToDraw.clear();
		m_EntitiesByName.clear();
		m_Entities.clear();
		m_PseudoEntities.clear();

		m_ChangedUpdateStateTags.clear();
		m_ChangedDrawStateTags.clear();
	}

	void updateRenderables(EntityPtr &entity, float split)
	{
		//const Vector2 &position = entity->GetPosition();
		//float angle = entity->GetAngle();

		RenderableArray &renderables = entity->GetRenderables();
		for (RenderableArray::iterator it = renderables.begin(), end = renderables.end(); it != end; ++it)
		{
			(*it)->Update(split/*, position, angle*/);
		}
	}

	void EntityManager::Update(EntityDomain idx, float split)
	{
		EntityDeserialiser entityDeserialiser(this);

		EntityArray::iterator it = m_EntitiesToUpdate[idx].begin(),
			end = m_EntitiesToUpdate[idx].end();
		while (it != end)
		{
			EntityPtr &entity = *it;

			//updateTags(entity);

			// Check for reasons to remove the
			//  entity from the update domain
			if (entity->IsMarkedToRemove())
			{
				it = m_EntitiesToUpdate[idx].erase(it);
			}
			else if (entity->GetTagFlags() & m_ToDeleteFlags)
			{
				RemoveEntity(entity);
				it = m_EntitiesToUpdate[idx].erase(it);
			}

			// Also make sure the entity isn't blocked by a flag
			else if ((entity->GetTagFlags() & m_UpdateBlockedFlags) == 0)
			{
				if (entity->Wait())
				{
					m_EntitySynchroniser->ReceiveSync(entity, entityDeserialiser);
					entity->Update(split);
					m_EntitySynchroniser->SendSync(entity);

					updateRenderables(entity, split);

					if (entity->IsStreamedOut())
						entity->SetWait(2);
				}

				// Next entity
				++it;
			}
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

			m_Renderer->Remove(*it);
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

	void EntityManager::Draw()
	{
	}

	void EntityManager::SetDomainState(EntityDomain domain_index, bool active)
	{
		m_DomainState[domain_index] = active;
	}

	bool EntityManager::DomainIsActive(EntityDomain domain_index) const
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
		return ScriptedEntity::GetScriptObject( obj->InstanceEntity(type).get() );
	}

	asIScriptObject* EntityManager_InstanceEntity(const std::string &type, ObjectID owner, EntityManager *obj)
	{
		return ScriptedEntity::GetScriptObject( obj->InstanceEntity(type, "default", owner).get() );
	}

	asIScriptObject* EntityManager_InstanceEntity(const std::string &type, const std::string &name, EntityManager *obj)
	{
		return ScriptedEntity::GetScriptObject( obj->InstanceEntity(type, name).get() );
	}

	asIScriptObject* EntityManager_InstanceEntity(const std::string &type, const std::string &name, ObjectID owner, EntityManager *obj)
	{
		return ScriptedEntity::GetScriptObject( obj->InstanceEntity(type, name, owner).get() );
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
			"IEntity@ instance(const string &in)",
			asFUNCTIONPR(EntityManager_InstanceEntity, (const std::string &, ObjectID, EntityManager*), asIScriptObject*), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"IEntity@ instance(const string &in, const string &in)",
			asFUNCTIONPR(EntityManager_InstanceEntity, (const std::string &, const std::string &, EntityManager*), asIScriptObject*), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"IEntity@ instance(const string &in)",
			asFUNCTIONPR(EntityManager_InstanceEntity, (const std::string &, const std::string &, ObjectID, EntityManager*), asIScriptObject*), asCALL_CDECL_OBJLAST);
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

	std::string EntityManager::generateName(EntityPtr entity)
	{
		std::stringstream stream;
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
