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

#include "FusionStableHeaders.h"

#include "FusionGameMapLoader.h"

#include <boost/crc.hpp>
#include <boost/lexical_cast.hpp>
#include <RakNetTypes.h>

#include "FusionClientOptions.h"
#include "FusionEntityFactory.h"
#include "FusionEntityManager.h"
#include "FusionNetworkManager.h"
#include "FusionNetworkTypes.h"
#include "FusionPlayerRegistry.h"

namespace FusionEngine
{

	GameMapLoader::GameMapLoader(ClientOptions *options, EntityFactory *factory, EntityManager *manager)
		: m_ClientOptions(options),
		m_Factory(factory),
		m_Manager(manager),
		m_NextTypeIndex(0)
	{
		m_FactoryConnection = factory->SignalEntityInstanced.connect( boost::bind(&GameMapLoader::onEntityInstanced, this, _1) );

		NetworkManager::getSingleton().Subscribe(MTID_LOADMAP, this);
	}

	GameMapLoader::~GameMapLoader()
	{
		NetworkManager* netMan = NetworkManager::getSingletonPtr();
		if (netMan != nullptr)
			netMan->Unsubscribe(MTID_LOADMAP, this);
		m_FactoryConnection.disconnect();
	}

	void GameMapLoader::HandlePacket(Packet *packet)
	{
		RakNet::BitStream bitStream(packet->data, packet->length, false);
		{
			unsigned char packetType;
			bitStream.Read(packetType);
		}

		std::string::size_type filename_length;
		bitStream.Read(filename_length);
		std::string filename; filename.resize(filename_length);
		bitStream.Read(&filename[0], filename_length);

		uint32_t expectedChecksum;
		bitStream.Read(expectedChecksum);

		CL_VirtualDirectory directory;
		CL_IODevice device = directory.open_file(filename, CL_File::open_existing, CL_File::access_read);

		boost::crc_32_type crc;
		int count = 0;
		do
		{
			char buffer[2048];
			count = device.read(buffer, 2048);
			crc.process_bytes(buffer, 2048);
		} while (count == 2048);

		if (crc.checksum() == expectedChecksum)
		{
			LoadMap(filename, directory, false);
		}
		else
			SendToConsole("Host map is different to local map.");
	}

	void GameMapLoader::LoadEntityTypes(const std::string &filename, CL_VirtualDirectory &directory)
	{
		CL_IODevice device = directory.open_file(filename, CL_File::open_existing, CL_File::access_read);
		LoadEntityTypes(device);
	}

	void GameMapLoader::LoadEntityTypes(CL_IODevice &device)
	{
		EntityFactory *factory = m_Factory;
		// Read the entity type count
		cl_int32 numberEntityTypes = device.read_int32();
		// Tell the entity-factory to load each entity type listed in the map file
		//CL_String8 entityTypename;
		for (cl_int32 i = 0; i < numberEntityTypes; i++)
		{
			//entityTypename = device.read_string_a();
			factory->LoadScriptedType(/*entityTypename*/device.read_string_a());
		}
	}

	void GameMapLoader::LoadMap(const std::string &filename, CL_VirtualDirectory &directory, bool include_synced)
	{
		CL_IODevice device = directory.open_file(filename, CL_File::open_existing, CL_File::access_read);

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
			archetype.packet.data = device.read_string_a();
		}

		IDTranslator translator;

		loadPseudoEntities(device, archetypeArray, translator);
		if (include_synced)
			loadEntities(device, archetypeArray, translator);
	}

	void GameMapLoader::loadPseudoEntities(CL_IODevice &device, const ArchetypeArray &archetypeArray, const IDTranslator &translator)
	{
		EntityFactory *factory = m_Factory;

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

				// Load custom properties
				state.mask = device.read_uint32();
				state.data = device.read_string_a();

				entity->DeserialiseState(state, true, entity_deserialiser);
			}
		}
	}

	void GameMapLoader::loadEntities(CL_IODevice &device, const ArchetypeArray &archetypeArray, const IDTranslator &translator)
	{
		EntityFactory *factory = m_Factory;

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

				if (entity)
				{
					entity->SetID(translator(entityID));
					m_Manager->AddEntity(entity);

					instancedEntities.push_back(entity);
				}
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

				// Load custom properties
				state.mask = device.read_uint32();
				state.data = device.read_string_a();

				entity->DeserialiseState(state, true, entity_deserialiser);
			}
		}
	}

	void GameMapLoader::LoadSavedGame(const std::string &filename, CL_VirtualDirectory &directory)
	{
		CL_IODevice device = directory.open_file(filename, CL_File::open_existing, CL_File::access_read);

		// General info
		std::string date;
		date.resize(19);
		device.read(&date[0], 19);

		//// Local players
		//PlayerRegistry::Clear();

		//size_t numLocalPlayers = device.read_uint32();
		//m_ClientOptions->SetOption("num_local_players", boost::lexical_cast<std::string>(numLocalPlayers));
		//for (unsigned int i = 0; i < numLocalPlayers; ++i)
		//{
		//	ObjectID netId = device.read_uint16();
		//	PlayerRegistry::AddLocalPlayer(netId, i);
		//}

		// Map filename
		std::string mapFilename = device.read_string_a();

		if (!mapFilename.empty() && mapFilename != m_MapFilename)
			LoadMap(mapFilename, directory, true);
		else
			m_Manager->ClearSyncedEntities();

		EntityFactory *factory = m_Factory;

		// Load & instance Entities (this section is loaded the same way as the equivilant section in the map file)
		// TODO: share code with map file loading?
		cl_uint32 numberEntities = device.read_uint32();
		EntityArray instancedEntities;
		instancedEntities.reserve(numberEntities);
		// The TypeIndex bimap maps typename to index, so the right_map is index to name
		TypeIndex::right_map &indexToName = m_TypeIndex.right;
		{
			std::string entityName; ObjectID entityID;
			EntityPtr entity;
			for (cl_uint32 i = 0; i < numberEntities; i++)
			{
				cl_uint32 typeIndex = device.read_uint32();

				entityName = device.read_string_a();
				device.read((void*)&entityID, sizeof(ObjectID));

				{
					const std::string &entityTypename = indexToName.at(typeIndex);
					entity = factory->InstanceEntity(entityTypename, entityName);
				}

				entity->SetID(entityID);
				m_Manager->AddEntity(entity);

				instancedEntities.push_back(entity);
			}
		}

		// Deserialise each instanced entity (the only difference between this and what is done to load
		//  a _map_ file is that the Entity's Spawn() method is not called)
		EntityDeserialiser entity_deserialiser(m_Manager);
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

			// Load custom properties
			state.mask = device.read_uint32();
			state.data = device.read_string_a();

			entity->DeserialiseState(state, true, entity_deserialiser);
		}
	}

	void GameMapLoader::SaveGame(const std::string &filename, CL_VirtualDirectory &directory)
	{
		CL_IODevice device = directory.open_file(filename, CL_File::create_always, CL_File::access_write, 0);

		// Write save info
		//  Date
		CL_String date = CL_DateTime::get_current_local_time().to_short_datetime_string();
		device.write(CL_StringHelp::text_to_utf8(date).c_str(), 19);
		////  Players
		//int numLocalPlayers;
		//m_ClientOptions->GetOption("num_local_players", &numLocalPlayers);
		//device.write_uint32((unsigned)numLocalPlayers);
		////  Write net-indicies so they can be restored - net-indicies
		////  must be the same from session to session for Entity ownership.
		//for (unsigned int i = 0; i < (unsigned)numLocalPlayers; i++)
		//	device.write_uint16(PlayerRegistry::GetPlayerByLocalIndex(i).NetIndex);

		// Map filename
		device.write_string_a(m_MapFilename);

		const EntityManager::IDEntityMap &entities = m_Manager->GetEntities();

		// Write Entities
		device.write_uint32(entities.size());
		for (EntityManager::IDEntityMap::const_iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const EntityPtr &entity = it->second;
			
			// Write the type index (refers to type-index in the compiled map file)
			device.write_uint32( m_TypeIndex.left.at(entity->GetType()) );

			// Write the Entity name
			if (!entity->HasDefaultName())
				device.write_string_a(entity->GetName());
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

			// Write custom properties
			entity->SerialiseState(state, true);

			device.write_uint32(state.mask);
			device.write_string_a(state.data);
		}
	}


	void GameMapLoader::CompileMap(CL_IODevice &device, const StringSet &used_entity_types, const GameMapLoader::ArchetypeMap &archetypes, const GameMapLoader::MapEntityArray &pseudo_entities, const GameMapLoader::MapEntityArray &entities)
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

		// Write Pseudo-Entities
		device.write_uint32(pseudo_entities.size());
		for (MapEntityArray::const_iterator it = pseudo_entities.begin(), end = pseudo_entities.end(); it != end; ++it)
		{
			const MapEntityPtr &mapEntity = *it;
			const EntityPtr &entity = mapEntity->entity;
			
			// Write the type index (refers to used-type-index at the top of the file)
			device.write_uint32(usedTypeIndexes[entity->GetType()]);

			// Write the Entity name
			if (mapEntity->hasName)
				device.write_string_a(entity->GetName());
			else
				device.write_string_a(CL_String8());
		}

		// Write Pseudo-Entity state data
		SerialisedData state;
		for (MapEntityArray::const_iterator it = pseudo_entities.begin(), end = pseudo_entities.end(); it != end; ++it)
		{
			const MapEntityPtr &mapEntity = *it;
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

			// Write custom properties
			state.mask = mapEntity->stateMask;
			entity->SerialiseState(state, true);

			device.write_uint32(state.mask);
			device.write_string_a(state.data);
		}

		// Write Entities
		device.write_uint32(entities.size());
		for (MapEntityArray::const_iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const MapEntityPtr &mapEntity = *it;
			const EntityPtr &entity = mapEntity->entity;
			
			// Write the type index (refers to used-type-index at the top of the file)
			device.write_uint32(usedTypeIndexes[entity->GetType()]);

			// Write the Entity name
			if (mapEntity->hasName)
				device.write_string_a(entity->GetName());
			else
				device.write_string_a(CL_String8());
			// Write the Entity ID
			//device.write_uint16(entity->GetID());
			ObjectID id = entity->GetID();
			device.write(&id, sizeof(ObjectID));
		}

		// Write Entity state data
		for (MapEntityArray::const_iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const MapEntityPtr &mapEntity = *it;
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

			// Write the custom properties
			state.mask = mapEntity->stateMask;
			entity->SerialiseState(state, true);

			device.write_uint32(state.mask);
			device.write_string_a(state.data);
		}
	}

	void GameMapLoader::SaveEntity(const EntityPtr &entity, CL_IODevice &device)
	{
		// Type
		device.write_string_a(entity->GetType());

		// Entity name
		if (!entity->HasDefaultName())
			device.write_string_a(entity->GetName());
		else
			device.write_string_a(CL_String8());

		// Entity ID
		ObjectID id = entity->GetID();
		device.write(&id, sizeof(ObjectID));
		
		// Write basic Entity properties (position, angle)
		const Vector2 &position = entity->GetPosition();
		device.write_float(position.x);
		device.write_float(position.y);
		device.write_float(entity->GetAngle());

		// Custom properties
		SerialisedData state;
		entity->SerialiseState(state, true);
		device.write_uint32(state.mask);
		device.write_string_a(state.data);
	}

	EntityPtr GameMapLoader::LoadEntity(CL_IODevice &device, EntityFactory *factory, const IEntityRepo *manager)
	{
		EntityPtr entity;
		try
		{
			// Load the entity type, name and id
			std::string entityTypename = device.read_string_a();
			std::string entityName = device.read_string_a();
			ObjectID entityID;
			device.read((void*)&entityID, sizeof(ObjectID));
			// Create the instance
			entity = factory->InstanceEntity(entityTypename, entityName);
			entity->SetID(entityID);

			// Basic Entity properties (position, angle)
			Vector2 position;
			position.x = device.read_float();
			position.y = device.read_float();
			entity->SetPosition(position);

			entity->SetAngle(device.read_float());

			// The custom properties
			EntityDeserialiser entity_deserialiser(manager);
			SerialisedData state;
			state.mask = device.read_uint32();
			state.data = device.read_string_a();

			entity->DeserialiseState(state, true, entity_deserialiser);
		}
		catch (CL_Exception &io_exception)
		{
			Logger::getSingleton().Add(io_exception.what());
			// Failed to read: reset the shared-pointer before returning to prevent returning an invalid Entity
			entity.reset();
		}
		return entity;
	}


	void GameMapLoader::onEntityInstanced(EntityPtr &entity)
	{
		if (entity)
		{
			std::pair<TypeIndex::left_iterator, bool> r = m_TypeIndex.left.insert( TypeIndex::left_value_type(entity->GetType(), m_NextTypeIndex) );
			if (r.second)
				++m_NextTypeIndex;
		}
	}


	Entity* MapEntity_getEntity(const GameMapLoader::MapEntity* obj)
	{
		if (obj->entity)
		{
			obj->entity->addRef();
			return obj->entity.get();
		}
		else
			return nullptr;
	}

	void GameMapLoader::Register(asIScriptEngine *engine)
	{
		int r;
		MapEntity::RegisterType<MapEntity>(engine, "MapEntity");

		r = engine->RegisterObjectProperty("MapEntity", "bool hasName", offsetof(MapEntity, hasName)); FSN_ASSERT(r >= 0);
		engine->RegisterObjectProperty("MapEntity", "bool synced", offsetof(MapEntity, synced));
		r = engine->RegisterObjectMethod("MapEntity", "Entity@ get_entity() const", asFUNCTION(MapEntity_getEntity), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

}
