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

#include "PrecompiledHeaders.h"

#include "FusionRegionMapLoader.h"

#include "FusionRegionCellCache.h"

#include "FusionAnyFS.h"
#include "FusionGameMapLoader.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionEntityInstantiator.h"
#include "FusionVirtualFileSource_PhysFS.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIOStream.h"
#include "FusionLogger.h"
#include "FusionZipArchive.h"

#include "FusionBinaryStream.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/array.hpp>

#include <numeric>

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

	extern void AddHist(const CellHandle& loc, const std::string& l, unsigned int n = -1);

	//! boost::iostreams filter that counts bytes written
	struct CharCounter
	{
		typedef char char_type;
		typedef bio::multichar_output_filter_tag category;

		std::shared_ptr<std::streamsize> totalWritten;

		CharCounter() : totalWritten(new std::streamsize(0))
		{}

		std::streamsize count() const { return *totalWritten; }

		template <typename Sink>
		std::streamsize write(Sink& snk, const char_type* s, std::streamsize n)
		{
			*totalWritten += n;
			bio::write(snk, s, n);
			return n;
		}
	};

	static void storeEntityLocation(kyotocabinet::HashDB& db, ObjectID id, RegionMapLoader::CellCoord_t new_loc, std::streamoff offset, std::streamsize length)
	{
		std::array<char, sizeof(new_loc) + sizeof(offset) + sizeof(length)> data;

		if (id == 1)
		{
			SendToConsole("One");
		}

		// Convert to little-endian
		if (CL_Endian::is_system_big())
		{
			CL_Endian::swap((void*)new_loc.x, sizeof(new_loc.x));
			CL_Endian::swap((void*)new_loc.y, sizeof(new_loc.y));
			CL_Endian::swap((void*)offset, sizeof(offset));
			CL_Endian::swap((void*)length, sizeof(length));
		}

		// Copy the values into a buffer
		std::memcpy(data.data(), (char*)&new_loc, sizeof(new_loc));
		std::memcpy(&data[sizeof(new_loc)], (char*)&offset, sizeof(offset));
		std::memcpy(&data[sizeof(new_loc) + sizeof(offset)], (char*)&length, sizeof(length));

#ifdef _DEBUG
		// Make sure the cell-location data is stored as expected
		FSN_ASSERT(sizeof(new_loc) == sizeof(int64_t));
		{
			FSN_ASSERT(std::memcmp(&new_loc.x, data.data(), sizeof(new_loc.x)) == 0);
		}
#endif

		// Write the buffer to the DB
		db.set((const char*)&id, sizeof(id), data.data(), data.size());
	}

	static bool getEntityLocation(kyotocabinet::HashDB& db, RegionMapLoader::CellCoord_t& cell_loc, std::streamoff& data_offset, std::streamsize& data_length, ObjectID id)
	{
		std::array<char, sizeof(cell_loc) + sizeof(data_offset) + sizeof(data_length)> data;

		auto retrieved = db.get((const char*)&id, sizeof(id), data.data(), data.size());

		if (retrieved == data.size())
		{
			std::memcpy((void*)&cell_loc, data.data(), sizeof(cell_loc));
			std::memcpy((void*)&data_offset, &data[sizeof(cell_loc)], sizeof(data_offset));
			std::memcpy((void*)&data_length, &data[sizeof(cell_loc) + sizeof(data_offset)], sizeof(data_length));

			if (CL_Endian::is_system_big())
			{
				CL_Endian::swap((void*)cell_loc.x, sizeof(cell_loc.x));
				CL_Endian::swap((void*)cell_loc.y, sizeof(cell_loc.y));
				CL_Endian::swap((void*)data_offset, sizeof(data_offset));
				CL_Endian::swap((void*)data_length, sizeof(data_length));
			}

			return true;
		}
		else
			return false;
	}

	static void setupTuning(kyotocabinet::HashDB* db)
	{
		db->tune_defrag(8);
		db->tune_map(2LL << 20); // 2MB memory-map
	}

	RegionMapLoader::RegionMapLoader(bool edit_mode, const std::string& cache_path)
		: m_EditMode(edit_mode),
		m_Running(false),
		m_RegionSize(s_DefaultRegionSize),
		m_SavePath(s_SavePath),
		m_CachePath(cache_path),
		m_Instantiator(nullptr),
		m_Factory(nullptr),
		m_EntityManager(nullptr),
		m_NewData(false),
		m_TransactionEnded(false)
	{
		m_FullBasePath = PHYSFS_getWriteDir();
		m_FullBasePath += m_CachePath + "/";

		std::for_each(m_FullBasePath.begin(), m_FullBasePath.end(), [](std::string::value_type& c) { if (c == '\\') c = '/'; });
		
		m_Cache = new RegionCellCache(m_FullBasePath);

		if (m_EditMode)
		{
			m_EntityLocationDB.reset(new kyotocabinet::HashDB);
			m_EntityLocationDB->tune_options(kyotocabinet::HashDB::TSMALL);
			setupTuning(m_EntityLocationDB.get());
			m_EntityLocationDB->open(m_FullBasePath + "entitylocations.kc", kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OCREATE);

			m_Cache->SetupEditMode(true);
		}
	}

	RegionMapLoader::~RegionMapLoader()
	{
		Stop();

		delete m_Cache;
	}

	void RegionMapLoader::SetInstantiator(EntityInstantiator* instantiator, ComponentFactory* component_factory, EntityManager* manager)
	{
		m_Instantiator = instantiator;
		m_Factory = component_factory;
		m_EntityManager = manager;
	}

	void RegionMapLoader::SetMap(const std::shared_ptr<GameMap>& map)
	{
		m_Map = map;

		PhysFSHelp::copy_file(m_Map->GetName() + ".endb", m_CachePath + "/entitylocations.kc");
		if (m_EntityLocationDB)
			m_EntityLocationDB->close();
		m_EntityLocationDB.reset(new kyotocabinet::HashDB);
		setupTuning(m_EntityLocationDB.get());
		m_EntityLocationDB->open(m_FullBasePath + "entitylocations.kc", kyotocabinet::HashDB::OWRITER);

		m_Cache->SetupEditMode(m_EditMode, m_Map->GetBounds());
	}

	void RegionMapLoader::Update(ObjectID id, const RegionMapLoader::CellCoord_t& new_location, std::vector<unsigned char>&& continuous, std::vector<unsigned char>&& occasional)
	{
		m_ObjectUpdateQueue.push(std::make_tuple(id, UpdateOperation::UPDATE, new_location, std::move(continuous), std::move(occasional)));
		m_NewData.set();
	}

	void RegionMapLoader::Update(ObjectID id, int32_t new_x, int32_t new_y)
	{
		m_ObjectUpdateQueue.push(std::make_tuple(id, UpdateOperation::UPDATE, CellCoord_t(new_x, new_y), std::vector<unsigned char>(), std::vector<unsigned char>()));
		m_NewData.set();
	}

	void RegionMapLoader::ActiveUpdate(ObjectID id, int32_t new_x, int32_t new_y)
	{
		// TODO: store active entity locations in-memory (in StreamingManager)?
		FSN_ASSERT(m_EntityLocationDB);
		CellCoord_t loc;
		std::streamoff offset = 0; std::streamsize length = 0;
		getEntityLocation(*m_EntityLocationDB, loc, offset, length, id);
		storeEntityLocation(*m_EntityLocationDB, id, CellCoord_t(new_x, new_y), offset, length);
	}

	void RegionMapLoader::Remove(ObjectID id)
	{
		m_ObjectUpdateQueue.push(std::make_tuple(id, UpdateOperation::REMOVE, CellCoord_t(), std::vector<unsigned char>(), std::vector<unsigned char>()));
		m_NewData.set();
	}

	Vector2T<int32_t> RegionMapLoader::GetEntityLocation(ObjectID id)
	{
		CellCoord_t loc(std::numeric_limits<CellCoord_t::type>::max(), std::numeric_limits<CellCoord_t::type>::max());
		if (m_EntityLocationDB) // TODO: lock while loading save-game and wait here
		{
			std::streamoff offset; std::streamsize length;
			getEntityLocation(*m_EntityLocationDB, loc, offset, length, id);
		}
		return loc;
	}

	void RegionMapLoader::Store(int32_t x, int32_t y, std::shared_ptr<Cell> cell)
	{
		if (cell->waiting.fetch_and_store(Cell::Store) != Cell::Store && cell->loaded)
		{
			AddHist(CellCoord_t(x, y), "Enqueued Out");
			// The last tuple param indicates whether the cell should be cleared (unloaded)
			//  when the write operation is done: it checks whether this is the only reference
			//  to the cell
			m_WriteQueue.push(std::make_tuple(cell, CellCoord_t(x, y), cell.unique()));
			m_NewData.set();
			
			TransactionMutex_t::scoped_try_lock lock(m_TransactionMutex);
			FSN_ASSERT_MSG(lock, "Concurrent Store/Retrieve access isn't allowed");
			m_CellsBeingProcessed[CellCoord_t(x, y)] = std::move(cell);
		}
	}

	std::shared_ptr<Cell> RegionMapLoader::Retrieve(int32_t x, int32_t y)
	{
		TransactionMutex_t::scoped_try_lock lock(m_TransactionMutex);
		FSN_ASSERT_MSG(lock, "Concurrent Store/Retrieve access isn't allowed");
#ifdef _DEBUG
		auto r = m_CellsBeingProcessed.insert(std::make_pair(CellCoord_t(x, y), std::make_shared<Cell>()));
		auto _where = r.first;
#else
		auto _where = m_CellsBeingProcessed.insert(std::make_pair(CellCoord_t(x, y), std::make_shared<Cell>())).first;
#endif
		auto& cell = _where->second;

		if (cell->waiting.fetch_and_store(Cell::Retrieve) != Cell::Retrieve && !cell->loaded)
		{
			AddHist(CellCoord_t(x, y), "Enqueued In");
			m_ReadQueue.push(std::make_tuple(cell, CellCoord_t(x, y)));
			m_NewData.set();
		}
		else if (cell->loaded)
		{
			AddHist(CellCoord_t(x, y), "Already loaded");
			cell->waiting = Cell::Ready;
			std::shared_ptr<Cell> retVal = std::move(cell); // move the ptr so the stored one can be erased
			m_CellsBeingProcessed.erase(_where);
			return std::move(retVal);
		}

		return cell;
	}

	RegionMapLoader::TransactionLock::TransactionLock(RegionMapLoader::TransactionMutex_t& mutex, CL_Event& ev)
		: lock(mutex),
		endEvent(ev)
	{}

	RegionMapLoader::TransactionLock::~TransactionLock()
	{
		endEvent.set();
	}

	std::unique_ptr<RegionMapLoader::TransactionLock> RegionMapLoader::MakeTransaction()
	{
		return std::unique_ptr<TransactionLock>(new TransactionLock(m_TransactionMutex, m_TransactionEnded));
	}

	void RegionMapLoader::BeginTransaction()
	{
		m_TransactionMutex.lock();
	}

	void RegionMapLoader::EndTransaction()
	{
		m_TransactionMutex.unlock();
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

	std::unique_ptr<std::istream> RegionMapLoader::GetCellStreamForReading(int32_t cell_x, int32_t cell_y)
	{
		return m_Cache->GetCellStreamForReading(cell_x, cell_y);
	}

	std::unique_ptr<std::ostream> RegionMapLoader::GetCellStreamForWriting(int32_t cell_x, int32_t cell_y)
	{
		return m_Cache->GetCellStreamForWriting(cell_x, cell_y);
	}

	void RegionMapLoader::SaveEntityLocationDB(const std::string& filename)
	{
		if (m_EntityLocationDB)
		{
			std::string fullPath = boost::filesystem::path(filename).generic_string();
			std::string writeDir = boost::filesystem::path(PHYSFS_getWriteDir()).generic_string();

			if (fullPath.length() < writeDir.length() || fullPath.substr(0, writeDir.length()) != writeDir)
				fullPath = writeDir + "/" + fullPath;

			if (m_Thread.joinable())
			{
				// Thread is running, DB may be in use: do hot copy
				m_EntityLocationDB->copy(fullPath);
			}
			else
			{
				// Close the DB to do a clean copy (closed DB can be smaller)
				std::string dbPath = m_EntityLocationDB->path();
				m_EntityLocationDB->close();
				m_EntityLocationDB.reset();

				try
				{
					boost::filesystem::copy_file(dbPath, fullPath, boost::filesystem::copy_option::overwrite_if_exists);
				}
				catch (boost::filesystem::filesystem_error& e)
				{
					FSN_EXCEPT(FileSystemException, std::string("Failed to copy entity location db: ") + e.what());
				}

				m_EntityLocationDB.reset(new kyotocabinet::HashDB);
				setupTuning(m_EntityLocationDB.get());
				m_EntityLocationDB->open(dbPath, kyotocabinet::HashDB::OWRITER);
			}
		}
		else
		{
			FSN_ASSERT_FAIL("Entity location DB should be open");
		}
	}

	void RegionMapLoader::EnqueueQuickSave(const std::string& save_name)
	{
		m_SaveQueue.push(save_name);
		m_NewData.set();
	}

	void RegionMapLoader::Save(const std::string& save_name)
	{
		const bool wasRunning = m_Thread.joinable();
		if (wasRunning)
			Stop();
		PerformSave(save_name);
		CompressSave(save_name);
		if (wasRunning)
			Start();
	}

	void RegionMapLoader::EnqueueQuickLoad(const std::string& save_name)
	{
		// Stop and clear the cache
		PrepareLoad(save_name);

		m_Cache->SetSavePath(save_name);
		m_NewData.set();
	}

	void RegionMapLoader::Load(const std::string& save_name)
	{
		const bool wasRunning = m_Thread.joinable();
		if (wasRunning)
			Stop();
		PrepareLoad(save_name);
		PerformLoad(save_name);
		if (wasRunning)
			Start();
	}

	std::unique_ptr<std::ostream> RegionMapLoader::CreateDataFile(const std::string& filename)
	{
		namespace bfs = boost::filesystem;
		namespace io = boost::iostreams;

		auto savePath = bfs::path(m_FullBasePath);

		auto filePath = savePath;
		filePath /= (filename + ".dat");

		try
		{
			io::file_descriptor_sink file(filePath.generic_string(), std::ios::out | std::ios::trunc | std::ios::binary);
			if (file.is_open())
			{
				auto stream = std::unique_ptr<io::filtering_ostream>(new io::filtering_ostream());
				stream->push(io::zlib_compressor());
				stream->push(file);
				return std::move(stream);
			}
		}
		catch (...)
		{
		}
		return std::unique_ptr<io::filtering_ostream>();
	}

	std::unique_ptr<std::istream> RegionMapLoader::LoadDataFile(const std::string& filename)
	{
		namespace bfs = boost::filesystem;
		namespace io = boost::iostreams;

		auto savePath = bfs::path(m_FullBasePath);

		auto filePath = savePath;
		filePath /= (filename + ".dat");

		if (bfs::exists(filePath))
		{
			io::file_descriptor_source file(filePath.generic_string(), std::ios::in | std::ios::binary);
			if (file.is_open())
			{
				auto stream = std::unique_ptr<io::filtering_istream>(new io::filtering_istream());
				stream->push(io::zlib_decompressor());
				stream->push(file);
				return std::move(stream);
			}
		}
		return std::unique_ptr<io::filtering_istream>();
	}

	EntityPtr RegionMapLoader::LoadEntity(ICellStream& file, bool includes_id, ObjectID id)
	{
		return EntitySerialisationUtils::LoadEntity(file, includes_id, id, m_Factory, m_EntityManager, m_Instantiator);
	}

	size_t RegionMapLoader::LoadEntitiesFromCellData(const CellCoord_t& coord, Cell* cell, ICellStream& file, bool data_includes_ids)
	{
		size_t numEntries;
		file.read(reinterpret_cast<char*>(&numEntries), sizeof(size_t));

		FSN_ASSERT_MSG(numEntries < (1 << 16), "Probably invalid data");

		std::vector<ObjectID> idIndex;

		if (data_includes_ids)
		{
			/*const auto headerSize = numEntries * (sizeof(ObjectID) + sizeof(std::streamoff));
			std::vector<char> bleh(headerSize);
			file.read(bleh.data(), headerSize);*/
			IO::Streams::CellStreamReader reader(&file);
			for (size_t i = 0; i < numEntries; ++i)
			{
				ObjectID id;
				//std::streamoff bleh;
				reader.Read(id);
				//reader.Read(bleh);
				idIndex.push_back(id);
			}
		}

		// Read entity data
		for (size_t n = 0; n < numEntries; ++n)
		{
			//auto& archivedEntity = *it;
			auto archivedEntity = LoadEntity(file, false/*data_includes_ids*/, data_includes_ids ? idIndex[n] : 0);

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
	void RegionMapLoader::WriteCell(std::ostream& file_param, const CellCoord_t& loc, const Cell* cell, size_t expectedNumEntries, const bool synched)
	{
		using namespace EntitySerialisationUtils;

		CharCounter counter;
		bio::filtering_ostream file;
		file.push(counter, 0);
		file.push(file_param);

		IO::Streams::CellStreamWriter writer(&file);

		writer.Write(expectedNumEntries);

		std::vector<std::tuple<ObjectID, std::streamoff, std::streamsize>> dataPositions;
		//std::streamoff headerPos = sizeof(expectedNumEntries);
		if (synched)
		{
			// Leave some space for the header data
			//const std::vector<char> headerSpace(expectedNumEntries * (sizeof(ObjectID) /*+ sizeof(std::streamoff)*/));
			//file.write(headerSpace.data(), headerSpace.size());

			for (auto it = cell->objects.cbegin(), end = cell->objects.cend(); it != end; ++it)
			{
				const bool entSynched = it->first->IsSyncedEntity();
				if (entSynched)
					writer.Write(it->first->GetID());
			}

			dataPositions.reserve(expectedNumEntries);
		}

		std::streamoff beginPos;
		for (auto it = cell->objects.cbegin(), end = cell->objects.cend(); it != end; ++it)
		{
			const bool entSynched = it->first->IsSyncedEntity();
			if (entSynched == synched)
			{
				if (entSynched)
				{
					beginPos = counter.count();
				}

				SaveEntity(file, it->first, false);

				if (entSynched)
				{
					auto endPos = counter.count();
					std::streamsize length = endPos - beginPos;
					dataPositions.push_back(std::make_tuple(it->first->GetID(), beginPos, length));
				}

				FSN_ASSERT(expectedNumEntries-- > 0); // Confirm the number of synched / unsynched entities expected
			}
		}

		// Write the header if this is ID'd ("synched") data
		if (synched)
		{
			//file.seekp(headerPos);

			for (auto it = dataPositions.cbegin(), end = dataPositions.cend(); it != end; ++it)
			{
				//writer.Write(it->first);
				//writer.Write(it->second);
				storeEntityLocation(*m_EntityLocationDB, std::get<0>(*it), loc, std::get<1>(*it), std::get<2>(*it));
			}
		}
	}

	void RegionMapLoader::Run()
	{
		using namespace EntitySerialisationUtils;

		std::list<CellCoord_t> readyCells;
		bool retrying = false;
		while (true)
		{
			const int eventId = CL_Event::wait(m_Quit, m_TransactionEnded, m_NewData, retrying ? 100 : -1);
			if (eventId == 1) // TransactionEnded
			{
				ClearReadyCells(readyCells);
			}
			{
				// Process quick-save queue
				{
					std::string saveName;
					while (m_SaveQueue.try_pop(saveName))
					{
						PerformSave(saveName);
					}
				}

				std::list<WriteQueue_t::value_type> writesToRetry;
				std::list<ReadQueue_t::value_type> readsToRetry;
				// Read / write cell data
				{
					std::tuple<std::weak_ptr<Cell>, CellCoord_t, bool> toWrite;
					while (m_WriteQueue.try_pop(toWrite))
					{
						const std::weak_ptr<Cell>& cellWpt = std::get<0>(toWrite);
						const auto& cell_coord = std::get<1>(toWrite);
						const bool unload_when_done = std::get<2>(toWrite);
						if (auto cell = cellWpt.lock()) // Make sure the queue item is valid
						{
							Cell::mutex_t::scoped_try_lock lock(cell->mutex);
							if (lock)
							{
								// Check active_entries since the Store request may be stale
								if (cell->waiting == Cell::Store && cell->loaded)
								{
									try
									{
										if (cell->active_entries != 0 && !m_EditMode)
											AddLogEntry("Warning: writing cell with active entries");

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
											//if (numSynched > 0 || numPseudo > 0)
											{
												auto filePtr = GetCellStreamForWriting(cell_coord.x, cell_coord.y);
												if (filePtr && *filePtr)
												{
													auto& file = *filePtr;

													// Need a seekable stream, so write to a temp. one
													std::stringstream tempStream(std::ios::in | std::ios::out | std::ios::binary);
													std::streampos start = tempStream.tellp();
													// Write un-synched entity data (written to cache since this is edit mode)
													WriteCell(tempStream, cell_coord, cell.get(), numPseudo, false);
													std::streamsize dataLength = tempStream.tellp() - start;

													WriteCell(tempStream, cell_coord, cell.get(), numSynched, true);

													// Write to the file-stream
													IO::Streams::CellStreamWriter writer(&file);
													writer.Write(dataLength);

													file << tempStream.rdbuf();
												}
												else
													FSN_EXCEPT(FileSystemException, "Failed to open file in order to dump edit-mode cache");
											}
										}
										else // Not EditMode
										{
											auto filePtr = GetCellStreamForWriting(cell_coord.x, cell_coord.y);
											// Need a seekable stream, so write to a temp. one
											//std::stringstream tempStream(std::ios::in | std::ios::out | std::ios::binary);
											WriteCell(*filePtr, cell_coord, cell.get(), numSynched, true);
											//*filePtr << tempStream.rdbuf();
										}

										AddHist(cell_coord, "Written and cleared", numSynched);

										if (unload_when_done)
										{
											if (cell->active_entries != 0)
												AddLogEntry("Warning: unloading active cell");
											cell->objects.clear();
											cell->loaded = false;
										}
									}
									catch (...)
									{
										std::stringstream str; str << cell_coord.x << "," << cell_coord.y;
										std::string message = "Exception streaming out cell [" + str.str() + "]";
										SendToConsole(message);
										AddLogEntry(message);
									}
								}
								else
								{
									std::stringstream str; str << cell_coord.x << "," << cell_coord.y;
									SendToConsole("Cell write canceled: " + str.str());
									//writesToRetry.push_back(toWrite);
								}
								cell->waiting = Cell::Ready;
								readyCells.push_back(cell_coord);
								//cell->mutex.unlock();
							}
							else
							{
#ifdef _DEBUG
								std::stringstream str; str << cell_coord.x << "," << cell_coord.y;
								SendToConsole("Retrying write on cell [" + str.str() + "]");
#endif
								AddHist(cell_coord, "Cell locked (will retry write later)");
								writesToRetry.push_back(toWrite);
							}
						}
					}
				}
				{
					std::tuple<std::weak_ptr<Cell>, CellCoord_t> toRead;
					while (m_ReadQueue.try_pop(toRead))
					{
						std::weak_ptr<Cell>& cellWpt = std::get<0>(toRead);
						const CellCoord_t& cell_coord = std::get<1>(toRead);
						if (auto cell = cellWpt.lock())
						{
							Cell::mutex_t::scoped_try_lock lock(cell->mutex);
							if (lock)
							{
								// Make sure this cell hasn't been re-activated:
								if (cell->active_entries == 0 && cell->waiting == Cell::Retrieve)
								{
									FSN_ASSERT(!cell->loaded);
									try
									{
										// Get the cached cell data (if available)
										auto filePtr = GetCellStreamForReading(cell_coord.x, cell_coord.y);

										// Last param makes the method load synched entities from the map if the cache file isn't available:
										if (m_Map)
										{
											// Load synched entities if this cell is un-cached (hasn't been loaded before)
											const bool uncached = !filePtr;//m_SynchLoaded.insert(std::make_pair(cell_coord.x, cell_coord.y)).second;

											auto data = m_Map->GetRegionData(cell_coord.x, cell_coord.y, uncached);

											if (!data.empty())
											{
												bio::filtering_istream inflateStream;
												inflateStream.push(bio::zlib_decompressor());
												inflateStream.push(bio::array_source(data.data(), data.size()));

												IO::Streams::CellStreamReader reader(&inflateStream);
												// This value is the length of the pseudo-entity data (TODO: read this in GetRegionData and only return the data after this length if uncached is false)
												auto mapDataLength = reader.ReadValue<std::streamsize>();

												if (mapDataLength > 0)
													LoadEntitiesFromCellData(cell_coord, cell.get(), inflateStream, false);
												if (uncached)
													LoadEntitiesFromCellData(cell_coord, cell.get(), inflateStream, true);
											}
										}

										
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
												LoadEntitiesFromCellData(cell_coord, cell.get(), file, false); // In edit-mode unsynched entities are also written to the cache
#ifdef _DEBUG
												// Make sure all the data was read
												// TODO: in Release builds: skip to the end if it wasn't all read?
												//FSN_ASSERT(unsynchedDataLength == file.tellg() - startRead);
#endif
											}
											size_t num = LoadEntitiesFromCellData(cell_coord, cell.get(), file, true);

											//std::stringstream str; str << i;
											//SendToConsole("Cell " + str.str() + " streamed in");

											AddHist(cell_coord, "Loaded", num);
											cell->loaded = true;
										}
										else
										{
											cell->loaded = true; // No data to load
											AddHist(cell_coord, "Loaded (no data)");
										}
									}
									catch (std::exception& e)
									{
										std::string message("Exception while streaming in cell: ");
										message += e.what();
										AddHist(cell_coord, message);
										AddLogEntry(message);
									}
									catch (...)
									{
										AddLogEntry("Unknown exception while streaming in cell");
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
#ifdef _DEBUG
								std::stringstream str; str << cell_coord.x << "," << cell_coord.y;
								SendToConsole("Retrying read on cell [" + str.str() + "]");
#endif
								AddHist(cell_coord, "Cell locked (will retry read later)");
								readsToRetry.push_back(toRead);
							}
						}
					}
				}
				// Re-enqueue blocked writes/reads
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

					std::tuple<ObjectID, UpdateOperation, CellCoord_t, std::vector<unsigned char>, std::vector<unsigned char>> objectUpdateData;
					while (m_ObjectUpdateQueue.try_pop(objectUpdateData))
					{
						const ObjectID id = std::get<0>(objectUpdateData);
						UpdateOperation operation = std::get<1>(objectUpdateData);
						CellCoord_t new_loc = std::get<2>(objectUpdateData);
						auto& incommingConData = std::get<3>(objectUpdateData);
						auto& incommingOccData = std::get<4>(objectUpdateData);

						CellCoord_t loc;
						std::streamoff dataOffset;
						std::streamsize dataLength;
						if (!getEntityLocation(*m_EntityLocationDB, loc, dataOffset, dataLength, id))
							continue; // This entity hasn't been stored (so, why are you trying to move an active entity!?)

						if (new_loc == CellCoord_t(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()))
							new_loc = loc;

						// Skip if nothing has changed
						if (new_loc == loc && incommingConData.empty() && incommingOccData.empty())
							continue;


						auto inSourceData = GetCellStreamForReading(loc.x, loc.y);
						auto outDestData = GetCellStreamForWriting(new_loc.x, new_loc.y);
						// If the data is being moved there are some more streams that need to be prepared:
						std::unique_ptr<std::istream> inDestData;
						std::unique_ptr<std::ostream> outSourceData;
						if (new_loc != loc)
						{
							inDestData = GetCellStreamForReading(new_loc.x, new_loc.y);
							outSourceData = GetCellStreamForWriting(loc.x, loc.y);
						}

						if (!inSourceData || !outDestData)
							continue;

						auto newDataLength = dataLength;

						std::vector<ObjectID> displacedObjects;
						std::vector<ObjectID> displacedObjectsBackward;

						if (operation == UpdateOperation::UPDATE)
						{
							if (!incommingConData.empty() || !incommingOccData.empty())
							{
								RakNet::BitStream iConDataStream(incommingConData.data(), incommingConData.size(), false);
								RakNet::BitStream iOccDataStream(incommingOccData.data(), incommingOccData.size(), false);

								if (new_loc == loc)
								{
									// Note that outDest and inSource are re-used for both sets of in and out streams
									newDataLength = MergeEntityData(displacedObjects, displacedObjectsBackward, id, dataOffset, dataLength, *inSourceData, *outDestData, *inSourceData, *outDestData, iConDataStream, iOccDataStream);
								}
								else
								{
									if (!outSourceData || !inDestData)
										continue;
									newDataLength = MergeEntityData(displacedObjects, displacedObjectsBackward, id, dataOffset, dataLength, *inSourceData, *outSourceData, *inDestData, *outDestData, iConDataStream, iOccDataStream);
								}
							}
							else
							{
								if (!inSourceData || !outDestData || !outSourceData || !inDestData)
									continue;

								MoveEntityData(displacedObjectsBackward, id, dataOffset, dataLength, *inSourceData, *outSourceData, *inDestData, *outDestData);
							}

							storeEntityLocation(*m_EntityLocationDB, id, new_loc, dataOffset, dataLength);
						}
						else if (operation == UpdateOperation::REMOVE)
						{
							DeleteEntityData(displacedObjectsBackward, id, dataOffset, dataLength, *inSourceData, *outDestData);

							m_EntityLocationDB->remove((const char*)&id, sizeof(id));
						}

						// Update the offsets of entities affected by data getting longer / shorter
						if (newDataLength != dataLength)
						{
							auto lengthDiff = newDataLength - dataLength;
							for (auto it = displacedObjects.begin(), end = displacedObjects.end(); it != end; ++it)
							{
								ObjectID displaced_id = *it;

								CellCoord_t displaced_loc;
								std::streamoff displaced_dataOffset;
								std::streamsize displaced_dataLength;
								getEntityLocation(*m_EntityLocationDB, displaced_loc, displaced_dataOffset, displaced_dataLength, displaced_id);

								storeEntityLocation(*m_EntityLocationDB, displaced_id, displaced_loc, displaced_dataOffset + lengthDiff, displaced_dataLength);
							}
						}
						// Update the offsets of entities affected by data getting removed
						for (auto it = displacedObjectsBackward.begin(), end = displacedObjectsBackward.end(); it != end; ++it)
						{
							ObjectID displaced_id = *it;

							CellCoord_t displaced_loc;
							std::streamoff displaced_dataOffset;
							std::streamsize displaced_dataLength;
							getEntityLocation(*m_EntityLocationDB, displaced_loc, displaced_dataOffset, displaced_dataLength, displaced_id);

							storeEntityLocation(*m_EntityLocationDB, displaced_id, displaced_loc, displaced_dataOffset - dataLength, displaced_dataLength);
						}

					}
				}

				if (eventId == 0) // Quit
				{
					ClearReadyCells(readyCells);
					break;
				}
			}
		}

		asThreadCleanup();
	}

	void RegionMapLoader::ClearReadyCells(std::list<CellCoord_t>& readyCells)
	{
		// Drop references to cells that are done processing (if m_CellsBeingProcessed isn't locked by a current transaction)
		if (!readyCells.empty())
		{
			{
				TransactionMutex_t::scoped_lock lock(m_TransactionMutex);
				if (lock)
				{
					for (auto it = readyCells.begin(); it != readyCells.end(); ++it)
						m_CellsBeingProcessed.erase(*it);
				}
			}
			readyCells.clear();
		}
	}

	void RegionMapLoader::PerformSave(const std::string& saveName)
	{
		namespace bfs = boost::filesystem;

		// TODO: replace all SendToConsole calls with signals
		try
		{
			SendToConsole("Quick-Saving...");

			auto savePath = boost::filesystem::path(PHYSFS_getWriteDir()) / m_SavePath / saveName;

			SendToConsole("QS Path: " + savePath.string());

			AddLogEntry("Saving: " + savePath.string(), LOG_INFO);

			auto physFsPath = m_SavePath + saveName;
			if (!bfs::is_directory(savePath))
			{
				if (PHYSFS_mkdir(physFsPath.c_str()) == 0)
				{
					FSN_EXCEPT(FileSystemException, "Failed to create save path (" + physFsPath + "): " + std::string(PHYSFS_getLastError()));
				}
			}
			else
			{
				auto fileList = PHYSFS_enumerateFiles(physFsPath.c_str());
				for (auto it = fileList; *it; ++it)
				{
					auto filePath = physFsPath + "/" + *it;
					if (PHYSFS_delete(filePath.c_str()) == 0)
					{
						PHYSFS_freeList(fileList);
						FSN_EXCEPT(FileSystemException, "Failed to delete file while clearing save path (" + filePath + "): " + std::string(PHYSFS_getLastError()));
					}
				}
				PHYSFS_freeList(fileList);
			}
			FSN_ASSERT(bfs::is_directory(savePath));
			FSN_ASSERT(bfs::is_empty(savePath));

			// Flush the cache
			// TODO: flush without unloading? (Cache->FlushFiles())
			m_Cache->DropCache();

			std::vector<bfs::path> regionFiles;
			std::vector<bfs::path> dataFiles;
			for(bfs::directory_iterator it(m_FullBasePath); it != bfs::directory_iterator(); ++it)
			{
				if (it->path().extension() == ".celldata")
					regionFiles.push_back(it->path());
				if (it->path().extension() == ".dat")
					dataFiles.push_back(it->path());
			}

			std::stringstream numStr; numStr << regionFiles.size();
			std::string numRegionFilesStr = numStr.str();
			numStr.str(""); numStr << dataFiles.size();
			std::string numDataFilesStr = numStr.str();

			unsigned int i = 0;
			for (auto it = regionFiles.begin(); it != regionFiles.end(); ++it)
			{
				std::stringstream str; str << ++i;
				SendToConsole("QS Progress: copying " + str.str() + " / " + numRegionFilesStr + " regions");

				auto dest = savePath; dest /= it->filename();
				boost::filesystem::copy_file(*it, dest, boost::filesystem::copy_option::overwrite_if_exists);
			}
			i = 0;
			for (auto it = dataFiles.begin(); it != dataFiles.end(); ++it)
			{
				std::stringstream str; str << ++i;
				SendToConsole("QS Progress: copying " + str.str() + " / " + numDataFilesStr + " data files");

				auto dest = savePath; dest /= it->filename();
				boost::filesystem::copy_file(*it, dest, boost::filesystem::copy_option::overwrite_if_exists);
			}
			SendToConsole("QS Progress: copying entitylocations.kc");
			SaveEntityLocationDB((savePath /= "entitylocations.kc").string());
			SendToConsole("QS Progress: done copying entitylocations.kc");

			SendToConsole("Done Quick-Saving.");
		}
		catch (bfs::filesystem_error& e)
		{
			SendToConsole("QS Failed");

			AddLogEntry(std::string("Quick-Save failed: ") + e.what(), LOG_CRITICAL);
		}
		catch (FileSystemException& e)
		{
			SendToConsole("QS Failed");

			AddLogEntry(std::string("Quick-Save failed: ") + e.what(), LOG_CRITICAL);
		}
	}

	void getArchiveFileList(std::vector<boost::filesystem::path>& results, const boost::filesystem::path& path, const std::string& ext)
	{
		namespace bfs = boost::filesystem;

		auto fileList = PHYSFS_enumerateFiles(path.generic_string().c_str());
		for (auto it = fileList; *it; ++it)
		{
			auto filePath = path / (*it);
			if (filePath.extension() == ext)
				results.push_back(filePath);
		}
		PHYSFS_freeList(fileList);
	}

	void getNativeFileList(std::vector<boost::filesystem::path>& results, const boost::filesystem::path& path, const std::string& ext)
	{
		namespace bfs = boost::filesystem;

		for(bfs::directory_iterator it(path); it != bfs::directory_iterator(); ++it)
		{
			if (it->path().extension() == ext)
				results.push_back(it->path());
		}
	}

	void copyArchivedFiles(std::vector<boost::filesystem::path>& files, const boost::filesystem::path& target_path)
	{
		namespace bfs = boost::filesystem;

		std::stringstream numStr; numStr << files.size();
		std::string numDataFilesStr = numStr.str();
		unsigned int i = 0;
		for (auto it = files.begin(); it != files.end(); ++it)
		{
			std::stringstream str; str << ++i;
			SendToConsole("Load Progress: copying " + str.str() + " / " + numDataFilesStr + " data files");

			auto dest = target_path / it->filename();
			PhysFSHelp::copy_file(it->generic_string(), dest.generic_string());
		}
	}

	void copyNativeFiles(std::vector<boost::filesystem::path>& files, const boost::filesystem::path& target_path, const std::function<void (size_t, size_t)>& callback)
	{
		namespace bfs = boost::filesystem;

		std::stringstream numStr; numStr << files.size();
		std::string numDataFilesStr = numStr.str();
		unsigned int i = 0;
		for (auto it = files.begin(); it != files.end(); ++it)
		{
			std::stringstream str; str << ++i;
			SendToConsole("Load Progress: copying " + str.str() + " / " + numDataFilesStr + " data files");
			if (callback)
				callback(i, files.size());

			auto dest = target_path / it->filename();
			boost::filesystem::copy_file(*it, dest, bfs::copy_option::overwrite_if_exists);
		}
	}

	void RegionMapLoader::PrepareLoad(const std::string& saveName)
	{
		//namespace bfs = boost::filesystem;
		//using namespace IO;

		//FSN_ASSERT(boost::this_thread::get_id() != m_Thread.get_id());

		//{
		//	boost::mutex::scoped_lock lock(m_SaveToLoadMutex);
		//	m_SaveToLoad = saveName;
		//}

		//auto physSavePath = bfs::path(m_SavePath) / saveName;
		//auto savePath = bfs::path(PHYSFS_getWriteDir()) / physSavePath;

		//auto archivePath = physSavePath;
		//if (!archivePath.has_extension())
		//	archivePath.replace_extension(".zip");

		//// Try to mount a save archive with the given name
		//bool archived = PHYSFS_exists(archivePath.generic_string().c_str()) && PHYSFS_mount(archivePath.generic_string().c_str(), physSavePath.string().c_str(), 1);

		////if (archived)
		////{
		////	struct testArchive
		////	{
		////		unzFile file;
		////		testArchive(const std::string& filename)
		////		{
		////			file = unzOpen(filename.c_str());
		////		}
		////		~testArchive()
		////		{
		////			if (file)
		////				unzClose(file);
		////		}
		////		std::string getComment()
		////		{
		////			std::string comment(256, ' ');
		////			auto actualSize = unzGetGlobalComment(file, comment.data(), comment.size());
		////			comment.resize(actualSize);
		////			return std::move(comment);
		////		}
		////	};
		////	auto absoluteArchivePath = bfs::path(PHYSFS_getWriteDir()) / archivePath;
		////	if (testArchive(absoluteArchivePath.string()).getComment() != ":)")
		////	{
		////		AddLogEntry("Save archive may not have been written correctly (doesn't have expected comment");
		////	}
		////}

		//if (!archived && (!bfs::is_directory(savePath) || bfs::is_empty(savePath)))
		//{
		//	SendToConsole("Load Failed: save doesn't exist");
		//	AddLogEntry("Failed to load save: either " + savePath.string() + " doesn't exist or there was an error"
		//		"reading the archive. PhysFS last error (may not be applicable): " + std::string(PHYSFS_getLastError()));
		//	return;
		//}

		//const bool threadRunning = m_Thread.joinable();
		//if (threadRunning)
		//	Stop();

		//// Just to be safe
		//m_CellsBeingProcessed.clear();
		//m_ReadQueue.clear();
		//m_WriteQueue.clear();

		//// Everything needs to reload
		//m_SynchLoaded.clear();
		//// Drop file handles (files need to be deletable below)
		//m_Cache->DropCache();

		//// Unload the location DB
		//std::string cacheDbPath = m_EntityLocationDB->path();
		//if (cacheDbPath.empty())
		//	cacheDbPath = m_FullBasePath + "entitylocations.kc";
		//m_EntityLocationDB.reset();

		//// Delete the cache files
		//try
		//{
		//	if (!bfs::is_empty(m_FullBasePath))
		//	{
		//		auto fileList = PHYSFS_enumerateFiles(m_CachePath.c_str());
		//		for (auto it = fileList; *it; ++it)
		//		{
		//			auto filePath = m_CachePath + "/" + *it;
		//			if (PHYSFS_delete(filePath.c_str()) == 0)
		//			{
		//				PHYSFS_freeList(fileList);
		//				FSN_EXCEPT(FileSystemException, "Failed to delete file while clearing cache (" + filePath + "): " + std::string(PHYSFS_getLastError()));
		//			}
		//		}
		//		PHYSFS_freeList(fileList);
		//	}

		//	// Copy the saved location DB into the cache
		//	SendToConsole("Loading: copying entitylocations.kc");
		//	{
		//		auto saveDbPath = savePath;
		//		saveDbPath /= "entitylocations.kc";

		//		if (archived)
		//			PhysFSHelp::copy_file(saveDbPath.generic_string().c_str(), cacheDbPath.c_str());
		//		else
		//			boost::filesystem::copy_file(saveDbPath, cacheDbPath, bfs::copy_option::overwrite_if_exists);
		//	}
		//	SendToConsole("Loading: done copying entitylocations.kc");

		//	// Copy saved custom-data files
		//	std::vector<bfs::path> dataFiles;
		//	if (archived)
		//		getArchiveFileList(dataFiles, m_CachePath, ".dat");
		//	else
		//		getNativeFileList(dataFiles, m_FullBasePath, ".dat");

		//	if (archived)
		//		copyArchivedFiles(dataFiles, m_CachePath);
		//	else
		//		copyNativeFiles(dataFiles, m_FullBasePath);
		//}
		//catch (FileSystemException& e)
		//{
		//	AddLogEntry(e.what());
		//}
		//catch (bfs::filesystem_error& e)
		//{
		//	AddLogEntry(e.what());
		//}
		//// Reload the location DB
		//m_EntityLocationDB.reset(new kyotocabinet::HashDB);
		//setupTuning(m_EntityLocationDB.get());
		//m_EntityLocationDB->open(cacheDbPath, kyotocabinet::HashDB::OWRITER);

		//if (threadRunning)
		//	Start();
	}

	std::string make_relative(const std::string& base, const std::string& path)
	{
		namespace bfs = boost::filesystem;

		bfs::path relativePath;

		bfs::path basePath(base);
		bfs::path fullPath(path);
		basePath.make_preferred();
		fullPath.make_preferred();
		auto fit = fullPath.begin(), fend = fullPath.end();
		for (auto it = basePath.begin(), end = basePath.end(); it != end && fit != fend; ++it, ++fit)
		{
			if (*it != *fit)
				break;
		}
		for (; fit != fend; ++fit)
		{
			relativePath /= fit->string();
		}

		return relativePath.generic_string();
	}

	void RegionMapLoader::PerformLoad(const std::string& saveName)
	{
		namespace bfs = boost::filesystem;

		// TODO: replace all SendToConsole calls with signals
		{
			boost::mutex::scoped_lock lock(m_SaveToLoadMutex);
			//saveName = m_SaveToLoad;
			m_SaveToLoad.clear();
		}

		namespace bfs = boost::filesystem;
		using namespace IO;

		FSN_ASSERT(boost::this_thread::get_id() != m_Thread.get_id());

		{
			boost::mutex::scoped_lock lock(m_SaveToLoadMutex);
			m_SaveToLoad = saveName;
		}

		auto physSavePath = bfs::path("/" + m_SavePath) / saveName;
		auto savePath = bfs::path(PHYSFS_getWriteDir()) / physSavePath;

		auto archivePath = PHYSFS_getWriteDir() / physSavePath;
		if (!archivePath.has_extension())
			archivePath.replace_extension(".zip");
		auto archiveMountpoint = physSavePath.parent_path();
		archivePath.make_preferred();

		// Try to mount a save archive with the given name
		bool archived = bfs::exists(archivePath) && PHYSFS_mount(archivePath.string().c_str(), archiveMountpoint.generic_string().c_str(), 1);

		if (!archived)
			SendToConsole("Load Path: " + savePath.string());
		else
			SendToConsole("Load Path: " + physSavePath.string() + ", in archive: " + archivePath.string());

		//if (archived)
		//{
		//	struct testArchive
		//	{
		//		unzFile file;
		//		testArchive(const std::string& filename)
		//		{
		//			file = unzOpen(filename.c_str());
		//		}
		//		~testArchive()
		//		{
		//			if (file)
		//				unzClose(file);
		//		}
		//		std::string getComment()
		//		{
		//			std::string comment(256, ' ');
		//			auto actualSize = unzGetGlobalComment(file, comment.data(), comment.size());
		//			comment.resize(actualSize);
		//			return std::move(comment);
		//		}
		//	};
		//	auto absoluteArchivePath = bfs::path(PHYSFS_getWriteDir()) / archivePath;
		//	if (testArchive(absoluteArchivePath.string()).getComment() != ":)")
		//	{
		//		AddLogEntry("Save archive may not have been written correctly (doesn't have expected comment");
		//	}
		//}

		if (!archived && (!bfs::is_directory(savePath) || bfs::is_empty(savePath)))
		{
			SendToConsole("Load Failed: save doesn't exist");
			AddLogEntry("Failed to load save: either " + savePath.string() + " doesn't exist or there was an error"
				"reading the archive. PhysFS last error (may not be applicable): " + std::string(PHYSFS_getLastError()));
			return;
		}

		const bool threadRunning = m_Thread.joinable();
		if (threadRunning)
			Stop();

		// Just to be safe
		m_CellsBeingProcessed.clear();
		m_ReadQueue.clear();
		m_WriteQueue.clear();

		// Everything needs to reload
		m_SynchLoaded.clear();
		// Drop file handles (files need to be deletable below)
		m_Cache->DropCache();

		// Unload the location DB
		std::string cacheDbPath = m_EntityLocationDB->path();
		if (cacheDbPath.empty())
			cacheDbPath = m_FullBasePath + "entitylocations.kc";
		m_EntityLocationDB.reset();

		// Delete the cache files
		try
		{
			if (!bfs::is_empty(m_FullBasePath))
			{
				auto fileList = PHYSFS_enumerateFiles(m_CachePath.c_str());
				for (auto it = fileList; *it; ++it)
				{
					auto filePath = m_CachePath + "/" + *it;
					if (PHYSFS_delete(filePath.c_str()) == 0)
					{
						PHYSFS_freeList(fileList);
						FSN_EXCEPT(FileSystemException, "Failed to delete file while clearing cache (" + filePath + "): " + std::string(PHYSFS_getLastError()));
					}
				}
				PHYSFS_freeList(fileList);
			}

			// Copy the saved location DB into the cache
			SendToConsole("Loading: copying entitylocations.kc");
			{
				if (archived)
				{
					auto saveDbPath = physSavePath / "entitylocations.kc";
					auto physFsTargetPath = make_relative(PHYSFS_getWriteDir(), cacheDbPath);
					PhysFSHelp::copy_file(saveDbPath.generic_string(), physFsTargetPath);
				}
				else
				{
					auto saveDbPath = savePath / "entitylocations.kc";
					boost::filesystem::copy_file(saveDbPath, cacheDbPath, bfs::copy_option::overwrite_if_exists);
				}
			}
			SendToConsole("Loading: done copying entitylocations.kc");

			// Copy saved custom-data files
			std::vector<bfs::path> dataFiles;
			if (archived)
			{
				getArchiveFileList(dataFiles, physSavePath, ".dat");
				copyArchivedFiles(dataFiles, m_CachePath);
			}
			else
			{
				getNativeFileList(dataFiles, savePath, ".dat");
				copyNativeFiles(dataFiles, m_FullBasePath, [](size_t, size_t)
				{
				});
			}

			// Reload the location DB
			m_EntityLocationDB.reset(new kyotocabinet::HashDB);
			setupTuning(m_EntityLocationDB.get());
			m_EntityLocationDB->open(cacheDbPath, kyotocabinet::HashDB::OWRITER);

			std::vector<bfs::path> regionFiles;
			if (archived)
			{
				getArchiveFileList(regionFiles, physSavePath, ".celldata");
				copyArchivedFiles(regionFiles, m_CachePath);
			}
			else
			{
				getNativeFileList(regionFiles, savePath, ".celldata");
				copyNativeFiles(regionFiles, m_FullBasePath, [](size_t, size_t)
				{
					//SendToConsole("Load Progress: copying " + str.str() + " / " + numDataFilesStr + " region files");
				});
			}

			SendToConsole("Done Loading.");
		}
		catch (bfs::filesystem_error& e)
		{
			SendToConsole("Load Failed");

			AddLogEntry(std::string("Load failed: ") + e.what(), LOG_CRITICAL);
		}
		catch (FileSystemException& e)
		{
			SendToConsole("Load Failed");

			AddLogEntry(std::string("Load failed: ") + e.what(), LOG_CRITICAL);
		}

		if (archived)
		{
			int r = PHYSFS_removeFromSearchPath(archivePath.string().c_str());
			FSN_ASSERT(r != 0);
		}

		if (threadRunning)
			Start();
	}

	void RegionMapLoader::CompressSave(const std::string& saveName)
	{
		namespace bfs = boost::filesystem;

		// TODO: replace all SendToConsole calls with signals
		try
		{
			SendToConsole("Compressing save...");

			auto savePath = boost::filesystem::path(PHYSFS_getWriteDir()) / m_SavePath / saveName;

			SendToConsole("Original path: " + savePath.string());

			auto physFsPath = m_SavePath + saveName;
			if (!bfs::is_directory(savePath))
			{
				FSN_EXCEPT(FileSystemException, "Save path (" + physFsPath + ") not found.");
			}

			auto archiveFilePath = savePath;
			archiveFilePath.replace_extension(".zip");

			AddLogEntry("Creating compressed save: " + savePath.string() + " -> " + archiveFilePath.string(), LOG_INFO);

			{
				IO::FileType::ZipArchive archive(archiveFilePath);
				archive.AddPath(savePath, savePath.stem());
			}

			//std::vector<bfs::path> regionFiles;
			//std::vector<bfs::path> dataFiles;
			//for(bfs::directory_iterator it(m_FullBasePath); it != bfs::directory_iterator(); ++it)
			//{
			//	if (it->path().extension() == ".celldata")
			//		regionFiles.push_back(it->path());
			//	if (it->path().extension() == ".dat")
			//		dataFiles.push_back(it->path());
			//}

			//std::stringstream numStr; numStr << regionFiles.size();
			//std::string numRegionFilesStr = numStr.str();
			//numStr.str(""); numStr << dataFiles.size();
			//std::string numDataFilesStr = numStr.str();

			//unsigned int i = 0;
			//for (auto it = regionFiles.begin(); it != regionFiles.end(); ++it)
			//{
			//	std::stringstream str; str << ++i;
			//	SendToConsole("QS Progress: copying " + str.str() + " / " + numRegionFilesStr + " regions");

			//	auto dest = savePath; dest /= it->filename();
			//	boost::filesystem::copy_file(*it, dest, boost::filesystem::copy_option::overwrite_if_exists);
			//}
			//i = 0;
			//for (auto it = dataFiles.begin(); it != dataFiles.end(); ++it)
			//{
			//	std::stringstream str; str << ++i;
			//	SendToConsole("QS Progress: copying " + str.str() + " / " + numDataFilesStr + " data files");

			//	auto dest = savePath; dest /= it->filename();
			//	boost::filesystem::copy_file(*it, dest, boost::filesystem::copy_option::overwrite_if_exists);
			//}
			//SendToConsole("QS Progress: copying entitylocations.kc");
			//SaveEntityLocationDB((savePath /= "entitylocations.kc").string());
			//SendToConsole("QS Progress: done copying entitylocations.kc");

			{
				auto fileList = PHYSFS_enumerateFiles(physFsPath.c_str());
				for (auto it = fileList; *it; ++it)
				{
					auto filePath = physFsPath + "/" + *it;
					if (PHYSFS_delete(filePath.c_str()) == 0)
					{
						PHYSFS_freeList(fileList);
						FSN_EXCEPT(FileSystemException, "Failed to delete file while clearing save path (" + filePath + "): " + std::string(PHYSFS_getLastError()));
					}
				}
				PHYSFS_freeList(fileList);
			}
			PHYSFS_delete(physFsPath.c_str());

			SendToConsole("Done compressing.");
		}
		catch (bfs::filesystem_error& e)
		{
			SendToConsole("Save compression failed");

			AddLogEntry(std::string("Failed to compress save: ") + e.what(), LOG_CRITICAL);
		}
		catch (FileSystemException& e)
		{
			SendToConsole("Save compression failed");

			AddLogEntry(std::string("Failed to compress save: ") + e.what(), LOG_CRITICAL);
		}
	}

	typedef std::array<char, 4096> CopyBuffer_t;

	static inline void CopyData(CopyBuffer_t& buffer, ICellStream& source, OCellStream& dest)
	{
		while (!source.eof())
		{
			source.read(buffer.data(), std::streamsize(buffer.size()));
			const auto gcount = source.gcount();
			dest.write(buffer.data(), gcount);
		}
	}

	static inline void CopyData(CopyBuffer_t& buffer, ICellStream& source, OCellStream& dest, std::streamsize remaining_length)
	{
		while (remaining_length > 0 && !source.eof())
		{
			source.read(buffer.data(), std::min(std::streamsize(buffer.size()), remaining_length));
			const auto gcount = source.gcount();
			dest.write(buffer.data(), gcount);

			FSN_ASSERT(gcount <= remaining_length);
			remaining_length -= gcount;
		}
	}

	static inline void SkipData(CopyBuffer_t& buffer, ICellStream& source, std::streamsize remaining_length)
	{
		while (remaining_length > 0 && !source.eof())
		{
			source.read(buffer.data(), std::min(std::streamsize(buffer.size()), remaining_length));
			const auto gcount = source.gcount();

			FSN_ASSERT(gcount <= remaining_length);
			remaining_length -= gcount;
		}
	}

	static inline size_t RemoveID(std::vector<ObjectID>& objects_displaced, ObjectID id_to_remove, ICellStream& source_in, OCellStream& source_out)
	{
		IO::Streams::CellStreamReader src_reader(&source_in);
		IO::Streams::CellStreamWriter src_writer(&source_out);

		// Read the ID list from the source and figure out what entity data will be offset by deleting this data
		size_t numEntsInSource = src_reader.ReadValue<size_t>();
		bool encounteredId = false;
		for (size_t i = 0; i < numEntsInSource; ++i)
		{
			const ObjectID iID = src_reader.ReadValue<ObjectID>();
			if (iID != id_to_remove)
			{
				src_writer.Write(iID);
				if (encounteredId)
					objects_displaced.push_back(iID);
			}
			else
			{
				FSN_ASSERT(!encounteredId); // make sure the IDs aren't repeated (indicates a bug somewhere)
				encounteredId = true;
			}
		}
		return numEntsInSource;
	}

	static inline void CopyIDList(std::vector<ObjectID>& objects_displaced, size_t numEnts, ObjectID id_causing_displacement, ICellStream& source_in, OCellStream& source_out)
	{
		IO::Streams::CellStreamReader src_reader(&source_in);
		IO::Streams::CellStreamWriter src_writer(&source_out);

		// Read the ID list from the source and figure out what entity data will be offset by modifying this data
		bool encounteredId = false;
		for (size_t i = 0; i < numEnts; ++i)
		{
			const ObjectID iID = src_reader.ReadValue<ObjectID>();
			src_writer.Write(iID);
			if (encounteredId)
				objects_displaced.push_back(iID);
			if (iID == id_causing_displacement)
			{
				FSN_ASSERT(!encounteredId); // make sure the IDs aren't repeated (indicates a bug somewhere)
				encounteredId = true;
			}
		}
	}

	std::streamsize RegionMapLoader::MergeEntityData(std::vector<ObjectID>& objects_displaced_for, std::vector<ObjectID>& objects_displaced_back, ObjectID id, std::streamoff data_offset, std::streamsize data_length, ICellStream& source_in, OCellStream& source_out, ICellStream& dest_in, OCellStream& out, RakNet::BitStream& mergeCon, RakNet::BitStream& mergeOcc) const
	{
		CopyBuffer_t buffer;

		const bool destChanged = &source_in != &dest_in;

		IO::Streams::CellStreamReader src_reader(&source_in);
		IO::Streams::CellStreamReader dst_reader(&dest_in);
		IO::Streams::CellStreamWriter writer(&out);

		// Skip the un-synced entity data that is present in Edit Mode
		if (m_EditMode)
		{
			src_reader.ReadValue<std::streamsize>();
			std::streamsize unsynchedDataLength = dst_reader.ReadValue<std::streamsize>();

			FSN_ASSERT(unsynchedDataLength >= 0);
			std::streamsize remainingData = unsynchedDataLength;

			CopyData(buffer, dest_in, out, remainingData);
		}

		size_t numEnts = dst_reader.ReadValue<size_t>();
		if (destChanged)
		{
			numEnts += 1; // Since an entry is being added

			// Remove the ID being moved from the source header
			RemoveID(objects_displaced_back, id, source_in, source_out);
		}
		writer.Write(numEnts);

		const size_t headerLength = numEnts * sizeof(ObjectID);

		// Copy the existing object IDs list in the dest
		if (destChanged)
		{
			std::vector<char> headerData((numEnts - 1) * sizeof(ObjectID));
			dest_in.read(headerData.data(), headerData.size());
			out.write(headerData.data(), headerData.size());

			// The the ID for the entity being merged in
			writer.Write(id);
		}
		else
		{
			// Copy the existing ID list and note which objects come after the one being
			//  updated, and thus will be displaced if the length changes
			CopyIDList(objects_displaced_for, numEnts, id, dest_in, out);
		}


		if (!destChanged)
		{
			auto remainingData = data_offset - (sizeof(size_t) + headerLength); // The offset is relative to the start of the cell data
			// Copy existing cell data to the relevant entity data
			CopyData(buffer, dest_in, out, remainingData);
		}
		else
		{
			auto remainingData = data_offset - (sizeof(size_t) + headerLength); // The offset is relative to the start of the cell data
			// Re-write the source up to the relevant entity data
			CopyData(buffer, source_in, source_out, remainingData);

			// Re-write rest of the existing dest cell data (changed data will be written at the end)
			CopyData(buffer, dest_in, out);
		}

		// Merge the data
		std::streamsize newEntityDataLength;
		{
			CharCounter counter;
			bio::filtering_ostream countingOut;
			countingOut.push(counter);
			countingOut.push(out, 0);
			/*auto newEntityDataLength = */EntitySerialisationUtils::MergeEntityData(source_in, countingOut, mergeCon, mergeOcc);
			countingOut.flush();
			newEntityDataLength = counter.count();
		}

		if (destChanged)
		{
			// Re-write the rest of the source (sans the moved entity)
			CopyData(buffer, source_in, source_out);
		}
		else
		{
			// Re-write the rest of the un-changed cell data
			CopyData(buffer, dest_in, out);
		}

		return newEntityDataLength;
	}

	void RegionMapLoader::MoveEntityData(std::vector<ObjectID>& objects_displaced, ObjectID id, std::streamoff data_offset, std::streamsize data_length, ICellStream& source_in, OCellStream& source_out, ICellStream& dest_in, OCellStream& dest) const
	{
		// Not implemented yet (need to copy pseudo data like above):
		FSN_ASSERT(!m_EditMode);
		// Make sure the data is actually being moved!
		FSN_ASSERT(&source_in != &dest_in);

		CopyBuffer_t buffer;

		IO::Streams::CellStreamReader src_reader(&source_in);
		IO::Streams::CellStreamWriter src_writer(&source_out);

		IO::Streams::CellStreamReader dst_reader(&dest_in);
		IO::Streams::CellStreamWriter dst_writer(&dest);

		size_t numEntsInSource;// = src_reader.ReadValue<size_t>();
		numEntsInSource = RemoveID(objects_displaced, id, source_in, source_out);

		const size_t sourceHeaderLength = numEntsInSource * sizeof(ObjectID);

		{
			auto remainingData = data_offset - sourceHeaderLength;
			// Copy the source up to the relevant entity data
			CopyData(buffer, source_in, source_out, remainingData);
		}

		if (source_in.eof())
			FSN_EXCEPT(FileSystemException, "Failed to move entity data: source was missing data / invalid");

		size_t numEnts = dst_reader.ReadValue<size_t>();
		++numEnts;
		dst_writer.Write(numEnts);

		// Copy the dest's existing ID list
		std::vector<char> headerData((numEnts - 1) * sizeof(ObjectID));
		dest_in.read(headerData.data(), headerData.size());
		dest.write(headerData.data(), headerData.size());
		// Write the new ID
		dst_writer.Write(id);

		// Copy the rest of the existing dest data
		CopyData(buffer, dest_in, dest);

		// Copy entity data from the source into the dest (note that not copying this back into source_out effectively deletes it from there)
		CopyData(buffer, source_in, dest, data_length);

		// Copy the remaining source data back into the source
		CopyData(buffer, source_in, source_out);
	}

	void RegionMapLoader::DeleteEntityData(std::vector<ObjectID>& objects_displaced, ObjectID id, std::streamoff data_offset, std::streamsize data_length, ICellStream& source_in, OCellStream& source_out) const
	{
		// Not implemented yet (need to copy pseudo data like above):
		FSN_ASSERT(!m_EditMode);

		CopyBuffer_t buffer;

		IO::Streams::CellStreamReader src_reader(&source_in);
		IO::Streams::CellStreamWriter src_writer(&source_out);

		// Read the ID list from the source and figure out what entity data will be offset by deleting this data
		auto numEnts = RemoveID(objects_displaced, id, source_in, source_out);

		auto headerLength = sizeof(numEnts) + numEnts * sizeof(ObjectID);

		// Copy the data for the entities before the one being removed
		CopyData(buffer, source_in, source_out, data_offset - headerLength);
		// Skip the removed entity
		SkipData(buffer, source_in, data_length);
		// Copy the rest of the data until EOF
		CopyData(buffer, source_in, source_out);
	}

}
