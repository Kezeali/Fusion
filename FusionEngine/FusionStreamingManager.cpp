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

#include "FusionStableHeaders.h"

#include "FusionStreamingManager.h"

#include "FusionMaths.h"
#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{

	const float StreamingManager::s_SmoothTightness = 0.1f;
	const float StreamingManager::s_FastTightness = 0.3f;

	const float s_DefaultActivationRange = 1500.f;
	const float s_DefaultCellSize = 500.f;
	const float s_DefaultWorldSize = 200000.f;

	const float s_DefaultDeactivationTime = 0.1f;

	StreamingManager::StreamingManager(CellArchiver* archivist)
		: m_DeactivationTime(s_DefaultDeactivationTime),
		m_Archivist(archivist)
	{
		m_Bounds.x = s_DefaultWorldSize / 2.f;
		m_Bounds.y = s_DefaultWorldSize / 2.f;

		m_Range = s_DefaultActivationRange;
		m_RangeSquared = m_Range * m_Range;

		m_CellSize = s_DefaultCellSize;
		m_InverseCellSize = 1.f / m_CellSize;

		m_XCellCount = (size_t)(m_Bounds.x * 2.0f * m_InverseCellSize) + 1;
		m_YCellCount = (size_t)(m_Bounds.y * 2.0f * m_InverseCellSize) + 1;

		m_Cells = new Cell[m_XCellCount * m_YCellCount];
	}

	StreamingManager::~StreamingManager()
	{
		delete[] m_Cells;
	}

#ifndef STREAMING_USEMAP
	//! Finds and removes entry for the given Entity from the given cell
	static inline void removeEntityFromCell(Cell* cell, const Entity* const entity)
	{
		auto newEnd = std::remove_if(cell->objects.begin(), cell->objects.end(), [entity](const Cell::EntityEntryPair& pair)->bool {
			return pair.first == entity;
		});
		cell->objects.erase(newEnd);
	}

	//! Finds and removes the given entity from the given cell, starting the search from the end of the list
	static inline void rRemoveEntityFromCell(Cell* cell, const Entity* const entity)
	{
		auto rNewEnd = std::remove_if(cell->objects.rbegin(), cell->objects.rend(), [entity](const Cell::EntityEntryPair& pair)->bool {
			return pair.first == entity;
		});
		cell->objects.erase((++rNewEnd).base());
	}

	//! Finds the entry for the given Entity in the given cell
	static inline Cell::CellEntryMap::iterator findEntityInCell(Cell* cell, const Entity* const entity)
	{
		return std::find_if(cell->objects.begin(), cell->objects.end(), [entity](const Cell::EntityEntryPair& pair)->bool {
			return pair.first == entity;
		});
	}

	//! Finds the entry for the given Entity in the given cell, starting the search from the end of the list
	static inline Cell::CellEntryMap::iterator rFindEntityInCell(Cell* cell, const Entity* const entity)
	{
		auto rWhere = std::find_if(cell->objects.rbegin(), cell->objects.rend(), [&entity](const Cell::EntityEntryPair& pair)->bool {
			return pair.first == entity;
		});
		return (++rWhere).base();
	}

	//! Creates an entry for the given Entity in the given cell
	static inline CellEntry& createEntry(Cell* cell, Entity* const entity, const CellEntry& copy_entry)
	{
		cell->objects.emplace_back( std::make_pair(entity, copy_entry) );
		return cell->objects.back().second;
	}

	//! Creates an entry for the given Entity in the given cell
	static inline CellEntry& createEntry(Cell* cell, Entity* const entity)
	{
		return createEntry(cell, entity, CellEntry());
	}
#endif

	void StreamingManager::AddCamera(const CameraPtr &cam)
	{
		if (std::any_of(m_Cameras.begin(), m_Cameras.end(), StreamingCamera::IsObserver(cam)))
		{
			FSN_EXCEPT(InvalidArgumentException, "Tried to add a camera to StreamingManager that is already being tracked");
		}

		m_Cameras.push_back(StreamingCamera());
		auto& streamingCam = m_Cameras.back();

		streamingCam.firstUpdate = true;
		streamingCam.camera = cam;
		streamingCam.lastPosition = Vector2(cam->GetPosition().x, cam->GetPosition().y);
		streamingCam.streamPosition = streamingCam.lastPosition;
		streamingCam.tightness = s_SmoothTightness;
	}

	void StreamingManager::RemoveCamera(const CameraPtr &cam)
	{
		auto new_end = std::remove_if(m_Cameras.begin(), m_Cameras.end(), StreamingCamera::IsObserver(cam));
		m_Cameras.erase(new_end, m_Cameras.end());
	}

	void StreamingManager::SetRange(float game_units)
	{
		m_Range = game_units;
		m_RangeSquared = game_units * game_units;
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

	Cell *StreamingManager::CellAtPosition(float x, float y)
	{
#ifdef INFINITE_STREAMING
		Maths::ClampThis(x, -m_Bounds.x, m_Bounds.x);
		Maths::ClampThis(y, -m_Bounds.y, m_Bounds.y);
#endif
		FSN_ASSERT( x >= -m_Bounds.x );
		FSN_ASSERT( x <= +m_Bounds.x );
		FSN_ASSERT( y >= -m_Bounds.y );
		FSN_ASSERT( y <= +m_Bounds.y );
		unsigned int ix = Maths::Clamp<unsigned int>( (unsigned int)( (x + m_Bounds.x) * m_InverseCellSize ), 0, m_XCellCount - 1 );
		unsigned int iy = Maths::Clamp<unsigned int>( (unsigned int)( (y + m_Bounds.y) * m_InverseCellSize ), 0, m_YCellCount - 1 );
		FSN_ASSERT( iy*m_XCellCount+ix < (m_XCellCount * m_YCellCount)/*sizeof(m_Cells)*/ );
		return &m_Cells[iy*m_XCellCount+ix];
	}
	
	Cell *StreamingManager::CellAtPosition(const Vector2 &position)
	{
		return CellAtPosition(position.x, position.y);
	}

	void StreamingManager::AddEntity(const EntityPtr &entity)
	{
		Vector2 entityPosition = entity->GetPosition();
		entityPosition.x = ToGameUnits(entityPosition.x); entityPosition.y = ToGameUnits(entityPosition.y);
		Cell *cell = CellAtPosition(entityPosition);
		tbb::mutex::scoped_lock lock(cell->mutex);
#ifdef STREAMING_USEMAP
		CellEntry &entry = cell->objects[entity.get()];
#else
		CellEntry &entry = createEntry(cell, entity.get());
#endif
		entry.x = entityPosition.x; entry.y = entityPosition.y;
		entity->SetStreamingCellIndex((size_t)(cell - m_Cells));

		activateInView(cell, &entry, entity, true);
		if (!entry.active) // activateInView assumes that the entry's current state has been propagated - it hasn't in this case, since the entry was just added
			QueueEntityForDeactivation(entry, true);

		//OnUpdated(entity, 0.0f);
	}

//#define STREAMING_AUTOADD

	void StreamingManager::RemoveEntity(const EntityPtr &entity)
	{
		FSN_ASSERT(entity->GetStreamingCellIndex() < (m_XCellCount * m_YCellCount)/*sizeof(m_Cells)*/);
		Cell *cell = &m_Cells[entity->GetStreamingCellIndex()];
#ifdef STREAMING_USEMAP
		cell->objects.erase(entity.get());
#else
		removeEntityFromCell(cell, entity.get());
#endif
	}

	void StreamingManager::OnUpdated(const EntityPtr &entity, float split)
	{
		FSN_ASSERT(entity);
		Entity* entityKey = entity.get();

		// clamp the new position within bounds
		Vector2 newPos = entity->GetPosition();
		float new_x = fe_clamped(ToGameUnits(newPos.x), -m_Bounds.x, +m_Bounds.x);
		float new_y = fe_clamped(ToGameUnits(newPos.y), -m_Bounds.y, +m_Bounds.y);

		// gather all of the data we need about this object
		Cell *currentCell = nullptr;
		CellEntry *cellEntry = nullptr;
		Cell::CellEntryMap::iterator _where;

		if (entity->GetStreamingCellIndex() < (m_XCellCount * m_YCellCount)/*sizeof(m_Cells)*/)
		{
			currentCell = &m_Cells[entity->GetStreamingCellIndex()];

			tbb::mutex::scoped_lock lock(currentCell->mutex);
#ifdef STREAMING_USEMAP
			_where = currentCell->objects.find(entityKey);
#else
			_where = rFindEntityInCell(currentCell, entityKey);
#endif
			FSN_ASSERT( _where != currentCell->objects.end() );
			cellEntry = &_where->second;
		}
#ifdef STREAMING_AUTOADD
		else // add the entity to the grid automatically
		{
			currentCell = CellAtPosition(new_x, new_y);

			tbb::mutex::scoped_lock lock(currentCell->mutex);
			currentCell->objects.push_back(std::make_pair(entity.get(), CellEntry()));
			cellEntry = &currentCell->objects.back().second;

			entity->SetStreamingCellIndex((size_t)(currentCell - m_Cells));
		}
#endif

		FSN_ASSERT( cellEntry != nullptr );

		bool move = !fe_fequal(cellEntry->x, new_x) || !fe_fequal(cellEntry->y, new_y);
		bool warp = diff(cellEntry->x, new_x) > 50.0f || diff(cellEntry->y, new_y) > 50.0f;

		// move the object, updating the current cell if necessary
		Cell *newCell = CellAtPosition(new_x, new_y);
		FSN_ASSERT( newCell );
		if ( currentCell == newCell )
		{
			// common case: same cell
			cellEntry->x = new_x;
			cellEntry->y = new_y;
		}
		else
		{
			tbb::mutex::scoped_lock lock(newCell->mutex);

			// add the entity to its new cell
#ifdef STREAMING_USEMAP
			CellEntry &newEntry = newCell->objects[entityKey];
			newEntry = *cellEntry; // Copy the current cell data
#else
			newCell->objects.emplace_back( std::make_pair(entityKey, *cellEntry) );
			CellEntry &newEntry = newCell->objects.back().second;
#endif
			cellEntry = &newEntry; // Change the pointer (since it is used again below)

			if (cellEntry->active)
				newCell->EntryActivated();

			// remove from current cell
			if (currentCell != nullptr)
			{
				tbb::mutex::scoped_lock lock(currentCell->mutex);
#ifdef STREAMING_USEMAP
				currentCell->objects.erase(_where);
#else
				rRemoveEntityFromCell(currentCell, entityKey);
#endif
				if (cellEntry->active)
				{
					currentCell->EntryDeactivated();
				}
			}

			currentCell = newCell;
			//m_EntityToCellIndex[entity] = (size_t)(currentCell - m_Cells);

			cellEntry->x = new_x;
			cellEntry->y = new_y;

			entity->SetStreamingCellIndex((size_t)(currentCell - m_Cells));
		}

		// see if the object needs to be activated or deactivated
		if (move)
			activateInView(currentCell, cellEntry, entity, warp);

		if (cellEntry->pendingDeactivation)
		{
			cellEntry->pendingDeactivationTime -= split;
			if (cellEntry->pendingDeactivationTime <= 0.0f)
				DeactivateEntity(*currentCell, entity, *cellEntry);
		}
	}

	void StreamingManager::activateInView(Cell *cell, CellEntry *cell_entry, const EntityPtr &entity, bool warp)
	{
		FSN_ASSERT(cell);

		Vector2 entityPosition = entity->GetPosition();
		entityPosition.x = ToGameUnits(entityPosition.x); entityPosition.y = ToGameUnits(entityPosition.y);
		for (auto it = m_Cameras.begin(), end = m_Cameras.end(); it != end; ++it)
		{
			const StreamingCamera &cam = *it;
			if ((entityPosition - cam.streamPosition).length() < m_Range)
			{
				if (!cell_entry->active)
					ActivateEntity(*cell, entity, *cell_entry);
				cell_entry->pendingDeactivation = false;
			}
			else if (cell_entry->active)
			{
				if (!cell_entry->pendingDeactivation)
					QueueEntityForDeactivation(*cell_entry, warp);
			}
		}
	}

	void StreamingManager::ActivateEntity(Cell& cell, const EntityPtr& entity, CellEntry& cell_entry)
	{
		FSN_ASSERT( !cell_entry.active );

		entity->SetStreamingCellIndex((size_t)(&cell - m_Cells));
		//activeObject.cellObjectIndex = cell.GetCellObjectIndex( cellObject );
		cell_entry.pendingDeactivation = false;
		cell_entry.active = true;

		cell.EntryActivated();

		GenerateActivationEvent( entity );
	}

	void StreamingManager::DeactivateEntity(const EntityPtr &entity)
	{
		Cell& cell = m_Cells[entity->GetStreamingCellIndex()];
#ifdef STREAMING_USEMAP
		auto _where = cell.objects.find(entity.get());
#else
		auto _where = findEntityInCell(&cell, entity.get());
#endif
		if (_where == cell.objects.end()) return;
		CellEntry &cellEntry = _where->second;

		DeactivateEntity(cell, entity, cellEntry);
	}

	void StreamingManager::DeactivateEntity(Cell &cell, const EntityPtr &entity, CellEntry &cell_entry)
	{
		FSN_ASSERT( cell_entry.active );
		cell_entry.active = false;
		cell_entry.pendingDeactivation = false;

		cell.EntryDeactivated();

		GenerateDeactivationEvent(entity);

		if (!cell.IsActive())
		{
			//SendToConsole("Cell Deactivated");
			//m_Archivist->Enqueue(&cell, (size_t)(&cell - m_Cells));
		}
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

	//const std::set<EntityPtr> &StreamingManager::GetActiveEntities() const
	//{
	//	return m_ActiveEntities;
	//}

	bool StreamingManager::updateStreamingCamera(StreamingManager::StreamingCamera &cam, CameraPtr camera)
	{
		FSN_ASSERT(camera);

		const CL_Vec2f &camPos = camera->GetPosition();

		bool pointChanged = false;

		Vector2 velocity(camPos.x - cam.lastPosition.x, camPos.y - cam.lastPosition.y);
		// Only interpolate / anticipate movement if the velocity doesn't indicate a jump in position
		if (velocity.squared_length() > 0.01f && velocity.squared_length() < m_RangeSquared)
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

	void StreamingManager::getCellRange(CL_Rect& range, const Vector2& pos)
	{
		range.left = (int)std::floor((pos.x - m_Range + m_Bounds.x) * m_InverseCellSize) - 1;
		range.right = (int)std::floor((pos.x + m_Range + m_Bounds.x) * m_InverseCellSize) + 1;

		range.top = (int)std::floor((pos.y - m_Range + m_Bounds.y) * m_InverseCellSize) - 1;
		range.bottom = (int)std::floor((pos.y + m_Range + m_Bounds.y) * m_InverseCellSize) + 1;

		fe_clamp(range.left, 0, (int)m_XCellCount - 1);
		fe_clamp(range.right, 0, (int)m_XCellCount - 1);
		fe_clamp(range.top, 0, (int)m_YCellCount - 1);
		fe_clamp(range.bottom, 0, (int)m_YCellCount - 1);
	}

	void StreamingManager::Update(const bool refresh)
	{
		// Each vector element represents a range of cells to update - overlapping active-areas
		//  are merged, so this may be < m_Cameras.size()
		//std::vector<CL_Rect> indexRanges;
		//indexRanges.reserve(m_Cameras.size());
		auto it = m_Cameras.begin();
		while (it != m_Cameras.end())
		{
			StreamingCamera &cam = *it;

			CameraPtr camera = cam.camera.lock();
			if (!camera)
			{
				it = m_Cameras.erase(it); // Remove expired cameras
				continue;
			}
			++it;

			
			updateStreamingCamera(cam, std::move(camera));
			const Vector2 &newPosition = cam.streamPosition;

			if (refresh || cam.firstUpdate || !v2Equal(cam.lastUsedPosition, cam.streamPosition, 0.5f))
			{
				Vector2 oldPosition = cam.lastUsedPosition;
				cam.lastUsedPosition = newPosition;
				cam.firstUpdate = false;
				// Find the minimum & maximum cell indicies that have to be checked
				CL_Rect range;
				range.left = (int)std::floor((std::min(oldPosition.x, newPosition.x) - m_Range + m_Bounds.x) * m_InverseCellSize) - 1;
				range.right = (int)std::floor((std::max(oldPosition.x, newPosition.x) + m_Range + m_Bounds.x) * m_InverseCellSize) + 1;

				range.top = (int)std::floor((std::min(oldPosition.y, newPosition.y) - m_Range + m_Bounds.y) * m_InverseCellSize) - 1;
				range.bottom = (int)std::floor((std::max(oldPosition.y, newPosition.y) + m_Range + m_Bounds.y) * m_InverseCellSize) + 1;

				fe_clamp(range.left, 0, (int)m_XCellCount - 1);
				fe_clamp(range.right, 0, (int)m_XCellCount - 1);
				fe_clamp(range.top, 0, (int)m_YCellCount - 1);
				fe_clamp(range.bottom, 0, (int)m_YCellCount - 1);

				CL_Rect inactiveRange;
				getCellRange(inactiveRange, oldPosition);
				//inactiveRange.left = (int)std::floor((oldPosition.x - m_Range + m_Bounds.x) * m_InverseCellSize) - 1;
				//inactiveRange.right = (int)std::floor((oldPosition.x + m_Range + m_Bounds.x) * m_InverseCellSize) + 1;

				//inactiveRange.top = (int)std::floor((oldPosition.y - m_Range + m_Bounds.y) * m_InverseCellSize) - 1;
				//inactiveRange.bottom = (int)std::floor((oldPosition.y + m_Range + m_Bounds.y) * m_InverseCellSize) + 1;

				//fe_clamp(inactiveRange.left, 0, (int)m_XCellCount - 1);
				//fe_clamp(range.right, 0, (int)m_XCellCount - 1);
				//fe_clamp(range.top, 0, (int)m_YCellCount - 1);
				//fe_clamp(range.bottom, 0, (int)m_YCellCount - 1);

				CL_Rect activeRange;
				getCellRange(activeRange, newPosition);
				//activeRange.left = (int)std::floor((newPosition.x - m_Range + m_Bounds.x) * m_InverseCellSize) - 1;
				//activeRange.right = (int)std::floor((newPosition.x + m_Range + m_Bounds.x) * m_InverseCellSize) + 1;

				//activeRange.top = (int)std::floor((newPosition.y - m_Range + m_Bounds.y) * m_InverseCellSize) - 1;
				//activeRange.bottom = (int)std::floor((newPosition.y + m_Range + m_Bounds.y) * m_InverseCellSize) + 1;

				//fe_clamp(range.left, 0, (int)m_XCellCount - 1);
				//fe_clamp(range.right, 0, (int)m_XCellCount - 1);
				//fe_clamp(range.top, 0, (int)m_YCellCount - 1);
				//fe_clamp(range.bottom, 0, (int)m_YCellCount - 1);

				auto deactivateCells = [this](const CL_Rect &inactiveRange)
				{
					unsigned int iy = (unsigned int)inactiveRange.top;
					unsigned int ix = (unsigned int)inactiveRange.left;
					unsigned int i = iy * m_XCellCount + ix;
					unsigned int stride = m_XCellCount - ( inactiveRange.right - inactiveRange.left + 1 );
					for (; iy <= (unsigned int)inactiveRange.bottom; ++iy)
					{
						FSN_ASSERT( iy >= 0 );
						FSN_ASSERT( iy < m_YCellCount );
						for (ix = (unsigned int)inactiveRange.left; ix <= (unsigned int)inactiveRange.right; ++ix)
						{
							FSN_ASSERT( ix >= 0 );
							FSN_ASSERT( ix < m_XCellCount );
							FSN_ASSERT( i == iy * m_XCellCount + ix );
							Cell &cell = m_Cells[i++];

							// Attempt to access the cell (it will be locked if the archivist is in the process of loading it)
							if (cell.mutex.try_lock())
							{
								for (auto cell_it = cell.objects.begin(), cell_end = cell.objects.end(); cell_it != cell_end; ++cell_it)
								{
									CellEntry &cellEntry = cell_it->second;

									if (cellEntry.active && !cellEntry.pendingDeactivation)
									{
										QueueEntityForDeactivation(cellEntry);
									}
								}
								cell.mutex.unlock();
							}

							// Unload cell
							//m_Archivist->Enqueue(&cell, i-1);
						}
						i += stride;
					}
				};

				if (inactiveRange != activeRange)
				{
					// Partial overlap
					if (inactiveRange.is_overlapped(activeRange))
					{
						CL_Rect inactiveRangeY(inactiveRange);
						CL_Rect inactiveRangeX(inactiveRange);

						if (inactiveRange.top > activeRange.top)
							inactiveRangeY.top = activeRange.bottom;
						else
							inactiveRangeY.bottom = activeRange.top;

						if (inactiveRange.bottom < activeRange.bottom)
							inactiveRangeY.bottom = activeRange.top;
						else
							inactiveRangeY.top = activeRange.bottom;

						inactiveRangeY.bottom = std::max(inactiveRangeY.top, inactiveRangeY.bottom);

						if (inactiveRange.left > activeRange.left)
							inactiveRangeX.left = activeRange.right;
						else
							inactiveRangeX.right = activeRange.left;

						if (inactiveRange.right < activeRange.right)
							inactiveRangeX.right = activeRange.left;
						else
							inactiveRangeX.left = activeRange.right;

						inactiveRangeX.right = std::max(inactiveRangeX.left, inactiveRangeX.right);

						// Don't include stuff in the X range that has already been included in the Y range:
						inactiveRangeX.top = activeRange.top;
						inactiveRangeX.bottom = activeRange.bottom;

						deactivateCells(inactiveRangeY);
						deactivateCells(inactiveRangeX);
					}
					// No overlap
					else
					{
						deactivateCells(inactiveRange);
					}
				}

				{
					unsigned int iy = (unsigned int)activeRange.top;
					unsigned int ix = (unsigned int)activeRange.left;
					unsigned int i = iy * m_XCellCount + ix;
					unsigned int stride = m_XCellCount - ( activeRange.right - activeRange.left + 1 );
					for (; iy <= (unsigned int)activeRange.bottom; ++iy)
					{
						FSN_ASSERT( iy >= 0 );
						FSN_ASSERT( iy < m_YCellCount );
						for (ix = (unsigned int)activeRange.left; ix <= (unsigned int)activeRange.right; ++ix)
						{
							FSN_ASSERT( ix >= 0 );
							FSN_ASSERT( ix < m_XCellCount );
							FSN_ASSERT( i == iy * m_XCellCount + ix );
							Cell &cell = m_Cells[i++];

							// Check if the cell needs to be loaded
							/*if (!cell.IsLoaded())
							{
								m_Archivist->Retrieve(&cell, i-1);
							}*/

							// Attempt to access the cell (it will be locked if the archivist is in the process of loading it)
							if (cell.mutex.try_lock())
							{
								try
								{
									for (auto cell_it = cell.objects.begin(), cell_end = cell.objects.end(); cell_it != cell_end; ++cell_it)
									{
//#ifdef _DEBUG
//										FSN_ASSERT(std::count_if(cell.objects.begin(), cell.objects.end(), [&](const Cell::EntityEntryPair& p) { return p.first == cell_it->first; }) == 1);
//#endif
										CellEntry &cellEntry = cell_it->second;
										//Vector2 entityPosition = entity->GetPosition();
										//entityPosition.x = ToGameUnits(entityPosition.x); entityPosition.y = ToGameUnits(entityPosition.y);
										Vector2 entityPosition(cellEntry.x, cellEntry.y);

										if ((entityPosition - cam.streamPosition).length() <= m_Range)
										{
											if (!cellEntry.active)
											{
												ActivateEntity(cell, cell_it->first->shared_from_this(), cellEntry);
											}
											else
											{
												cellEntry.pendingDeactivation = false;						
											}
										}
										else if (cellEntry.active)
										{
											if (!cellEntry.pendingDeactivation)
												QueueEntityForDeactivation(cellEntry);
										}
									}
								}
								catch (Exception&)
								{
									cell.mutex.unlock();
									throw;
								}
								cell.mutex.unlock();
							}
						}
						i += stride;
					}
				}

				//bool merged = false;
				//for (auto it = indexRanges.begin(), end = indexRanges.end(); it != end; ++it)
				//{
				//	CL_Rect &existingRange = *it;
				//	if (existingRange.is_overlapped(range))
				//	{
				//		existingRange.bounding_rect(range); // Expand the existing rect to encoumpass the new one
				//		merged = true;
				//	}
				//}
				//if (!merged)
				//	indexRanges.push_back(range);
			}
		}

		//for (auto it = indexRanges.begin(), end = indexRanges.end(); it != end; ++it)
		//{
		//	const CL_Rect &range = *it;
		//	
		//}
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
			asFUNCTION(StreamingManager_AddCamera), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("StreamingManager",
			"void removeCamera(Camera)",
			asFUNCTION(StreamingManager_RemoveCamera), asCALL_CDECL_OBJLAST);
	}

}
