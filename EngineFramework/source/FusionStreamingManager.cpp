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
*  File Author:
*    Elliot Hayward
*/

#include "PrecompiledHeaders.h"

#include "FusionStreamingManager.h"

#include "FusionBinaryStream.h"
#include "FusionCellDataSource.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionMaths.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionPlayerRegistry.h"
#include "FusionNetworkManager.h"
#include "FusionRakNetwork.h"
#include "FusionNetDestinationHelpers.h"

#include "FusionLogger.h"

#include <cmath>

namespace FusionEngine
{

	const float StreamingManager::s_SmoothTightness = 0.1f;
	const float StreamingManager::s_FastTightness = 0.3f;

	const float s_DefaultActivationRange = 16.f;
	const float s_DefaultCellSize = 8.f;
	const float s_DefaultWorldSize = 200000.f;

	const float s_DefaultDeactivationTime = 0.1f;

	static const CellHandle s_VoidCellIndex = CellHandle(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max());

#define FSN_CELL_LOG

	// TODO: make this a preprocessor macro that reduces to nothing when FSN_CELL_LOG is false (so params don't get evaluated)
	void AddHist(const CellHandle& loc, const std::string& l, unsigned int n = -1)
	{
#ifdef FSN_CELL_LOG
		std::string name = "cell_log_";
		{
			std::stringstream str; str << loc.x << "." << loc.y;
			name += str.str();
		}
		if (n == -1)
			AddLogEntry(name, l, LOG_INFO);
		else
		{
			std::stringstream str; str << n;
			AddLogEntry(name, l + " (" + str.str() + ")", LOG_INFO);
		}
#endif
	}

	void Cell::AddHist(const std::string& l, unsigned int n)
	{
#ifdef FSN_CELL_HISTORY
		if (!m_Log)
		{
			std::stringstream str; str << loc.x << "." << loc.y;
			m_Log = Logger::OpenLog("cell_log_" + str.str());
		}
		std::string message = l;
		if (n != -1)
		{
			std::stringstream str; str << n;
			message += " (" + str.str() + ")";
		}
		m_Log->AddEntry(message, LOG_INFO);
#endif
	}

	StreamingManager::StreamingManager(CellDataSource* archivist)
		: m_DeactivationTime(s_DefaultDeactivationTime),
		m_Archivist(archivist)
	{
		m_Range = s_DefaultActivationRange;
		m_RangeSquared = m_Range * m_Range;

		m_CellSize = s_DefaultCellSize;
		m_InverseCellSize = 1.f / m_CellSize;

		//NetworkManager::getSingleton().Subscribe(MTID_REMOTECAMERA_ADD, this);
		//NetworkManager::getSingleton().Subscribe(MTID_REMOTECAMERA_REMOVE, this);
		//NetworkManager::getSingleton().Subscribe(MTID_REMOTECAMERA_MOVE, this);
	}

	StreamingManager::~StreamingManager()
	{
		//NetworkManager::getSingleton().Unsubscribe(MTID_REMOTECAMERA_ADD, this);
		//NetworkManager::getSingleton().Unsubscribe(MTID_REMOTECAMERA_REMOVE, this);
		//NetworkManager::getSingleton().Unsubscribe(MTID_REMOTECAMERA_MOVE, this);
		//delete[] m_Cells;
	}

	void StreamingManager::Initialise(float cell_size)
	{
		m_CellSize = cell_size;
		m_InverseCellSize = 1.f / cell_size;

		if (!m_Cells.empty())
		{
			AddLogEntry("Streaming", "Warning: StreamingManager initialised while holding retrieved cells.");
			Reset();
		}
	}

	void StreamingManager::Reset()
	{
		m_Cells.clear();
		m_CellsBeingLoaded.clear();
		m_RequestedEntities.clear();

		m_TheVoid.Reset();

		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);
		m_Cameras.clear();
		m_CamIds.freeAll();
	}

	void StreamingManager::Save(std::ostream& stream)
	{
		IO::Streams::CellStreamWriter writer(&stream);

		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);
		writer.Write(m_Cameras.size());
		for (auto it = m_Cameras.begin(), end = m_Cameras.end(); it != end; ++it)
		{
			auto& cam = *it;
			writer.Write(cam.activeCellRange.left);
			writer.Write(cam.activeCellRange.top);
			writer.Write(cam.activeCellRange.right);
			writer.Write(cam.activeCellRange.bottom);
		}
	}

	void StreamingManager::Load(std::istream& stream)
	{
		FSN_ASSERT_FAIL("Not implemented");
		IO::Streams::CellStreamReader reader(&stream);

		size_t num = 0;
		reader.Read(num);
		for (size_t i = 0; i < num; ++i)
		{
			CL_Rect activeCellRange;
			reader.Read(activeCellRange.left);
			reader.Read(activeCellRange.top);
			reader.Read(activeCellRange.right);
			reader.Read(activeCellRange.bottom);
		}
	}

#ifndef STREAMING_USEMAP
	//! Finds and removes entry for the given Entity from the given cell
	static inline void removeEntityFromCell(Cell* cell, const EntityPtr& entity)
	{
		if (!cell->objects.empty())
		{
			auto newEnd = std::remove_if(cell->objects.begin(), cell->objects.end(), [&](const Cell::EntityEntryPair& pair)->bool {
				return pair.first == entity;
			});
			cell->objects.erase(newEnd);
		}
	}

	//! Finds and removes the given entity from the given cell, starting the search from the end of the list
	static inline void rRemoveEntityFromCell(Cell* cell, const EntityPtr& entity)
	{
		if (cell->objects.size() > 1)
		{
			auto rEntry = std::find_if(cell->objects.rbegin(), cell->objects.rend(), [&](const Cell::EntityEntryPair& pair)->bool {
				return pair.first == entity;
			});
			FSN_ASSERT(rEntry != cell->objects.rend());
			if (rEntry != cell->objects.rend())
				cell->objects.erase((++rEntry).base());
		}
		else if (!cell->objects.empty())
			cell->objects.pop_back();
	}

	//! Finds the entry for the given Entity in the given cell
	static inline Cell::CellEntryMap::iterator findEntityInCell(Cell* cell, const EntityPtr& entity)
	{
		return std::find_if(cell->objects.begin(), cell->objects.end(), [&](const Cell::EntityEntryPair& pair)->bool {
			return pair.first == entity;
		});
	}

	//! Finds the entry for the given Entity in the given cell, starting the search from the end of the list
	static inline Cell::CellEntryMap::iterator rFindEntityInCell(Cell* cell, const EntityPtr& entity)
	{
		auto rWhere = std::find_if(cell->objects.rbegin(), cell->objects.rend(), [&](const Cell::EntityEntryPair& pair)->bool {
			return pair.first == entity;
		});
		FSN_ASSERT(rWhere != cell->objects.rend());
		if (rWhere != cell->objects.rend())
			return rWhere.base() - 1;
		else
			return cell->objects.end();
	}

	//! Creates an entry for the given Entity in the given cell
	static inline CellEntry& createEntry(Cell* cell, const EntityPtr& entity, const CellEntry& copy_entry)
	{
		cell->objects.push_back( std::make_pair(entity, copy_entry) );
		return cell->objects.back().second;
	}

	//! Creates an entry for the given Entity in the given cell
	static inline CellEntry& createEntry(Cell* cell, const EntityPtr& entity)
	{
		return createEntry(cell, entity, CellEntry());
	}
#endif

	StreamingManager::StreamingCamera& StreamingManager::createStreamingCamera(PlayerID owner, const CameraPtr& cam, float range)
	{
		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);

		m_Cameras.push_back(StreamingCamera());
		auto& streamingCam = m_Cameras.back();

		streamingCam.firstUpdate = true;
		streamingCam.tightness = s_SmoothTightness;
		if (cam)
		{
			streamingCam.camera = cam;
			streamingCam.streamPosition = streamingCam.lastPosition = Vector2(ToSimUnits(cam->GetPosition().x), ToSimUnits(cam->GetPosition().y));// = cam->GetSimPosition();
		}
		streamingCam.owner = owner;
		if (range > 0.f)
		{
			streamingCam.range = range;
			streamingCam.rangeSquared = range * range;
		}
		else
		{
			streamingCam.range = m_Range;
			streamingCam.rangeSquared = m_RangeSquared;
			streamingCam.defaultRange = true;
		}

		return streamingCam;
	}

	void StreamingManager::AddCamera(const CameraPtr &cam, float range)
	{
		if (!cam)
			FSN_EXCEPT(InvalidArgumentException, "Tried to add a NULL camera ptr to StreamingManager");

		if (std::any_of(m_Cameras.begin(), m_Cameras.end(), StreamingCamera::HasSameCamera(cam)))
		{
			FSN_EXCEPT(InvalidArgumentException, "Tried to add a camera to StreamingManager that is already being tracked");
		}

		createStreamingCamera(0, cam, range);
	}

	void StreamingManager::AddOwnedCamera(PlayerID owner, const CameraPtr& cam, float range)
	{
		if (!cam)
			FSN_EXCEPT(InvalidArgumentException, "Tried to add a NULL camera ptr to StreamingManager");

		if (std::any_of(m_Cameras.begin(), m_Cameras.end(), StreamingCamera::HasSameCamera(cam)))
		{
			FSN_EXCEPT(InvalidArgumentException, "Tried to add a camera to StreamingManager that is already being tracked");
		}

		auto& streamingCam = createStreamingCamera(owner, cam, range);

		if (owner != 0)
		{
			streamingCam.id = m_CamIds.getFreeID();

			//RakNet::BitStream bs;
			//bs.Write(streamingCam.id);
			//bs.Write(streamingCam.owner);
			//bs.Write(streamingCam.streamPosition.x);
			//bs.Write(streamingCam.streamPosition.y);
			//NetworkManager::GetNetwork()->Send(To::Populace(), false, MTID_REMOTECAMERA_ADD, &bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_VIEWMANAGER);
		}
	}

	void StreamingManager::RemoveCamera(const CameraPtr &cam)
	{
		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);
		auto streamingCameraEntry = std::find_if(m_Cameras.begin(), m_Cameras.end(), StreamingCamera::HasSameCamera(cam));
		if (streamingCameraEntry != m_Cameras.end())
		{
			const auto& streamingCam = *streamingCameraEntry;
			//RakNet::BitStream bs;
			//bs.Write(streamingCam.id);
			//NetworkManager::GetNetwork()->Send(To::Populace(), false, MTID_REMOTECAMERA_REMOVE, &bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_VIEWMANAGER);

			m_Cameras.erase(streamingCameraEntry, m_Cameras.end());
		}
	}

	void StreamingManager::SetRange(float sim_units)
	{
		m_Range = sim_units;
		m_RangeSquared = sim_units * sim_units;

		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);
		for (auto it = m_Cameras.begin(), end = m_Cameras.end(); it != end; ++it)
		{
			auto& cam = *it;
			if (cam.defaultRange)
			{
				cam.range = m_Range;
				cam.rangeSquared = m_RangeSquared;
				cam.firstUpdate = true;
			}
		}
	}

	float StreamingManager::GetRange() const
	{
		return m_Range;
	}

	//CL_Rectf StreamingManager::CalculateActiveArea(ObjectID net_idx) const
	//{
	//	CL_Rectf area;

	//	StreamingCameraMap::const_iterator _where = m_Cameras.find(net_idx);
	//	if (_where != m_Cameras.end())
	//	{
	//		const StreamingCamera &stCam = _where->second;

	//		area.left = stCam.streamPosition.x - m_Range;
	//		area.top = stCam.streamPosition.y - m_Range;
	//		area.right = stCam.streamPosition.x + m_Range;
	//		area.bottom = stCam.streamPosition.y + m_Range;
	//	}

	//	return area;
	//}

	CellHandle StreamingManager::ToCellLocation(float x, float y) const
	{
#if _MSC_VER > 1000
		const int32_t ix = static_cast<int32_t>(std::floorf(x * m_InverseCellSize));
		const int32_t iy = static_cast<int32_t>(std::floorf(y * m_InverseCellSize));
#else
		const int32_t ix = static_cast<int32_t>(std::floor(x * m_InverseCellSize));
		const int32_t iy = static_cast<int32_t>(std::floor(y * m_InverseCellSize));
#endif
		return CellHandle(ix, iy);
	}

	CellHandle StreamingManager::ToCellLocation(const Vector2& position) const
	{
		return ToCellLocation(position.x, position.y);
	}

	Cell *StreamingManager::CellAtCellLocation(const CellHandle& cell_location)
	{
		//return &m_Cells[iy*m_XCellCount+ix];
		auto _where = m_Cells.find(cell_location);
		if (_where != m_Cells.end())
			return _where->second.get();
		else
			return nullptr;
	}

	Cell *StreamingManager::CellAtPosition(float x, float y)
	{
//#ifdef INFINITE_STREAMING
//		Maths::ClampThis(x, -m_Bounds.x, m_Bounds.x);
//		Maths::ClampThis(y, -m_Bounds.y, m_Bounds.y);
//#endif
//		FSN_ASSERT( x >= -m_Bounds.x );
//		FSN_ASSERT( x <= +m_Bounds.x );
//		FSN_ASSERT( y >= -m_Bounds.y );
//		FSN_ASSERT( y <= +m_Bounds.y );
//		unsigned int ix = Maths::Clamp<unsigned int>( (unsigned int)( (x + m_Bounds.x) * m_InverseCellSize ), 0, m_XCellCount - 1 );
//		unsigned int iy = Maths::Clamp<unsigned int>( (unsigned int)( (y + m_Bounds.y) * m_InverseCellSize ), 0, m_YCellCount - 1 );
//		FSN_ASSERT( iy*m_XCellCount+ix < (m_XCellCount * m_YCellCount)/*sizeof(m_Cells)*/ );
		
		return CellAtCellLocation(ToCellLocation(x, y));
	}
	
	Cell *StreamingManager::CellAtPosition(const Vector2 &position)
	{
		return CellAtPosition(position.x, position.y);
	}

	std::pair<CellHandle, Cell*> StreamingManager::CellAndLocationAtPosition(const Vector2& position)
	{
		const auto location = ToCellLocation(position.x, position.y);
		Cell* cell = CellAtCellLocation(location);
		return std::make_pair(std::move(location), cell);
	}

	inline bool isWithinBounds(const Vector2& pos, const Vector2& lb, const Vector2& ub)
	{
		return pos.x > lb.x && pos.y > lb.y && pos.x < ub.x && pos.y < ub.y;
	}

	inline bool runQueryOnObjects(const Cell::CellEntryMap& objects, const std::function<bool (const EntityPtr&)>& fn, const Vector2& lb, const Vector2& ub)
	{
		for (auto it = objects.begin(), end = objects.end(); it != end; ++it)
		{
			const auto& entity = it->first;
			if (it->second.active && isWithinBounds(entity->GetPosition(), lb, ub) && !fn(entity))
				return false;
		}
		return true;
	}

	void StreamingManager::QueryRect(const std::function<bool (const EntityPtr&)>& fn, const Vector2& lb, const Vector2& ub) const
	{
		auto lbLoc = ToCellLocation(lb);
		auto ubLoc = ToCellLocation(ub);

		if (!runQueryOnObjects(m_TheVoid.objects, fn, lb, ub))
			return;

		auto lbEntry = m_Cells.lower_bound(lbLoc);
		for (auto cit = lbEntry, cend = m_Cells.upper_bound(ubLoc); cit != cend; ++cit)
		{
			if (!runQueryOnObjects(cit->second->objects, fn, lb, ub))
				return;
		}
	}

	std::shared_ptr<Cell>& StreamingManager::RetrieveCell(const CellHandle &location)
	{
		auto _where = m_Cells.lower_bound(location);
		if (_where != m_Cells.end() && _where->first == location)
		{
			auto& cell = _where->second;

			// If the held pointer is invalid, the cell is waiting to be stored or the cell is unloaded (and not waiting to be loaded): Retrieve
			if (!cell || cell->waiting == Cell::Store || (!cell->IsLoaded() && cell->waiting == Cell::Ready))
			{
				cell = m_Archivist->Retrieve(location.x, location.y);
				return cell;
			}
			else
				return cell;
		}
		else
		{
			// Retrieve and insert a new cell entry
			auto cell = m_Archivist->Retrieve(location.x, location.y);
			return m_Cells.insert(_where, std::make_pair(location, cell))->second;
		}
	}

	bool StreamingManager::ConfirmRetrieval(const CellHandle &location, Cell* cell)
	{
		if (cell->IsRetrieved())
			return true;
		else
		{
			m_Archivist->Retrieve(location.x, location.y);
			return false;
		}
	}

	inline void StreamingManager::StoreWhenDereferenced(const CellHandle& location)
	{
		auto entry = m_Cells.find(location);
		if (entry != m_Cells.end())
			m_CellsToStore.insert(*entry);
	}

	inline void StreamingManager::StoreWhenDereferenced(const CellHandle& location, const std::shared_ptr<Cell>& cell)
	{
		m_CellsToStore.insert(std::make_pair(location, cell));
	}

	inline void StreamingManager::StoreWhenDereferenced(const CellMap_t::iterator& entry)
	{
		FSN_ASSERT(entry != m_Cells.end());
		m_CellsToStore.insert(*entry);
	}

	inline void StreamingManager::StoreCell(const CellHandle& location)
	{
		StoreCell(m_Cells.find(location));
	}

	inline void StreamingManager::StoreCell(const CellMap_t::iterator& entry)
	{
		m_Archivist->Store(entry->first.x, entry->first.y, std::move(entry->second));
		m_Cells.erase(entry);
	}

	void StreamingManager::StoreAllCells(bool setup_refresh)
	{
		while (!m_TheVoid.objects.empty())
			Update(false);

		for (auto it = m_Cells.begin(), end = m_Cells.end(); it != end; ++it)
		{
			const auto& loc = it->first;
			auto& cell = it->second;
			FSN_ASSERT(cell); // validate the entry

			m_Archivist->Store(loc.x, loc.y, cell);
		}
		//m_Cells.clear();

		if (setup_refresh)
		{
			// This will make the next call to Update do a full refresh
			CamerasMutex_t::scoped_lock lock(m_CamerasMutex);
			for (auto it = m_Cameras.begin(), end = m_Cameras.end(); it != end; ++it)
			{
				it->firstUpdate = true;
			}
		}
	}

	void StreamingManager::AddEntity(const EntityPtr &entity)
	{
		Vector2 entityPosition = entity->GetPosition();
		//entityPosition.x = ToRenderUnits(entityPosition.x); entityPosition.y = ToRenderUnits(entityPosition.y);

		CellHandle cellIndex = ToCellLocation(entityPosition);
		auto cellSpt = RetrieveCell(cellIndex);
		Cell* cell = cellSpt.get();
		CellEntry* entry = nullptr;

		Cell::mutex_t::scoped_try_lock lock(cell->mutex);
		if (lock && cell->IsLoaded())
		{
			entity->SetStreamingCellIndex(cellIndex);

#ifdef STREAMING_USEMAP
			entry = &cell->objects[entity.get()];
#else
			entry = &createEntry(cell, entity/*.get()*/);
#endif
		}
		else
		{
			Cell::mutex_t::scoped_lock voidLock(m_TheVoid.mutex);
			cell = &m_TheVoid;
			entity->SetStreamingCellIndex(s_VoidCellIndex);

#ifdef STREAMING_USEMAP
			entry = &cell->objects[entity.get()];
#else
			entry = &createEntry(cell, entity/*.get()*/);
#endif

			m_CellsBeingLoaded.insert(std::make_pair(cellIndex, cellSpt));
		}

		FSN_ASSERT(entry);
		entry->x = entityPosition.x; entry->y = entityPosition.y;

		activateInView(cellIndex, cell, entry, entity, true);
		if (!entry->active) // activateInView assumes that the entry's current state has been propagated - it hasn't in this case, since the entry was just added
		{
			//cell->EntryReferenced(); // Otherwise the counter will be off when OnDeactivated is called
			ActivateEntity(cellIndex, *cell, entity, *entry);
			QueueEntityForDeactivation(*entry, true);
		}

		//OnUpdated(entity, 0.0f);
	}

//#define STREAMING_AUTOADD

	void StreamingManager::RemoveEntity(const EntityPtr &entity)
	{
		Cell *cell = nullptr;
		if (entity->GetStreamingCellIndex() != s_VoidCellIndex)
		{
			auto cellEntry = m_Cells.find(entity->GetStreamingCellIndex());
			if (cellEntry != m_Cells.end())
			{
				cell = cellEntry->second.get();
			}
		}
		else
		{
			cell = &m_TheVoid;
		}
		if (cell)
		{
			Cell::mutex_t::scoped_lock lock(cell->mutex);

#ifdef STREAMING_USEMAP
			auto _where = cell->objects.find(entity.get());
#else
			auto _where = findEntityInCell(cell, entity/*.get()*/);
#endif
			if (_where != cell->objects.end())
			{
				if (_where->second.active)
					cell->EntryUnreferenced();
				cell->objects.erase(_where);
				AddHist(entity->GetStreamingCellIndex(), "Entry removed due to entity being destroyed");
			}
			else
			{
				AddHist(entity->GetStreamingCellIndex(), "Entry being removed was missing from loaded cell");
			}
		}
		
		m_Archivist->Remove(entity->GetID());
	}

	void StreamingManager::OnUpdated(const EntityPtr &entity, float split)
	{
		FSN_ASSERT(entity);
		const EntityPtr& entityKey = entity;

		// clamp the new position within bounds
		Vector2 newPos = entity->GetPosition();
		float new_x = newPos.x;//fe_clamped(ToRenderUnits(newPos.x), -m_Bounds.x, +m_Bounds.x);
		float new_y = newPos.y;//fe_clamped(ToRenderUnits(newPos.y), -m_Bounds.y, +m_Bounds.y);

		// gather all of the data we need about this object
		Cell *currentCell = nullptr;
		CellEntry *cellEntry = nullptr;
		Cell::CellEntryMap::iterator _where;

		Cell::mutex_t::scoped_lock currentCell_lock;
		if (entity->GetStreamingCellIndex() != s_VoidCellIndex)
		{
			currentCell = CellAtCellLocation(entity->GetStreamingCellIndex());
			FSN_ASSERT(currentCell);
			if (currentCell && currentCell->IsLoaded())
			{
				currentCell_lock = Cell::mutex_t::scoped_lock(currentCell->mutex);
				// TODO: maybe: rather than this (and setting StreamingCellIndex to VoidCellIndex), entities could be implicitly in the void if the cell isn't loaded
				FSN_ASSERT(currentCell->IsLoaded());
#ifdef STREAMING_USEMAP
				_where = currentCell->objects.find(entityKey);
#else
				_where = rFindEntityInCell(currentCell, entityKey);
#endif
				FSN_ASSERT(_where != currentCell->objects.end());
				if (_where != currentCell->objects.end())
					cellEntry = &_where->second;
				else
				{
					FSN_EXCEPT(Exception, "Entity missing from loaded cell.");
				}
			}
			else
			{
				AddLogEntry("Streaming", "The cell that an active object was supposed to be in was missing.", LOG_CRITICAL);
				AddHist(entity->GetStreamingCellIndex(), "This cell was missing during a call to OnUpdate() on an active entity that expected to be here");
				return;
			}
		}
		else// if (entity->GetStreamingCellIndex() == s_VoidCellIndex)
		{
			currentCell = &m_TheVoid;

			currentCell_lock = Cell::mutex_t::scoped_lock(currentCell->mutex);
			_where = rFindEntityInCell(currentCell, entityKey);
			FSN_ASSERT( _where != currentCell->objects.end() );
			cellEntry = &_where->second;
		}
#ifdef STREAMING_AUTOADD
		else // add the entity to the grid automatically
		{
			auto currentLocation = ToCellLocation(new_x, new_y);
			currentCell = CellAtCellLocation(currentLocation);

			currentCell_lock = Cell::mutex_t::scoped_lock(currentCell->mutex);
			currentCell->objects.push_back(std::make_pair(entity, CellEntry()));
			cellEntry = &currentCell->objects.back().second;

			entity->SetStreamingCellIndex(currentLocation);
		}
#endif

		FSN_ASSERT(cellEntry != nullptr);
		FSN_ASSERT(currentCell_lock && currentCell_lock.owns_lock());

		bool move = !fe_fequal(cellEntry->x, new_x, 0.001f) || !fe_fequal(cellEntry->y, new_y, 0.001f);
		const bool warp = diff(cellEntry->x, new_x) > 50.0f || diff(cellEntry->y, new_y) > 50.0f;

		// Move the object, updating the current cell if necessary
		CellHandle newCellLocation = ToCellLocation(new_x, new_y);
		Cell* newCell = RetrieveCell(newCellLocation).get();
		FSN_ASSERT( newCell );
		if (currentCell == newCell)
		{
			// Common case: same cell (just update the stored position)
			// Don't update the stored position if the object hasn't moved far enough to be re-checked for activation
			//  (this way slow moving objects will (hopefully) eventually get re-checked)
			if (move)
			{
				cellEntry->x = new_x;
				cellEntry->y = new_y;
			}
		}
		else
		{
			Cell::mutex_t::scoped_try_lock newCell_lock(newCell->mutex);
			if (!newCell_lock || !newCell->IsLoaded())
			{
				// Since the target cell isn't ready, move the entity into The Void (temporarily)
				newCell = &m_TheVoid;
				newCellLocation = s_VoidCellIndex;

				move = true;
				
				if (currentCell != &m_TheVoid)
				{
					newCell_lock = Cell::mutex_t::scoped_try_lock(m_TheVoid.mutex);
					FSN_ASSERT(newCell_lock.owns_lock());
				}
				else
					newCell_lock = Cell::mutex_t::scoped_try_lock();
			}
			
			{
				AddHist(newCellLocation, "Entry added from another cell");

				// add the entity to its new cell
#ifdef STREAMING_USEMAP
				CellEntry &newEntry = newCell->objects[entityKey];
				newEntry = *cellEntry; // Copy the current cell data
#else
				newCell->objects.emplace_back( std::make_pair(entityKey, *cellEntry) );
				CellEntry &newEntry = newCell->objects.back().second;
#endif
				cellEntry = &newEntry; // Change the pointer (since it is used again below)
			}

			if (cellEntry->active)
				newCell->EntryReferenced();

			/*if (lock.owns_lock())
				lock.unlock();*/

			// remove from current cell
			if (currentCell != nullptr)
			{
				FSN_ASSERT(currentCell_lock);
#ifdef STREAMING_USEMAP
				currentCell->objects.erase(_where);
#else
				rRemoveEntityFromCell(currentCell, entityKey);
#endif
				AddHist(entity->GetStreamingCellIndex(), "Entry moved to another cell");
				if (cellEntry->active)
				{
					currentCell->EntryUnreferenced();
				}
			}

			currentCell = newCell;

			cellEntry->x = new_x;
			cellEntry->y = new_y;

			entity->SetStreamingCellIndex(newCellLocation);
			if (entity->IsSyncedEntity())
				m_Archivist->ActiveUpdate(entity->GetID(), newCellLocation.x, newCellLocation.y);
		}

		// see if the object needs to be activated or deactivated
		if (move && !entity->IsTerrain())
		{
			activateInView(newCellLocation, currentCell, cellEntry, entity, warp);
		}

		if (cellEntry->pendingDeactivation)
		{
			cellEntry->pendingDeactivationTime -= split;
			if (cellEntry->pendingDeactivationTime <= 0.0f)
				DeactivateEntity(*currentCell, entity, *cellEntry);
		}
	}

	static bool findEntityById(EntityPtr& out, CellEntry*& out2, Cell* cell, ObjectID id)
	{
		FSN_ASSERT(cell);

		auto found = std::find_if(cell->objects.begin(), cell->objects.end(), [id](const Cell::EntityEntryPair& entry)
		{
			return entry.first->GetID() == id;
		});
		if (found != cell->objects.end())
		{
			out = found->first;
			out2 = &found->second;
			return true;
		}
		else
			return false;
	}

	void StreamingManager::UpdateInactiveEntity(ObjectID id, const Vector2& position, const std::shared_ptr<RakNet::BitStream>& continuous_data, const std::shared_ptr<RakNet::BitStream>& occasional_data)
	{
		CellHandle location = ToCellLocation(position);
		auto _where = m_Cells.find(location);
		if (_where == m_Cells.end())
		{
			auto con = continuous_data ? continuous_data->GetData() : nullptr;
			auto conLength = continuous_data ? continuous_data->GetNumberOfBytesUsed() : 0;

			auto occ = occasional_data ? occasional_data->GetData() : nullptr;
			auto occLength = occasional_data ? occasional_data->GetNumberOfBytesUsed() : 0;
			
			m_Archivist->Update(id, location.x, location.y,
				con, conLength,
				occ, occLength);
		}
		else
		{
			const auto& cell = _where->second;
			EntityPtr entity; CellEntry* cellEntry;
			if (findEntityById(entity, cellEntry, cell.get(), id))
			{
				if (continuous_data)
					EntitySerialisationUtils::DeserialiseContinuous(*continuous_data, entity, IComponent::All);
				if (occasional_data)
					EntitySerialisationUtils::DeserialiseOccasional(*occasional_data, entity, IComponent::All);
			}
		}
	}

	void StreamingManager::OnUnreferenced(const EntityPtr& entity)
	{
		if (entity->GetStreamingCellIndex() != s_VoidCellIndex)
		{
			const auto& currentCell = m_Cells[entity->GetStreamingCellIndex()];
			FSN_ASSERT(currentCell);

			Cell::mutex_t::scoped_lock lock(currentCell->mutex);

			auto it = findEntityInCell(currentCell.get(), entity);
			FSN_ASSERT(it->second.active != CellEntry::Inactive);
			it->second.active = CellEntry::Inactive;

			currentCell->EntryUnreferenced();

			// TODO: record this entity somehow (std::set or flag) so that an assertion can be made
			//  in OnUpdated to ensure that the entity manager is honest about what it 'unreferences'

			if (!currentCell->IsActive() && !currentCell->inRange)
			{
				//StoreWhenDereferenced(entity->GetStreamingCellIndex(), currentCell);
				AddHist(entity->GetStreamingCellIndex(), "Store Attempted on Deactivation");
				StoreCell(entity->GetStreamingCellIndex());
			}
		}
		else// if (entity->GetStreamingCellIndex() == s_VoidCellIndex)
		{
			auto& currentCell = m_TheVoid;

			Cell::mutex_t::scoped_lock lock(currentCell.mutex);

			auto it = findEntityInCell(&currentCell, entity/*.get()*/);
			FSN_ASSERT(it->second.active != CellEntry::Inactive);
			it->second.active = CellEntry::Inactive;

			currentCell.EntryUnreferenced();
		}
	}

	bool StreamingManager::ActivateEntity(ObjectID id)
	{
		auto loc = m_Archivist->GetEntityLocation(id);
		//auto it = m_EntityDirectory.find(id);
		//if (it != m_EntityDirectory.end())
		{
			if (loc != s_VoidCellIndex)
			{
				auto cell = RetrieveCell(loc);
				Cell::mutex_t::scoped_try_lock lock(cell->mutex);
				if (!lock || !cell->IsLoaded())
				{
					//m_Archivist->Retrieve(loc.x, loc.y);
					m_RequestedEntities[loc].insert(id);
				}
				else
				{
					EntityPtr entity;
					CellEntry* entry = nullptr;
					if (findEntityById(entity, entry, cell.get(), id))
					{
						FSN_ASSERT(entity && entry);
						if (!entry->active)
							//ActivateEntity(cell, entity, *entry);
							m_RequestedEntities[loc].insert(id);
					}
					else
						return false;
				}
				return true;
			}
			else// if (loc == s_VoidCellIndex)
			{
				Cell* cell = &m_TheVoid;
				EntityPtr entity;
				CellEntry* entry = nullptr;
				if (findEntityById(entity, entry, cell, id))
				{
					FSN_ASSERT(entity && entry);
					if (!entry->active)
						ActivateEntity(s_VoidCellIndex, *cell, entity, *entry);
					return true;
				}
			}
		}
		
		return false;
	}

	void StreamingManager::activateInView(const CellHandle& cell_location, Cell *cell, CellEntry *cell_entry, const EntityPtr &entity, bool warp)
	{
		FSN_ASSERT(cell);

		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);

		FSN_ASSERT(cell_entry->x == entity->GetPosition().x); // Make sure the entry is up to date whenever this is called

		Vector2 entityPosition(cell_entry->x, cell_entry->y);// = entity->GetPosition();
		//entityPosition.x = ToRenderUnits(entityPosition.x); entityPosition.y = ToRenderUnits(entityPosition.y);
		if (std::any_of(m_Cameras.begin(), m_Cameras.end(), [&](const StreamingCamera& cam) { return (entityPosition - cam.streamPosition).length() <= cam.range; }))
		{
			if (cell_entry->active != CellEntry::Active)
				ActivateEntity(cell_location, *cell, entity, *cell_entry);
			cell_entry->pendingDeactivation = false;
		}
		else if (cell_entry->active)
		{
			if (!cell_entry->pendingDeactivation)
				QueueEntityForDeactivation(*cell_entry, warp);
		}
	}

	void StreamingManager::ActivateEntity(const CellHandle& location, Cell& cell, const EntityPtr& entity, CellEntry& cell_entry)
	{
		FSN_ASSERT(cell_entry.active != CellEntry::Active);

		if (&cell != &m_TheVoid)
			entity->SetStreamingCellIndex(location);
		//activeObject.cellObjectIndex = cell.GetCellObjectIndex( cellObject );
		cell_entry.pendingDeactivation = false;
		cell_entry.active = CellEntry::Active;

		cell.EntryReferenced();

		GenerateActivationEvent( entity );
	}
	
	void StreamingManager::RemoteActivateEntity(CellEntry& cell_entry, ObjectID entity, PlayerID viewer, std::shared_ptr<RakNet::BitStream> state)
	{
		//FSN_ASSERT(cell_entry.active != CellEntry::Active); // TODO: Maybe not neccessary?

		// Note that this intentionally doesn't increase the ref-count of the local cell, because the cell
		//  is no longer needed now that the state has been extracted and passed to the interested parties

		GenerateRemoteActivationEvent(entity, viewer, state);
	}

	void StreamingManager::DeactivateEntity(const EntityPtr &entity)
	{
		Cell* cell = CellAtCellLocation(entity->GetStreamingCellIndex());
#ifdef STREAMING_USEMAP
		auto _where = cell.objects.find(entity.get());
#else
		auto _where = findEntityInCell(cell, entity/*.get()*/);
#endif
		if (_where == cell->objects.end()) return;
		CellEntry &cellEntry = _where->second;

		DeactivateEntity(*cell, entity, cellEntry);
	}

	void StreamingManager::DeactivateEntity(Cell &cell, const EntityPtr &entity, CellEntry &cell_entry)
	{
		FSN_ASSERT(cell_entry.active);
		cell_entry.active = CellEntry::Waiting;
		cell_entry.pendingDeactivation = false;

		GenerateDeactivationEvent(entity);
	}

	void StreamingManager::QueueEntityForDeactivation(CellEntry &entry, bool warp)
	{
		FSN_ASSERT( !entry.pendingDeactivation );
		entry.pendingDeactivation = true;
		entry.pendingDeactivationTime = warp ? 0.0f : m_DeactivationTime;
	}

	void StreamingManager::GenerateActivationEvent(const EntityPtr &entity)
	{
		ActivationEvent ev;
		ev.type = ActivationEvent::Activate;
		ev.entity = entity;
		SignalActivationEvent(ev);
	}

	void StreamingManager::GenerateDeactivationEvent(const EntityPtr &entity)
	{
		ActivationEvent ev;
		ev.type = ActivationEvent::Deactivate;
		ev.entity = entity;
		SignalActivationEvent(ev);
	}

	void StreamingManager::GenerateRemoteActivationEvent(ObjectID entity, PlayerID viewer, std::shared_ptr<RakNet::BitStream> state)
	{
		RemoteActivationEvent ev;
		ev.type = RemoteActivationEvent::Activate;
		ev.entity = entity;
		ev.viewer = viewer;
		ev.state = state;
		SignalRemoteActivationEvent(ev);
	}

	void StreamingManager::GenerateRemoteDeactivationEvent(ObjectID entity, PlayerID viewer)
	{
		RemoteActivationEvent ev;
		ev.type = RemoteActivationEvent::Deactivate;
		ev.entity = entity;
		ev.viewer = viewer;
		SignalRemoteActivationEvent(ev);
	}

	//const std::set<EntityPtr> &StreamingManager::GetActiveEntities() const
	//{
	//	return m_ActiveEntities;
	//}

	bool StreamingManager::updateStreamingCamera(StreamingManager::StreamingCamera &cam, CameraPtr camera)
	{
		FSN_ASSERT(camera);

		CL_Vec2f camPos = camera->GetPosition();
		camPos *= s_SimUnitsPerGameUnit;
		//Vector2 camPos = camera->GetSimPosition();

		bool pointChanged = false;

		Vector2 velocity(camPos.x - cam.lastPosition.x, camPos.y - cam.lastPosition.y);
		// Only interpolate / anticipate movement if the velocity is significant while not indicating a jump in position
		if (velocity.squared_length() > 0.001f && velocity.squared_length() < m_RangeSquared)
		{
			Vector2 target(camPos.x + velocity.x, camPos.y + velocity.y);

			cam.streamPosition = cam.streamPosition + (target - cam.streamPosition) * cam.tightness;
			pointChanged = true;
		}
		else
		{
			if (!fe_fequal(cam.streamPosition.x, camPos.x) || !fe_fequal(cam.streamPosition.x, camPos.y))
				pointChanged = true;
			cam.streamPosition.set(camPos.x, camPos.y);
		}

		// If the velocity has changed, smooth (over sudden changes in velocity) by adjusting interpolation tightness
		if (!v2Equal(cam.lastVelocity, velocity, 0.1f))
			cam.tightness = s_SmoothTightness;
		else
			cam.tightness += (s_FastTightness - cam.tightness) * 0.01f;

		cam.lastPosition.x = camPos.x; cam.lastPosition.y = camPos.y;
		cam.lastVelocity = velocity;

		return pointChanged;
	}

	void StreamingManager::getCellRange(CL_Rect& range, const Vector2& pos, const float cam_range)
	{
		// Expand range a little bit to make sure all relevant cells are checked
		const auto expandedRange = cam_range + 0.001f;

		range.left = (int)std::floor((pos.x - expandedRange) * m_InverseCellSize);
		range.right = (int)std::ceil((pos.x + expandedRange) * m_InverseCellSize);

		range.top = (int)std::floor((pos.y - expandedRange) * m_InverseCellSize);
		range.bottom = (int)std::ceil((pos.y + expandedRange) * m_InverseCellSize);
	}

	void StreamingManager::deactivateCells(const CL_Rect& inactiveRange)
	{
		int iy = inactiveRange.top;
		int ix = inactiveRange.left;
		//unsigned int i = iy * m_XCellCount + ix;
		//unsigned int stride = m_XCellCount - ( inactiveRange.right - inactiveRange.left + 1 );

		auto it = m_Cells.find(CellHandle(ix, iy));

		for (; iy <= inactiveRange.bottom; ++iy)
		{
			//FSN_ASSERT( iy >= 0 );
			//FSN_ASSERT( iy < m_YCellCount );
			for (ix = inactiveRange.left; ix <= inactiveRange.right; ++ix)
			{
				//FSN_ASSERT( ix >= 0 );
				//FSN_ASSERT( ix < m_XCellCount );
				//FSN_ASSERT( i == iy * m_XCellCount + ix );
				//Cell* cell = m_Cells[i++];

				// NOTE: this is simmilar to the code in Update, but it DOESN'T insert a new
				//  cell if the expected cell is missing:
				// Check that the next stored cell is the next cell expected in the sequence...
				if (it == m_Cells.end() || it->first.x != ix || it->first.y != iy)
				{
					// ... if not, skip to the next cell
					CellHandle expectedLocation(ix, iy);
					// Using lower_bound here means that there is a possibility the cell found will be the NEXT expected cell,
					//  even if it misses this time (the missing cell will be skipped by 'continue'-ing below)
					auto _where = m_Cells.lower_bound(expectedLocation);
					if (_where != m_Cells.end() && _where->first == expectedLocation)
						it = _where;
					else
					{
						it = _where;
						continue;
					}
				}
				auto& cell = it->second;
				

				cell->inRange = false;

				if (cell->IsLoaded())
				{
					if (!cell->IsActive() && !cell->inRange)
					{
						AddHist(CellHandle(ix, iy), "Store Attempted due to leaving range");
						m_Archivist->Store(ix, iy, std::move(cell));
						m_Cells.erase(it++);
						//StoreCell(it++);
						continue;
					}
					else
					{
						// Attempt to access the cell (it will be locked if the archivist is in the process of loading it)
						Cell::mutex_t::scoped_try_lock lock(cell->mutex);
						if (lock)
						{
							for (auto cell_it = cell->objects.begin(), cell_end = cell->objects.end(); cell_it != cell_end; ++cell_it)
							{
								CellEntry &cellEntry = cell_it->second;

								if (cellEntry.active && !cellEntry.pendingDeactivation)
								{
									QueueEntityForDeactivation(cellEntry);
								}
							}
						}
					}
				}
				// Increment the iterator (ready for next iteration) - done
				//  here so it is still available for potential erase() above
				++it;
			}
			//i += stride;
		}
	}

	void StreamingManager::processCell(const CellHandle& cell_location, Cell& cell, const std::list<std::pair<Vector2, float>>& cams, const std::list<std::pair<std::pair<Vector2, float>, PlayerID>>& remote_cams)
	{
		Cell::mutex_t::scoped_try_lock lock(cell.mutex);
		if (lock && cell.IsLoaded())
		{
			//try
			//{
				for (auto cell_it = cell.objects.begin(), cell_end = cell.objects.end(); cell_it != cell_end; ++cell_it)
				{
//#ifdef _DEBUG
//				FSN_ASSERT(std::count_if(cell.objects.begin(), cell.objects.end(), [&](const Cell::EntityEntryPair& p) { return p.first == cell_it->first; }) == 1);
//#endif
					CellEntry &cellEntry = cell_it->second;
					Vector2 entityPosition(cellEntry.x, cellEntry.y);

					// Check if the entry is in range of any cameras, or if it is terrain
					//  Note that this means terrain will only be deactivated when deactivateCells
					//  is called on this cell (a range that includes this cell, to be precise)
					if (cell_it->first->IsTerrain() || std::any_of(cams.begin(), cams.end(),
						[&](const std::pair<Vector2, float>& cam) { return (entityPosition - cam.first).length() <= cam.second; }))
					{
						if (cellEntry.active != CellEntry::Active)
						{
							ActivateEntity(cell_location, cell, cell_it->first/*->shared_from_this()*/, cellEntry);
						}
						else
						{
							cellEntry.pendingDeactivation = false;						
						}
					}
					else
					{
						auto camEntry = std::find_if(remote_cams.begin(), remote_cams.end(),
							[&](const std::pair<std::pair<Vector2, float>, PlayerID>& remote_cam) { return (entityPosition - remote_cam.first.first).length() <= remote_cam.first.second; });
						if (camEntry != remote_cams.end())
						{
							RemoteActivateEntity(cellEntry, cellEntry.id, camEntry->second, cellEntry.data);
						}

						if (cellEntry.active)
						{
							if (!cellEntry.pendingDeactivation)
								QueueEntityForDeactivation(cellEntry);
						}
					}
				}
			//}
			//catch (Exception&)
			//{
			//	cell.mutex.unlock();
			//	throw;
			//}
			//cell.mutex.unlock();
		}
	}

	static bool inclusiveOverlap(const CL_Rect& l, const CL_Rect& r)
	{
		return (r.left <= l.right && r.right >= l.left && r.top <= l.bottom && r.bottom >= l.top);
	}

	template <class T>
	static inline void clipRange(T&& out_it, CL_Rect& inactiveRange, const CL_Rect& activeRange)
	{
		// Partial overlap
		//if (inactiveRange.is_overlapped(activeRange))
		if (inclusiveOverlap(inactiveRange, activeRange))
		{
			// Fully overlapped: inactive range obliterated!
			// Note that this checks if inactive range is inside active active
			//  range, despite what the syntax of the statement suggests (oh, ClanLib -_-)
			if (activeRange.is_inside(inactiveRange))
				return;

			enum RangeSide : size_t
			{
				Left = 0, Top, Right, Bottom
			};

			std::array<CL_Rect, 4> clippedInactiveRange;
			clippedInactiveRange.fill(inactiveRange);

			// + 1 / - 1 are due to the ranges being used inclusively when processed

			clippedInactiveRange[Top].bottom = activeRange.top - 1;
			clippedInactiveRange[Bottom].top = activeRange.bottom + 1;

			clippedInactiveRange[Left].right = activeRange.left - 1;
			clippedInactiveRange[Right].left = activeRange.right + 1;

			// Exclude cells from the side ranges that are covered by the top & bottom ranges
			clippedInactiveRange[Left].top = clippedInactiveRange[Right].top = activeRange.top;
			clippedInactiveRange[Left].bottom = clippedInactiveRange[Right].bottom = activeRange.bottom;

			for (auto it = clippedInactiveRange.begin(), end = clippedInactiveRange.end(); it != end; ++it)
			{
				// Output the ranges that still include cells
				if (it->get_width() >= 0 && it->get_height() >= 0)
				{
					FSN_ASSERT(!inclusiveOverlap(*it, activeRange));
					*out_it++ = *it;
				}
			}
		}
		// No overlap
		else
		{
			*out_it++ = inactiveRange;
		}
	}

	//void mergeRange(std::list<std::tuple<CL_Rect, std::list<Vector2>, std::list<std::pair<Vector2, PlayerID>>>>& activeRanges, CL_Rect& new_activeRange, StreamingManager::StreamingCamera& cam, const bool localCam)
	//{
	//	bool merged = false;
	//	for (auto it = activeRanges.begin(), end = activeRanges.end(); it != end; ++it)
	//	{
	//		auto& existingRange = std::get<0>(*it);
	//		//if (activeRange.is_overlapped(existingRange))
	//		if (inclusiveOverlap(new_activeRange, existingRange))
	//		{
	//			existingRange.bounding_rect(activeRange);
	//			if (localCam)
	//				std::get<1>(*it).push_back(cam.streamPosition);
	//			else
	//				std::get<2>(*it).push_back(std::make_pair(cam.streamPosition, cam.owner));
	//		}
	//	}
	//	if (!merged)
	//	{
	//		// Create a new list with just this position (this will be appended if this region is merged)
	//		std::list<Vector2> localPosList;
	//		std::list<std::pair<Vector2, PlayerID>> remotePosList;
	//		if (localCam)
	//			localPosList.push_back(cam.streamPosition);
	//		else
	//			remotePosList.push_back(std::make_pair(cam.streamPosition, cam.owner));
	//		activeRanges.push_back(std::make_tuple(cam.streamPosition, std::move(localPosList), std::move(remotePosList)));
	//	}
	//}

	void StreamingManager::Update(const bool refresh)
	{
		// TODO: Don't check the void every frame, wait a few milis in between queries to give the loader a chance
		//  Also don't check requested entities every frame 
		// Try to clear The Void
		{
			Cell::mutex_t::scoped_try_lock lock(m_TheVoid.mutex);
			if (lock)
			{
				for (auto it = m_TheVoid.objects.begin(), end = m_TheVoid.objects.end(); it != end;/* ++it*/)
				{
					auto location = ToCellLocation(it->second.x, it->second.y);
					auto actualCell = RetrieveCell(location);

					if (!actualCell->IsLoaded())
					{
						//Cell::mutex_t::scoped_try_lock lock(actualCell->mutex);
						//if (lock && !actualCell->IsLoaded())
						//{
							m_CellsBeingLoaded.insert(std::make_pair(std::move(location), std::move(actualCell)));
						//}
					}
					else
					{
						Cell::mutex_t::scoped_try_lock lock(actualCell->mutex);
						if (lock && actualCell->IsLoaded())
						{
							auto entity = it->first;
							// TODO:
							//changeCell(*it, m_TheVoid, *cell);
							// OR
							// m_TheVoid.RemoveEntry(it); <- one of the overloads will take an iterator, the others will take index / entry
							// cell->AddEntry(*it);

							actualCell->objects.push_back( std::move(*it) );
							auto& newEntry = actualCell->objects.back().second;

							if (newEntry.active)
								actualCell->EntryReferenced();
							else if (!actualCell->IsActive())
							{
								AddHist(location, "Storing cell again after Retrieving it for an entity spawned in The Void");
								m_Archivist->Store(location.x, location.y, std::move(actualCell));
								m_Cells.erase(location);
								//StoreCell(location);
							}

							// remove from current cell
							{
								it = m_TheVoid.objects.erase(it);
								end = m_TheVoid.objects.end();
								if (newEntry.active)
								{
									AddHist(s_VoidCellIndex, "Entry transferred to correct cell:");
									m_TheVoid.EntryUnreferenced();
								}
							}

							entity->SetStreamingCellIndex(location);
							continue;
						}
					}
					++it;
				}
			}
		}

		std::vector<ObjectID> failedRequests;
		// Check for requested cells that have finished loading
		for (auto it = m_RequestedEntities.begin(), end = m_RequestedEntities.end(); it != end;)
		{
			auto& cellLocation = it->first;

			FSN_ASSERT(m_Cells.find(cellLocation) != m_Cells.end());
			auto cell = CellAtCellLocation(cellLocation);

			if (cell->IsLoaded())
			{
				Cell::mutex_t::scoped_try_lock lock(cell->mutex);
				if (lock && cell->IsLoaded())
				{
					auto& requestedIds = it->second;
					for (auto eit = cell->objects.begin(), eend = cell->objects.end(); eit != eend; ++eit)
					{
						if (requestedIds.count(eit->first->GetID()) != 0)
						{
							requestedIds.erase(eit->first->GetID());
							if (!eit->second.active)
							{
								ActivateEntity(cellLocation, *cell, eit->first, eit->second);
								if (!cell->inRange)
									QueueEntityForDeactivation(eit->second, true);
							}
						}
					}
					// List requested entities that weren't found in the expected cell
					for (auto rit = requestedIds.begin(), rend = requestedIds.end(); rit != rend; ++rit)
					{
						if (m_Archivist->GetEntityLocation(*rit) != cellLocation) // Location changed
						{
							failedRequests.push_back(*rit);
						}
						else // Missing altogether (listed in DB, but entry is inaccurate)
						{
							std::stringstream str; str << "ObjectID [" << *rit << "] was expected in cell [" << cellLocation.x << "," << cellLocation.y << "]";
							AddLogEntry("Streaming", "Failed to load requested entity (missing from cell data): " + str.str(), LOG_CRITICAL);
							m_Archivist->Remove(*rit);
						}
					}
					it = m_RequestedEntities.erase(it);
					end = m_RequestedEntities.end();
					continue;
				} // if (lock)
			} // if (loaded)
			++it;
		}
		// Retry any entities that werent present in this cell anymore (they were there when the cell was requested, but were moved before it was loaded)
		for (auto it = failedRequests.begin(), end = failedRequests.end(); it != end; ++it)
			ActivateEntity(*it);

		for (auto it = m_CellsToStore.begin(), end = m_CellsToStore.end(); it != end;)
		{
			StoreCell(it->first);
		}

		// Each vector element represents a range of cells to update - overlapping areas
		//  are split / merged into no-overlapping sub-regions (to avoid processing cells
		//  twice in a step)
		std::list<CL_Rect> inactiveRanges;
		typedef std::pair<Vector2, float> StreamPosition_t;
		typedef std::list<StreamPosition_t> LocalStreamPositionsList_t;
		typedef std::list<std::pair<StreamPosition_t, PlayerID>> RemoteStreamPositionsList_t;
		std::list<std::tuple<CL_Rect, LocalStreamPositionsList_t, RemoteStreamPositionsList_t>> activeRanges;
		// Used to process The Void
		LocalStreamPositionsList_t allLocalStreamPositions;
		RemoteStreamPositionsList_t allRemoteStreamPositions;

		bool allActiveRangesStale = true;

		{
		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);
		auto it = m_Cameras.begin();
		while (it != m_Cameras.end())
		{
			StreamingCamera &cam = *it;

			// Update streaming position of this camera
			{
				CameraPtr camera = cam.camera.lock();
				if (!camera)
				{
					it = m_Cameras.erase(it); // Remove expired (local) cameras
					continue;
				}
				++it;

				if (camera)
					updateStreamingCamera(cam, std::move(camera));
			}
			const Vector2 &newPosition = cam.streamPosition;
			
			const bool localCam = cam.owner == 0 || PlayerRegistry::IsLocal(cam.owner);

			auto mergeRange = [&](CL_Rect& new_activeRange)
			{
				bool merged = false;
				for (auto it = activeRanges.begin(), end = activeRanges.end(); it != end; ++it)
				{
					auto& existingRange = std::get<0>(*it);
					//if (activeRange.is_overlapped(existingRange))
					if (inclusiveOverlap(new_activeRange, existingRange))
					{
						merged = true;
						existingRange.bounding_rect(new_activeRange);
						if (localCam)
							std::get<1>(*it).push_back(std::make_pair(cam.streamPosition, cam.range));
						else
							std::get<2>(*it).push_back(std::make_pair(std::make_pair(cam.streamPosition, cam.range), cam.owner));
					}
				}
				if (!merged)
				{
					// Create a new list with just this position (this will be appended if this region is merged)
					std::list<std::pair<Vector2, float>> localPosList;
					std::list<std::pair<std::pair<Vector2, float>, PlayerID>> remotePosList;
					if (localCam)
						localPosList.push_back(std::make_pair(cam.streamPosition, cam.range));
					else
						remotePosList.push_back(std::make_pair(std::make_pair(cam.streamPosition, cam.range), cam.owner));
					activeRanges.push_back(std::make_tuple(new_activeRange, std::move(localPosList), std::move(remotePosList)));
				}
			};

			// Figure out the active range of this camera
			if (refresh || cam.firstUpdate || !v2Equal(cam.lastUsedPosition, cam.streamPosition, 0.1f))
			{
				Vector2 oldPosition = cam.lastUsedPosition;
				cam.lastUsedPosition = newPosition;
				cam.firstUpdate = false;

				allActiveRangesStale = false;

				// List this camera position (for processing The Void)
				if (localCam)
					allLocalStreamPositions.push_back(std::make_pair(cam.streamPosition, cam.range));
				else
					allRemoteStreamPositions.push_back(std::make_pair(std::make_pair(cam.streamPosition, cam.range), cam.owner));

				// Update the (in)active cell ranges for this camera
				CL_Rect inactiveRange;
				// Use the previously calculated range if it seems to be correct:
				if (cam.activeCellRange.contains(CL_Vec2f(oldPosition.x, oldPosition.y)))
					inactiveRange = cam.activeCellRange;
				else // Recalculate the inactive cell range
					getCellRange(inactiveRange, oldPosition, cam.range);
				
				CL_Rect& activeRange = cam.activeCellRange;
				getCellRange(activeRange, newPosition, cam.range);

				if (inactiveRange != activeRange && (inactiveRange.get_width() != 0 && inactiveRange.get_height() != 0))
					inactiveRanges.push_back(std::move(inactiveRange));

				// TODO: don't create one big range for all overlapping ranges: split ranges like
				//  when an inactive range overlaps an active range
				mergeRange(activeRange);
			}
			else // Camera hasn't moved
			{
				// Add this cell's most recently calculated active range to make sure it doesn't get deactivated
				if (localCam)
					allLocalStreamPositions.push_back(std::make_pair(cam.lastUsedPosition, cam.range));
				else
					allRemoteStreamPositions.push_back(std::make_pair(std::make_pair(cam.lastUsedPosition, cam.range), cam.owner));
				
				mergeRange(cam.activeCellRange);
			}
		}
		}

		// TODO: seperate activeRanges and staleActiveRanges? (processCell on activeRanges, check m_CellsBeingLoaded for cells to process on staleActiveRanges)
		// This set is just used to make sure cells are processed if they finish loading after the camera that requested them stops moving (see else clause below):
		if (!allActiveRangesStale)
		{
			std::list<CL_Rect> clippedInactiveRanges;
			{
				// Clip inactive ranges against all active ranges
				std::deque<CL_Rect> toProcess(inactiveRanges.begin(), inactiveRanges.end());
				while (!toProcess.empty())
				{
					CL_Rect inactiveRange = toProcess.back();
					toProcess.pop_back();
					bool hasOverlap = false;
					for (auto ait = activeRanges.begin(), aend = activeRanges.end(); ait != aend; ++ait)
					{
						auto& activeRange = std::get<0>(*ait);
						if (inclusiveOverlap(inactiveRange, activeRange))
						{
							hasOverlap = true;
							clipRange(std::back_inserter(toProcess), inactiveRange, activeRange);
							break;
						}
					}
					if (!hasOverlap)
						clippedInactiveRanges.push_back(inactiveRange);
				}
			}

			// Sort to improve cache performance
			clippedInactiveRanges.sort([this](const CL_Rect& a, const CL_Rect& b)
			{
				return (a.top != b.top) ? (a.top < b.top) : (a.left < b.left);
			});
			// TODO: merge adjcent ranges

			for (auto it = clippedInactiveRanges.begin(), end = clippedInactiveRanges.end(); it != end; ++it)
			{
				//std::stringstream inactivestr;
				//inactivestr << it->left << ", " << it->top << ", " << it->right << ", " << it->bottom;
				//SendToConsole("Deactivated " + inactivestr.str());
#ifdef _DEBUG
				// Make sure there is no overlap (overlapping ranges should have been clipped above)
				const bool noOverlap = std::all_of(activeRanges.begin(), activeRanges.end(),
					[it](const std::tuple<CL_Rect, std::list<std::pair<Vector2, float>>, std::list<std::pair<std::pair<Vector2, float>, PlayerID>>>& v)
				{ return !inclusiveOverlap(*it, std::get<0>(v)); });
				FSN_ASSERT(noOverlap);
#endif
				deactivateCells(*it);
			}

			processCell(s_VoidCellIndex, m_TheVoid, allLocalStreamPositions, allRemoteStreamPositions);

			for (auto it = activeRanges.begin(), end = activeRanges.end(); it != end; ++it)
			{
				auto& activeRange = std::get<0>(*it);
				auto& streamPositions = std::get<1>(*it);
				auto& remotePositions = std::get<2>(*it);

				if (activeRange.get_width() >= 0 && activeRange.get_height() >= 0)
				{
					//if (!clippedInactiveRanges.empty())
					//{
					//	std::stringstream activestr;
					//	activestr << activeRange.left << ", " << activeRange.top << ", " << activeRange.right << ", " << activeRange.bottom;
					//	SendToConsole("Activated " + activestr.str());
					//}

					int iy = activeRange.top;
					int ix = activeRange.left;

					auto it = m_Cells.find(CellHandle(ix, iy));

					Cell::mutex_t::scoped_lock lock(m_TheVoid.mutex); // So m_CellsBeingLoaded can be accessed safely

					for (; iy <= activeRange.bottom; ++iy)
					{
						for (ix = activeRange.left; ix <= activeRange.right; ++ix)
						{
							// Check that the next stored cell is the next cell expected in the sequence...
							if (it == m_Cells.end() || it->first.x != ix || it->first.y != iy)
							{
								// ... if not, find the next cell or create a new cell to load
								CellHandle expectedLocation(ix, iy);
								auto _where = m_Cells.lower_bound(expectedLocation);
								if (_where != m_Cells.end() && _where->first == expectedLocation)
									it = _where;
								else
								{
									auto newCell = m_Archivist->Retrieve(expectedLocation.x, expectedLocation.y);
									auto result = m_Cells.insert(_where, std::make_pair(std::move(expectedLocation), std::move(newCell)));
									it = result;

									AddHist(expectedLocation, "Retrieved due to entering range");
									
									m_CellsBeingLoaded.insert(*it);
								}
							}

							Cell* cell = it->second.get();

							cell->inRange = true;

							// Check if the cell needs to be loaded
							if (cell->IsLoaded())
							{
								// Attempt to access the cell (it will be locked if the archivist is in the process of loading it)
								processCell(it->first, *cell, streamPositions, remotePositions);
							}
							// Failsafe (in case this cell was indexed while in some broken state)
							else if (cell->waiting != Cell::Retrieve)
							{
								AddLogEntry("Streaming", "Cell loading failed to work as expected; cell had to be re-retrieved.", LOG_CRITICAL);
								it->second = m_Archivist->Retrieve(ix, iy);
							}
							// Go to the next cell in the sequence
							++it;
						}
						//i += stride;
					}

				}
			}
		}
		else // All active ranges stale
		{
			// Clear loaded cells
			Cell::mutex_t::scoped_try_lock lock(m_TheVoid.mutex); // TheVoid's mutex is used to lock m_CellsBeingLoaded
			if (lock)
			for (auto it = m_CellsBeingLoaded.begin(), end = m_CellsBeingLoaded.end(); it != end;)
			{
				auto cell = it->second;
				if (cell->IsLoaded())
				{
					processCell(it->first, *cell, allLocalStreamPositions, allRemoteStreamPositions);
					it = m_CellsBeingLoaded.erase(it);
					end = m_CellsBeingLoaded.end();
				}
				else
					++it;
			}
		}
	}

	void StreamingManager_AddCamera(CameraPtr camera, StreamingManager *obj)
	{
		obj->AddCamera(camera);
	}

	void StreamingManager_RemoveCamera(CameraPtr camera, StreamingManager *obj)
	{
		obj->RemoveCamera(camera);
	}

	void StreamingManager::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSingletonType<StreamingManager>("StreamingManager", engine);
		r = engine->RegisterObjectMethod("StreamingManager",
			"void addCamera(Camera)",
			asFUNCTION(StreamingManager_AddCamera), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("StreamingManager",
			"void addOwnedCamera(PlayerID, Camera)",
			asMETHOD(StreamingManager, AddOwnedCamera), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("StreamingManager",
			"void removeCamera(Camera)",
			asFUNCTION(StreamingManager_RemoveCamera), asCALL_CDECL_OBJLAST);
	}

}
