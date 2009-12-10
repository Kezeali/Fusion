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

#include "FusionClientOptions.h"
#include "FusionPlayerRegistry.h"

#include "FusionGameMapLoader.h"
#include "FusionEntityManager.h"
#include "FusionEntityFactory.h"

#include <boost/lexical_cast.hpp>


namespace FusionEngine
{

	GameMapLoader::GameMapLoader(ClientOptions *options, EntityManager *manager)
		: m_ClientOptions(options),
		m_Manager(manager),
		m_NextTypeIndex(0)
	{
		m_FactoryConnection = m_Manager->GetFactory()->SignalEntityInstanced.connect( boost::bind(&GameMapLoader::onEntityInstanced, this, _1) );
	}

	GameMapLoader::~GameMapLoader()
	{
		m_FactoryConnection.disconnect();
	}

	void GameMapLoader::LoadEntityTypes(const std::string &filename, CL_VirtualDirectory &directory)
	{
		CL_IODevice device = directory.open_file(fe_widen(filename), CL_File::open_existing, CL_File::access_read);
		LoadEntityTypes(device);
	}

	void GameMapLoader::LoadEntityTypes(CL_IODevice &device)
	{
		EntityFactory *factory = m_Manager->GetFactory();
		// Read the entity type count
		cl_int32 numberEntityTypes = device.read_int32();
		// Tell the entity-factory to load each entity type listed in the map file
		//CL_String8 entityTypename;
		for (cl_int32 i = 0; i < numberEntityTypes; i++)
		{
			//entityTypename = device.read_string_a();
			factory->LoadScriptedType(/*entityTypename*/device.read_string_a().c_str());
		}
	}

	void GameMapLoader::LoadMap(const std::string &filename, CL_VirtualDirectory &directory, bool pseudo_only)
	{
		CL_IODevice device = directory.open_file(fe_widen(filename), CL_File::open_existing, CL_File::access_read);

		m_MapFilename = filename;

		m_Manager->Clear();

		// Read the entity type count
		cl_uint32 numberEntityTypes = device.read_uint32();
		// List each entity type
		//StringVector entityTypeArray(numberEntityTypes);
		m_TypeIndex.clear();
		{
			CL_String8 entityTypename;
			for (m_NextTypeIndex = 0; m_NextTypeIndex < numberEntityTypes; ++m_NextTypeIndex)
			{
				entityTypename = device.read_string_a();
				//entityTypeArray.push_back(entityTypename.c_str());
				m_TypeIndex.insert( TypeIndex::value_type(entityTypename.c_str(), m_NextTypeIndex) );
			}
		}

		TypeIndex::right_map &entityTypeArray = m_TypeIndex.right;

		// Load Archetypes
		cl_uint32 numberArchetypes = device.read_uint32();
		ArchetypeArray archetypeArray(numberArchetypes);
		for (cl_uint32 i = 0; i < numberArchetypes; i++)
		{
			cl_uint32 entityTypeIndex = device.read_uint32();
			const std::string &entityTypename = entityTypeArray.at(entityTypeIndex);

			Archetype &archetype = archetypeArray[i];

			archetype.entityIndex = entityTypeIndex;
			archetype.entityTypename = entityTypename;

			archetype.packet.mask = device.read_uint32();
			archetype.packet.data = device.read_string_a().c_str();
		}

		IDTranslator translator;

		loadPseudoEntities(device, archetypeArray, translator);
		loadEntities(device, archetypeArray, translator);
	}

	void GameMapLoader::loadPseudoEntities(CL_IODevice &device, const ArchetypeArray &archetypeArray, const IDTranslator &translator)
	{
		EntityFactory *factory = m_Manager->GetFactory();

		TypeIndex::right_map &entityTypeArray = m_TypeIndex.right;

		// Load & instance Entities
		cl_uint32 numberEntities = device.read_uint32();
		EntityArray instancedEntities;
		instancedEntities.reserve(numberEntities);
		{
			std::string entityName;
			EntityPtr entity;
			for (cl_uint32 i = 0; i < numberEntities; i++)
			{
				cl_uint32 typeIndex = device.read_uint32();

				entityName = device.read_string_a().c_str();
				if (entityName.empty())
					entityName = "default";

				{
					const std::string &entityTypename = entityTypeArray.at(typeIndex);
					entity = factory->InstanceEntity(entityTypename, entityName);
				}

				m_Manager->AddPseudoEntity(entity);

				instancedEntities.push_back(entity);
			}
		}

		// Spawn then deserialise each instanced entity
		{
			EntityDeserialiser entity_deserialiser(m_Manager, translator);
			SerialisedData state;
			for (EntityArray::iterator it = instancedEntities.begin(), end = instancedEntities.end(); it != end; ++it)
			{
				EntityPtr &entity = (*it);

				// Basic Entity properties (position, angle)
				Vector2 position;
				position.x = device.read_float();
				position.y = device.read_float();
				entity->SetPosition(position);

				entity->SetAngle(device.read_float());

				// Call the entity's spawn method
				entity->Spawn();

				cl_uint8 typeFlags = device.read_uint8();
				// Load archetype
				if (typeFlags & ArchetypeFlag) // Check for archetype flag
				{
					cl_uint32 typeIndex = device.read_uint32();
					const Archetype &archetype = archetypeArray[typeIndex];

					// Check that the archetype data is for the correct entity type before deserializing
					FSN_ASSERT(entity->GetType() == archetype.entityTypename);
					entity->DeserialiseState(archetype.packet, true, entity_deserialiser);
				}

				// Load specific entity state
				state.mask = device.read_uint32();
				state.data = device.read_string_a().c_str();

				entity->DeserialiseState(state, true, entity_deserialiser);
			}
		}
	}

	void GameMapLoader::loadEntities(CL_IODevice &device, const ArchetypeArray &archetypeArray, const IDTranslator &translator)
	{
		EntityFactory *factory = m_Manager->GetFactory();

		TypeIndex::right_map &entityTypeArray = m_TypeIndex.right;

		// Load & instance Entities
		cl_uint32 numberEntities = device.read_uint32();
		EntityArray instancedEntities;
		instancedEntities.reserve(numberEntities);
		{
			std::string entityName; ObjectID entityID;
			EntityPtr entity;
			for (cl_uint32 i = 0; i < numberEntities; i++)
			{
				cl_uint32 typeIndex = device.read_uint32();

				entityName = device.read_string_a().c_str();
				if (entityName.empty())
					entityName = "default";

				device.read((void*)&entityID, sizeof(ObjectID));

				{
					const std::string &entityTypename = entityTypeArray.at(typeIndex);
					entity = factory->InstanceEntity(entityTypename, entityName);
				}

				entity->SetID(translator(entityID));
				m_Manager->AddEntity(entity);

				instancedEntities.push_back(entity);
			}
		}

		// Spawn then deserialise each instanced entity
		{
			EntityDeserialiser entity_deserialiser(m_Manager, translator);
			SerialisedData state;
			for (EntityArray::iterator it = instancedEntities.begin(), end = instancedEntities.end(); it != end; ++it)
			{
				EntityPtr &entity = (*it);

				// Basic Entity properties (position, angle)
				Vector2 position;
				position.x = device.read_float();
				position.y = device.read_float();
				entity->SetPosition(position);

				entity->SetAngle(device.read_float());

				// Call the entity's spawn method
				entity->Spawn();

				cl_uint8 typeFlags = device.read_uint8();
				// Load archetype
				if (typeFlags & ArchetypeFlag) // Check for archetype flag
				{
					cl_uint32 typeIndex = device.read_uint32();
					const Archetype &archetype = archetypeArray[typeIndex];

					// Check that the archetype data is for the correct entity type before deserializing
					FSN_ASSERT(entity->GetType() == archetype.entityTypename);
					entity->DeserialiseState(archetype.packet, true, entity_deserialiser);
				}

				// Load specific entity state
				state.mask = device.read_uint32();
				state.data = device.read_string_a().c_str();

				entity->DeserialiseState(state, true, entity_deserialiser);
			}
		}
	}

	void GameMapLoader::LoadSavedGame(const std::string &filename, CL_VirtualDirectory &directory)
	{
		CL_IODevice device = directory.open_file(fe_widen(filename), CL_File::open_existing, CL_File::access_read);

		// General info
		std::string date;
		date.resize(19);
		device.read(&date[0], 19);

		// Local players
		PlayerRegistry::Clear();

		size_t numLocalPlayers = device.read_uint32();
		m_ClientOptions->SetOption("num_local_players", boost::lexical_cast<std::string>(numLocalPlayers));
		for (unsigned int i = 0; i < numLocalPlayers; ++i)
		{
			ObjectID netIndex = device.read_uint16();
			PlayerRegistry::AddPlayer(netIndex, i);
		}

		// Map filename
		std::string mapFilename = device.read_string_a().c_str();

		if (!mapFilename.empty() && mapFilename != m_MapFilename)
			LoadMap(mapFilename, directory, true);
		else
			m_Manager->ClearRealEntities();

		EntityFactory *factory = m_Manager->GetFactory();
		IDTranslator translator = m_Manager->MakeIDTranslator();

		// Load & instance Entities (this section is loaded the same way as the equivilant section in the map file)
		// TODO: share code with map file loading?
		cl_uint32 numberEntities = device.read_uint32();
		EntityArray instancedEntities;
		instancedEntities.reserve(numberEntities);
		// The TypeIndex bimap maps type-name to index, so the right_map is index to name
		TypeIndex::right_map &indexToName = m_TypeIndex.right;
		{
			std::string entityName; ObjectID entityID;
			EntityPtr entity;
			for (cl_uint32 i = 0; i < numberEntities; i++)
			{
				cl_uint32 typeIndex = device.read_uint32();

				entityName = device.read_string_a().c_str();
				device.read((void*)&entityID, sizeof(ObjectID));

				{
					const std::string &entityTypename = indexToName.at(typeIndex);
					entity = factory->InstanceEntity(entityTypename, entityName);
				}

				entity->SetID(translator(entityID));
				m_Manager->AddEntity(entity);

				instancedEntities.push_back(entity);
			}
		}

		// Deserialise each instanced entity (the only difference between this and what is done to load
		//  a _map_ file is that the Entity's Spawn() method is not called)
		{
			EntityDeserialiser entity_deserialiser(m_Manager, translator);
			SerialisedData state;
			for (EntityArray::iterator it = instancedEntities.begin(), end = instancedEntities.end(); it != end; ++it)
			{
				EntityPtr &entity = (*it);

				// Basic Entity properties (position, angle)
				Vector2 position;
				position.x = device.read_float();
				position.y = device.read_float();
				entity->SetPosition(position);

				entity->SetAngle(device.read_float());

				// Load entity state
				state.mask = device.read_uint32();
				state.data = device.read_string_a().c_str();

				entity->DeserialiseState(state, true, entity_deserialiser);
			}
		}
	}

	void GameMapLoader::SaveGame(const std::string &filename, CL_VirtualDirectory &directory)
	{
		CL_IODevice device = directory.open_file(fe_widen(filename), CL_File::create_always, CL_File::access_write, 0);

		// Write save info
		//  Date
		CL_String date = CL_DateTime::get_current_local_time().to_short_datetime_string();
		device.write(CL_StringHelp::text_to_utf8(date).c_str(), 19);
		//  Players
		int numLocalPlayers;
		m_ClientOptions->GetOption("num_local_players", &numLocalPlayers);
		device.write_uint32((unsigned)numLocalPlayers);
		//  Write net-indicies so they can be restored - net-indicies
		//  must be the same from session to session for Entity ownership.
		for (unsigned int i = 0; i < (unsigned)numLocalPlayers; i++)
			device.write_uint16(PlayerRegistry::GetPlayerByLocalIndex(i).NetIndex);

		// Map filename
		device.write_string_a(m_MapFilename.c_str());

		const EntityManager::IDEntityMap &entities = m_Manager->GetEntities();

		// Write Entities
		device.write_uint32(entities.size());
		for (EntityManager::IDEntityMap::const_iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const EntityPtr &entity = it->second;
			
			// Write the type index (refers to type-index in the map file)
			device.write_uint32( m_TypeIndex.left.at(entity->GetType()) );

			// Write the Entity name
			if (!entity->HasDefaultName())
				device.write_string_a(entity->GetName().c_str());
			else
				device.write_string_a(CL_String8());

			// Write the Entity ID
			device.write_uint16(entity->GetID());
		}
		// Entity data
		SerialisedData state;
		for (EntityManager::IDEntityMap::const_iterator it = entities.begin(), end = entities.end();
			it != end; ++it)
		{
			const EntityPtr &entity = it->second;

			// Write basic Entity properties (position, angle)
			const Vector2 &position = entity->GetPosition();
			device.write_float(position.x);
			device.write_float(position.y);
			device.write_float(entity->GetAngle());

			// Write the entity state
			entity->SerialiseState(state, false);

			device.write_uint32(state.mask);
			device.write_string_a(state.data.c_str());
		}
	}


	void GameMapLoader::CompileMap(CL_IODevice &device, const StringSet &used_entity_types, const GameMapLoader::ArchetypeMap &archetypes, const GameMapLoader::GameMapEntityArray &pseudo_entities, const GameMapLoader::GameMapEntityArray &entities)
	{
		// Write used types list
		// Number of types:
		device.write_uint32(used_entity_types.size());

		// Used for getting the index at which a given type was listed in the used type list (which is about to be written)
		typedef std::tr1::unordered_map<std::string, cl_uint32> UsedTypeMap;
		UsedTypeMap usedTypeIndexes;

		{
			cl_uint32 type_index = 0;
			for (StringSet::const_iterator it = used_entity_types.begin(), end = used_entity_types.end(); it != end; ++it)
			{
				device.write_string_a(it->c_str());
				
				usedTypeIndexes[*it] = type_index++;
			}
		}

		// Write archetype list
		device.write_uint32(archetypes.size());

		UsedTypeMap usedArchetypeIndexes;

		{
			cl_uint32 type_index = 0;
			for (ArchetypeMap::const_iterator it = archetypes.begin(), end = archetypes.end(); it != end; ++it)
			{
				// Write the type index (refers to the previously written used-type-list)
				device.write_uint32(usedTypeIndexes[it->second.entityTypename]);

				// Write the state information
				device.write_uint32(it->second.packet.mask);
				device.write_string_a(it->second.packet.data);

				usedArchetypeIndexes[it->first] = type_index++;
			}
		}

		// Write Pseudo-Entities
		device.write_uint32(pseudo_entities.size());
		for (GameMapEntityArray::const_iterator it = pseudo_entities.begin(), end = pseudo_entities.end(); it != end; ++it)
		{
			const GameMapEntityPtr &mapEntity = *it;
			const EntityPtr &entity = mapEntity->entity;
			
			// Write the type index (refers to used-type-index at the top of the file)
			device.write_uint32(usedTypeIndexes[entity->GetType()]);

			// Write the Entity name
			if (mapEntity->hasName)
				device.write_string_a(entity->GetName().c_str());
			else
				device.write_string_a(CL_String8());
		}

		// Write Pseudo-Entity state data
		SerialisedData state;
		for (GameMapEntityArray::const_iterator it = pseudo_entities.begin(), end = pseudo_entities.end(); it != end; ++it)
		{
			const GameMapEntityPtr &mapEntity = *it;
			const EntityPtr &entity = mapEntity->entity;

			// Write basic Entity properties (position, angle)
			const Vector2 &position = entity->GetPosition();
			device.write_float(position.x);
			device.write_float(position.y);

			device.write_float(entity->GetAngle());

			// Write the type-flags for the entity data
			if (!mapEntity->archetypeId.empty())
			{
				device.write_uint8(ArchetypeFlag);
				// Write the index of the archetype used
				device.write_uint32(usedArchetypeIndexes[mapEntity->archetypeId]);
			}
			else
				device.write_uint8(NoTypeFlags);

			// Write the entity state
			state.mask = mapEntity->stateMask;
			entity->SerialiseState(state, true);

			device.write_uint32(state.mask);
			device.write_string_a(state.data.c_str());
		}

		// Write Entities
		device.write_uint32(entities.size());
		for (GameMapEntityArray::const_iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const GameMapEntityPtr &mapEntity = *it;
			const EntityPtr &entity = mapEntity->entity;
			
			// Write the type index (refers to used-type-index at the top of the file)
			device.write_uint32(usedTypeIndexes[entity->GetType()]);

			// Write the Entity name
			if (mapEntity->hasName)
				device.write_string_a(entity->GetName().c_str());
			else
				device.write_string_a(CL_String8());
			// Write the Entity ID
			device.write_uint16(entity->GetID());
			//device.write((void*)&entity->GetID(), sizeof(ObjectID));
		}

		// Write Entity state data
		for (GameMapEntityArray::const_iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const GameMapEntityPtr &mapEntity = *it;
			const EntityPtr &entity = mapEntity->entity;

			// Write basic Entity properties (position, angle)
			const Vector2 &position = entity->GetPosition();
			device.write_float(position.x);
			device.write_float(position.y);

			device.write_float(entity->GetAngle());

			// Write the type-flags for the entity data
			if (!mapEntity->archetypeId.empty())
			{
				device.write_uint8(ArchetypeFlag);
				// Write the index of the archetype used
				device.write_uint32(usedArchetypeIndexes[mapEntity->archetypeId]);
			}
			else
				device.write_uint8(NoTypeFlags);

			// Write the entity state
			state.mask = mapEntity->stateMask;
			entity->SerialiseState(state, true);

			device.write_uint32(state.mask);
			device.write_string_a(state.data.c_str());
		}
	}


	void GameMapLoader::onEntityInstanced(EntityPtr &entity)
	{
		std::pair<TypeIndex::left_iterator, bool> r = m_TypeIndex.left.insert( TypeIndex::left_value_type(entity->GetType(), m_NextTypeIndex) );
		if (r.second)
			++m_NextTypeIndex;
	}

}
