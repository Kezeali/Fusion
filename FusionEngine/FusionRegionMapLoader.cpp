/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionRegionMapLoader.h"

#include "FusionRegionCellCache.h"

#include "FusionGameMapLoader.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionInstanceSynchroniser.h"
#include "FusionVirtualFileSource_PhysFS.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIOStream.h"
#include "FusionLogger.h"

#include "FusionBinaryStream.h"

#pragma warning( push );
#pragma warning( disable: 244; )
#include <kchashdb.h>
#pragma warning( pop );

namespace FusionEngine
{

	static const size_t s_DefaultRegionSize = 4;

	RegionMapLoader::RegionMapLoader(bool edit_mode, const std::string& cache_path)
		: m_CachePath(cache_path),
		m_NewData(false),
		m_Instantiator(nullptr),
		m_Running(false),
		m_EditMode(edit_mode),
		m_BeginIndex(std::numeric_limits<size_t>::max()),
		m_EndIndex(0),
		m_RegionSize(s_DefaultRegionSize)
	{
		m_Cache = new RegionCellCache(cache_path);
		m_FullBasePath = PHYSFS_getWriteDir();
		m_FullBasePath += m_CachePath + "/";
	}

	RegionMapLoader::~RegionMapLoader()
	{
		Stop();
	}

	void RegionMapLoader::SetSynchroniser(InstancingSynchroniser* instantiator)
	{
		m_Instantiator = instantiator;
	}

	void RegionMapLoader::SetMap(const std::shared_ptr<GameMap>& map)
	{
		m_Map = map;

		PhysFSHelp::copy_file(m_Map->GetName() + ".endb", m_CachePath + "entitylocations.kc");
		if (m_EntityLocationDB)
			m_EntityLocationDB->close();
		m_EntityLocationDB.reset(new kyotocabinet::HashDB);
		m_EntityLocationDB->open(m_FullBasePath + "entitylocations.kc", kyotocabinet::HashDB::OWRITER);
	}

	void RegionMapLoader::Update(ObjectID id, const RegionMapLoader::CellCoord_t& new_location, std::vector<unsigned char>&& continuous, std::vector<unsigned char>&& occasional)
	{
		m_ObjectUpdateQueue.push(std::make_tuple(id, new_location, std::move(continuous), std::move(occasional)));
		m_NewData.set();
	}

	void RegionMapLoader::Update(ObjectID id, int32_t new_x, int32_t new_y)
	{
		m_ObjectUpdateQueue.push(std::make_tuple(id, CellCoord_t(new_x, new_y), std::vector<unsigned char>(), std::vector<unsigned char>()));
		m_NewData.set();
	}

	void RegionMapLoader::Store(int32_t x, int32_t y, Cell* cell)
	{
		if (cell->waiting.fetch_and_store(Cell::Store) != Cell::Store && cell->loaded)
		{
			cell->AddHist("Enqueued Out");
			m_WriteQueue.push(std::make_tuple(cell, CellCoord_t(x, y)));
			m_NewData.set();
		}
	}

	bool RegionMapLoader::Retrieve(int32_t x, int32_t y, Cell* cell)
	{
		if (cell->waiting.fetch_and_store(Cell::Retrieve) != Cell::Retrieve && !cell->loaded)
		{
			cell->AddHist("Enqueued In");
			m_ReadQueue.push(std::make_tuple(cell, CellCoord_t(x, y)));
			m_NewData.set();
			return true;
		}
		return false;
	}

	void RegionMapLoader::Start()
	{
		m_Running = true;

		m_Quit.reset();
		m_Thread = boost::thread(&RegionMapLoader::Run, this);
#ifdef _WIN32
		SetThreadPriority(m_Thread.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
#endif
	}

	void RegionMapLoader::Stop()
	{
		m_Quit.set();
		m_Thread.join();

		m_Running = false;
	}

	CL_IODevice RegionMapLoader::GetFile(size_t cell_index, bool write) const
	{
		try
		{
			std::stringstream str; str << cell_index;
			std::string filename = "cache/" + str.str();
			if (write || PHYSFS_exists(filename.c_str()))
			{
				CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
				auto file = vdir.open_file(filename, write ? CL_File::create_always : CL_File::open_existing, write ? CL_File::access_write : CL_File::access_read);
				return file;
			}
			else return CL_IODevice();
		}
		catch (CL_Exception& ex)
		{
			SendToConsole(ex.what());
			return CL_IODevice();
		}
	}

	std::unique_ptr<std::istream> RegionMapLoader::GetCellStreamForReading(int32_t cell_x, int32_t cell_y)
	{
		return m_Cache->GetCellStreamForReading(cell_x, cell_y);
	}

	std::unique_ptr<std::ostream> RegionMapLoader::GetCellStreamForWriting(int32_t cell_x, int32_t cell_y)
	{
		return m_Cache->GetCellStreamForWriting(cell_x, cell_y);
	}

	CL_IODevice RegionMapLoader::GetCellData(size_t index) const
	{
		if (m_EditMode && !m_Running)
			return GetFile(index, false);
		else
			FSN_EXCEPT(InvalidArgumentException, "Can't access cell data while running");
	}

	size_t RegionMapLoader::GetDataBegin() const
	{
		if (m_EditMode)
			return m_BeginIndex;
		else
			FSN_EXCEPT(InvalidArgumentException, "This function is only available in edit mode");
	}

	size_t RegionMapLoader::GetDataEnd() const
	{
		if (m_EditMode)
			return m_EndIndex;
		else
			FSN_EXCEPT(InvalidArgumentException, "This function is only available in edit mode");
	}

	EntityPtr RegionMapLoader::Load(ICellStream& file, bool includes_id)
	{
		return EntitySerialisationUtils::LoadEntity(file, includes_id, m_Instantiator->m_Factory, m_Instantiator->m_EntityManager, m_Instantiator);
	}

	size_t RegionMapLoader::LoadEntitiesFromCellData(const CellCoord_t& coord, Cell* cell, ICellStream& file, bool data_includes_ids)
	{
		size_t numEntries;
		file.read(reinterpret_cast<char*>(&numEntries), sizeof(size_t));

		// Skip the ID header-data (is only used when updating inactive cells)
		if (data_includes_ids)
		{
			file.seekg(numEntries * (sizeof(ObjectID) + sizeof(std::streamoff)), std::ios::cur);
		}

		// Read entity data
		for (size_t n = 0; n < numEntries; ++n)
		{
			//auto& archivedEntity = *it;
			auto archivedEntity = Load(file, data_includes_ids);

			Vector2 pos = archivedEntity->GetPosition();
			// TODO: Cell::Add(entity, CellEntry = def) rather than this bullshit
			CellEntry entry;
			entry.x = ToGameUnits(pos.x); entry.y = ToGameUnits(pos.y);

			archivedEntity->SetStreamingCellIndex(coord);

			cell->objects.push_back(std::make_pair(archivedEntity, std::move(entry)));
		}
		return numEntries;
	}

	// expectedNumEntries is used because this can be counted once when WriteCell is called multiple times
	void WriteCell(std::ostream& file, const Cell* cell, size_t expectedNumEntries, const bool synched)
	{
		using namespace EntitySerialisationUtils;

		IO::Streams::CellStreamWriter writer(&file);

		writer.Write(expectedNumEntries);

		std::vector<std::pair<ObjectID, std::streamoff>> dataPositions;
		auto headerPos = file.tellp();
		if (synched)
		{
			// Leave some space for the header data
			const std::vector<char> headerSpace(expectedNumEntries * (sizeof(ObjectID) + sizeof(std::streamoff)));
			file.write(headerSpace.data(), headerSpace.size());

			dataPositions.reserve(expectedNumEntries);
		}

		for (auto it = cell->objects.cbegin(), end = cell->objects.cend(); it != end; ++it)
		{
			const bool entSynched = it->first->IsSyncedEntity();
			if (entSynched == synched)
			{
				if (entSynched)
					dataPositions.push_back(std::make_pair(it->first->GetID(), file.tellp())); // Remember where this data starts, so the header can be written

				SaveEntity(file, it->first, false);
				FSN_ASSERT(expectedNumEntries-- > 0); // Confirm the number of synched / unsynched entities expected
			}
		}

		// Write the header if this is ID'd ("synched") data
		if (synched)
		{
			file.seekp(headerPos);

			for (auto it = dataPositions.cbegin(), end = dataPositions.cend(); it != end; ++it)
			{
				writer.Write(it->first);
				writer.Write(it->second);
			}
		}
	}

	void RegionMapLoader::Run()
	{
		using namespace EntitySerialisationUtils;

		bool retrying = false;
		// TODO: make m_NewData not auto-reset
		while (CL_Event::wait(m_Quit, m_NewData, retrying ? 100 : -1) != 0)
		{
			std::list<std::tuple<Cell*, CellCoord_t>> writesToRetry;
			std::list<std::tuple<Cell*, CellCoord_t>> readsToRetry;

			// Read / write cell data
			{
				std::tuple<Cell*, CellCoord_t> toWrite;
				while (m_WriteQueue.try_pop(toWrite))
				{
					Cell*& cell = std::get<0>(toWrite);
					const auto& cell_coord = std::get<1>(toWrite);
					Cell::mutex_t::scoped_try_lock lock(cell->mutex);
					if (lock)
					{
						// Check active_entries since the Store request may be stale
						if ((m_EditMode || cell->active_entries == 0) && cell->waiting == Cell::Store)
						{
							try
							{
								FSN_ASSERT(cell->loaded == true); // Don't want to create an inaccurate cache (without entities from the map file)

								size_t numSynched = 0;
								size_t numPseudo = 0;
								std::for_each(cell->objects.begin(), cell->objects.end(), [&](const Cell::CellEntryMap::value_type& obj)
								{
									if (!obj.first->IsSyncedEntity())
										++numPseudo;
									else
										++numSynched;
								});

								if (m_EditMode)
								{
									if (numSynched > 0 || numPseudo > 0)
									{
										auto filePtr = GetCellStreamForWriting(cell_coord.x, cell_coord.y);
										if (filePtr && *filePtr)
										{
											auto& file = *filePtr;
											//m_BeginIndex = std::min(cell_coord, m_BeginIndex);
											//m_EndIndex = std::max(cell_coord, m_EndIndex);
											WriteCell(file, cell, numPseudo, false);
											WriteCell(file, cell, numSynched, true);
										}
										else
											FSN_EXCEPT(FileSystemException, "Failed to open file in order to dump edit-mode cache");
									}
								}
								else if (numSynched > 0)
								{
									auto filePtr = GetCellStreamForWriting(cell_coord.x, cell_coord.y);
									WriteCell(*filePtr, cell, numSynched, true);
								}

								cell->AddHist("Written and cleared", numSynched);

								//std::stringstream str; str << i;
								//SendToConsole("Cell " + str.str() + " streamed out");

								// TEMP
								if (!m_EditMode || cell->active_entries == 0)
								{
									cell->objects.clear();
									cell->loaded = false;
								}
							}
							catch (...)
							{
								std::stringstream str; str << cell_coord.x << "," << cell_coord.y;
								SendToConsole("Exception streaming out cell [" + str.str() + "]");
							}
						}
						else
						{
							//std::stringstream str; str << i;
							//SendToConsole("Cell still in use " + str.str());
							//writesToRetry.push_back(toWrite);
						}
						cell->waiting = Cell::Ready;
						//cell->mutex.unlock();
					}
					else
					{
						cell->AddHist("Cell locked (will retry write later)");
						std::stringstream str; str << cell_coord.x << "," << cell_coord.y;
						SendToConsole("Retrying write on cell [" + str.str() + "]");
						writesToRetry.push_back(toWrite);
					}
				}
			}
			{
				std::tuple<Cell*, CellCoord_t> toRead;
				while (m_ReadQueue.try_pop(toRead))
				{
					Cell*& cell = std::get<0>(toRead);
					const CellCoord_t& cell_coord = std::get<1>(toRead);
					Cell::mutex_t::scoped_try_lock lock(cell->mutex);
					if (lock)
					{
						// Make sure this cell hasn't been re-activated:
						if (cell->active_entries == 0 && cell->waiting == Cell::Retrieve)
						{
							try
							{
								// Last param makes the method load synched entities from the map if the cache file isn't available:
								if (m_Map)
								{
									// Load synched entities if this cell is un-cached (hasn't been loaded before)
									bool uncached = m_SynchLoaded.insert(std::make_pair(cell_coord.x, cell_coord.y)).second;

									//m_Map->LoadCell(cell, i, uncached, m_Instantiator->m_Factory, m_Instantiator->m_EntityManager, m_Instantiator);
									auto data = m_Map->GetRegionData(cell_coord.x, cell_coord.y, uncached);

									namespace io = boost::iostreams;
									io::filtering_istream inflateStream;
									inflateStream.push(io::zlib_decompressor());
									inflateStream.push(io::array_source(data.data(), data.size()));

									LoadEntitiesFromCellData(cell_coord, cell, inflateStream, false);
									if (uncached)
										LoadEntitiesFromCellData(cell_coord, cell, inflateStream, true);
								}

								auto filePtr = GetCellStreamForReading(cell_coord.x, cell_coord.y);
								if (filePtr && *filePtr && !filePtr->eof())
								{
									auto& file = *filePtr;
									if (m_EditMode)
										LoadEntitiesFromCellData(cell_coord, cell, file, false); // In edit-mode unsynched entities are also written to the cache
									size_t num = LoadEntitiesFromCellData(cell_coord, cell, file, true);

									//std::stringstream str; str << i;
									//SendToConsole("Cell " + str.str() + " streamed in");

									cell->AddHist("Loaded", num);
									cell->loaded = true;
								}
								else
								{
									cell->loaded = true; // No data to load
									cell->AddHist("Loaded (no data)");
								}
							}
							catch (...)
							{
								//std::stringstream str; str << i;
								//SendToConsole("Exception streaming in cell " + str.str());
								//readsToRetry.push_back(toRead);
							}
						}
						cell->waiting = Cell::Ready;
						//cell->mutex.unlock();
					}
					else
					{
						cell->AddHist("Cell locked (will retry read later)");
						readsToRetry.push_back(toRead);
					}
				}
			}
			retrying = false;
			if (!writesToRetry.empty())
			{
				retrying = true;
				for (auto it = writesToRetry.begin(), end = writesToRetry.end(); it != end; ++it)
					m_WriteQueue.push(*it);
				writesToRetry.clear();
			}
			if (!readsToRetry.empty())
			{
				retrying = true;
				for (auto it = readsToRetry.begin(), end = readsToRetry.end(); it != end; ++it)
					m_ReadQueue.push(*it);
				readsToRetry.clear();
			}

			using namespace IO;

			{
				std::tuple<ObjectID, CellCoord_t, std::vector<unsigned char>, std::vector<unsigned char>> objectUpdateData;
				while (m_ObjectUpdateQueue.try_pop(objectUpdateData))
				{
					const ObjectID id = std::get<0>(objectUpdateData);
					CellCoord_t new_loc = std::get<1>(objectUpdateData);
					auto& incommingConData = std::get<2>(objectUpdateData);
					auto& incommingOccData = std::get<3>(objectUpdateData);

					CellCoord_t loc;
					m_EntityLocationDB->get((const char*)&id, sizeof(id), (char*)&loc, sizeof(loc));

					// Skip if nothing has changed
					if (new_loc == loc && incommingConData.empty() && incommingOccData.empty())
						continue;

					m_EntityLocationDB->set((const char*)&id, sizeof(id), (char*)&new_loc, sizeof(new_loc));
					//const auto loc = m_EntityLocations[id];

					auto inData = GetCellStreamForReading(loc.x, loc.y);
					auto outData = GetCellStreamForWriting(new_loc.x, new_loc.y);

					if (!incommingConData.empty() || !incommingOccData.empty())
					{
						RakNet::BitStream iConDataStream(incommingConData.data(), incommingConData.size(), false);
						RakNet::BitStream iOccDataStream(incommingOccData.data(), incommingOccData.size(), false);

						MergeEntityData(id, *inData, *outData, iConDataStream, iOccDataStream);
					}
					else
					{
						MoveEntityData(id, *inData, *outData);
					}
				}
			}

			/*std::stringstream str;
			str << m_Archived.size();
			SendToConsole(str.str() + " cells archived");*/
		}
	}

	void RegionMapLoader::MergeEntityData(ObjectID id, ICellStream& in, OCellStream& out, RakNet::BitStream& mergeCon, RakNet::BitStream& mergeOcc) const
	{
		std::array<char, 4096> buffer;

		while (!in.eof())
		{
			in.read(buffer.data(), buffer.size());
			out.write(buffer.data(), in.gcount());
		}
	}

	void RegionMapLoader::MoveEntityData(ObjectID id, ICellStream& in, OCellStream& out) const
	{
		std::array<char, 4096> buffer;

		while (!in.eof())
		{
			in.read(buffer.data(), buffer.size());
			out.write(buffer.data(), in.gcount());
		}
	}

}
