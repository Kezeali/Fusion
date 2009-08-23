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


namespace FusionEngine
{

	GameMapLoader::GameMapLoader(ClientOptions *options, EntityManager *manager)
		: m_ClientOptions(options),
		m_Manager(manager)
	{
	}

	void GameMapLoader::LoadEntityTypes(CL_IODevice device)
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

	void GameMapLoader::LoadMap(CL_IODevice device)
	{
		// Read the entity type count
		cl_uint32 numberEntityTypes = device.read_uint32();
		// List each entity type
		StringVector entityTypeArray(numberEntityTypes);
		{
			CL_String8 entityTypename;
			for (cl_uint32 i = 0; i < numberEntityTypes; i++)
			{
				entityTypename = device.read_string_a();
				entityTypeArray.push_back(entityTypename.c_str());
			}
		}

		// Load Archetypes
		cl_uint32 numberArchetypes = device.read_uint32();
		ArchetypeArray archetypeArray(numberArchetypes);
		for (cl_uint32 i = 0; i < numberArchetypes; i++)
		{
			cl_uint32 entityTypeIndex = device.read_uint32();
			const std::string &entityTypename = entityTypeArray[entityTypeIndex];

			Archetype &archetype = archetypeArray[i];

			archetype.entityIndex = entityTypeIndex;
			archetype.entityTypename = entityTypename;

			archetype.packet.mask = device.read_uint32();
			archetype.packet.data = device.read_string_a().c_str();
		}

		EntityFactory *factory = m_Manager->GetFactory();
		IDTranslator translator = m_Manager->MakeIDTranslator();

		//typedef std::pair<EntityPtr, cl_uint8> EntityToLoad;
		//typedef std::vector<EntityToLoad> EntityToLoadArray;

		// Load & instance Entities
		cl_uint32 numberEntities = device.read_uint32();
		EntityArray instancedEntities(numberEntities);
		{
			std::string entityName; ObjectID entityID;
			EntityPtr entity;
			for (cl_uint32 i = 0; i < numberEntities; i++)
			{
				cl_uint8 typeFlags = device.read_uint8(); // Flags indicating Pseudo-Entity, Archetype, etc.
				cl_uint32 typeIndex = device.read_uint32();

				entityName = device.read_string_a().c_str();

				bool isPseudoEntity = (typeFlags & PseudoEntityFlag) == PseudoEntityFlag;
				if (isPseudoEntity)
					entityID = 0;
				else
					device.read((void*)&entityID, sizeof(ObjectID));
					

				//if (typeFlags & ArchetypeFlag)
				//{
				//	const Archetype &archetype = archetypeArray[typeIndex];
				//	entity = factory->InstanceEntity(archetype.entityTypename, entityName);/*m_Manager->InstanceEntity(archetype.entityTypename, entityName);*/
				//}
				//else
				{
					const std::string &entityTypename = entityTypeArray[typeIndex];
					entity = factory->InstanceEntity(entityTypename, entityName);/*m_Manager->InstanceEntity(archetype.entityTypename, entityName);*/
				}

				if (isPseudoEntity)
				{
					m_Manager->AddPseudoEntity(entity);
				}
				else
				{
					entity->SetID(translator(entityID));
					m_Manager->AddEntity(entity);
				}

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

				// Call the entity's spawn method
				entity->Spawn();

				// Load archetype
				if (device.read_uint8() != 0) // Same type-flags as above (see cl_uint8 typeFlags), but we only care about the archetype flag this time
				{
					cl_uint32 typeIndex = device.read_uint32();
					const Archetype &archetype = archetypeArray[typeIndex];

					entity->DeserialiseState(archetype.packet, true, entity_deserialiser);
				}

				// Load specific entity state
				state.mask = device.read_uint32();
				state.data = device.read_string_a().c_str();

				entity->DeserialiseState(state, true, entity_deserialiser);
			}
		}
	}

	void GameMapLoader::LoadSavedGame(CL_IODevice device)
	{
	}

	void GameMapLoader::CompileMap(CL_IODevice device, const StringSet &used_entity_types, const GameMapLoader::ArchetypeMap &archetypes, const GameMapLoader::GameMapEntityArray &entities)
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
				device.write_string_a(*it);
				
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

		// Write Entities
		device.write_uint32(entities.size());

		for (GameMapEntityArray::const_iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const EntityPtr &entity = it->entity;

			// Write the type-flags for this entity
			cl_uint8 typeFlags;
			if (entity->IsPseudoEntity())
				typeFlags &= PseudoEntityFlag;
			if (!it->archetypeId.empty())
				typeFlags &= ArchetypeFlag;

			device.write_uint8(typeFlags);
			
			// Write the type index (refers to used-type-index at the top of the file)
			device.write_uint32(usedTypeIndexes[entity->GetType()]);

			// Write the Entity name
			device.write_string_a(entity->GetName());
			// Write the Entity ID (if it isn't a pseudo-entity)
			if (!entity->IsPseudoEntity())
				device.write_uint16(entity->GetID());
				//device.write((void*)&entity->GetID(), sizeof(ObjectID));
		}

		// Write Entity state data
		SerialisedData state;
		for (GameMapEntityArray::const_iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const EntityPtr &entity = it->entity;

			// Write the type-flags for the entity data
			if (!it->archetypeId.empty())
			{
				device.write_uint8(ArchetypeFlag);
				// Write the index of the archetype used
				device.write_uint32(usedArchetypeIndexes[it->archetypeId]);
			}
			else
				device.write_uint8(NoTypeFlags);

			// Write the entity state
			state.mask = it->stateMask;
			entity->SerialiseState(state, true);

			device.write_uint32(state.mask);
			device.write_string_a(state.data);
		}
	}

	void GameMapLoader::SaveGame(CL_IODevice device)
	{
		// Write save info
		//  Date
		device.write_uint64(CL_DateTime::get_current_utc_time().to_ticks());
		//  Players
		int numLocalPlayers;
		m_ClientOptions->GetOption("num_local_players", &numLocalPlayers);
		device.write_uint32((unsigned)numLocalPlayers);
		//  Write net-indicies so they can be restored - net-indicies
		//  must be the same from session to session for Entity ownership.
		for (unsigned int i = 0; i < (unsigned)numLocalPlayers; i++)
			device.write_uint16(PlayerRegistry::GetPlayerByLocalIndex(i).NetIndex);

		// Map filename
		device.write_string_a(m_MapFilename);

		const EntityManager::IDEntityMap &entities = m_Manager->GetEntities();
		for (EntityManager::IDEntityMap::const_iterator it = entities.begin(), end = entities.end();
			it != end; ++it)
		{
			const EntityPtr &entity = it->second;

			if (entity->GetDomain() == GAME_DOMAIN)
			{
				// Save entity...
			}
		}
	}

}
