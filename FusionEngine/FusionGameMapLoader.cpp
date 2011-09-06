/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#include "FusionGameMapLoader.h"

#include <boost/crc.hpp>
#include <boost/lexical_cast.hpp>
#include <RakNetTypes.h>

#include "FusionClientOptions.h"
#include "FusionEntityFactory.h"
#include "FusionEntityManager.h"
#include "FusionInstanceSynchroniser.h"
#include "FusionNetDestinationHelpers.h"
#include "FusionNetworkManager.h"
#include "FusionNetworkTypes.h"
#include "FusionPlayerRegistry.h"

#include "FusionPhysFSIOStream.h"

// TODO: remove
#include "FusionVirtualFileSource_PhysFS.h"

using namespace std::placeholders;

namespace FusionEngine
{

	static void WriteComponent(CL_IODevice& out, IComponent* component)
	{
		RakNet::BitStream stream;
		const bool conData = component->SerialiseContinuous(stream);
		const bool occData = component->SerialiseOccasional(stream, IComponent::All);

		out.write_uint8(conData ? 0xFF : 0x00); // Flag indicating data presence
		out.write_uint8(occData ? 0xFF : 0x00);

		out.write_uint32(stream.GetNumberOfBytesUsed());
		out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
	}

	static void DeserialiseComponent(CL_IODevice& in, IComponent* component)
	{
		const bool conData = in.read_uint8() != 0x00;
		const bool occData = in.read_uint8() != 0x00;

		const auto dataLen = in.read_uint32();
		std::vector<unsigned char> data(dataLen);
		in.read(data.data(), data.size());

		RakNet::BitStream stream(data.data(), data.size(), false);

		if (conData)
			component->DeserialiseContinuous(stream);
		if (occData)
			component->DeserialiseOccasional(stream, IComponent::All);

		//stream.AssertStreamEmpty();
		if (stream.GetNumberOfUnreadBits() >= 8)
			SendToConsole("Not all serialised data was used when reading a " + component->GetType());
	}

	static void WriteEntity(CL_IODevice& out, EntityPtr entity)
	{
		ObjectID id = entity->GetID();
		if (id != 0)
			out.write(&id, sizeof(ObjectID));

		{
			RakNet::BitStream stream;
			entity->SerialiseReferencedEntitiesList(stream);

			out.write_uint32(stream.GetNumberOfBytesUsed());
			out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
		}

		auto& components = entity->GetComponents();
		size_t numComponents = components.size() - 1; // - transform

		auto transform = dynamic_cast<IComponent*>(entity->GetTransform().get());
		out.write_string_a(transform->GetType());
		WriteComponent(out, transform);

		out.write(&numComponents, sizeof(size_t));
		for (auto it = components.begin(), end = components.end(); it != end; ++it)
		{
			auto& component = *it;
			if (component.get() != transform)
				out.write_string_a(component->GetType());
		}
		for (auto it = components.begin(), end = components.end(); it != end; ++it)
		{
			auto& component = *it;
			if (component.get() != transform)
				WriteComponent(out, component.get());
		}
	}

	static EntityPtr LoadEntity(CL_IODevice& in, bool synched, EntityFactory* factory, EntityManager* entityManager, InstancingSynchroniser* instantiator)
	{
		
		ObjectID id = 0;
		if (synched)
		{
			in.read(&id, sizeof(ObjectID));
			instantiator->TakeID(id);
		}

		const auto dataLen = in.read_uint32();
		std::vector<unsigned char> referencedEntitiesData(dataLen);
		in.read(referencedEntitiesData.data(), referencedEntitiesData.size());

		ComponentPtr transform;
		{
			std::string transformType = in.read_string_a();
			transform = factory->InstanceComponent(transformType);

			DeserialiseComponent(in, transform.get());
		}

		auto entity = std::make_shared<Entity>(entityManager, &entityManager->m_PropChangedQueue, transform);
		entity->SetID(id);

		transform->SynchronisePropertiesNow();

		{
			RakNet::BitStream stream(referencedEntitiesData.data(), referencedEntitiesData.size(), false);
			entity->DeserialiseReferencedEntitiesList(stream, EntityDeserialiser(instantiator->m_EntityManager));
		}

		size_t numComponents;
		in.read(&numComponents, sizeof(size_t));
		for (size_t i = 0; i < numComponents; ++i)
		{
			std::string type = in.read_string_a();
			auto& component = factory->InstanceComponent(type);
			entity->AddComponent(component);
		}
		if (numComponents != 0)
		{
			auto& components = entity->GetComponents();
			auto it = components.begin(), end = components.end();
			for (++it; it != end; ++it)
			{
				auto& component = *it;
				FSN_ASSERT(component != transform);

				DeserialiseComponent(in, component.get());
			}
		}

		return entity;
	}

	GameMap::GameMap(CL_IODevice& file)
		: m_File(file)
	{
		auto fileSize = m_File.get_size();

		m_XCells = m_File.read_uint32();
		m_CellSize = m_File.read_uint32();

		m_CellLocations.resize(m_XCells * m_XCells);
		for (unsigned int i = 0; i < m_CellLocations.size(); ++i)
		{
			uint32_t begin = m_File.read_uint32();
			uint32_t length = m_File.read_uint32();
			m_CellLocations[i] = std::make_pair(begin, length);
			FSN_ASSERT(begin && begin + length <= (unsigned int)fileSize);
		}

		m_NonStreamingEntitiesLocation = m_File.read_uint32();
	}

	unsigned int GameMap::GetNumCellsAcross() const
	{
		return m_XCells;
	}

	unsigned int GameMap::GetCellSize() const
	{
		return m_CellSize;
	}

	void GameMap::LoadCell(Cell* out, size_t index, bool include_synched, EntityFactory* factory, EntityManager* entityManager, InstancingSynchroniser* instantiator)
	{
		FSN_ASSERT(out);
		FSN_ASSERT(index < m_CellLocations.size());

		Cell::mutex_t::scoped_lock lock(out->mutex);

		auto pos = m_CellLocations[index];
		FSN_ASSERT(pos.first < (unsigned int)std::numeric_limits<int>::max());
		m_File.seek((int)pos.first);

		size_t numPseudoEnts;
		m_File.read(&numPseudoEnts, sizeof(size_t));
		for (size_t i = 0; i < numPseudoEnts; ++i)
		{
			auto entity = LoadEntity(m_File, false, factory, entityManager, instantiator);
			out->objects.push_back(std::make_pair(std::move(entity), CellEntry()));
		}
		if (include_synched)
		{
			size_t numSynchedEnts;
			m_File.read(&numSynchedEnts, sizeof(size_t));
			for (size_t i = 0; i < numSynchedEnts; ++i)
			{
				auto entity = LoadEntity(m_File, true, factory, entityManager, instantiator);
				out->objects.push_back(std::make_pair(std::move(entity), CellEntry()));
			}
		}
	}

	void GameMap::LoadNonStreamingEntities(bool include_synched, EntityManager* entityManager, EntityFactory* factory, InstancingSynchroniser* instantiator)
	{
		m_File.seek(m_NonStreamingEntitiesLocation);

		size_t numPseudoEnts;
		m_File.read(&numPseudoEnts, sizeof(size_t));
		for (size_t i = 0; i < numPseudoEnts; ++i)
		{
			auto entity = LoadEntity(m_File, false, factory, entityManager, instantiator);
			entity->SetDomain(SYSTEM_DOMAIN);
			entityManager->AddEntity(entity);
		}
		if (include_synched)
		{
			size_t numSynchedEnts;
			m_File.read(&numSynchedEnts, sizeof(size_t));
			for (size_t i = 0; i < numSynchedEnts; ++i)
			{
				auto entity = LoadEntity(m_File, true, factory, entityManager, instantiator);
				entity->SetDomain(SYSTEM_DOMAIN);
				entityManager->AddEntity(entity);
			}
		}
	}

	void GameMap::CompileMap(/*CL_VirtualDirectory& temp_dir, */CL_IODevice &device, unsigned int num_cells_across, unsigned int cell_size, /*const std::vector<EntityPtr> &pseudo_entities, */const std::vector<EntityPtr> &entities)
	{
		device.write(&num_cells_across, sizeof(unsigned int));
		device.write(&cell_size, sizeof(unsigned int));

		std::vector<std::pair<std::vector<EntityPtr>, std::vector<EntityPtr>>> cells(num_cells_across * num_cells_across);
		std::vector<EntityPtr> nonStreamingEntities;
		std::vector<EntityPtr> nonStreamingEntitiesSynched;
		for (auto it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			auto& entity = *it;
			if (entity->GetStreamingCellIndex() < cells.size())
			{
				if (!entity->IsSyncedEntity())
					cells[entity->GetStreamingCellIndex()].first.push_back(entity);
				else
					cells[entity->GetStreamingCellIndex()].second.push_back(entity);
			}
			else
			{
				if (!entity->IsSyncedEntity())
					nonStreamingEntities.push_back(entity);
				else
					nonStreamingEntitiesSynched.push_back(entity);
			}
		}

		FSN_ASSERT(nonStreamingEntities.empty() && nonStreamingEntitiesSynched.empty());

		//std::vector<CL_IODevice> tempFiles;
		//unsigned int cellIndex = 0;

		auto locationsOffset = device.get_position();
		{
			uint32_t locationsSpaceSize = cells.size() * sizeof(uint32_t) * 2;
			std::vector<unsigned char> space(locationsSpaceSize);
			device.write(space.data(), space.size()); // leave some space for the cell-data offsets
		}
		device.write_uint32(0); // Non-streaming entities location
#ifdef _DEBUG
		auto locationsEndOffset = device.get_position();
#endif

		std::vector<std::pair<uint32_t, uint32_t>> cellDataLocations(cells.size());

		auto cellDataLocationIt = cellDataLocations.begin();
		for (auto it = cells.begin(), end = cells.end(); it != end; ++it)
		{
			auto& cell = *it;

			//CL_IODevice tempFile;
			//if (temp_dir.get_file_system().is_null())
			//{
			//	tempFile = CL_IODevice_Memory();
			//}
			//else
			//{
			//	std::stringstream str; str << cellIndex++;
			//	tempFile = temp_dir.open_file(str.str());
			//	tempFiles.push_back(tempFile);
			//}

			auto& cellDataLocation = *cellDataLocationIt;
			cellDataLocation.first = uint32_t(device.get_position());

			// Pseudo ents
			size_t numEnts = cell.first.size();
			device.write(&numEnts, sizeof(size_t));
			for (auto cit = cell.first.begin(), cend = cell.first.end(); cit != cend; ++cit)
			{
				auto& entity = *cit;

				WriteEntity(device, entity);
			}
			// Synched ents
			numEnts = cell.second.size();
			device.write(&numEnts, sizeof(size_t));
			for (auto cit = cell.second.begin(), cend = cell.second.end(); cit != cend; ++cit)
			{
				auto& entity = *cit;

				WriteEntity(device, entity);
			}

			cellDataLocation.second = uint32_t(device.get_position() - cellDataLocation.first);

			++cellDataLocationIt;
		}

		uint32_t nonStreamingEntitiesLocation = device.get_position();
		device.write_uint32(nonStreamingEntities.size());

		device.seek(locationsOffset);
		for (auto it = cellDataLocations.begin(), end = cellDataLocations.end(); it != end; ++it)
		{
			device.write_uint32(it->first);
			device.write_uint32(it->second);
			FSN_ASSERT(device.get_position() <= locationsEndOffset);
		}

		device.write_uint32(nonStreamingEntitiesLocation);

		FSN_ASSERT(device.get_position() == locationsEndOffset);

		//{		
		//for (auto it = tempFiles.begin(), end = tempFiles.end(); it != end; ++it)
		//{
		//	device.write_uint32(offset);
		//	auto cellDataLength = it->get_size();
		//	device.write_uint32(cellDataLength);
		//	offset += cellDataLength;
		//}
		//}

		//{
		//std::vector<unsigned char> data;
		//for (auto it = tempFiles.begin(), end = tempFiles.end(); it != end; ++it)
		//{
		//	data.resize(it->get_size());

		//	it->seek(0);
		//	it->read(&data[0], data.size());

		//	device.write(data.data(), data.size());
		//}
		//}
	}

	GameMapLoader::GameMapLoader(ClientOptions *options, EntityFactory *factory, EntityManager *manager, std::shared_ptr<CL_VirtualFileSource> filesource)
		: m_ClientOptions(options),
		m_Factory(factory),
		m_Manager(manager),
		m_FileSource(filesource),
		m_NextTypeIndex(0),
		m_MapChecksum(0)
	{
		m_FactoryConnection = factory->SignalEntityInstanced.connect( std::bind(&GameMapLoader::onEntityInstanced, this, _1) );

		NetworkManager::getSingleton().Subscribe(MTID_LOADMAP, this);
		NetworkManager::getSingleton().Subscribe(ID_NEW_INCOMING_CONNECTION, this);
	}

	GameMapLoader::~GameMapLoader()
	{
		NetworkManager* netMan = NetworkManager::getSingletonPtr();
		if (netMan != nullptr)
		{
			netMan->Unsubscribe(MTID_LOADMAP, this);
			netMan->Unsubscribe(ID_NEW_INCOMING_CONNECTION, this);
		}
		m_FactoryConnection.disconnect();
	}

	void GameMapLoader::HandlePacket(RakNet::Packet *packet)
	{
		RakNet::BitStream bitStream(packet->data, packet->length, false);
		unsigned char packetType;
		bitStream.Read(packetType);

		if (packetType == ID_NEW_INCOMING_CONNECTION && NetworkManager::ArbitratorIsLocal())
		{
			// Tell the new peer what the current map is
			RakNet::BitStream replyBitstream;
			replyBitstream.Write(m_MapFilename.size());
			replyBitstream.Write(m_MapFilename.data(), m_MapFilename.size());
			replyBitstream.Write(m_MapChecksum);

			NetworkManager::getSingleton().GetNetwork()->Send(
				NetDestination(packet->guid, false), !Timestamped, MTID_LOADMAP, &replyBitstream, HIGH_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER);
		}
		else if (packetType == MTID_LOADMAP)
		{
			std::string::size_type filename_length;
			bitStream.Read(filename_length);
			std::string filename; filename.resize(filename_length);
			bitStream.Read(&filename[0], filename_length);

			uint32_t expectedChecksum;
			bitStream.Read(expectedChecksum);

			CL_VirtualDirectory directory(CL_VirtualFileSystem(m_FileSource.get()), "");
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
				m_MapChecksum = crc.checksum();
				LoadMap(filename, directory, nullptr);
			}
			else
				SendToConsole("Host map is different to local map.");
		}
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
			factory->LoadPrefabType(/*entityTypename*/device.read_string_a());
		}
	}

	std::shared_ptr<GameMap> GameMapLoader::LoadMap(const std::string &filename, CL_VirtualDirectory &directory, InstancingSynchroniser* synchroniser)
	{
		CL_IODevice device = directory.open_file(filename, CL_File::open_existing, CL_File::access_read);

		m_MapFilename = filename;

		// Calculate checksum if this is the host (indicated by having a synchroniser)
		if (synchroniser != nullptr)
		{
			boost::crc_32_type crc;
			int count = 0;
			do
			{
				char buffer[2048];
				count = device.read(buffer, 2048);
				crc.process_bytes(buffer, 2048);
			} while (count == 2048);

			m_MapChecksum = crc.checksum();

			device.seek(0, CL_IODevice::seek_set);

			// Send map change notification to peers
			RakNet::BitStream bitStream;
			bitStream.Write(m_MapFilename.size());
			bitStream.Write(m_MapFilename.data(), m_MapFilename.size());
			bitStream.Write(m_MapChecksum);

			NetworkManager::getSingleton().GetNetwork()->Send(
				Dear::Populace(), !Timestamped, MTID_LOADMAP, &bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER);
		}

		m_Manager->Clear();

		auto map = std::make_shared<GameMap>(device);

		//// Read the entity type count
		//cl_uint32 numberEntityTypes = device.read_uint32();
		//// List each entity type
		////StringVector entityTypeArray(numberEntityTypes);
		//m_TypeIndex.clear();
		//{
		//	CL_String8 entityTypename;
		//	for (m_NextTypeIndex = 0; m_NextTypeIndex < numberEntityTypes; ++m_NextTypeIndex)
		//	{
		//		entityTypename = device.read_string_a();
		//		//entityTypeArray.push_back(entityTypename.c_str());
		//		m_TypeIndex.insert( TypeIndex::value_type(entityTypename.c_str(), m_NextTypeIndex) );
		//	}
		//}

		//TypeIndex::right_map &entityTypeArray = m_TypeIndex.right;

		//// Load Archetypes
		//cl_uint32 numberArchetypes = device.read_uint32();
		//ArchetypeArray archetypeArray(numberArchetypes);
		//for (cl_uint32 i = 0; i < numberArchetypes; i++)
		//{
		//	cl_uint32 entityTypeIndex = device.read_uint32();
		//	const std::string &entityTypename = entityTypeArray.at(entityTypeIndex);

		//	Archetype &archetype = archetypeArray[i];

		//	archetype.entityIndex = entityTypeIndex;
		//	archetype.entityTypename = entityTypename;

		//	archetype.packet.mask = device.read_uint32();
		//	archetype.packet.data = device.read_string_a();
		//}

		//IDTranslator translator;

		//loadPseudoEntities(device, archetypeArray, translator);
		//if (synchroniser != nullptr)
		//	loadEntities(device, archetypeArray, translator, synchroniser);

		return map;
	}

	std::shared_ptr<GameMap> GameMapLoader::LoadMap(const std::string &filename, InstancingSynchroniser* synchroniser)
	{
		return LoadMap(filename, CL_VirtualDirectory(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), ""), synchroniser);
	}

	void GameMapLoader::deserialiseBasicProperties(EntityPtr& entity, CL_IODevice &device)
	{
		// Basic Entity properties (owner, position, angle)
		//device.read((void*)&m_OwnerID, sizeof(PlayerID));
		Vector2 position;
		position.x = device.read_float();
		position.y = device.read_float();
		entity->SetPosition(position);

		entity->SetAngle(device.read_float());
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
				//if (entityName.empty())
				//	entityName = "default";

				{
					const std::string &entityTypename = entityTypeArray.at(typeIndex);
					entity = factory->InstanceEntity(entityTypename, Vector2::zero(), 0.f);
				}

				if (entity)
				{
					entity->SetName(entityName);

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

				deserialiseBasicProperties(entity, device);

				cl_uint8 typeFlags = device.read_uint8();
				// Load archetype
				if (typeFlags & ArchetypeFlag) // Check for archetype flag
				{
					cl_uint32 typeIndex = device.read_uint32();
					const Archetype &archetype = archetypeArray[typeIndex];

					// Check that the archetype data is for the correct entity type before deserializing
					FSN_ASSERT(entity->GetType() == archetype.entityTypename);
					//entity->DeserialiseState(archetype.packet, true, entity_deserialiser);
				}

				// Load type-specific properties
				state.mask = device.read_uint32();
				state.data = device.read_string_a();

				//entity->DeserialiseState(state, true, entity_deserialiser);
			}
		}
	}

	void GameMapLoader::loadEntities(CL_IODevice &device, const ArchetypeArray &archetypeArray, const IDTranslator &translator, InstancingSynchroniser* synchroniser)
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
					entity = factory->InstanceEntity(entityTypename, Vector2::zero(), 0.f);
				}

				if (entity)
				{
					entity->SetName(entityName);

					entityID = translator(entityID);
					synchroniser->TakeID(entityID); // remove the ID from the available pool (so entities created after the map is loaded wont take this ID)
					entity->SetID(entityID);

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

				m_Manager->AddEntity(entity);

				cl_uint8 typeFlags = device.read_uint8();
				// Load archetype
				if (typeFlags & ArchetypeFlag) // Check for archetype flag
				{
					cl_uint32 typeIndex = device.read_uint32();
					const Archetype &archetype = archetypeArray[typeIndex];

					// Check that the archetype data is for the correct entity type before deserializing
					FSN_ASSERT(entity->GetType() == archetype.entityTypename);
					//entity->DeserialiseState(archetype.packet, true, entity_deserialiser);
				}

				// Load custom properties
				state.mask = device.read_uint32();
				state.data = device.read_string_a();

				//entity->DeserialiseState(state, true, entity_deserialiser);
			}
		}
	}

	void GameMapLoader::LoadSavedGame(const std::string &filename, CL_VirtualDirectory &directory, InstancingSynchroniser* synchroniser)
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
			LoadMap(mapFilename, directory, synchroniser);
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
					entity = factory->InstanceEntity(entityTypename, Vector2::zero(), 0.f);
				}

				entity->SetName(entityName);

				if (entity)
				{
					synchroniser->TakeID(entityID); // remove the ID from the available pool (so entities created after the map is loaded wont take this ID)
					entity->SetID(entityID);

					instancedEntities.push_back(entity);
				}
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

			m_Manager->AddEntity(entity);

			// Load custom properties
			state.mask = device.read_uint32();
			state.data = device.read_string_a();

			//entity->DeserialiseState(state, true, entity_deserialiser);
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
			//entity->SerialiseState(state, true);

			device.write_uint32(state.mask);
			device.write_string_a(state.data);
		}
	}


	void GameMapLoader::CompileMap(CL_IODevice &device, const StringSet &used_entity_types, const GameMapLoader::ArchetypeMap &archetypes, const GameMapLoader::MapEntityArray &pseudo_entities, const GameMapLoader::MapEntityArray &entities)
	{
		//device.set_system_mode();

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
			//entity->SerialiseState(state, true);

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
			//entity->SerialiseState(state, true);

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
		//entity->SerialiseState(state, true);
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
			entity = factory->InstanceEntity(entityTypename, Vector2::zero(), 0.f);
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

			//entity->DeserialiseState(state, true, entity_deserialiser);
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
		return obj->entity.get();
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
