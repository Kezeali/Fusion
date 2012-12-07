/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#include "FusionActiveEntityDirectory.h"
#include "FusionAnyFS.h"
#include "FusionArchetypeFactory.h"
#include "FusionBinaryStream.h"
#include "FusionGameMapLoader.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionEntityInstantiator.h"
#include "FusionVirtualFileSource_PhysFS.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIOStream.h"
#include "FusionLogger.h"
#include "FusionZipArchive.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

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

	static void storeEntityLocation(kyotocabinet::HashDB& db, ObjectID id, RegionCellArchivist::CellCoord_t new_loc, std::streamoff offset, std::streamsize length)
	{
		std::array<char, sizeof(new_loc) + sizeof(offset) + sizeof(length)> data;

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

	static bool getEntityLocation(kyotocabinet::HashDB& db, RegionCellArchivist::CellCoord_t& cell_loc, std::streamoff& data_offset, std::streamsize& data_length, ObjectID id)
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
		{
			switch (db.error().code())
			{
			case kyotocabinet::BasicDB::Error::NOREC:
				AddLogEntry(db.error().message(), LOG_INFO); // Missing record is an expected error, since this method is called for every retrieval
				break;
			default:
				AddLogEntry(db.error().message(), LOG_NORMAL);
				break;
			}

			return false;
		}
	}

	inline void setupTuning(kyotocabinet::HashDB* db)
	{
		db->tune_defrag(8);
		db->tune_map(2LL << 20); // 2MB memory-map
	}

	RegionCellArchivist::RegionCellArchivist(bool edit_mode, const std::string& cache_path)
		: m_EditMode(edit_mode),
		m_Running(false),
		m_RegionSize(s_DefaultRegionSize),
		m_SavePath(s_SavePath),
		m_CachePath(cache_path),
		m_ActiveEntityDirectory(new ActiveEntityDirectory()),
		m_Instantiator(nullptr),
		m_Factory(nullptr),
		m_EntityManager(nullptr),
		m_ArchetypeFactory(nullptr),
		m_NewData(false),
		m_TransactionEnded(false)
	{
		m_FullBasePath = PHYSFS_getWriteDir();
		m_FullBasePath += m_CachePath + "/";

		std::for_each(m_FullBasePath.begin(), m_FullBasePath.end(), [](std::string::value_type& c) { if (c == '\\') c = '/'; });
		
		if (!m_EditMode)
			m_Cache = new RegionCellCache(m_FullBasePath, 16);
		else // edit mode
		{
			// These cells will be used as the static map file files (after defragmentation)
			m_Cache = new RegionCellCache(m_FullBasePath, 24);

			m_EntityLocationDB.reset(new kyotocabinet::HashDB);
			m_EntityLocationDB->tune_options(kyotocabinet::HashDB::TSMALL);
			setupTuning(m_EntityLocationDB.get());
			m_EntityLocationDB->open(m_FullBasePath + "entitylocations.kc", kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OCREATE);

			m_Cache->SetupEditMode(true);

			// Generate the path and cache object for the editable data cache (used to store data from entities serialised in "Editable"
			//  mode - which includes metadata that allows them to be deserialised even if scripts / class definitions change)
			std::string editorCachePath_physfs = m_CachePath + "/editable";
			if (PHYSFS_mkdir(editorCachePath_physfs.c_str()) == 0)
			{
				FSN_EXCEPT(FileSystemException, "Failed to create path (" + editorCachePath_physfs + "): " + std::string(PHYSFS_getLastError()));
			}
			auto fullEditorCachePath = PHYSFS_getWriteDir() + editorCachePath_physfs + "/";

			m_EditableCache = new RegionCellCache(fullEditorCachePath);
		}
	}

	RegionCellArchivist::~RegionCellArchivist()
	{
		Stop();

		if (!m_EntityLocationDB->close())
			AddLogEntry(m_EntityLocationDB->error().message());

		delete m_Cache;
		if (m_EditableCache)
			delete m_EditableCache;
	}

	void RegionCellArchivist::SetInstantiator(EntityInstantiator* instantiator, ComponentFactory* component_factory, EntityManager* manager, ArchetypeFactory* arc_factory)
	{
		m_Instantiator = instantiator;
		m_Factory = component_factory;
		m_EntityManager = manager;
		m_ArchetypeFactory = arc_factory;
	}

	void RegionCellArchivist::SetMap(const std::shared_ptr<GameMap>& map)
	{
		m_Map = map;

		PhysFSHelp::copy_file(m_Map->GetName() + ".endb", m_CachePath + "/entitylocations.kc");
		if (m_EntityLocationDB)
		{
			if (!m_EntityLocationDB->close())
				AddLogEntry(m_EntityLocationDB->error().message());
		}
		m_EntityLocationDB.reset(new kyotocabinet::HashDB);
		setupTuning(m_EntityLocationDB.get());
		m_EntityLocationDB->open(m_FullBasePath + "entitylocations.kc", kyotocabinet::HashDB::OWRITER);

		m_Cache->SetupEditMode(m_EditMode, m_Map->GetBounds());
	}

	void RegionCellArchivist::Update(ObjectID id, const RegionCellArchivist::CellCoord_t& new_location, std::vector<unsigned char>&& continuous, std::vector<unsigned char>&& occasional)
	{
		m_ObjectUpdateQueue.push(std::make_shared<UpdateJob>(id, UpdateOperation::UPDATE, new_location, std::move(continuous), std::move(occasional)));
		m_NewData.set();
	}

	void RegionCellArchivist::Update(ObjectID id, int32_t new_x, int32_t new_y)
	{
		m_ObjectUpdateQueue.push(std::make_shared<UpdateJob>(id, UpdateOperation::UPDATE, CellCoord_t(new_x, new_y), std::vector<unsigned char>(), std::vector<unsigned char>()));
		m_NewData.set();
	}

	void RegionCellArchivist::UpdateActiveEntityLocation(ObjectID id, const Vector2T<int32_t>& location)
	{
		m_ActiveEntityDirectory->StoreEntityLocation(id, location);
	}

	bool RegionCellArchivist::GetActiveEntityLocation(ObjectID id, Vector2T<int32_t>& location)
	{
		return m_ActiveEntityDirectory->RetrieveEntityLocation(id, location);
	}

	void RegionCellArchivist::Remove(ObjectID id)
	{
		m_ObjectUpdateQueue.push(std::make_shared<UpdateJob>(id, UpdateOperation::REMOVE, CellCoord_t(), std::vector<unsigned char>(), std::vector<unsigned char>()));
		m_NewData.set();
	}

	Vector2T<int32_t> RegionCellArchivist::GetEntityLocation(ObjectID id)
	{
		CellCoord_t loc(std::numeric_limits<CellCoord_t::type>::max(), std::numeric_limits<CellCoord_t::type>::max());
		if (m_EntityLocationDB) // TODO: lock while loading save-game and wait here
		{
			std::streamoff offset; std::streamsize length;
			getEntityLocation(*m_EntityLocationDB, loc, offset, length, id);
		}
		return loc;
	}

	void RegionCellArchivist::Store(int32_t x, int32_t y, std::shared_ptr<Cell> cell)
	{
		if (cell->waiting.fetch_and_store(Cell::Store) != Cell::Store)
		{
			AddHist(CellCoord_t(x, y), "Enqueued Out");
			// The last tuple param indicates whether the cell should be cleared (unloaded)
			//  when the write operation is done: it checks whether this is the only reference
			//  to the cell
			m_WriteQueue.push(WriteJob(cell, CellCoord_t(x, y), cell.unique()));
			m_NewData.set();
			
			//TransactionMutex_t::scoped_try_lock lock(m_TransactionMutex);
			//FSN_ASSERT_MSG(lock, "Concurrent Store/Retrieve access isn't allowed");
			CellsBeingProcessedMap_t::accessor accessor;
			m_CellsBeingProcessed.insert(accessor, CellCoord_t(x, y));
			accessor->second = std::move(cell);
		}
	}

	std::shared_ptr<Cell> RegionCellArchivist::Retrieve(int32_t x, int32_t y)
	{
		//TransactionMutex_t::scoped_try_lock lock(m_TransactionMutex);
		//FSN_ASSERT_MSG(lock, "Concurrent Store/Retrieve access isn't allowed");
		CellsBeingProcessedMap_t::accessor accessor;
		m_CellsBeingProcessed.insert(accessor, std::make_pair(CellCoord_t(x, y), std::make_shared<Cell>()));
		auto& cell = accessor->second;

		auto state = cell->waiting.fetch_and_store(Cell::Retrieve);
		if (state != Cell::Retrieve && (state != Cell::Ready || !cell->loaded))
		{
			AddHist(CellCoord_t(x, y), "Enqueued In");
			m_ReadQueueGetCellData.push(std::make_shared<ReadJob>(cell, CellCoord_t(x, y)));
			m_NewData.set();
		}
		else if (cell->loaded)
		{
			AddHist(CellCoord_t(x, y), "Already loaded");
			cell->waiting = Cell::Ready;
			std::shared_ptr<Cell> retVal = std::move(cell); // move the ptr so the stored one can be erased
			m_CellsBeingProcessed.erase(accessor);
			return std::move(retVal);
		}

		return cell;
	}

	RegionCellArchivist::TransactionLock::TransactionLock(RegionCellArchivist::TransactionMutex_t& mutex, CL_Event& ev)
		: lock(mutex),
		endEvent(ev)
	{}

	RegionCellArchivist::TransactionLock::~TransactionLock()
	{
		endEvent.set();
	}

	//std::unique_ptr<RegionCellArchivist::TransactionLock> RegionCellArchivist::MakeTransaction()
	//{
	//	return std::unique_ptr<TransactionLock>(new TransactionLock(m_TransactionMutex, m_TransactionEnded));
	//}

	void RegionCellArchivist::BeginTransaction()
	{
		//m_TransactionMutex.lock();
	}

	void RegionCellArchivist::EndTransaction()
	{
		//m_TransactionMutex.unlock();
		m_TransactionEnded.set();
	}

	void RegionCellArchivist::Start()
	{
		m_Running = true;

		m_Quit.reset();
		m_Thread = boost::thread(&RegionCellArchivist::Run, this);
#ifdef _WIN32
		SetThreadPriority(m_Thread.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
#endif
	}

	void RegionCellArchivist::Stop()
	{
		m_Quit.set();
		m_Thread.join();

		m_Running = false;
	}

	void RegionCellArchivist::GetCellStreamForReading(std::function<void (std::shared_ptr<std::istream>)> callback, int32_t cell_x, int32_t cell_y)
	{
		return m_Cache->GetCellStreamForReading(callback, cell_x, cell_y);
	}

	std::unique_ptr<std::ostream> RegionCellArchivist::GetCellStreamForWriting(int32_t cell_x, int32_t cell_y)
	{
		return m_Cache->GetCellStreamForWriting(cell_x, cell_y);
	}

	void RegionCellArchivist::SaveEntityLocationDB(const std::string& filename)
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
				if (!m_EntityLocationDB->close())
					AddLogEntry(m_EntityLocationDB->error().message());
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

	void RegionCellArchivist::EnqueueQuickSave(const std::string& save_name)
	{
		m_SaveQueue.push(save_name);
		m_NewData.set();
	}

	void RegionCellArchivist::Save(const std::string& save_name)
	{
		const bool wasRunning = m_Thread.joinable();
		if (wasRunning)
			Stop();

		if (PerformSave(save_name))
			CompressSave(save_name);

		if (wasRunning)
			Start();
	}

	void RegionCellArchivist::EnqueueQuickLoad(const std::string& save_name)
	{
		// Stop and clear the cache
		PrepareLoad(save_name);
		m_NewData.set();
	}

	void RegionCellArchivist::Load(const std::string& save_name)
	{
		const bool wasRunning = m_Thread.joinable();
		if (wasRunning)
			Stop();
		PrepareLoad(save_name);
		PerformLoad(save_name);
		if (wasRunning)
			Start();
	}

	std::unique_ptr<std::ostream> RegionCellArchivist::CreateDataFile(const std::string& filename)
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

	std::unique_ptr<std::istream> RegionCellArchivist::LoadDataFile(const std::string& filename)
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

	void RegionCellArchivist::OnGotCellStreamForReading(std::shared_ptr<std::istream> cellDataStream, std::shared_ptr<ReadJob> job)
	{
		std::stringstream str; str << job->coord.x << "," << job->coord.y;
		AddLogEntry("cells_loaded", "  Got [" + str.str() + "]");
		AddHist(job->coord, "Got cell data stream");
		job->cellDataStream = std::move(cellDataStream);
		m_ReadQueueLoadEntities.push(job);
	}

	std::shared_ptr<EntitySerialisationUtils::EntityFuture> RegionCellArchivist::LoadEntity(std::shared_ptr<ICellStream> file, bool includes_id, ObjectID id, const EntitySerialisationUtils::SerialisedDataStyle data_style)
	{
		return EntitySerialisationUtils::LoadEntity(std::move(file), includes_id, id, data_style, m_Factory, m_EntityManager, m_Instantiator);
	}

	std::pair<size_t, std::vector<ObjectID>> RegionCellArchivist::ReadCellIntro(const CellCoord_t& coord, ICellStream& file, bool data_includes_ids, const EntitySerialisationUtils::SerialisedDataStyle)
	{
		size_t numEntries;
		file.read(reinterpret_cast<char*>(&numEntries), sizeof(size_t));

		FSN_ASSERT_MSG(numEntries < 65535, "Probably invalid data: entry count is implausible");

		std::vector<ObjectID> ids;

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
				ids.push_back(id);
			}
		}

		return std::make_pair(numEntries, ids);
	}

	std::tuple<bool, size_t, std::shared_ptr<ICellStream>> RegionCellArchivist::ContinueReadingCell(const CellCoord_t& coord, const std::shared_ptr<Cell>& conveniently_locked_cell, size_t num_entities, size_t progress, std::shared_ptr<EntitySerialisationUtils::EntityFuture>& incomming_entity, const std::vector<ObjectID>& ids, std::shared_ptr<ICellStream> file, const EntitySerialisationUtils::SerialisedDataStyle data_style)
	{
		std::list<EntityPtr> loaded_entities;

		FSN_ASSERT(incomming_entity || file);

		if (!incomming_entity)
		{
			// Read as many entities as possible before hitting one that requires other resources (e.g. archetype factories)
			while (progress < num_entities)
			{
				++progress;

				incomming_entity = LoadEntity(std::move(file), false, !ids.empty() ? ids[progress] : 0, data_style);
				FSN_ASSERT(incomming_entity);
				if (incomming_entity->is_ready())
				{
					loaded_entities.push_back(incomming_entity->get_entity());
					file = incomming_entity->get_file();
					FSN_ASSERT(file);

					incomming_entity.reset();
				}
				else
					break;
			}
		}
		else if (incomming_entity->is_ready())
		{
			loaded_entities.push_back(incomming_entity->get_entity());
			file = incomming_entity->get_file();
			FSN_ASSERT(file);

			incomming_entity.reset();
		}

		// Add the loaded entities to the cell
		for (auto it = loaded_entities.begin(); it != loaded_entities.end(); ++it)
		{
			const auto& entity = *it;

			Vector2 pos = entity->GetPosition();
			// TODO: Cell::Add(entity, CellEntry = def) rather than this bullshit
			CellEntry entry;
			entry.x = pos.x; entry.y = pos.y;

			entity->SetStreamingCellIndex(coord);

			conveniently_locked_cell->objects.push_back(std::make_pair(std::move(entity), std::move(entry)));
		}
		loaded_entities.clear();

		// After all the entities are constructed:
		if (progress >= num_entities && !incomming_entity)
		{
			// Remove the archetype agent if edit-mode is disabled
			if (!m_EditMode)
			{
				for (auto it = conveniently_locked_cell->objects.begin(); it != conveniently_locked_cell->objects.end(); ++it)
					it->first->ResetArchetypeAgent();
			}

			FSN_ASSERT(file);
			return std::make_tuple(true, progress, std::move(file));
		}

		return std::make_tuple(false, progress, file ? std::move(file) : std::shared_ptr<ICellStream>());
	}

	void RegionCellArchivist::WriteCellIntro(std::ostream& file_param, const CellCoord_t& loc, const Cell* cell, size_t expectedNumEntries, const bool synched, const EntitySerialisationUtils::SerialisedDataStyle data_style)
	{
		using namespace EntitySerialisationUtils;
	}

	// expectedNumEntries is used because this can be counted once when WriteCell is called multiple times
	void RegionCellArchivist::WriteCellData(std::ostream& file_param, const CellCoord_t& loc, const Cell* cell, size_t expectedNumEntries, const bool synched, const EntitySerialisationUtils::SerialisedDataStyle data_style)
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

				SaveEntity(file, it->first, false, data_style);

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
		if (synched && data_style == FastBinary)
		{
			for (auto it = dataPositions.cbegin(), end = dataPositions.cend(); it != end; ++it)
			{
				storeEntityLocation(*m_EntityLocationDB, std::get<0>(*it), loc, std::get<1>(*it), std::get<2>(*it));

				// The saved entity location is now up to date: don't need the location in the active db anymore
				m_ActiveEntityDirectory->DropEntityLocation(std::get<0>(*it));
			}
		}
	}

	void RegionCellArchivist::WriteCellDataForEditMode(const std::unique_ptr<std::ostream>& file, const CellCoord_t& cell_coord, const std::shared_ptr<Cell>& cell, size_t numPseudo, size_t numSynched, const EntitySerialisationUtils::SerialisedDataStyle data_style)
	{
		if (file && *file)
		{
			{
				// Need write the length of the data written up front, so a temp stream is needed
				std::stringstream tempStream(std::ios::in | std::ios::out | std::ios::binary);
				std::streampos start = tempStream.tellp();

				// Write un-synched entity data (written to cache since this is edit mode)
				WriteCellData(tempStream, cell_coord, cell.get(), numPseudo, false, data_style);

				std::streamsize dataLength = tempStream.tellp() - start;

				// Write the length of the psuedo-entity data
				IO::Streams::CellStreamWriter writer(file.get());
				writer.Write(dataLength);

				// Write the pseudo-entity data from the temp stream to the actual output file
				(*file) << tempStream.rdbuf();
			}

			// Write the non-pseudo-entity data
			WriteCellData(*file, cell_coord, cell.get(), numSynched, true, data_style);
		}
		else
			FSN_EXCEPT(FileSystemException, "Failed to open file in order to dump edit-mode cache");
	}

	void RegionCellArchivist::StartJob(const std::shared_ptr<ReadJob>& job, const std::shared_ptr<Cell>& a_cell_that_is_locked)
	{
		using namespace EntitySerialisationUtils;

		auto cellCoord = job->coord;
#ifdef _DEBUG
		if (a_cell_that_is_locked->waiting == Cell::Ready)
		{
			std::stringstream str; str << cellCoord.x << "," << cellCoord.y;
			SendToConsole("A loaded cell was pointlessly re-enqueued for loading [" + str.str() + "]");
		}
#endif
		// Make sure the cell requester hasn't stopped asking for this cell.
		//  !cell->loaded is for the situation where a cell was stored then retrieved
		//  before the store action was executed
		if (a_cell_that_is_locked->active_entries == 0 && a_cell_that_is_locked->waiting == Cell::Retrieve && !a_cell_that_is_locked->loaded)
		{
			try
			{
				auto& cellDataStream = job->cellDataStream;

				// Last param makes the method load synched entities from the map if the cache file isn't available:
				if (m_Map)
				{
					// Load synched entities if this cell is un-cached (hasn't been loaded before)
					const bool uncached = !cellDataStream;//m_SynchLoaded.insert(std::make_pair(cell_coord.x, cell_coord.y)).second;

					auto data = m_Map->GetRegionData(cellCoord.x, cellCoord.y, uncached);

					if (!data.empty())
					{
						std::unique_ptr<bio::filtering_istream> inflateStream(new bio::filtering_istream());
						inflateStream->push(bio::zlib_decompressor());
						inflateStream->push(bio::array_source(data.data(), data.size()));

						IO::Streams::CellStreamReader reader(inflateStream.get());
						auto pseudoEntityDataLength = reader.ReadValue<std::streamsize>();

						// Create the sub-job for loading this map cell
						job->mapSubjob = std::make_shared<ReadJob>(a_cell_that_is_locked, cellCoord);
						job->mapSubjob->dataStyle = FastBinary; // Map data is always FastBinary

						job->mapSubjob->cellDataStream = std::move(inflateStream);

						// Read pseudo-entities
						if (pseudoEntityDataLength > 0)
						{
							std::tie(job->mapSubjob->entitiesExpected, job->mapSubjob->ids) = ReadCellIntro(cellCoord, *inflateStream, false, FastBinary);
							// Remember that there are synced-entities to read next if this cell is uncached:
							job->thereIsSyncedDataToReadNext = uncached;
						}
						else if (uncached)
							std::tie(job->mapSubjob->entitiesExpected, job->mapSubjob->ids) = ReadCellIntro(cellCoord, *inflateStream, true, FastBinary);

						// Enqueue the map data load job if there is anything to load for this map cell
						if (job->mapSubjob->entitiesExpected > 0)
							m_IncommingCells.push_back(std::move(job->mapSubjob));
					}
				}

				// Load normal data
				if (cellDataStream && *cellDataStream && !cellDataStream->eof())
				{
					// Set the data-style to expect for this job (will be used when the actual entity data is read)
					job->dataStyle = m_EditMode ? EditableBinary : FastBinary;

					// In edit-mode unsynched entities are also written to the cache; they will be read first
					if (m_EditMode)
					{
						// The length of this (edit-mode only) data is written in front of it so that it can be skipped when merging incoming data
						//  (we don't need to know this length to actually load the data):
						IO::Streams::CellStreamReader reader(cellDataStream.get());
						std::streamsize unsynchedDataLength = reader.ReadValue<std::streamsize>();

						// TODO: (or not) Could create a second buffer for the pseudo data, then load it all at once into a new subjob (like the map file)
						//std::vector<char> buffer(unsynchedDataLength);
						//cellDataStream->read(buffer.data(), unsynchedDataLength);
						//auto pseudoDataStream = std::make_shared<std::stringstream>();
						//pseudoDataStream->write(buffer.data(), unsynchedDataLength);

						std::tie(job->entitiesExpected, job->ids) = ReadCellIntro(cellCoord, *cellDataStream, false, EditableBinary); // This is edit mode, so editable data

						// Remember to read the synced-entity data, too:
						job->thereIsSyncedDataToReadNext = true;
					}
					else
						std::tie(job->entitiesExpected, job->ids) = ReadCellIntro(cellCoord, *cellDataStream, true, FastBinary); // FastBinary since this isn't edit mode

					//std::stringstream str; str << i;
					//SendToConsole("Cell " + str.str() + " streamed in");

					AddHist(cellCoord, "Loading (waiting for any archetypes)");

					// Try to process all the entities in the cell. If there isn't any archetypes to load,
					//  the cell will be available immediately, otherwise it will have to be processed later:
					bool done = false;
					//done = ContinueJob(job, a_cell_that_is_locked);
					if (!done)
					{
						// Enqueue the cell to process again later
						m_IncommingCells.push_back(job);
						return;
					}
					else
					{
						// No archetypes, cell done loading
						AddHist(cellCoord, "Note: No Archetypes");
					}
				}
				else
				{
					a_cell_that_is_locked->loaded = true; // No data to load
					AddHist(cellCoord, "Loaded (no data)");
				}
			}
			catch (std::exception& e)
			{
				std::string message("Exception while streaming in cell: ");
				message += e.what();
				AddHist(cellCoord, message);
				AddLogEntry(message);
			}
			catch (...)
			{
				AddLogEntry("Unknown exception while streaming in cell");
			}
		}
		else
			AddHist(cellCoord, "Cell load aborted: already loaded or request canceled");

		// Cell finished loading or failed to load and is ready for other operations
		a_cell_that_is_locked->waiting = Cell::Ready;
	}

	bool RegionCellArchivist::ContinueJob(const std::shared_ptr<ReadJob>& job, const std::shared_ptr<Cell>& the_cell_that_locks)
	{
		bool done = false;

		std::tie(done, job->entitiesReadSoFar, job->cellDataStream) = ContinueReadingCell(
			job->coord, the_cell_that_locks,
			job->entitiesExpected, job->entitiesReadSoFar,
			job->entityInTransit,
			job->ids, std::move(job->cellDataStream),
			job->dataStyle);

		FSN_ASSERT(job->entityInTransit || job->cellDataStream);

		if (done && job->thereIsSyncedDataToReadNext) // but wait, there's more
		{
			if (!job->cellDataStream)
				FSN_EXCEPT(FileSystemException, "This shit done broke");

			done = false;
			job->thereIsSyncedDataToReadNext = false;

			std::tie(job->entitiesExpected, job->ids) = ReadCellIntro(job->coord, *job->cellDataStream, true, job->dataStyle);
			job->entitiesReadSoFar = 0;
		}

		return done;
	}

	void RegionCellArchivist::Run()
	{
		using namespace EntitySerialisationUtils;

		std::list<CellCoord_t> readyCells;
		bool retrying = false;
		while (true)
		{
			const int eventId = CL_Event::wait(m_Quit, m_TransactionEnded, m_NewData, retrying ? 100 : -1);
			if (!m_WriteQueue.empty() || !m_ReadQueueGetCellData.empty() || !m_ReadQueueLoadEntities.empty())
				SendToConsole(retrying ? "Retrying" : "Archive Running");
			if (eventId == 1) // TransactionEnded
			{
				ClearReadyCells(readyCells);
			}
			{
				// Process quick-save queue
				{
					//m_Cache->Sustain(); // Prevent unexpected writes to the region files -- Commented out because this is done in PerformSave now
					//m_EditableCache->Sustain();
					std::string saveName;
					while (m_SaveQueue.try_pop(saveName))
					{
						PerformSave(saveName);
					}
					//m_Cache->EndSustain();
					//m_EditableCache->EndSustain();
				}

				std::list<WriteQueue_t::value_type> writesToRetry;
				std::list<ReadQueue_t::value_type> readsToRetry;				

				bool readingMsg = false;
				if (!m_ReadQueueGetCellData.empty())
				{
					readingMsg = true;
					SendToConsole("Reading");
				}

				// Request cell data
				{
					std::shared_ptr<ReadJob> toRead;
					while (m_ReadQueueGetCellData.try_pop(toRead))
					{
						const CellCoord_t& cell_coord = toRead->coord;

						const auto cell = toRead->cell.lock();
						if (cell && cell->waiting == Cell::Retrieve && !cell->loaded)
						{
							AddHist(cell_coord, "Getting cell data stream");

							using namespace std::placeholders;
							// Request the cached cell data (if available)
							if (!m_EditMode)
							{
								GetCellStreamForReading(
									std::bind(&RegionCellArchivist::OnGotCellStreamForReading, this, _1, toRead),
									cell_coord.x, cell_coord.y);
							}
							else
							{
								// Use the editable data cache in edit mode (the other cache is informally write-only to in this mode)
								m_EditableCache->GetCellStreamForReading(
									std::bind(&RegionCellArchivist::OnGotCellStreamForReading, this, _1, toRead),
									cell_coord.x, cell_coord.y);
							}
						}
						else
							AddHist(cell_coord, "Aborting cell data stream retrieval (request canceled / already loaded)");
					}
				}

				// Read cell data
				{
					// Keep archetype factory resources loaded during this stage
					ArchetypeFactoryManager::Sustain();

					std::shared_ptr<ReadJob> toRead;
					while (m_ReadQueueLoadEntities.try_pop(toRead))
					{
						std::weak_ptr<Cell>& cellWpt = toRead->cell;
						const CellCoord_t& cellCoord = toRead->coord;
						if (auto cell = cellWpt.lock())
						{
							Cell::mutex_t::scoped_lock lock;
							if (lock.try_acquire(cell->mutex))
							{
								StartJob(toRead, cell);
								if (cell->waiting == Cell::Ready) // Sometimes there isn't much to do, and the job finishes immediately
									readyCells.push_back(cellCoord);
							}
							else
							{
#ifdef _DEBUG
								std::stringstream str; str << cellCoord.x << "," << cellCoord.y;
								SendToConsole("Retrying read on cell [" + str.str() + "]");
#endif
								AddHist(cellCoord, "Cell locked (will retry read later)");
								readsToRetry.push_back(toRead);
							}
						}
					}

					ArchetypeFactoryManager::EndSustain();
				}

				for (auto it = m_IncommingCells.begin(); it != m_IncommingCells.end();)
				{
					auto& job = *it;
					if (auto lockedCell = job->cell.lock())
					{
						Cell::mutex_t::scoped_lock lock;
						if (lock.try_acquire(lockedCell->mutex))
						{
							SendToConsole("Processing incoming cells");
							if (lockedCell->waiting == Cell::Retrieve)
							{
								try
								{
									bool done = ContinueJob(job, lockedCell);
									// The map is a separate file so it can be processed in parallel (while the pseudo-entity / synced entity subsections of each cell must be processed serially)
									bool mapDone = job->mapSubjob ? ContinueJob(job->mapSubjob, lockedCell) : true;
	
									// Note that at this point it is possible for job->cellDataStream to be null (if it is still held by an entity-in-transit)
	
									if (mapDone && done)
									{
										AddHist(job->coord, "Loaded", lockedCell->objects.size());
										lockedCell->loaded = true;
										lockedCell->waiting = Cell::Ready;

										readyCells.push_back(job->coord);

										it = m_IncommingCells.erase(it);
									}
									else
										++it;
								}
								catch (Exception& ex)
								{
									AddHist(job->coord, "Failed to load entity data: " + ex.ToString());

									lockedCell->waiting = Cell::Ready;
									readyCells.push_back(job->coord);

									it = m_IncommingCells.erase(it);
								}
							}
							else
							{
								// The requester asked to store the cell, but it was never loaded so it can simply be marked ready
								lockedCell->waiting = Cell::Ready;
								readyCells.push_back(job->coord);

								it = m_IncommingCells.erase(it);
							}
						}
					}
				}

				if (readingMsg)
					SendToConsole("Done Reading");

				bool writingMsg = false;
				if (!m_WriteQueue.empty())
				{
					writingMsg = true;
					SendToConsole("Writing");
				}

				{
					WriteJob toWrite;
					while (m_WriteQueue.try_pop(toWrite))
					{
						const std::weak_ptr<Cell>& cellWpt = toWrite.cell;
						const auto& cell_coord = toWrite.coord;
						const bool unload_when_done = toWrite.unloadWhenDone;
						if (auto cell = cellWpt.lock()) // Make sure the queue item is valid
						{
							Cell::mutex_t::scoped_lock lock;
							if (lock.try_acquire(cell->mutex))
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
											// The editable cache is used when saving / loading map data in the editor
											//  (it contains additional information to make it more robust when dealing
											//  with component script changes, etc.)
											WriteCellDataForEditMode(m_EditableCache->GetCellStreamForWriting(cell_coord.x, cell_coord.y), cell_coord, cell, numPseudo, numSynched, EntitySerialisationUtils::EditableBinary);
											// The normal cache is saved also so that this data can be used when compiling the map
											WriteCellDataForEditMode(GetCellStreamForWriting(cell_coord.x, cell_coord.y), cell_coord, cell, numPseudo, numSynched, EntitySerialisationUtils::FastBinary);
										}
										else // Not EditMode
										{
											auto filePtr = GetCellStreamForWriting(cell_coord.x, cell_coord.y);
											// Need a seekable stream, so write to a temp. one
											//std::stringstream tempStream(std::ios::in | std::ios::out | std::ios::binary);
											WriteCellData(*filePtr, cell_coord, cell.get(), numSynched, true, EntitySerialisationUtils::FastBinary);
											//*filePtr << tempStream.rdbuf();
										}

										if (unload_when_done)
										{
											AddHist(cell_coord, "Written and cleared", m_EditMode ? cell->objects.size() : numSynched);

											if (cell->active_entries != 0)
												AddLogEntry("Warning: unloading active cell");
											cell->objects.clear();
											cell->loaded = false;
										}
										else
											AddHist(cell_coord, "Written (not cleared, cell still active)", m_EditMode ? cell->objects.size() : numSynched);
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
									AddHist(cell_coord, "Write canceled");
									//writesToRetry.push_back(toWrite);
								}
								// The cell was written or the write was canceled - the cell is now "ready" for other operations
								cell->waiting = Cell::Ready;
								readyCells.push_back(cell_coord);
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

						// Check for read requests (only write one cell at a time when there are read requests queued)
						//  eventId 0 is m_Quit (don't let reads preempt writes when quiting / saving)
						if (eventId != 0 && (!m_ReadQueueGetCellData.empty() || !m_ReadQueueLoadEntities.empty()))
						{
							m_NewData.set();
							break;
						}
					}
				}
				if (writingMsg)
					SendToConsole("Done Writing");

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
						m_ReadQueueLoadEntities.push(*it);
					readsToRetry.clear();
				}

				{
					using namespace IO;

					std::shared_ptr<UpdateJob> objectUpdateData;
					while (m_ObjectUpdateQueue.try_pop(objectUpdateData))
					{
						const ObjectID id = objectUpdateData->id;
						const UpdateOperation operation = objectUpdateData->operation;
						CellCoord_t new_loc = objectUpdateData->cellCoord;
						auto& incommingConData = objectUpdateData->incommingConData;
						auto& incommingOccData = objectUpdateData->incommingOccData;

						CellCoord_t loc;
						std::streamoff dataOffset;
						std::streamsize dataLength;
						if (!getEntityLocation(*m_EntityLocationDB, loc, dataOffset, dataLength, id))
							continue; // This entity hasn't been stored (may have become active again since the update request was queued)

						if (operation == UpdateOperation::REMOVE || new_loc == CellCoord_t(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()))
							new_loc = loc;

						// Skip if nothing has changed
						if (operation != UpdateOperation::REMOVE && new_loc == loc && incommingConData.empty() && incommingOccData.empty())
							continue;

						// Request the existing cell data from the disc, is it isn't already available
						if (!objectUpdateData->existingSourceCellDataStream)
						{
							GetCellStreamForReading([this, objectUpdateData](std::shared_ptr<std::istream> dataStream)
							{
								objectUpdateData->existingSourceCellDataStream = std::move(dataStream);
								this->m_ObjectUpdateQueue.push(objectUpdateData);
							}, loc.x, loc.y);
							continue;
						}
						auto& inSourceData = objectUpdateData->existingSourceCellDataStream;
						auto outDestData = GetCellStreamForWriting(new_loc.x, new_loc.y);
						
						if (!inSourceData || !outDestData)
								continue;

						// If the data is being moved there are some more streams that need to be prepared:
						std::shared_ptr<std::istream> inDestData;
						std::shared_ptr<std::ostream> outSourceData;
						if (new_loc != loc)
						{
							if (!objectUpdateData->existingDestCellDataStream)
							{
								GetCellStreamForReading([this, objectUpdateData](std::shared_ptr<std::istream> dataStream)
								{
									objectUpdateData->existingDestCellDataStream = std::move(dataStream);
									this->m_ObjectUpdateQueue.push(objectUpdateData);
								}, new_loc.x, new_loc.y);
								continue;
							}
							inDestData = std::move(objectUpdateData->existingDestCellDataStream);
							outSourceData = GetCellStreamForWriting(loc.x, loc.y);
						}

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

						// New read jobs preempt update jobs
						if (!m_ReadQueueLoadEntities.empty())
						{
							break;
						}
					}
				}

				if (readingMsg || writingMsg)
					SendToConsole("Done Running Archive");

				if (eventId == 0) // Quit
				{
					ClearReadyCells(readyCells);
					break;
				}
			}
		}

		asThreadCleanup();
	}

	void RegionCellArchivist::ClearReadyCells(std::list<CellCoord_t>& readyCells)
	{
		// Drop references to cells that are done processing (if m_CellsBeingProcessed isn't locked by a current transaction)
		if (!readyCells.empty())
		{
			{
				//TransactionMutex_t::scoped_lock lock(m_TransactionMutex);
				//if (lock)
				{
					for (auto it = readyCells.begin(); it != readyCells.end(); ++it)
					{
						CellsBeingProcessedMap_t::const_accessor accessor;
						if (m_CellsBeingProcessed.find(accessor, *it))
						{
							AddHist(*it, "Cell operation finished, archivist dropping ref. Refs remaining", accessor->second.use_count() - 1);
							if (accessor->second->waiting == Cell::Ready)
								m_CellsBeingProcessed.erase(accessor);
						}
						else
							AddHist(*it, "Cell operation finished, but archivist wasn't longer holding a ref. anyway???");
					}
				}
			}
			readyCells.clear();
		}
	}

	boost::filesystem::path make_relative(const boost::filesystem::path& base, const boost::filesystem::path& path)
	{
		namespace bfs = boost::filesystem;

		bfs::path relativePath;

		auto basePath = base;
		auto fullPath = path;
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

	bool RegionCellArchivist::PerformSave(const std::string& saveName)
	{
		namespace bfs = boost::filesystem;

		// TODO: replace all SendToConsole calls with signals
		try
		{
			SendToConsole("Quick-Saving...");

			auto savePath = boost::filesystem::path(PHYSFS_getWriteDir()) / m_SavePath / saveName;

			auto physFsPath = m_SavePath + saveName;

			// Remove any extraneous extensions
			if (savePath.has_extension())
			{
				AddLogEntry("Save name " + saveName + " has an extension; save names should be folders, not files: the extension will be removed.", LOG_NORMAL);
				savePath.replace_extension();

				physFsPath = m_SavePath + bfs::path(saveName).replace_extension().string();
			}

			SendToConsole("QS Path: " + savePath.string());
			AddLogEntry("Saving: " + savePath.string(), LOG_INFO);

			if (!bfs::is_directory(savePath))
			{
				if (PHYSFS_mkdir(physFsPath.c_str()) == 0)
				{
					FSN_EXCEPT(FileSystemException, "Failed to create save path (" + physFsPath + "): " + std::string(PHYSFS_getLastError()));
				}
			}
			else
			{
				try
				{
					PhysFSHelp::clear_folder(physFsPath + "/");
				}
				catch (FileSystemException& ex)
				{
					FSN_EXCEPT(FileSystemException, "Failed to clear save path: " + ex.GetDescription());
				}
			}
			FSN_ASSERT(bfs::is_directory(savePath));
			FSN_ASSERT(bfs::is_empty(savePath));

			if (m_EditableCache)
			{
				if (PHYSFS_mkdir((physFsPath + "/editable").c_str()) == 0)
					FSN_EXCEPT(FileSystemException, "Failed to create save path (" + physFsPath + "/editable): " + std::string(PHYSFS_getLastError()));
			}

			// Prevent unexpected writes (regions may have been queued to unload recently)
			m_Cache->Sustain();
			if (m_EditableCache) m_EditableCache->Sustain();

			// Flush the cache(s)
			m_Cache->FlushCache();
			if (m_EditableCache) m_EditableCache->FlushCache();

			std::vector<bfs::path> regionFiles;
			std::vector<bfs::path> dataFiles;
			for(bfs::directory_iterator it(m_FullBasePath); it != bfs::directory_iterator(); ++it)
			{
				if (it->path().extension() == ".celldata")
					regionFiles.push_back(it->path());
				if (it->path().extension() == ".dat")
					dataFiles.push_back(it->path());
			}
			// List editable data
			if (m_EditableCache)
			{
				auto editableDataPath = bfs::path(m_FullBasePath) / "editable";
				for(bfs::directory_iterator it(editableDataPath); it != bfs::directory_iterator(); ++it)
				{
					if (it->path().extension() == ".celldata")
						regionFiles.push_back(it->path());
				}
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

				auto dest = savePath; dest /= make_relative(m_FullBasePath, *it);
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

			// Allow regions to be unloaded
			if (m_EditableCache) m_EditableCache->EndSustain();
			m_Cache->EndSustain();

			SendToConsole("QS Progress: copying entitylocations.kc");
			SaveEntityLocationDB((savePath /= "entitylocations.kc").string());
			SendToConsole("QS Progress: done copying entitylocations.kc");

			SendToConsole("Done Quick-Saving.");

			return true;
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

		return false;
	}

	void getArchiveFileList(std::vector<boost::filesystem::path>& results, const boost::filesystem::path& path, const std::string& ext)
	{
		namespace bfs = boost::filesystem;

		std::deque<bfs::path> toProcess;
		toProcess.push_back(path);
		while (!toProcess.empty())
		{
			const auto& nextPath = toProcess.front();
			auto fileList = PHYSFS_enumerateFiles(nextPath.generic_string().c_str());
			for (auto it = fileList; *it; ++it)
			{
				auto filePath = nextPath / (*it);
				if (PHYSFS_isDirectory(filePath.generic_string().c_str()))
					toProcess.push_back(filePath);
				if (filePath.extension() == ext)
					results.push_back(filePath);
			}
			PHYSFS_freeList(fileList);
			toProcess.pop_front();
		}
	}

	void getNativeFileList(std::vector<boost::filesystem::path>& results, const boost::filesystem::path& path, const std::string& ext)
	{
		namespace bfs = boost::filesystem;

		std::deque<bfs::path> toProcess;
		toProcess.push_back(path);
		while (!toProcess.empty())
		{
			const auto& nextPath = toProcess.front();
			for (bfs::directory_iterator it(nextPath); it != bfs::directory_iterator(); ++it)
			{
				if (bfs::is_directory(*it))
					results.push_back(it->path());
				else if (it->path().extension() == ext)
					toProcess.push_back(*it);
			}
			toProcess.pop_front();
		}
	}

	void copyArchivedFiles(const boost::filesystem::path& source_path, std::vector<boost::filesystem::path>& files, const boost::filesystem::path& target_path)
	{
		namespace bfs = boost::filesystem;

		std::stringstream numStr; numStr << files.size();
		std::string numDataFilesStr = numStr.str();
		unsigned int i = 0;
		for (auto it = files.begin(); it != files.end(); ++it)
		{
			std::stringstream str; str << ++i;
			SendToConsole("Load Progress: copying " + str.str() + " / " + numDataFilesStr + " data files");

			auto dest = target_path / make_relative(source_path, *it);
			PhysFSHelp::copy_file(it->generic_string(), dest.generic_string());
		}
	}

	void copyNativeFiles(const boost::filesystem::path& source_path, std::vector<boost::filesystem::path>& files, const boost::filesystem::path& target_path, const std::function<void (size_t, size_t)>& callback)
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

			auto dest = target_path / make_relative(source_path, *it);
			boost::filesystem::copy_file(*it, dest, bfs::copy_option::overwrite_if_exists);
		}
	}

	void RegionCellArchivist::PrepareLoad(const std::string& saveName)
	{
	}

	void RegionCellArchivist::PerformLoad(const std::string& saveName)
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
		else
			physSavePath.replace_extension();
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
		m_ReadQueueGetCellData.clear();
		m_ReadQueueLoadEntities.clear();
		m_WriteQueue.clear();

		// Everything needs to reload after this
		m_SynchLoaded.clear();

		// Drop file handles (files need to be deletable below)
		m_Cache->DropCache();
		if (m_EditableCache)
			m_EditableCache->DropCache();

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
				try
				{
					PhysFSHelp::clear_folder(m_CachePath + "/");
				}
				catch (FileSystemException& ex)
				{
					FSN_EXCEPT(FileSystemException, "Failed to clear the map cache: " + ex.GetDescription());
				}
			}

			if (m_EditableCache)
			{
				if (PHYSFS_mkdir((m_CachePath + "/editable").c_str()) == 0)
				{
					FSN_EXCEPT(FileSystemException, "Failed to create editable cache path: " + std::string(PHYSFS_getLastError()));
				}
			}

			// Copy the saved location DB into the cache
			SendToConsole("Loading: copying entitylocations.kc");
			{
				if (archived)
				{
					auto saveDbPath = physSavePath / "entitylocations.kc";
					auto physFsTargetPath = make_relative(PHYSFS_getWriteDir(), cacheDbPath);
					PhysFSHelp::copy_file(saveDbPath.generic_string(), physFsTargetPath.generic_string());
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
				copyArchivedFiles(physSavePath, dataFiles, m_CachePath);
			}
			else
			{
				getNativeFileList(dataFiles, savePath, ".dat");
				copyNativeFiles(savePath, dataFiles, m_FullBasePath, [](size_t, size_t)
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
				copyArchivedFiles(physSavePath, regionFiles, m_CachePath);
			}
			else
			{
				getNativeFileList(regionFiles, savePath, ".celldata");
				copyNativeFiles(savePath, regionFiles, m_FullBasePath, [](size_t, size_t)
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

	void RegionCellArchivist::CompressSave(const std::string& saveName)
	{
		namespace bfs = boost::filesystem;

		// TODO: replace all SendToConsole calls with signals
		try
		{
			SendToConsole("Compressing save...");

			auto savePath = boost::filesystem::path(PHYSFS_getWriteDir()) / m_SavePath / saveName;

			auto physFsPath = m_SavePath + saveName;

			// Remove any extraneous extensions
			if (savePath.has_extension())
			{
				AddLogEntry("Save name " + saveName + " has an extension; save names should be folders, not files: the extension will be removed.", LOG_NORMAL);
				savePath.replace_extension();

				physFsPath = m_SavePath + bfs::path(saveName).replace_extension().string();
			}

			SendToConsole("Original path: " + savePath.string());
			
			if (!bfs::is_directory(savePath))
			{
				FSN_EXCEPT(FileSystemException, "Save path (" + physFsPath + ") not found.");
			}

			auto archiveFilePath = savePath;
			archiveFilePath.replace_extension(".zip");

			AddLogEntry("Creating compressed save: " + savePath.string() + " -> " + archiveFilePath.string(), LOG_INFO);

			if (bfs::is_regular_file(archiveFilePath))
			{
				auto physFsArchiveFilePath = bfs::path(physFsPath).replace_extension(".zip").generic_string();

				AddLogEntry("Deleting existing file at: " + archiveFilePath.string(), LOG_TRIVIAL);
				if (PHYSFS_delete(physFsArchiveFilePath.c_str()) == 0)
				{
					FSN_EXCEPT(FileSystemException, "Failed to delete existing compressed save file (" + physFsArchiveFilePath + "): " + std::string(PHYSFS_getLastError()));
				}
			}

			{
				IO::FileType::ZipArchive archive(archiveFilePath);
				archive.AddPath(savePath, savePath.stem());
			}

			try
			{
				PhysFSHelp::clear_folder(physFsPath + "/");
			}
			catch (FileSystemException& ex)
			{
				FSN_EXCEPT(FileSystemException, "Failed to delete uncompressed save data: " + ex.GetDescription());
			}
			if (PHYSFS_delete(physFsPath.c_str()) == 0)
				FSN_EXCEPT(FileSystemException, "Failed to delete uncompressed save data path: " + std::string(PHYSFS_getLastError()));

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

	inline void CopyEditModePseudoEntityData(CopyBuffer_t& buffer, ICellStream& source_in, ICellStream& dest_in, OCellStream& dest_out)
	{
		IO::Streams::CellStreamReader src_reader(&source_in);
		IO::Streams::CellStreamReader dst_reader(&dest_in);

		if (source_in != dest_in)
			src_reader.ReadValue<std::streamsize>();
		std::streamsize unsynchedDataLength = dst_reader.ReadValue<std::streamsize>();

		FSN_ASSERT(unsynchedDataLength >= 0 && unsynchedDataLength < (1 << 24));

		CopyData(buffer, dest_in, dest_out, unsynchedDataLength);
	}

	std::streamsize RegionCellArchivist::MergeEntityData(std::vector<ObjectID>& objects_displaced_for, std::vector<ObjectID>& objects_displaced_back, ObjectID id, std::streamoff data_offset, std::streamsize data_length, ICellStream& source_in, OCellStream& source_out, ICellStream& dest_in, OCellStream& dest_out, RakNet::BitStream& mergeCon, RakNet::BitStream& mergeOcc) const
	{
		CopyBuffer_t buffer;

		const bool destChanged = &source_in != &dest_in;

		IO::Streams::CellStreamReader src_reader(&source_in);
		IO::Streams::CellStreamReader dst_reader(&dest_in);
		IO::Streams::CellStreamWriter writer(&dest_out);

		// Skip the pseudo-entity data that is present in Edit Mode
		if (m_EditMode)
			CopyEditModePseudoEntityData(buffer, source_in, dest_in, dest_out);

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
			dest_out.write(headerData.data(), headerData.size());

			// The the ID for the entity being merged in
			writer.Write(id);
		}
		else
		{
			// Copy the existing ID list and note which objects come after the one being
			//  updated, and thus will be displaced if the length changes
			CopyIDList(objects_displaced_for, numEnts, id, dest_in, dest_out);
		}


		if (!destChanged)
		{
			auto remainingData = data_offset - (sizeof(size_t) + headerLength); // The offset is relative to the start of the cell data
			// Copy existing cell data to the relevant entity data
			CopyData(buffer, dest_in, dest_out, remainingData);
		}
		else
		{
			auto remainingData = data_offset - (sizeof(size_t) + headerLength); // The offset is relative to the start of the cell data
			// Re-write the source up to the relevant entity data
			CopyData(buffer, source_in, source_out, remainingData);

			// Re-write rest of the existing dest cell data (changed data will be written at the end)
			CopyData(buffer, dest_in, dest_out);
		}

		// Merge the data
		std::streamsize newEntityDataLength;
		{
			CharCounter counter;
			bio::filtering_ostream countingOut;
			countingOut.push(counter);
			countingOut.push(dest_out, 0);
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
			CopyData(buffer, dest_in, dest_out);
		}

		return newEntityDataLength;
	}

	void RegionCellArchivist::MoveEntityData(std::vector<ObjectID>& objects_displaced, ObjectID id, std::streamoff data_offset, std::streamsize data_length, ICellStream& source_in, OCellStream& source_out, ICellStream& dest_in, OCellStream& dest_out) const
	{
		// Make sure the data is actually being moved!
		FSN_ASSERT(&source_in != &dest_in);

		CopyBuffer_t buffer;

		IO::Streams::CellStreamReader src_reader(&source_in);
		IO::Streams::CellStreamWriter src_writer(&source_out);

		IO::Streams::CellStreamReader dst_reader(&dest_in);
		IO::Streams::CellStreamWriter dst_writer(&dest_out);

		if (m_EditMode)
			CopyEditModePseudoEntityData(buffer, source_in, dest_in, dest_out);

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
		dest_out.write(headerData.data(), headerData.size());
		// Write the new ID
		dst_writer.Write(id);

		// Copy the rest of the existing dest data
		CopyData(buffer, dest_in, dest_out);

		// Copy entity data from the source into the dest (note that not copying this back into source_out effectively deletes it from there)
		CopyData(buffer, source_in, dest_out, data_length);

		// Copy the remaining source data back into the source
		CopyData(buffer, source_in, source_out);
	}

	void RegionCellArchivist::DeleteEntityData(std::vector<ObjectID>& objects_displaced, ObjectID id, std::streamoff data_offset, std::streamsize data_length, ICellStream& source_in, OCellStream& source_out) const
	{
		CopyBuffer_t buffer;

		IO::Streams::CellStreamReader src_reader(&source_in);
		IO::Streams::CellStreamWriter src_writer(&source_out);

		if (m_EditMode)
			CopyEditModePseudoEntityData(buffer, source_in, source_in, source_out);

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
