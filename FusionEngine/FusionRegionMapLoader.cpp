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

#if _MSC_VER > 1000
#pragma warning( push )
#pragma warning( disable: 4244 4351; )
#endif
#include <kchashdb.h>
#if _MSC_VER > 1000
#pragma warning( pop )
#endif

namespace bio = boost::iostreams;

namespace FusionEngine
{

	static const size_t s_DefaultRegionSize = 4;

	RegionMapLoader::RegionMapLoader(bool edit_mode, const std::string& cache_path)
		: m_CachePath(cache_path),
		m_NewData(false),
		m_TransactionEnded(false),
		m_Instantiator(nullptr),
		m_Running(false),
		m_EditMode(edit_mode),
		m_BeginIndex(std::numeric_limits<size_t>::max()),
		m_EndIndex(0),
		m_RegionSize(s_DefaultRegionSize)
	{
		m_FullBasePath = PHYSFS_getWriteDir();
		m_FullBasePath += m_CachePath + "/";

		std::for_each(m_FullBasePath.begin(), m_FullBasePath.end(), [](std::string::value_type& c) { if (c == '\\') c = '/'; });
		
		m_Cache = new RegionCellCache(m_FullBasePath);

		if (m_EditMode)
		{
			m_EntityLocationDB.reset(new kyotocabinet::HashDB);
			m_EntityLocationDB->open(m_FullBasePath + "entitylocations.kc", kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OCREATE);
		}
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

		PhysFSHelp::copy_file(m_Map->GetName() + ".endb", m_CachePath + "/entitylocations.kc");
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

	void RegionMapLoader::Remove(ObjectID id)
	{
		//m_ObjectRemovalQueue.push(id);
		m_EntityLocationDB->remove((const char*)&id, sizeof(id));
		m_NewData.set();
	}

	Vector2T<int32_t> RegionMapLoader::GetEntityLocation(ObjectID id)
	{
		CellCoord_t loc;
		m_EntityLocationDB->get((const char*)&id, sizeof(id), (char*)&loc, sizeof(loc));
		if (CL_Endian::is_system_big())
		{
			CL_Endian::swap((void*)loc.x, sizeof(loc.x));
			CL_Endian::swap((void*)loc.y, sizeof(loc.y));
		}

		return loc;
	}

	void RegionMapLoader::Store(int32_t x, int32_t y, std::shared_ptr<Cell> cell)
	{
//#ifdef _DEBUG
//		// Make sure this method is only accessed within one thread
//		if (m_ControllerThreadId == boost::thread::id())
//			m_ControllerThreadId = boost::this_thread::get_id();
//		FSN_ASSERT(m_ControllerThreadId == boost::this_thread::get_id());
//#endif

		m_CellsBeingProcessed[CellCoord_t(x, y)] = cell;

		if (cell->waiting.fetch_and_store(Cell::Store) != Cell::Store && cell->loaded)
		{
			cell->AddHist("Enqueued Out");
			m_WriteQueue.push(std::make_tuple(cell.get(), CellCoord_t(x, y)));
			m_NewData.set();
		}
	}

	std::shared_ptr<Cell> RegionMapLoader::Retrieve(int32_t x, int32_t y)
	{
//#ifdef _DEBUG
//		if (m_ControllerThreadId == boost::thread::id())
//			m_ControllerThreadId = boost::this_thread::get_id();
//		FSN_ASSERT(m_ControllerThreadId == boost::this_thread::get_id());
//#endif

		auto _where = m_CellsBeingProcessed.insert(std::make_pair(CellCoord_t(x, y), std::make_shared<Cell>())).first;
		auto& cell = _where->second;

		if (cell->waiting.fetch_and_store(Cell::Retrieve) != Cell::Retrieve && !cell->loaded)
		{
			cell->AddHist("Enqueued In");
			m_ReadQueue.push(std::make_tuple(cell.get(), CellCoord_t(x, y)));
			m_NewData.set();
			//return std::shared_ptr<Cell>();
		}
		else if (cell->loaded)
		{
			std::shared_ptr<Cell> retVal = std::move(cell); // move the ptr so the stored one can be erased
			m_CellsBeingProcessed.erase(_where);
			return std::move(retVal);
		}

		return cell;
	}

	RegionMapLoader::TransactionLock::TransactionLock(boost::mutex& mutex, CL_Event& ev)
		: lock(mutex),
		endEvent(ev)
	{}

	RegionMapLoader::TransactionLock::~TransactionLock()
	{
		endEvent.set();
	}

	std::unique_ptr<RegionMapLoader::TransactionLock> RegionMapLoader::MakeTransaction()
	{
		return std::unique_ptr<TransactionLock>(new TransactionLock(m_Mutex, m_TransactionEnded));
	}

	void RegionMapLoader::BeginTransaction()
	{
		m_Mutex.lock();
	}

	void RegionMapLoader::EndTransaction()
	{
		m_Mutex.unlock();
		m_TransactionEnded.set();
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

	EntityPtr RegionMapLoader::Load(ICellStream& file, bool includes_id, ObjectID id)
	{
		return EntitySerialisationUtils::LoadEntity(file, includes_id, id, m_Instantiator->m_Factory, m_Instantiator->m_EntityManager, m_Instantiator);
	}

	size_t RegionMapLoader::LoadEntitiesFromCellData(const CellCoord_t& coord, Cell* cell, ICellStream& file, bool data_includes_ids)
	{
		size_t numEntries;
		file.read(reinterpret_cast<char*>(&numEntries), sizeof(size_t));

		std::vector<ObjectID> idIndex;

		// NOPE: Skip the ID header-data (is only used when updating inactive cells)
		if (data_includes_ids)
		{
			/*const auto headerSize = numEntries * (sizeof(ObjectID) + sizeof(std::streamoff));
			std::vector<char> bleh(headerSize);
			file.read(bleh.data(), headerSize);*/
			IO::Streams::CellStreamReader reader(&file);
			for (size_t i = 0; i < numEntries; ++i)
			{
				ObjectID id;
				std::streamoff bleh;
				reader.Read(id);
				reader.Read(bleh);
				idIndex.push_back(id);
			}
		}

		// Read entity data
		for (size_t n = 0; n < numEntries; ++n)
		{
			//auto& archivedEntity = *it;
			auto archivedEntity = Load(file, false/*data_includes_ids*/, idIndex[n]);

			Vector2 pos = archivedEntity->GetPosition();
			// TODO: Cell::Add(entity, CellEntry = def) rather than this bullshit
			CellEntry entry;
			entry.x = pos.x; entry.y = pos.y;

			archivedEntity->SetStreamingCellIndex(coord);

			cell->objects.push_back(std::make_pair(archivedEntity, std::move(entry)));
		}
		return numEntries;
	}

	// expectedNumEntries is used because this can be counted once when WriteCell is called multiple times
	void RegionMapLoader::WriteCell(std::ostream& file, const Cell* cell, size_t expectedNumEntries, const bool synched)
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
					dataPositions.push_back(std::make_pair(it->first->GetID(), file.tellp() - headerPos)); // Remember where this data starts, so the header can be written

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

	static void storeCellLocation(kyotocabinet::HashDB& db, ObjectID id, RegionMapLoader::CellCoord_t new_loc)
	{
		if (CL_Endian::is_system_big())
		{
			CL_Endian::swap((void*)new_loc.x, sizeof(new_loc.x));
			CL_Endian::swap((void*)new_loc.y, sizeof(new_loc.y));
		}
#ifdef _DEBUG
		// Make sure the cell-location data is stored as expected
		FSN_ASSERT(sizeof(new_loc) == sizeof(int64_t));
		{
			int64_t new_loc_test = new_loc.x;
			new_loc_test = new_loc_test << 32;
			new_loc_test |= new_loc.y;
			FSN_ASSERT(memcmp(&new_loc, &new_loc_test, sizeof(new_loc)) == 0);
		}
#endif
		db.set((const char*)&id, sizeof(id), (char*)&new_loc, sizeof(new_loc));
	}

	void RegionMapLoader::Run()
	{
		using namespace EntitySerialisationUtils;

		std::list<CellCoord_t> readyCells;
		bool retrying = false;
		// TODO: make m_NewData not auto-reset
		while (true)
		{
			const int eventId = CL_Event::wait(m_Quit, m_NewData, m_TransactionEnded, retrying ? 100 : -1);
			if (eventId == 0)
			{
				// Drop references to cells that are done processing (if m_CellsBeingProcessed isn't locked by a current transaction)
				if (!readyCells.empty())
				{
					{
						boost::mutex::scoped_lock lock(m_Mutex);
						if (lock)
						{
							for (auto it = readyCells.begin(); it != readyCells.end(); ++it)
								m_CellsBeingProcessed.erase(*it);
						}
					}
					readyCells.clear();
				}
				break;
			}
			else if (eventId == 2)
			{
				// Drop references to cells that are done processing (if m_CellsBeingProcessed isn't locked by a current transaction)
				if (!readyCells.empty())
				{
					{
						boost::mutex::scoped_lock lock(m_Mutex);
						if (lock)
						{
							for (auto it = readyCells.begin(); it != readyCells.end(); ++it)
								m_CellsBeingProcessed.erase(*it);
						}
					}
					readyCells.clear();
				}
			}
			else
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

									std::for_each(cell->objects.begin(), cell->objects.end(), [&](const Cell::CellEntryMap::value_type& obj)
									{
										if (obj.first->IsSyncedEntity())
										{
											ObjectID id = obj.first->GetID();
											storeCellLocation(*m_EntityLocationDB, id, cell_coord);
										}
									});

									if (m_EditMode)
									{
										if (numSynched > 0 || numPseudo > 0)
										{
											auto filePtr = GetCellStreamForWriting(cell_coord.x, cell_coord.y);
											if (filePtr && *filePtr)
											{
												auto& file = *filePtr;

												// Need a seekable stream, so write to a temp. one
												//bio::stream<bio::array_sink> tempStream();
												std::stringstream tempStream(std::ios::in | std::ios::out | std::ios::binary);
												std::streampos start = tempStream.tellp();
												// Write un-synched entity data (written to cache since this is edit mode)
												WriteCell(tempStream, cell, numPseudo, false);
												std::streamsize dataLength = tempStream.tellp() - start;

												WriteCell(tempStream, cell, numSynched, true);

												// Write to the file-stream
												IO::Streams::CellStreamWriter writer(&file);
												writer.Write(dataLength);

												file << tempStream.rdbuf();
											}
											else
												FSN_EXCEPT(FileSystemException, "Failed to open file in order to dump edit-mode cache");
										}
									}
									else if (numSynched > 0)
									{
										auto filePtr = GetCellStreamForWriting(cell_coord.x, cell_coord.y);
										// Need a seekable stream, so write to a temp. one
										//bio::stream<bio::array_sink> tempStream();
										std::stringstream tempStream(std::ios::in | std::ios::out | std::ios::binary);
										WriteCell(tempStream, cell, numSynched, true);
										*filePtr << tempStream.rdbuf();
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
							readyCells.push_back(cell_coord);
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

										if (!data.empty())
										{
											bio::filtering_istream inflateStream;
											inflateStream.push(bio::zlib_decompressor());
											inflateStream.push(bio::array_source(data.data(), data.size()));

											LoadEntitiesFromCellData(cell_coord, cell, inflateStream, false);
											if (uncached)
												LoadEntitiesFromCellData(cell_coord, cell, inflateStream, true);
										}
									}

									auto filePtr = GetCellStreamForReading(cell_coord.x, cell_coord.y);
									if (filePtr && *filePtr && !filePtr->eof())
									{
										auto& file = *filePtr;
										if (m_EditMode)
										{
											// The length of this (edit-mode only) data is written in front of it so that it can be skipped when merging incomming data
											//  (we don't need to know this length to actually load the data):
#ifdef _DEBUG
											IO::Streams::CellStreamReader reader(&file);
											std::streamsize unsynchedDataLength = reader.ReadValue<std::streamsize>();
											//std::streampos startRead = file.tellg();
#else
											IO::Streams::CellStreamReader reader(&file);
											reader.ReadValue<std::streamsize>();
											//file.seekg(std::streamoff(sizeof(std::streamsize)), std::ios::cur);
#endif
											LoadEntitiesFromCellData(cell_coord, cell, file, false); // In edit-mode unsynched entities are also written to the cache
#ifdef _DEBUG
											// Make sure all the data was read
											// TODO: in Release builds: skip to the end if it wasn't all read?
											//FSN_ASSERT(unsynchedDataLength == file.tellg() - startRead);
#endif
										}
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
							readyCells.push_back(cell_coord);
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

				{
					using namespace IO;

					std::tuple<ObjectID, CellCoord_t, std::vector<unsigned char>, std::vector<unsigned char>> objectUpdateData;
					while (m_ObjectUpdateQueue.try_pop(objectUpdateData))
					{
						const ObjectID id = std::get<0>(objectUpdateData);
						CellCoord_t new_loc = std::get<1>(objectUpdateData);
						auto& incommingConData = std::get<2>(objectUpdateData);
						auto& incommingOccData = std::get<3>(objectUpdateData);

						CellCoord_t loc;
						m_EntityLocationDB->get((const char*)&id, sizeof(id), (char*)&loc, sizeof(loc));
						if (CL_Endian::is_system_big())
						{
							CL_Endian::swap((void*)loc.x, sizeof(loc.x));
							CL_Endian::swap((void*)loc.y, sizeof(loc.y));
						}

						if (new_loc == CellCoord_t(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()))
							new_loc = loc;

						// Skip if nothing has changed
						if (new_loc == loc && incommingConData.empty() && incommingOccData.empty())
							continue;


						storeCellLocation(*m_EntityLocationDB, id, new_loc);

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

			}
		}
	}

	void RegionMapLoader::MergeEntityData(ObjectID id, ICellStream& in, OCellStream& out, RakNet::BitStream& mergeCon, RakNet::BitStream& mergeOcc) const
	{
		std::array<char, 4096> buffer;

		IO::Streams::CellStreamReader reader(&in);

		// Skip the un-synced entity data that is present in Edit Mode
		if (m_EditMode)
		{
			std::streamsize unsynchedDataLength = reader.ReadValue<std::streamsize>();

			FSN_ASSERT(unsynchedDataLength >= 0);
			std::streamsize remainingData = unsynchedDataLength;

			while (remainingData > 0 && !in.eof())
			{
				in.read(buffer.data(), std::min(std::streamsize(buffer.size()), remainingData));
				auto gcount = in.gcount();
				out.write(buffer.data(), gcount);
				FSN_ASSERT(gcount <= remainingData);
				remainingData -= gcount;
			}
		}

		size_t numEnts = reader.ReadValue<size_t>();

		std::streamoff dataOff = 0;
		//auto headerPos = in.tellg(); // The offsets given in the header are relative to the start of the header

		for (size_t i = 0; i < numEnts; ++i)
		{
			ObjectID iID = reader.ReadValue<ObjectID>();
			std::streamoff iDataOff = reader.ReadValue<std::streamoff>();

			if (iID == id)
			{
				FSN_ASSERT(dataOff == 0); // Make sure the ID isn't repeated (or the header is corrupted, I guess)
				dataOff = iDataOff;
			}
		}

		const size_t headerSize = numEnts * (sizeof(ObjectID) + sizeof(std::streamoff));
		auto remainingData = dataOff - headerSize; // The offsets given in the header are relative to the start of the header

		// Copy the data up to the relevant entity data
		while (remainingData > 0 && !in.eof())
		{
			in.read(buffer.data(), std::min(std::streamsize(buffer.size()), remainingData));
			auto gcount = in.gcount();
			out.write(buffer.data(), gcount);
			FSN_ASSERT(gcount <= remainingData);
			remainingData -= gcount;
		}

		// Merge the data
		EntitySerialisationUtils::MergeEntityData(in, out, mergeCon, mergeOcc);

		// Copy the rest of the entity data
		while (!in.eof())
		{
			in.read(buffer.data(), buffer.size());
			out.write(buffer.data(), in.gcount());
		}
	}

	void RegionMapLoader::MoveEntityData(ObjectID id, ICellStream& in, OCellStream& out) const
	{
		//std::array<char, 4096> buffer;

		//while (!in.eof())
		//{
		//	in.read(buffer.data(), buffer.size());
		//	out.write(buffer.data(), in.gcount());
		//}

		out << in.rdbuf();
	}

}
