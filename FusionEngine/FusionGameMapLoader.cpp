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

#include "FusionGameAreaLoader.h"
#include "FusionEntityManager.h"
#include "FusionEntityFactory.h"


namespace FusionEngine
{

	GameMapLoader::GameMapLoader(EntityManager *manager)
		: m_Manager(manager)
	{
	}

	void GameMapLoader::LoadEntityTypes(CL_IODevice device)
	{
		EntityFactory *factory = m_Manager->GetFactory();
		// Read the entity type count
		cl_int32 numberEntityTypes = device.read_int32();
		// Tell the entity-factory to load each entity type listed in the map file
		for (cl_int32 i = 0; i < numberEntityTypes; i++)
		{
			CL_String8 entityTypename = device.read_string_a();
			factory->LoadScriptedType(entityTypename.c_str());
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
		ArchetypeMap archetypeArray(numberArchetypes);
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

		// Load / instance Entities
		cl_uint32 numberEntities = device.read_uint32();
		EntityArray instancedEntities(numberEntities);
		{
			std::string entityName; ObjectID entityID;
			EntityPtr entity;
			for (cl_uint32 i = 0; i < numberArchetypes; i++)
			{
				cl_uint8 typeFlags = device.read_uint8(); // Flags indicating Pseudo-Entity, Archetype, etc.
				cl_uint32 typeIndex = device.read_uint32(); // Entity or Archetype index - previous byte indicates which

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

				entity->Spawn();

				// Load archetype
				if (device.read_uint8() != 0)
				{
					cl_uint32 typeIndex = device.read_uint32();
					const Archetype &archetype = archetypeArray[typeIndex];

					entity->DeserialiseState(archetype.packet, true, entity_deserialiser);
				}

				state.mask = device.read_uint32();
				state.data = device.read_string_a().c_str();

				entity->DeserialiseState(state, true, entity_deserialiser);
			}
		}
	}

	void GameMapLoader::LoadSavedGame(CL_IODevice device)
	{
	}

	void GameMapLoader::SaveMap(CL_IODevice device)
	{
	}

	void GameMapLoader::SaveGame(CL_IODevice device)
	{
	}

}
