/*
*  Copyright (c) 2009-2010 Fusion Project Team
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

	StreamingManager::StreamingManager()
		: m_DeactivationTime(s_DefaultDeactivationTime)
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
		auto newEnd = std::remove_if(cell->objects.rbegin(), cell->objects.rend(), [entity](const Cell::EntityEntryPair& pair)->bool {
			return pair.first == entity;
		});
		cell->objects.erase(newEnd.base());
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

	void StreamingManager::SetPlayerCamera(PlayerID net_idx, const CameraPtr &cam)
	{
		StreamingCamera &stCam = m_Cameras[net_idx];

		stCam.Camera = cam;
		stCam.LastPosition.x = cam->GetPosition().x; stCam.LastPosition.y = cam->GetPosition().y;
		stCam.StreamPosition = stCam.LastPosition;
		stCam.Tightness = s_SmoothTightness;
	}

	void StreamingManager::RemovePlayerCamera(PlayerID net_idx)
	{
		m_Cameras.erase(net_idx);
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

	//		area.left = stCam.StreamPosition.x - m_Range;
	//		area.top = stCam.StreamPosition.y - m_Range;
	//		area.right = stCam.StreamPosition.x + m_Range;
	//		area.bottom = stCam.StreamPosition.y + m_Range;
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
		Cell *cell = CellAtPosition(entity->GetPosition());
#ifdef STREAMING_USEMAP
		CellEntry &entry = cell->objects[entity.get()];
#else
		CellEntry &entry = createEntry(cell, entity.get());
#endif
		entry.x = entity->GetPosition().x; entry.y = entity->GetPosition().y;
		entity->SetStreamingCellIndex((size_t)(cell - m_Cells));

		activateInView(cell, &entry, entity, true);
		//OnUpdated(entity, 0.0f);
	}

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
		float new_x = fe_clamped(newPos.x, -m_Bounds.x, +m_Bounds.x);
		float new_y = fe_clamped(newPos.y, -m_Bounds.y, +m_Bounds.y);

		// gather all of the data we need about this object
		Cell *currentCell = nullptr;
		CellEntry *cellEntry = nullptr;
		Cell::CellEntryMap::iterator _where;

		if (entity->GetStreamingCellIndex() < (m_XCellCount * m_YCellCount)/*sizeof(m_Cells)*/)
		{
			currentCell = &m_Cells[entity->GetStreamingCellIndex()];
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
			currentCell = CellAtPosition(entity->GetPosition());
			Cell::CellEntryMap::value_type entryPair(entity, CellEntry());
			_where = currentCell->objects.insert(entryPair).first;
			cellEntry = &_where->second;

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
			// add the entity to its new cell
#ifdef STREAMING_USEMAP
			CellEntry &newEntry = newCell->objects[entityKey];
			newEntry = *cellEntry; // Copy the current cell data
#else
			newCell->objects.emplace_back( std::make_pair(entityKey, *cellEntry) );
			CellEntry &newEntry = newCell->objects.back().second;
#endif
			cellEntry = &newEntry; // Change the pointer (since it is used again below)

			// remove from current cell
			if (currentCell != nullptr)
#ifdef STREAMING_USEMAP
				currentCell->objects.erase(_where);
#else
				rRemoveEntityFromCell(currentCell, entityKey);
#endif

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
				DeactivateEntity(entity, *cellEntry);
		}
	}

	void StreamingManager::activateInView(Cell *cell, CellEntry *cell_entry, const EntityPtr &entity, bool warp)
	{
		const Vector2 &entityPosition = entity->GetPosition();
		for (auto it = m_Cameras.begin(), end = m_Cameras.end(); it != end; ++it)
		{
			const StreamingCamera &cam = it->second;
			if ((entityPosition - cam.StreamPosition).squared_length() < m_RangeSquared)
			{
				if (!cell_entry->active)
					ActivateEntity(entity, *cell_entry, *cell);
				cell_entry->pendingDeactivation = false;
			}
			else if (cell_entry->active)
			{
				if (!cell_entry->pendingDeactivation)
					QueueEntityForDeactivation(*cell_entry, warp);
			}
		}
	}

	void StreamingManager::ActivateEntity(const EntityPtr& entity, CellEntry& cell_entry, Cell& cell)
	{
		FSN_ASSERT( !cell_entry.active );

		entity->SetStreamingCellIndex((size_t)(&cell - m_Cells));
		//activeObject.cellObjectIndex = cell.GetCellObjectIndex( cellObject );
		cell_entry.pendingDeactivation = false;
		cell_entry.active = true;

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
		FSN_ASSERT( cellEntry.active );
		cellEntry.active = false;

		GenerateDeactivationEvent(entity);
	}

	void StreamingManager::DeactivateEntity(const EntityPtr &entity, CellEntry &cell_entry)
	{
		FSN_ASSERT( cell_entry.active );
		cell_entry.active = false;

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

	//const std::set<EntityPtr> &StreamingManager::GetActiveEntities() const
	//{
	//	return m_ActiveEntities;
	//}

	bool StreamingManager::updateStreamingCamera(StreamingManager::StreamingCamera &cam)
	{
		const CL_Vec2f &camPos = cam.Camera->GetPosition();

		bool pointChanged = false;

		Vector2 velocity( camPos.x - cam.LastPosition.x, camPos.y - cam.LastPosition.y );
		// Only interpolate / anticipate movement if the velocity doesn't indicate a jump in position
		if (velocity.squared_length() > 0.01f && velocity.squared_length() < m_RangeSquared)
		{
			Vector2 target( camPos.x + velocity.x, camPos.y + velocity.y );

			cam.StreamPosition = cam.StreamPosition + (target-cam.StreamPosition) * cam.Tightness;
			pointChanged = true;
		}
		else
		{
			if (!fe_fequal(cam.StreamPosition.x, camPos.x) || !fe_fequal(cam.StreamPosition.x, camPos.y))
				pointChanged = true;
			cam.StreamPosition.set(camPos.x, camPos.y);
		}

		// If the velocity has changed, smooth (over sudden changes in velocity) by adjusting interpolation tightness
		if (!v2Equal(cam.LastVelocity, velocity, 0.1f))
			cam.Tightness = s_SmoothTightness;
		else
			cam.Tightness += (s_FastTightness - cam.Tightness) * 0.01f;

		cam.LastPosition.x = camPos.x; cam.LastPosition.y = camPos.y;
		cam.LastVelocity = velocity;

		return pointChanged;
	}

	void StreamingManager::Update()
	{
		// Each vector element represents a range of cells to update - overlapping active-areas
		//  are merged, so this may be < m_Cameras.size()
		//std::vector<CL_Rect> indexRanges;
		//indexRanges.reserve(m_Cameras.size());

		for (StreamingCameraMap::iterator it = m_Cameras.begin(), end = m_Cameras.end(); it != end; ++it)
		{
			StreamingCamera &cam = it->second;

			Vector2 oldPosition = cam.StreamPosition;
			updateStreamingCamera(cam);
			const Vector2 &newPosition = cam.StreamPosition;

			if (!v2Equal(oldPosition, newPosition))
			{
				// Find the minimum & maximum cell indicies that have to be checked
				CL_Rect range;
				range.left = (int)std::floor((std::min(oldPosition.x, newPosition.x) - m_Range + m_Bounds.x) * m_InverseCellSize) - 1;
				range.right = (int)std::floor((std::max(oldPosition.x, newPosition.x) + m_Range + m_Bounds.x) * m_InverseCellSize) + 1;

				range.top = (int)std::floor((std::min(oldPosition.y, newPosition.y) - m_Range + m_Bounds.y) * m_InverseCellSize) - 1;
				range.bottom = (int)std::floor((std::min(oldPosition.y, newPosition.y) + m_Range + m_Bounds.y) * m_InverseCellSize) + 1;

				fe_clamp(range.left, 0, (int)m_XCellCount - 1);
				fe_clamp(range.right, 0, (int)m_XCellCount - 1);
				fe_clamp(range.top, 0, (int)m_YCellCount - 1);
				fe_clamp(range.bottom, 0, (int)m_YCellCount - 1);

				{
					unsigned int iy = (unsigned int)range.top;
					unsigned int ix = (unsigned int)range.left;
					unsigned int i = iy * m_XCellCount + ix;
					unsigned int stride = m_XCellCount - ( range.right - range.left + 1 );
					for (; iy <= (unsigned int)range.bottom; ++iy)
					{
						FSN_ASSERT( iy >= 0 );
						FSN_ASSERT( iy < m_YCellCount );
						for (ix = (unsigned int)range.left; ix <= (unsigned int)range.right; ++ix)
						{
							FSN_ASSERT( ix >= 0 );
							FSN_ASSERT( ix < m_XCellCount );
							FSN_ASSERT( i == iy * m_XCellCount + ix );
							Cell &cell = m_Cells[i++];
							for (auto cell_it = cell.objects.begin(), cell_end = cell.objects.end(); cell_it != cell_end; ++cell_it)
							{
								const EntityPtr &entity = cell_it->first; 
								CellEntry &cellEntry = cell_it->second;
								const Vector2 &entityPosition = entity->GetPosition();

								if ((entityPosition - cam.StreamPosition).squared_length() <= m_RangeSquared)
								{
									if (!cellEntry.active)
									{
										ActivateEntity(entity, cellEntry, cell);
										//entry.active = true;
										//m_ActiveEntities.insert(entity);
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

	void StreamingManager_SetPlayerCamera(ObjectID net_idx, Camera *camera, StreamingManager *obj)
	{
		obj->SetPlayerCamera(net_idx, CameraPtr(camera) );
		camera->release();
	}

	void StreamingManager_RemovePlayerCamera(ObjectID net_idx, StreamingManager *obj)
	{
		obj->RemovePlayerCamera( net_idx );
	}

	//void StreamingManager_AddCamera(Camera *camera, StreamingManager *obj)
	//{
	//	obj->AddCamera( CameraPtr(camera) );
	//}

	//void StreamingManager_RemoveCamera(Camera *camera, StreamingManager *obj)
	//{
	//	obj->RemoveCamera( CameraPtr(camera) );
	//}

	void StreamingManager::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSingletonType<StreamingManager>("StreamingManager", engine);
		r = engine->RegisterObjectMethod("StreamingManager",
			"void setPlayerCamera(uint16, Camera@)",
			asFUNCTION(StreamingManager_SetPlayerCamera), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("StreamingManager",
			"void removePlayerCamera(uint16)",
			asFUNCTION(StreamingManager_RemovePlayerCamera), asCALL_CDECL_OBJLAST);
	}

}
