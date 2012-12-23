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

#ifndef H_FusionCellArchivist
#define H_FusionCellArchivist

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionStreamingManager.h"

#include <array>
#include <boost/thread.hpp>
#include <ClanLib/Core/System/event.h>
#include <ClanLib/Core/IOData/iodevice.h>
#include <memory>
#include <unordered_set>
#include <tuple>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_hash_map.h>

#include "FusionCellDataSource.h"
#include "FusionGameMapLoader.h"
#include "FusionPhysFSIOStream.h"
#include "FusionSaveDataArchive.h"
#include "FusionCellStreamTypes.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionCellSerialisationUtils.h"

#include "FusionHashable.h"

namespace kyotocabinet
{
	class HashDB;
}

namespace FusionEngine
{

	class ArchetypeFactory;
	class RegionCellCache;
	class ActiveEntityDirectory;

	//! CellArchiver implementation
	class RegionCellArchivist : public CellArchiver, public SaveDataArchive
	{
	public:
		//! Ctor
		/*
		* The map provides a static entity source, whilst the CellArchiver (cache) provides methods for 
		* storing and retrieving entity states.
		*/
		RegionCellArchivist(bool edit_mode, const std::string& cache_path = "/cache");
		~RegionCellArchivist();

		void SetInstantiator(EntityInstantiator* instantiator, ComponentFactory* component_factory, EntityManager* manager, ArchetypeFactory* arc_factory);

		void SetSavePath(const std::string& save_path) { m_SavePath = save_path; }
		const std::string& GetSavePath() const { return m_SavePath; }

		std::shared_ptr<GameMap> m_Map;
		void SetMap(const std::shared_ptr<GameMap>& map);

		void UpdateActiveEntityLocation(ObjectID id, const Vector2T<int32_t>& location);
		bool GetActiveEntityLocation(ObjectID id, Vector2T<int32_t>& location);

		void Update(ObjectID id, int32_t new_x, int32_t new_y);

		void Update(ObjectID id, int32_t new_x, int32_t new_y, unsigned char* continuous, size_t con_length, unsigned char* occasional, size_t occ_length)
		{
			Update(id, CellCoord_t(new_x, new_y), std::vector<unsigned char>(continuous, continuous + con_length), std::vector<unsigned char>(occasional, occasional + occ_length));
		}

		void Update(ObjectID id, unsigned char* continuous, size_t con_length, unsigned char* occasional, size_t occ_length)
		{
			Update(id, CellCoord_t(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()), std::vector<unsigned char>(continuous, continuous + con_length), std::vector<unsigned char>(occasional, occasional + occ_length));
		}

		void Update(ObjectID id, const CellCoord_t& expected_location, std::vector<unsigned char>&& continuous, std::vector<unsigned char>&& occasional);

		void Remove(ObjectID id);

		Vector2T<int32_t> GetEntityLocation(ObjectID id);

		//! Stores the given cell
		void Store(int32_t x, int32_t y, std::shared_ptr<Cell> cell);
		//! Retrieves the given cell
		std::shared_ptr<Cell> Retrieve(int32_t x, int32_t y);

		typedef boost::recursive_mutex TransactionMutex_t;

		boost::mutex m_CellsBeingLoadedMutex;

		struct TransactionLock
		{
			TransactionLock(TransactionMutex_t& mutex, CL_Event& ev);
			//TransactionLock(TransactionLock&& other)
			//	: lock(std::move(other.lock)),
			//	endEvent(std::move(other.endEvent))
			//{
			//}
			~TransactionLock();
			TransactionMutex_t::scoped_lock lock;
			CL_Event& endEvent;
		};

		std::unique_ptr<TransactionLock> MakeTransaction();
		void BeginTransaction();
		void EndTransaction();

		boost::thread m_Thread;

		void Start();

		void Stop();

		void GetCellStreamForReading(std::function<void (std::shared_ptr<std::istream>)> callback, int32_t cell_x, int32_t cell_y);
		std::unique_ptr<std::ostream> GetCellStreamForWriting(int32_t cell_x, int32_t cell_y);

		RegionCellCache* GetCellCache() const { return m_Cache; }
		RegionCellCache* GetEditableCellCache() const { return m_EditableCache; }

		void SaveEntityLocationDB(const std::string& filename);

		void EnqueueQuickSave(const std::string& save_name);
		void Save(const std::string& save_name);

		void EnqueueQuickLoad(const std::string& save_name);
		void Load(const std::string& save_name);

		//! Create a file for storing custom data
		std::unique_ptr<std::ostream> CreateDataFile(const std::string& filename);
		//! Load a custom data file
		std::unique_ptr<std::istream> LoadDataFile(const std::string& filename);

		size_t GetDataBegin() const;
		size_t GetDataEnd() const;

		struct WriteJob
		{
			std::weak_ptr<Cell> cell;
			CellCoord_t coord;
			bool unloadWhenDone;

			WriteJob()
				: unloadWhenDone(false)
			{}

			WriteJob(const std::weak_ptr<Cell>& cell_, const CellCoord_t& coord_, const bool unload_when_done)
				: cell(cell_),
				coord(coord_),
				unloadWhenDone(unload_when_done)
			{}

			WriteJob(WriteJob&& other)
				: cell(std::move(other.cell)),
				coord(other.coord),
				unloadWhenDone(other.unloadWhenDone)
			{}

			//WriteJob(WriteJob& other)
			//	: cell(other.cell),
			//	coord(other.coord),
			//	unloadWhenDone(other.unloadWhenDone)
			//{}
		};

		struct ReadJob
		{
			std::weak_ptr<Cell> cell;
			CellCoord_t coord;
			EntitySerialisationUtils::SerialisedDataStyle dataStyle;
			std::shared_ptr<std::istream> cellDataStream;
			std::vector<ObjectID> ids;
			size_t entitiesExpected;
			size_t entitiesReadSoFar; // The count in cell.objects isn't used because some entities may (hope not) fail to instantiate
			bool thereIsSyncedDataToReadNext;
			std::shared_ptr<EntitySerialisationUtils::EntityFuture> entityInTransit;

			// map data comes in a separate cell that can be loaded in parallel, but the cell
			//  isn't done loading until the map is loaded, so the map job has to be tied to the main cell job
			std::shared_ptr<ReadJob> mapSubjob;
			
			ReadJob()
				: dataStyle(EntitySerialisationUtils::FastBinary),
				entitiesExpected(0),
				entitiesReadSoFar(0),
				thereIsSyncedDataToReadNext(false)
			{}

			ReadJob(const std::weak_ptr<Cell>& cell_, const CellCoord_t& coord_)
				: cell(cell_),
				coord(coord_),
				dataStyle(EntitySerialisationUtils::FastBinary),
				entitiesExpected(0),
				entitiesReadSoFar(0),
				thereIsSyncedDataToReadNext(false)
			{}

			ReadJob(ReadJob&& other)
				: cell(std::move(other.cell)),
				coord(other.coord),
				dataStyle(other.dataStyle),
				cellDataStream(std::move(other.cellDataStream)),
				ids(std::move(other.ids)),
				entitiesExpected(other.entitiesExpected),
				entitiesReadSoFar(other.entitiesReadSoFar),
				thereIsSyncedDataToReadNext(other.thereIsSyncedDataToReadNext),
				entityInTransit(std::move(other.entityInTransit))
			{}

		//private:
		//	ReadJob(const ReadJob& other)
		//	{}
		//	ReadJob& operator=(const ReadJob&)
		//	{}
		};

		void OnGotCellStreamForReading(std::shared_ptr<std::istream> cellDataStream, std::shared_ptr<ReadJob> job);

		std::shared_ptr<EntitySerialisationUtils::EntityFuture> LoadEntity(std::shared_ptr<ICellStream> file, bool includes_id, ObjectID id, const EntitySerialisationUtils::SerialisedDataStyle data_style);

		void Run();

	private:
		void StartJob(const std::shared_ptr<ReadJob>& job, const std::shared_ptr<Cell>& a_cell_that_is_locked);
		bool ContinueJob(const std::shared_ptr<ReadJob>& job, const std::shared_ptr<Cell>& the_cell_that_locks);

		void WriteCellIntro(std::ostream& file, const CellCoord_t& coord, const Cell* cell, size_t expectedNumEntries, const bool synched, const EntitySerialisationUtils::SerialisedDataStyle data_style);
		void WriteCellData(std::ostream& file, const CellCoord_t& coord, const Cell* cell, size_t expectedNumEntries, const bool synched, const EntitySerialisationUtils::SerialisedDataStyle data_style);

		void WriteCellDataForEditMode(const std::unique_ptr<std::ostream>& filePtr, const CellCoord_t& cell_coord, const std::shared_ptr<Cell>& cell, size_t numPseudo, size_t numSynched, const EntitySerialisationUtils::SerialisedDataStyle data_style);

		// Reads the number of entities, and optional IDs from the cell data
		//std::pair<size_t, std::vector<ObjectID>> ReadCellIntro(const CellCoord_t& coord, ICellStream& file, const bool data_includes_ids, const EntitySerialisationUtils::SerialisedDataStyle data_style);
		std::tuple<bool, size_t, std::shared_ptr<ICellStream>> ContinueReadingCell(const CellCoord_t& coord, const std::shared_ptr<Cell>& conveniently_locked_cell, size_t num_entities, size_t progress, std::shared_ptr<EntitySerialisationUtils::EntityFuture>& incomming_entity, const std::vector<ObjectID>& ids, std::shared_ptr<ICellStream> file, const EntitySerialisationUtils::SerialisedDataStyle data_style);

		std::streamsize MergeEntityData(std::vector<ObjectID>& objects_displaced, std::vector<ObjectID>& objects_displaced_backward, ObjectID id, std::streamoff data_offset, std::streamsize data_length, ICellStream& source_in, OCellStream& source_out, ICellStream& dest_in, OCellStream& dest, RakNet::BitStream& mergeCon, RakNet::BitStream& mergeOcc) const;
		void MoveEntityData(std::vector<ObjectID>& objects_displaced_backward, ObjectID id, std::streamoff data_offset, std::streamsize data_length, ICellStream& source_in, OCellStream& source_out, ICellStream& dest_in, OCellStream& dest) const;
		void DeleteEntityData(std::vector<ObjectID>& objects_displaced_backward, ObjectID id, std::streamoff data_offset, std::streamsize data_length, ICellStream& in, OCellStream& out) const;

		bool PerformSave(const std::string& save_name);
		void PrepareLoad(const std::string& save_name);
		void PerformLoad(const std::string& save_name);

		void CompressSave(const std::string& save_name);

		bool m_EditMode;
		bool m_Running;
		
		size_t m_RegionSize;

		std::string m_SavePath;

		std::string m_CachePath;
		RegionCellCache* m_Cache;

		RegionCellCache* m_EditableCache;

		RegionCellCache* m_MapCache;

		std::string m_FullBasePath;
		std::unique_ptr<kyotocabinet::HashDB> m_EntityLocationDB;

		std::unique_ptr<kyotocabinet::HashDB> m_SynchLoadedDB;

		std::unordered_set<std::pair<int32_t, int32_t>> m_SynchLoaded;

		typedef tbb::concurrent_hash_map<CellCoord_t, std::shared_ptr<Cell>> CellsBeingProcessedMap_t;
		// Cells who's ownership has been passed to this archiver via Store or created by Retrieve
		CellsBeingProcessedMap_t m_CellsBeingProcessed;

		//TransactionMutex_t m_TransactionMutex;

		std::unique_ptr<ActiveEntityDirectory> m_ActiveEntityDirectory;

		void ClearReadyCells(std::list<CellCoord_t>& readyCells);

		EntityInstantiator* m_Instantiator;
		ComponentFactory* m_Factory;
		EntityManager* m_EntityManager;
		ArchetypeFactory* m_ArchetypeFactory;

		// TODO: implement the no-tbb version
#ifdef FSN_TBB_AVAILABLE
		typedef tbb::concurrent_queue<WriteJob> WriteQueue_t;
		typedef tbb::concurrent_queue<std::shared_ptr<ReadJob>> ReadQueue_t;
#else
		boost::mutex m_WriteQueueMutex;
		boost::mutex m_ReadQueueMutex;
		typedef std::queue<std::tuple<WriteJob> WriteQueue_t;
		typedef std::queue<std::tuple<ReadJob> ReadQueue_t;
#endif
		WriteQueue_t m_WriteQueue;
		ReadQueue_t m_ReadQueueGetCellData;
		ReadQueue_t m_ReadQueueLoadEntities;

		// Cells waiting on archetypes to finish loading (put aside to not hold up the other cells)
		std::list<std::shared_ptr<ReadJob>> m_IncommingCells;

		//! TODO un-caps these when vc++ supports enum class
		enum UpdateOperation { UPDATE, REMOVE };

		// Data required to update an inactive object that has changed on another peer
		struct UpdateJob
		{
			ObjectID id;
			UpdateOperation operation;
			CellCoord_t cellCoord;
			std::shared_ptr<std::istream> existingSourceCellDataStream; // Data from disc
			std::shared_ptr<std::istream> existingDestCellDataStream;
			std::vector<unsigned char> incommingConData; // Data from network
			std::vector<unsigned char> incommingOccData;

			UpdateJob()
			{}

			UpdateJob(ObjectID id_, UpdateOperation operation_, const CellCoord_t& coord_, std::vector<unsigned char>&& incommingConData, std::vector<unsigned char>&& incommingOccData)
				: id(id_),
				operation(operation_),
				cellCoord(coord_),
				incommingConData(incommingConData),
				incommingOccData(incommingOccData)
			{}

			UpdateJob(UpdateJob&& other)
				: id(other.id),
				operation(other.operation),
				cellCoord(other.cellCoord),
				existingSourceCellDataStream(std::move(other.existingSourceCellDataStream)),
				existingDestCellDataStream(std::move(other.existingDestCellDataStream)),
				incommingConData(std::move(other.incommingConData)),
				incommingOccData(std::move(other.incommingOccData))
			{}

		private:
			UpdateJob(UpdateJob&) {}
		};

		tbb::concurrent_queue<std::shared_ptr<UpdateJob>> m_ObjectUpdateQueue;

		tbb::concurrent_queue<std::string> m_SaveQueue;
		boost::mutex m_SaveToLoadMutex;
		std::string m_SaveToLoad;

		CL_Event m_NewData;
		CL_Event m_TransactionEnded;
		CL_Event m_Quit;

#ifdef _DEBUG
		boost::thread::id m_ControllerThreadId;
#endif
	};

}

#endif
