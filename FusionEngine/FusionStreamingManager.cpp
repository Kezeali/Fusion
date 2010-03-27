#include "FusionStableHeaders.h"

#include "FusionStreamingManager.h"

#include "FusionMaths.h"
#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{

	const float StreamingManager::s_SmoothTightness = 0.1f;
	const float StreamingManager::s_FastTightness = 0.3f;

	const float s_DefaultActivationRange = 1500.f;
	const float s_DefaultCellSize = 200.f;
	const float s_DefaultWorldSize = 200000.f;

	StreamingManager::StreamingManager()
	{
		m_Bounds.x = s_DefaultWorldSize / 2.f;
		m_Bounds.y = s_DefaultWorldSize / 2.f;

		m_Range = s_DefaultActivationRange;
		m_RangeSquared = m_Range * m_Range;

		m_CellSize = s_DefaultCellSize;
		m_InverseCellSize = 1.f / m_CellSize;

		m_XCellCount = (int)(m_Bounds.x * m_InverseCellSize) + 1;
		m_YCellCount = (int)(m_Bounds.y * m_InverseCellSize) + 1;
	}

	StreamingManager::~StreamingManager()
	{
	}

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
		FSN_ASSERT( iy*m_XCellCount+ix < sizeof(m_Cells) );
		return &m_Cells[iy*m_XCellCount+ix];
	}
	
	Cell *StreamingManager::CellAtPosition(const Vector2 &position)
	{
		return CellAtPosition(position.x, position.y);
	}

	void StreamingManager::AddEntity(const EntityPtr &entity)
	{
		Cell *cell = CellAtPosition(entity->GetPosition());
		CellEntry &entry = cell->objects[entity];
		entry.x = entity->GetPosition().x; entry.y = entity->GetPosition().y;
		entity->SetStreamingCellIndex((size_t)(cell - m_Cells));

		OnUpdated(entity);
	}

	void StreamingManager::RemoveEntity(const EntityPtr &entity)
	{
		FSN_ASSERT(entity->GetStreamingCellIndex() < sizeof(m_Cells));
		Cell *cell = &m_Cells[entity->GetStreamingCellIndex()];
		cell->objects.erase(entity);
	}

	void StreamingManager::OnUpdated(const EntityPtr &entity)
	{
		FSN_ASSERT(entity);

		// clamp the new position within bounds
		Vector2 newPos = entity->GetPosition();
		float new_x = fe_clamped(newPos.x, -m_Bounds.x, +m_Bounds.x);
		float new_y = fe_clamped(newPos.y, -m_Bounds.y, +m_Bounds.y);

		// gather all of the data we need about this object
		Cell *currentCell = nullptr;
		CellEntry *cellEntry = nullptr;
		std::map<EntityPtr, CellEntry>::iterator _where;

		if (entity->GetStreamingCellIndex() < sizeof(m_Cells))
		{
			currentCell = &m_Cells[entity->GetStreamingCellIndex()];
			_where = currentCell->objects.find(entity);
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

		bool warp = std::abs(cellEntry->x - new_x) > 50.0f;

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
			// add to new cell
			CellEntry &newEntry = newCell->objects[entity];
			newEntry = *cellEntry; // Copy the current cell data
			cellEntry = &newEntry; // Change the pointer (since it is used again below)

			// remove from current cell
			if (currentCell != nullptr)
				currentCell->objects.erase(_where);

			currentCell = newCell;
			//m_EntityToCellIndex[entity] = (size_t)(currentCell - m_Cells);

			cellEntry->x = new_x;
			cellEntry->y = new_y;

			entity->SetStreamingCellIndex((size_t)(currentCell - m_Cells));
		}

		// see if the object needs to be activated or deactivate
		const Vector2 &entityPosition = entity->GetPosition();
		for (auto it = m_Cameras.begin(), end = m_Cameras.end(); it != end; ++it)
		{
			const StreamingCamera &cam = it->second;
			if ((entityPosition - cam.StreamPosition).squared_length() > m_RangeSquared)
			{
				if (!cellEntry->active)
					ActivateEntity(entity, *cellEntry, *currentCell);
				else if (!cellEntry->pendingDeactivation)
					QueueEntityForDeactivation(*cellEntry, warp);
			}
			else
				cellEntry->pendingDeactivation = false;
		}
	}

	bool StreamingManager::activateWithinRange(const StreamingManager::StreamingCamera &cam, const EntityPtr &entity, CellEntry &entry)
	{
		//const Vector2 &entityPosition = entity->GetPosition();
		//if ((entityPosition - cam.StreamPosition).squared_length() < m_RangeSquared)
		//{
		//	if (!entry.active)
		//	{
		//		ActivateEntity(entity, entry, cell);
		//		//entry.active = true;
		//		//m_ActiveEntities.insert(entity);
		//	}
		//	else
		//	{
		//		entry.pendingDeactivation = false;						
		//	}
		//}
		//else if (entry.active)
		//{
		//	if (!entry.pendingDeactivation)
		//		QueueEntityForDeactivation(entry);
		//}
		return false;
	}

	void StreamingManager::ActivateEntity(const EntityPtr &entity, CellEntry &cell_entry, Cell &cell)
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
		Cell & cell = m_Cells[entity->GetStreamingCellIndex()];
		auto _where = cell.objects.find(entity);
		if (_where == cell.objects.end()) return;
		CellEntry &cellEntry = _where->second;
		FSN_ASSERT( cellEntry.active );
		cellEntry.active = false;

		GenerateDeactivationEvent( entity );
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
				fe_clamp(range.top, 0, (int)m_YCellCount - 1);

				unsigned int i = 0;
				unsigned int stride = m_XCellCount - ( range.right - range.left + 1 );
				for (unsigned int iy = (unsigned int)range.top; iy <= (unsigned int)range.bottom; ++iy)
				{
					FSN_ASSERT( iy >= 0 );
					FSN_ASSERT( iy < m_YCellCount );
					for (unsigned int ix = (unsigned int)range.left; ix <= (unsigned int)range.right; ++ix)
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
