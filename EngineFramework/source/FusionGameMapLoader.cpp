/*
*  Copyright (c) 2009-2012 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionGameMapLoader.h"

#include <boost/crc.hpp>
#include <boost/lexical_cast.hpp>
#include <RakNet/RakNetTypes.h>
#include <RakNet/StringCompressor.h>

#include "FusionCellCache.h"
#include "FusionCellFileManager.h"
#include "FusionCellSerialisationUtils.h"
#include "FusionClientOptions.h"
#include "FusionComponentFactory.h"
#include "FusionEntityManager.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionEntityInstantiator.h"
#include "FusionNetDestinationHelpers.h"
#include "FusionNetworkManager.h"
#include "FusionNetworkTypes.h"
#include "FusionPlayerRegistry.h"

#include "FusionPhysFSIOStream.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include "FusionBinaryStream.h"
#include <yaml-cpp/yaml.h>

#include <boost/filesystem.hpp>

#include "FusionPhysFS.h"

using namespace std::placeholders;

namespace FusionEngine
{

	const std::string GameMap::metadataExtension = ".yaml";
	const std::string GameMap::instantiatorStateFilename = "instantiator_state";
	const std::string GameMap::tentDataFilename = "transcendental.entitydata";
	const std::string GameMap::entityDatabaseFilename = "entitylocations.kc";

	GameMap::GameMap(const std::string& path)
		: m_Path(path)
	{
		auto findResult = PhysFSHelp::regex_find(path, ".*\\." + metadataExtension);
		if (!findResult.empty())
		{
			m_Name = boost::filesystem::path(findResult[0]).filename().replace_extension().string();

			ReadMetadata();
		}
	}

	void GameMap::ReadMetadata()
	{
		try
		{
			IO::PhysFSStream stream(GetMetadataPath(), IO::Read);
	
			try
			{
				YAML::Node doc = YAML::Load(stream);
				if (doc)
				{
					m_CellSize = doc[std::string("cell_size")].as<float>();
				}
			}
			catch (YAML::ParserException&)
			{
				SendToConsole("Failed to parse map metadata");
			}
		}
		catch (FileSystemException&)
		{
			SendToConsole("Failed to load map metadata file");
		}
	}

	float GameMap::GetCellSize() const
	{
		return m_CellSize;
	}

	void GameMap::InitInstantiator(EntityInstantiator* instantiator)
	{
		try
		{
			IO::PhysFSStream file(GetInstantiatorStatePath(), IO::Read);

			instantiator->LoadState(file);
		}
		catch (FileSystemException&)
		{
			SendToConsole("Failed to load instantiator state");
		}
	}

	void GameMap::LoadNonStreamingEntities(bool include_synched, EntityManager* entityManager, ComponentFactory* factory, ArchetypeFactory* archetype_factory, EntityInstantiator* instantiator)
	{
		using namespace EntitySerialisationUtils;
		namespace io = boost::iostreams;

		IO::PhysFSStream entitydataFile(GetTentDataPath());
		{
			std::shared_ptr<std::istream> stream;
			{
				auto inflateStream = new io::filtering_istream();
				inflateStream->push(io::zlib_decompressor());
				inflateStream->push(entitydataFile);
				stream.reset(inflateStream);
			}
			IO::Streams::CellStreamReader reader(stream.get());

			size_t numPseudoEnts;
			numPseudoEnts = reader.ReadValue<size_t>();
			for (size_t i = 0; i < numPseudoEnts; ++i)
			{
				EntityPtr entity;
				auto result = LoadEntityImmeadiate(std::move(stream), false, 0, FastBinary, factory, entityManager, instantiator);
				entity = result.first; stream = std::move(result.second);
				entity->SetDomain(SYSTEM_DOMAIN);
				entityManager->AddEntity(entity);
			}
			if (include_synched)
			{
				size_t numSynchedEnts;
				numSynchedEnts = reader.ReadValue<size_t>();
				for (size_t i = 0; i < numSynchedEnts; ++i)
				{
					EntityPtr entity;
					auto result = LoadEntityImmeadiate(std::move(stream), true, 0, FastBinary, factory, entityManager, instantiator);
					entity = result.first; stream = std::move(result.second);
					entity->SetDomain(SYSTEM_DOMAIN);
					entityManager->AddEntity(entity);
				}
			}
		}
	}

	void GameMap::CompileMap(const VirtualFilesystem& vfs, const std::string& map_name, float cell_size, CellFileManager* cell_archiver, const std::vector<EntityPtr>& nsentities, EntityInstantiator* instantiator)
	{
		using namespace EntitySerialisationUtils;
		using namespace IO::Streams;

		namespace io = boost::iostreams;

		const std::string path = map_name;

		const std::string metadataPath = path + "/" + map_name + metadataExtension;
		const std::string instantiatorStatePath = path + "/" + instantiatorStateFilename;
		const std::string tentDataPath = path + "/" + tentDataFilename;
		const std::string entityDatabasePath = path + "/" + entityDatabaseFilename;

		// Entity locations database
		cell_archiver->CopyDatabase(entityDatabasePath);
		cell_archiver->CopyCellFiles(path);

		// Metadata
		//  cell size
		{
			auto metadataFile = vfs.OpenFileForWriting(metadataPath);

			YAML::Emitter emitter;

			emitter << YAML::BeginMap;
			emitter << YAML::Key << "cell_size";
			emitter << YAML::Value << cell_size;
			emitter << YAML::EndMap;

			metadataFile->write(emitter.c_str(), emitter.size());
		}

		// Instantiator state
		//  who knows
		{
			auto file = vfs.OpenFileForWriting(instantiatorStatePath);

			instantiator->SaveState(*file);
		}

		// Tents
		//  pseudo entity data
		//  synced entity data
		{
			auto tentsFile = vfs.OpenFileForWriting(tentDataPath);

			// Filter the pseudo / synced entities into separate lists
			std::vector<EntityPtr> nonStreamingEntities = nsentities;
			std::vector<EntityPtr> nonStreamingEntitiesSynched;
			for (auto it = nonStreamingEntities.begin(), end = nonStreamingEntities.end(); it != end;)
			{
				auto& entity = *it;
				if (!entity->IsSyncedEntity())
				{
					++it;
				}
				else
				{
					nonStreamingEntitiesSynched.push_back(entity);
					it = nonStreamingEntities.erase(it);
					end = nonStreamingEntities.end();
				}
			}

			boost::iostreams::filtering_ostream compressingStream;
			compressingStream.push(boost::iostreams::zlib_compressor());
			compressingStream.push(*tentsFile);

			IO::Streams::CellStreamWriter nonsWriter(&compressingStream);

			size_t numEnts = nonStreamingEntities.size();
			nonsWriter.Write(numEnts);
			for (auto it = nonStreamingEntities.begin(), end = nonStreamingEntities.end(); it != end; ++it)
			{
				auto& entity = *it;

				SaveEntity(compressingStream, entity, false, FastBinary);
			}
			numEnts = nonStreamingEntitiesSynched.size();
			nonsWriter.Write(numEnts);
			for (auto it = nonStreamingEntitiesSynched.begin(), end = nonStreamingEntitiesSynched.end(); it != end; ++it)
			{
				auto& entity = *it;

				SaveEntity(compressingStream, entity, true, FastBinary);
			}
		}
	}

	GameMapLoader::GameMapLoader(/*ClientOptions *options*/)
		: /*m_ClientOptions(options),
		m_Factory(factory),
		m_Manager(manager),
		m_FileSource(filesource),*/
		m_NextTypeIndex(0),
		m_MapChecksum(0)
	{
		//m_FactoryConnection = factory->SignalEntityInstanced.connect( std::bind(&GameMapLoader::onEntityInstanced, this, _1) );

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

		auto stringCompressor = RakNet::StringCompressor::Instance();

		if (packetType == ID_NEW_INCOMING_CONNECTION && NetworkManager::ArbitratorIsLocal())
		{
			if (!m_MapFilename.empty())
			{
				// Tell the new peer what the current map is
				RakNet::BitStream replyBitstream;
				//replyBitstream.Write(m_MapFilename.size());
				//replyBitstream.Write(m_MapFilename.data(), m_MapFilename.size());
				stringCompressor->EncodeString(m_MapFilename.c_str(), m_MapFilename.length() + 1, &replyBitstream);
				replyBitstream.Write(m_MapChecksum);

				NetworkManager::getSingleton().GetNetwork()->Send(
					NetDestination(packet->guid, false), !Timestamped, MTID_LOADMAP, &replyBitstream, HIGH_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER);
			}
		}
		else if (packetType == MTID_LOADMAP)
		{
			//std::string::size_type filename_length;
			//bitStream.Read(filename_length);
			std::string filename; filename.resize(256);
			//bitStream.Read(&filename[0], filename_length);
			stringCompressor->DecodeString(&filename[0], filename.length(), &bitStream);
			auto endPos = filename.find('\0');
			if (endPos != std::string::npos)
				filename.erase(endPos);

			uint32_t expectedChecksum;
			bitStream.Read(expectedChecksum);

			try
			{
				IO::PhysFSStream device(filename, IO::Read);

				boost::crc_32_type crc;
				std::array<char, 2048> buffer;
				while (!device.eof())
				{
					device.read(buffer.data(), buffer.size());
					auto count = device.gcount();
					if (count > 0)
						crc.process_bytes(buffer.data(), (std::size_t)count);
				}

				if (crc.checksum() == expectedChecksum)
				{
					m_MapChecksum = crc.checksum();
					//LoadMap(filename, directory, nullptr);
				}
				else
					SendToConsole("Host map is different to local map.");
			}
			catch (clan::Exception& ex)
			{
				SendToConsole("Failed to load host map: " + std::string(ex.what()));
			}
		}
	}

	std::shared_ptr<GameMap> GameMapLoader::LoadMap(const std::string &filename, bool synchronise)
	{
		auto stringCompressor = RakNet::StringCompressor::Instance();

		IO::PhysFSStream device(filename, IO::Read);

		m_MapFilename = filename;

		// Calculate checksum
		if (synchronise)
		{
			boost::crc_32_type crc;
			while (!device.eof())
			{
				char buffer[2048];
				device.read(buffer, 2048);
				crc.process_bytes(buffer, 2048);
			}

			m_MapChecksum = crc.checksum();

			device.seekg(0);

			// Send map change notification to peers
			RakNet::BitStream bitStream;
			//bitStream.Write(m_MapFilename.size());
			//bitStream.Write(m_MapFilename.data(), m_MapFilename.size());
			stringCompressor->EncodeString(m_MapFilename.c_str(), m_MapFilename.length() + 1, &bitStream);
			bitStream.Write(m_MapChecksum);

			NetworkManager::getSingleton().GetNetwork()->Send(
				To::Populace(), !Timestamped, MTID_LOADMAP, &bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, CID_ENTITYMANAGER);
		}

		boost::filesystem::path mapPath(filename);
		if (mapPath.has_parent_path())
			mapPath = mapPath.parent_path() / mapPath.stem();
		else
			mapPath = mapPath.stem();
		auto map = std::make_shared<GameMap>(mapPath.generic_string());

		return map;
	}

	void GameMapLoader::onEntityInstanced(EntityPtr &entity)
	{
	}

}
