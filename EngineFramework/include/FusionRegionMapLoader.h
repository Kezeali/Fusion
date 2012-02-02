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
#include <unordered_set>
#include <tuple>
#include <tbb/concurrent_queue.h>

#include "FusionCellDataSource.h"
#include "FusionGameMapLoader.h"
#include "FusionPhysFSIOStream.h"
#include "FusionSaveDataArchive.h"
// Only used for ICellStream; TODO: remove
#include "FusionEntitySerialisationUtils.h"

#include "FusionHashable.h"

namespace kyotocabinet
{
	class HashDB;
}

namespace FusionEngine
{

	class RegionCellCache;

	//! CellArchiver implementaion
	// TODO: rename to RegionCellArchivist
	class RegionMapLoader : public CellDataSource, public SaveDataArchive
	{
	public:
		typedef Vector2T<int32_t> CellCoord_t;

		//! Ctor
		/*
		* The map provides a static entity source, whilst the CellArchiver (cache) provides methods for 
		* storing and retrieving entity states.
		*/
		RegionMapLoader(bool edit_mod, const std::string& cache_path = "/cache");
		~RegionMapLoader();

		void SetInstantiator(EntityInstantiator* instantiator, ComponentFactory* component_factory, EntityManager* manager);

		void SetSavePath(const std::string& save_path) { m_SavePath = save_path; }
		const std::string& GetSavePath() const { return m_SavePath; }

		std::shared_ptr<GameMap> m_Map;
		void SetMap(const std::shared_ptr<GameMap>& map);

		void ActiveUpdate(ObjectID id, int32_t new_x, int32_t new_y);

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

		std::unique_ptr<std::istream> GetCellStreamForReading(int32_t cell_x, int32_t cell_y);
		std::unique_ptr<std::ostream> GetCellStreamForWriting(int32_t cell_x, int32_t cell_y);

		RegionCellCache* GetCellCache() const { return m_Cache; }

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

		EntityPtr LoadEntity(ICellStream& file, bool includes_id, ObjectID id);

		size_t LoadEntitiesFromCellData(const CellCoord_t& coord, Cell* cell, ICellStream& file, bool data_includes_ids);

		void WriteCell(std::ostream& file, const CellCoord_t& coord, const Cell* cell, size_t expectedNumEntries, const bool synched);

		void Run();

	private:
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

		std::string m_FullBasePath;
		std::unique_ptr<kyotocabinet::HashDB> m_EntityLocationDB;

		std::unique_ptr<kyotocabinet::HashDB> m_SynchLoadedDB;

		std::unordered_set<std::pair<int32_t, int32_t>> m_SynchLoaded;

		// Cells who's ownership has been passed to this archiver via Store or created by Retrieve
		std::unordered_map<CellCoord_t, std::shared_ptr<Cell>, boost::hash<CellCoord_t>> m_CellsBeingProcessed;

		TransactionMutex_t m_TransactionMutex;

		void ClearReadyCells(std::list<CellCoord_t>& readyCells);

		EntityInstantiator* m_Instantiator;
		ComponentFactory* m_Factory;
		EntityManager* m_EntityManager;

		// TODO: implement the no-tbb version
#ifdef FSN_TBB_AVAILABLE
		typedef tbb::concurrent_queue<std::tuple<std::weak_ptr<Cell>, CellCoord_t, bool>> WriteQueue_t;
		typedef tbb::concurrent_queue<std::tuple<std::weak_ptr<Cell>, CellCoord_t>> ReadQueue_t;
#else
		boost::mutex m_WriteQueueMutex;
		boost::mutex m_ReadQueueMutex;
		typedef std::queue<std::tuple<std::weak_ptr<Cell>, CellCoord_t, bool>> WriteQueue_t;
		typedef std::queue<std::tuple<std::weak_ptr<Cell>, CellCoord_t>> ReadQueue_t;
#endif
		WriteQueue_t m_WriteQueue;
		ReadQueue_t m_ReadQueue;


		//! TODO un-caps these when vc++ supports enum class
		enum UpdateOperation { UPDATE, REMOVE };

		tbb::concurrent_queue<std::tuple<ObjectID, UpdateOperation, CellCoord_t, std::vector<unsigned char>, std::vector<unsigned char>>> m_ObjectUpdateQueue;

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
