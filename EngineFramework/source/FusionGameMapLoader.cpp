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

#include "PrecompiledHeaders.h"

#include "FusionGameMapLoader.h"

#include <boost/crc.hpp>
#include <boost/lexical_cast.hpp>
#include <RakNetTypes.h>
#include <StringCompressor.h>

#include "FusionCellCache.h"
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

#include <boost/filesystem.hpp>

// TODO: remove
#include "FusionVirtualFileSource_PhysFS.h"

using namespace std::placeholders;

namespace FusionEngine
{

	GameMap::GameMap(CL_IODevice& file, const std::string& name)
		: m_File(file),
		m_Name(name)
	{
		m_MinCell.x = m_File.read_int32();
		m_MaxCell.x = m_File.read_int32();
		m_MinCell.y = m_File.read_int32();
		m_MaxCell.y = m_File.read_int32();
		m_CellSize = m_File.read_float();

		if (m_MaxCell.x < m_MinCell.x || m_MaxCell.y < m_MinCell.y)
		{
			FSN_EXCEPT(FileSystemException, "The given map file has an invalid header");
		}

		//m_NumCells = m_MaxCell - m_MinCell;
		// + 1 because both min and max are inclusive
		m_NumCells.x = (uint32_t)((int64_t)m_MaxCell.x - (int64_t)m_MinCell.x + 1);
		m_NumCells.y = (uint32_t)((int64_t)m_MaxCell.y - (int64_t)m_MinCell.y + 1); 

		m_CellLocations.resize(m_NumCells.x * m_NumCells.y);
		for (unsigned int i = 0; i < m_CellLocations.size(); ++i)
		{
			uint32_t begin = m_File.read_uint32();
			uint32_t length = m_File.read_uint32();
			m_CellLocations[i] = std::make_pair(begin, length);

			FSN_ASSERT((length == 0 || begin != 0) && begin + length <= (unsigned int)m_File.get_size());
		}

		m_NonStreamingEntitiesLocation = m_File.read_uint32();
		m_NonStreamingEntitiesDataLength = m_File.read_uint32();
	}

	CL_Rect GameMap::GetBounds() const
	{
		return CL_Rect(m_MinCell.x, m_MinCell.y, m_MaxCell.x, m_MaxCell.y);
	}

	unsigned int GameMap::GetNumCellsAcross() const
	{
		return m_XCells;
	}

	float GameMap::GetMapWidth() const
	{
		return m_MapWidth;
	}

	float GameMap::GetCellSize() const
	{
		return m_CellSize;
	}

	std::vector<char> GameMap::GetRegionData(int32_t x, int32_t y, bool include_synched)
	{
		if (x >= m_MinCell.x && x <= m_MaxCell.x && y >= m_MinCell.y && y <= m_MaxCell.y)
		{
			//Vector2T<uint32_t> size = m_MaxCell - m_MinCell;
			//size.x += 1; size.y += 1; // min and max are inclusive
			auto size = m_NumCells;
			uint32_t adj_x, adj_y;
			if (x < 0)
			{
				FSN_ASSERT(x - m_MinCell.x >= 0);
				FSN_ASSERT(y - m_MinCell.y >= 0);
				adj_x = x - m_MinCell.x;
				adj_y = y - m_MinCell.y;
			}
			else
			{
				// When [x > 0] it is possable that [x - mincell.x] will be greater than numeric_limits<int32_t>::max()
				// (actually, I'm not sure that this is correct - will revisit when I make a map bigger than 2147483647
				//  cells wide (never))
				adj_x = (uint32_t)x - m_MinCell.x;
				adj_y = (uint32_t)y - m_MinCell.y;
			}
			auto index = adj_x + adj_y * size.x;
			auto pos = m_CellLocations[index];

			if (pos.second == 0) // No data for this cell
				return std::vector<char>();

			FSN_ASSERT(pos.first < (unsigned int)std::numeric_limits<int>::max());
			m_File.seek((int)pos.first);

			std::vector<char> buf(pos.second);

			if (m_File.read(buf.data(), pos.second))
				return std::move(buf);
			else
			{
				FSN_EXCEPT(FileSystemException, "Failed to load static region data from the map file");
			}
		}
		else
		{
			return std::vector<char>();
		}
	}

	void GameMap::LoadNonStreamingEntities(bool include_synched, EntityManager* entityManager, ComponentFactory* factory, EntityInstantiator* instantiator)
	{
		using namespace EntitySerialisationUtils;
		namespace io = boost::iostreams;

		if (m_NonStreamingEntitiesDataLength == 0)
			return;

		m_File.seek(m_NonStreamingEntitiesLocation);

		std::vector<char> buf(m_NonStreamingEntitiesDataLength);
		if (m_File.read(buf.data(), m_NonStreamingEntitiesDataLength) == m_NonStreamingEntitiesDataLength)
		{
			io::filtering_istream inflateStream;
			inflateStream.push(io::zlib_decompressor());
			inflateStream.push(io::array_source(buf.data(), buf.size()));

			IO::Streams::CellStreamReader reader(&inflateStream);

			size_t numPseudoEnts;
			numPseudoEnts = reader.ReadValue<size_t>();
			for (size_t i = 0; i < numPseudoEnts; ++i)
			{
				auto entity = LoadEntity(inflateStream, false, 0, factory, entityManager, instantiator);
				entity->SetDomain(SYSTEM_DOMAIN);
				entityManager->AddEntity(entity);
			}
			if (include_synched)
			{
				size_t numSynchedEnts;
				numSynchedEnts = reader.ReadValue<size_t>();
				for (size_t i = 0; i < numSynchedEnts; ++i)
				{
					auto entity = LoadEntity(inflateStream, true, 0, factory, entityManager, instantiator);
					entity->SetDomain(SYSTEM_DOMAIN);
					entityManager->AddEntity(entity);
				}
			}
		}
	}

	// CellArchiver should throw if GetCellData is called while it is running (Stop() must be called first)
	// CellDataSource could be a class (that implements ICellDataSource::GetCellData()) that you create by passing
	//  a StreamingManager and a SimpleCellArchiver. It would call tell the streaming manager to dump all its
	//  cells, then call Stop on the archiver (which causes it to write all queued cells, then stop)
	//  Upon destruction, it would call Start on the archiver and allow the streaming manager to reload its active
	//  cells
	void GameMap::CompileMap(std::ostream &fileStream, float cell_size, CellCache* cell_cache, const std::vector<EntityPtr>& nsentities)
	{
		using namespace EntitySerialisationUtils;
		using namespace IO::Streams;

		namespace io = boost::iostreams;

		CellStreamWriter writer(&fileStream);

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

		auto bounds = cell_cache->GetUsedBounds();
		int32_t minX = bounds.left;
		int32_t maxX = bounds.right;
		int32_t minY = bounds.top;
		int32_t maxY = bounds.bottom;

		const size_t cellsAcross = maxX - minX + 1; // +1 because given bounds are inclusive
		const size_t cellsDown = maxY - minY + 1;

		const size_t numCells = cellsAcross * cellsDown;

		// Write the world bounds info
		writer.Write(minX);
		writer.Write(maxX);
		writer.Write(minY);
		writer.Write(maxY);
		writer.Write(cell_size);

		std::streampos locationsOffset = fileStream.tellp();
		{
			uint32_t locationsSpaceSize = numCells * sizeof(uint32_t) * 2;
			std::vector<char> space(locationsSpaceSize);
			fileStream.write(space.data(), space.size()); // leave some space for the cell-data offsets
		}
		writer.WriteAs<uint32_t>(0); // Non-streaming entities location
		writer.WriteAs<uint32_t>(0); //  '' length

		std::streampos locationsEndOffset;
#ifdef _DEBUG
		locationsEndOffset = fileStream.tellp();
#endif

		std::vector<std::pair<uint32_t, uint32_t>> cellDataLocations(numCells);

		{
			// Buffer for copying data out of the cache files
			std::vector<char> buffer(4096);

			for (int32_t y = minY; y <= maxY; ++y)
			{
				for (int32_t x = minX; x <= maxX; ++x)
				{
					auto cellData = cell_cache->GetRawCellStreamForReading(x, y);

					// Convert the X,Y grid location to a 1-D array index
					const size_t i = (y - minY) * cellsAcross + (x - minX);
					FSN_ASSERT(i < numCells);

					if (cellData)
					{
						auto& cellDataLocation = cellDataLocations[i];
						cellDataLocation.first = uint32_t(fileStream.tellp());

						int bytesRead = 0;
						while (!cellData->eof())
						{
							cellData->read(buffer.data(), buffer.size());
							if (cellData->gcount() > 0)
								fileStream.write(buffer.data(), cellData->gcount());
						}

						cellDataLocation.second = uint32_t(fileStream.tellp()) - cellDataLocation.first;
					}
				}
			}
		}

		// Store the position of this section to write later
		uint32_t nonStreamingEntitiesLocation = 0;
		std::streamoff nonStreamingDataBegin;
		{
			nonStreamingDataBegin = fileStream.tellp();
			if (nonStreamingDataBegin > 0)
				nonStreamingEntitiesLocation = (uint32_t)nonStreamingDataBegin;

			// The non-streaming entities section
			boost::iostreams::filtering_ostream compressingStream;
			compressingStream.push(boost::iostreams::zlib_compressor());
			compressingStream.push(fileStream);

			IO::Streams::CellStreamWriter nonsWriter(&compressingStream);

			size_t numEnts = nonStreamingEntities.size();
			nonsWriter.Write(numEnts);
			for (auto it = nonStreamingEntities.begin(), end = nonStreamingEntities.end(); it != end; ++it)
			{
				auto& entity = *it;

				SaveEntity(compressingStream, entity, false);
			}
			numEnts = nonStreamingEntitiesSynched.size();
			nonsWriter.Write(numEnts);
			for (auto it = nonStreamingEntitiesSynched.begin(), end = nonStreamingEntitiesSynched.end(); it != end; ++it)
			{
				auto& entity = *it;

				SaveEntity(compressingStream, entity, true);
			}
		}
		uint32_t nonStreamingEntitiesDataLength = (uint32_t)(std::streamoff(fileStream.tellp()) - nonStreamingDataBegin);

		fileStream.seekp(locationsOffset);
		for (auto it = cellDataLocations.begin(), end = cellDataLocations.end(); it != end; ++it)
		{
			writer.Write(it->first);
			writer.Write(it->second);
			FSN_ASSERT(fileStream.tellp() <= locationsEndOffset);
		}

		writer.Write(nonStreamingEntitiesLocation);
		writer.Write(nonStreamingEntitiesDataLength);

		FSN_ASSERT(uint32_t(fileStream.tellp()) == locationsEndOffset);
	}

	GameMapLoader::GameMapLoader(ClientOptions *options)
		: m_ClientOptions(options),
		/*m_Factory(factory),
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
				CL_VirtualDirectory directory(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
				CL_IODevice device = directory.open_file(filename, CL_File::open_existing, CL_File::access_read);

				boost::crc_32_type crc;
				int count = 0;
				static const size_t bufferSize = 2048;
				do
				{
					char buffer[bufferSize];
					count = device.read(buffer, bufferSize);
					if (count > 0)
						crc.process_bytes(buffer, (size_t)count);
				} while (count == bufferSize);

				if (crc.checksum() == expectedChecksum)
				{
					m_MapChecksum = crc.checksum();
					//LoadMap(filename, directory, nullptr);
				}
				else
					SendToConsole("Host map is different to local map.");
			}
			catch (CL_Exception& ex)
			{
				SendToConsole("Failed to load host map: " + std::string(ex.what()));
			}
		}
	}

	std::shared_ptr<GameMap> GameMapLoader::LoadMap(const std::string &filename, const CL_VirtualDirectory &directory, EntityInstantiator* synchroniser)
	{
		CL_IODevice device = directory.open_file_read(filename);

		m_MapFilename = filename;

		// Calculate checksum
		if (synchroniser)
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

		boost::filesystem::path mapPath(filename);
		if (mapPath.has_parent_path())
			mapPath = mapPath.parent_path() / mapPath.stem();
		else
			mapPath = mapPath.stem();
		auto map = std::make_shared<GameMap>(device, mapPath.generic_string());

		return map;
	}

	std::shared_ptr<GameMap> GameMapLoader::LoadMap(const std::string &filename, EntityInstantiator* synchroniser)
	{
		return LoadMap(filename, CL_VirtualDirectory(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), ""), synchroniser);
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

}
