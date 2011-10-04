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
#include "FusionPlayerRegistry.h"
#include "FusionNetworkManager.h"
#include "FusionRakNetwork.h"
#include "FusionNetDestinationHelpers.h"

#include <boost/date_time.hpp>

namespace FusionEngine
{

	const float StreamingManager::s_SmoothTightness = 0.1f;
	const float StreamingManager::s_FastTightness = 0.3f;

	const float s_DefaultActivationRange = 1500.f;
	const float s_DefaultCellSize = 500.f;
	const float s_DefaultWorldSize = 200000.f;

	const float s_DefaultDeactivationTime = 0.1f;

	void Cell::AddHist(const std::string& l, unsigned int n)
	{
#if 0
		mutex_t::scoped_lock lock(historyMutex);
		auto now = boost::posix_time::second_clock::local_time();
		std::string message = boost::posix_time::to_simple_string(now) + ": " + l;
		if (n != -1)
		{
			std::stringstream str; str << n;
			message += " (" + str.str() + ")";
		}
		history.push_back(message);
#endif
	}

	StreamingManager::StreamingManager(CellArchiver* archivist, bool initialise)
		: m_DeactivationTime(s_DefaultDeactivationTime),
		m_Archivist(archivist)
	{
		if (initialise) // default initialisation
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
		else // Object will still be valid, just pretty much useless (will also take no time to re-init)
		{
			m_Bounds.x = s_DefaultCellSize;
			m_Bounds.y = s_DefaultCellSize;

			m_Range = s_DefaultActivationRange;
			m_RangeSquared = m_Range * m_Range;

			m_CellSize = s_DefaultCellSize;
			m_InverseCellSize = 1.f / m_CellSize;

			m_XCellCount = (size_t)(m_Bounds.x * 2.0f * m_InverseCellSize) + 1;
			m_YCellCount = (size_t)(m_Bounds.y * 2.0f * m_InverseCellSize) + 1;

			m_Cells = new Cell[m_XCellCount * m_YCellCount];
		}

		NetworkManager::getSingleton().Subscribe(MTID_REMOTECAMERA_ADD, this);
		NetworkManager::getSingleton().Subscribe(MTID_REMOTECAMERA_REMOVE, this);
		NetworkManager::getSingleton().Subscribe(MTID_REMOTECAMERA_MOVE, this);
	}

	StreamingManager::~StreamingManager()
	{
		NetworkManager::getSingleton().Unsubscribe(MTID_REMOTECAMERA_ADD, this);
		NetworkManager::getSingleton().Unsubscribe(MTID_REMOTECAMERA_REMOVE, this);
		NetworkManager::getSingleton().Unsubscribe(MTID_REMOTECAMERA_MOVE, this);
		delete[] m_Cells;
	}

	void StreamingManager::Initialise(float map_width, unsigned int num_cells_across, float cell_size)
	{
		delete[] m_Cells;

		m_Bounds.x = map_width / 2.0f;
		m_Bounds.y = map_width / 2.0f;

		m_Range = s_DefaultActivationRange;
		m_RangeSquared = m_Range * m_Range;

		m_CellSize = cell_size;
		m_InverseCellSize = 1.f / cell_size;

		m_XCellCount = num_cells_across;//(size_t)(m_Bounds.x * 2.0f * m_InverseCellSize) + 1;
		m_YCellCount = num_cells_across;//(size_t)(m_Bounds.y * 2.0f * m_InverseCellSize) + 1;

		m_Cells = new Cell[m_XCellCount * m_YCellCount];
	}

#ifndef STREAMING_USEMAP
	//! Finds and removes entry for the given Entity from the given cell
	static inline void removeEntityFromCell(Cell* cell, const EntityPtr& entity)
	{
		auto newEnd = std::remove_if(cell->objects.begin(), cell->objects.end(), [&](const Cell::EntityEntryPair& pair)->bool {
			return pair.first == entity;
		});
		cell->objects.erase(newEnd);
	}

	//! Finds and removes the given entity from the given cell, starting the search from the end of the list
	static inline void rRemoveEntityFromCell(Cell* cell, const EntityPtr& entity)
	{
		if (cell->objects.size() > 1)
		{
			auto rEntry = std::find_if(cell->objects.rbegin(), cell->objects.rend(), [&](const Cell::EntityEntryPair& pair)->bool {
				return pair.first == entity;
			});
			cell->objects.erase((++rEntry).base());
		}
		else
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
		return (++rWhere).base();
	}

	//! Creates an entry for the given Entity in the given cell
	static inline CellEntry& createEntry(Cell* cell, const EntityPtr& entity, const CellEntry& copy_entry)
	{
		cell->objects.emplace_back( std::make_pair(entity, copy_entry) );
		return cell->objects.back().second;
	}

	//! Creates an entry for the given Entity in the given cell
	static inline CellEntry& createEntry(Cell* cell, const EntityPtr& entity)
	{
		return createEntry(cell, entity, CellEntry());
	}
#endif

	StreamingManager::StreamingCamera& StreamingManager::createStreamingCamera(PlayerID owner, const CameraPtr& cam)
	{
		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);

		m_Cameras.push_back(StreamingCamera());
		auto& streamingCam = m_Cameras.back();

		streamingCam.firstUpdate = true;
		streamingCam.tightness = s_SmoothTightness;
		if (cam)
		{
			streamingCam.camera = cam;
			streamingCam.streamPosition = streamingCam.lastPosition = Vector2(cam->GetPosition().x, cam->GetPosition().y);
		}
		streamingCam.owner = owner;

		return streamingCam;
	}

	void StreamingManager::AddCamera(const CameraPtr &cam)
	{
		if (!cam)
			FSN_EXCEPT(InvalidArgumentException, "Tried to add a NULL camera ptr to StreamingManager");

		if (std::any_of(m_Cameras.begin(), m_Cameras.end(), StreamingCamera::HasSameCamera(cam)))
		{
			FSN_EXCEPT(InvalidArgumentException, "Tried to add a camera to StreamingManager that is already being tracked");
		}

		createStreamingCamera(0, cam);
	}

	void StreamingManager::AddOwnedCamera(PlayerID owner, const CameraPtr& cam)
	{
		if (!cam)
			FSN_EXCEPT(InvalidArgumentException, "Tried to add a NULL camera ptr to StreamingManager");

		if (std::any_of(m_Cameras.begin(), m_Cameras.end(), StreamingCamera::HasSameCamera(cam)))
		{
			FSN_EXCEPT(InvalidArgumentException, "Tried to add a camera to StreamingManager that is already being tracked");
		}

		auto& streamingCam = createStreamingCamera(owner, cam);

		if (owner != 0)
		{
			streamingCam.id = m_CamIds.getFreeID();

			RakNet::BitStream bs;
			bs.Write(streamingCam.id);
			bs.Write(streamingCam.owner);
			bs.Write(streamingCam.streamPosition.x);
			bs.Write(streamingCam.streamPosition.y);
			NetworkManager::GetNetwork()->Send(To::Populace(), false, MTID_REMOTECAMERA_ADD, &bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_VIEWMANAGER);
		}
	}

	void StreamingManager::RemoveCamera(const CameraPtr &cam)
	{
		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);
		auto streamingCameraEntry = std::find_if(m_Cameras.begin(), m_Cameras.end(), StreamingCamera::HasSameCamera(cam));
		if (streamingCameraEntry != m_Cameras.end())
		{
			const auto& streamingCam = *streamingCameraEntry;
			RakNet::BitStream bs;
			bs.Write(streamingCam.id);
			NetworkManager::GetNetwork()->Send(To::Populace(), false, MTID_REMOTECAMERA_REMOVE, &bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_VIEWMANAGER);

			m_Cameras.erase(streamingCameraEntry, m_Cameras.end());
		}
	}

	void StreamingManager::HandlePacket(RakNet::Packet* packet)
	{
		RakNet::BitStream dataStream(packet->data, packet->length, false);

		unsigned char type;
		dataStream.Read(type);
		switch (type)
		{
		case MTID_REMOTECAMERA_ADD:
			{
				uint8_t camId;
				dataStream.Read(camId);
				PlayerID owner;
				dataStream.Read(owner);
				Vector2 pos;
				dataStream.Read(pos.x);
				dataStream.Read(pos.y);
				auto& streamingCam = createStreamingCamera(owner, CameraPtr());
				streamingCam.id = camId;
				streamingCam.lastPosition = streamingCam.streamPosition = pos;
			}
			break;
		case MTID_REMOTECAMERA_REMOVE:
			{
				uint8_t camId;
				dataStream.Read(camId);
				auto camEntry = std::find_if(m_Cameras.begin(), m_Cameras.end(), [camId](const StreamingCamera& cam) { return cam.id == camId; });
				if (camEntry != m_Cameras.end())
					m_Cameras.erase(camEntry);
			}
			break;
		case MTID_REMOTECAMERA_MOVE:
			{
				uint8_t camId;
				dataStream.Read(camId);
				auto camEntry = std::find_if(m_Cameras.begin(), m_Cameras.end(), [camId](const StreamingCamera& cam) { return cam.id == camId; });
				if (camEntry != m_Cameras.end())
				{
					auto& streamingCam = *camEntry;
					streamingCam.lastPosition = streamingCam.streamPosition;
					dataStream.Read(streamingCam.streamPosition.x);
					dataStream.Read(streamingCam.streamPosition.y);
				}
			}
			break;
		};
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

	void StreamingManager::DumpAllCells()
	{
		while (!m_TheVoid.objects.empty())
			Update(false);
		const auto size = m_XCellCount * m_YCellCount;
		for (size_t i = 0; i < size; ++i)
		{
			Cell* cell = &m_Cells[i];
			m_Archivist->Store(cell, i);
		}
	}

	void StreamingManager::AddEntity(const EntityPtr &entity)
	{
		Vector2 entityPosition = entity->GetPosition();
		entityPosition.x = ToGameUnits(entityPosition.x); entityPosition.y = ToGameUnits(entityPosition.y);
		Cell *cell = CellAtPosition(entityPosition);

		Cell::mutex_t::scoped_try_lock lock(cell->mutex);
		if (lock && cell->IsLoaded())
		{
			entity->SetStreamingCellIndex((size_t)(cell - m_Cells));
		}
		else
		{
			Cell::mutex_t::scoped_try_lock test(m_TheVoid.mutex);
			FSN_ASSERT(test);
			lock.swap(test);
			cell = &m_TheVoid;
			entity->SetStreamingCellIndex(std::numeric_limits<size_t>::max());
		}
		
#ifdef STREAMING_USEMAP
		CellEntry &entry = cell->objects[entity.get()];
#else
		CellEntry &entry = createEntry(cell, entity/*.get()*/);
#endif
		entry.x = entityPosition.x; entry.y = entityPosition.y;

		//if (lock.owns_lock())
		//	lock.unlock();

		activateInView(cell, &entry, entity, true);
		if (!entry.active) // activateInView assumes that the entry's current state has been propagated - it hasn't in this case, since the entry was just added
		{
			//cell->EntryReferenced(); // Otherwise the counter will be off when OnDeactivated is called
			ActivateEntity(*cell, entity, entry);
			QueueEntityForDeactivation(entry, true);
		}

		//OnUpdated(entity, 0.0f);
	}

//#define STREAMING_AUTOADD

	void StreamingManager::RemoveEntity(const EntityPtr &entity)
	{
		if (entity->GetStreamingCellIndex() != std::numeric_limits<size_t>::max())
		{
			FSN_ASSERT(entity->GetStreamingCellIndex() < (m_XCellCount * m_YCellCount)/*sizeof(m_Cells)*/);
			Cell *cell = &m_Cells[entity->GetStreamingCellIndex()];
#ifdef STREAMING_USEMAP
			cell->objects.erase(entity.get());
#else
			removeEntityFromCell(cell, entity/*.get()*/);
#endif
		}
		else
		{
			removeEntityFromCell(&m_TheVoid, entity/*.get()*/);
		}

		// Remove the entity from the ID -> cell directory
		m_EntityDirectory.erase(entity->GetID());
	}

	void StreamingManager::OnUpdated(const EntityPtr &entity, float split)
	{
		FSN_ASSERT(entity);
		EntityPtr entityKey = entity;

		// clamp the new position within bounds
		Vector2 newPos = entity->GetPosition();
		float new_x = fe_clamped(ToGameUnits(newPos.x), -m_Bounds.x, +m_Bounds.x);
		float new_y = fe_clamped(ToGameUnits(newPos.y), -m_Bounds.y, +m_Bounds.y);

		// gather all of the data we need about this object
		Cell *currentCell = nullptr;
		CellEntry *cellEntry = nullptr;
		Cell::CellEntryMap::iterator _where;

		Cell::mutex_t::scoped_lock currentCell_lock;
		if (entity->GetStreamingCellIndex() < (m_XCellCount * m_YCellCount))
		{
			currentCell = &m_Cells[entity->GetStreamingCellIndex()];

			currentCell_lock = Cell::mutex_t::scoped_lock(currentCell->mutex);
			// TODO: rather than this (and setting StreamingCellIndex to size_t::max), entities could be implicitly in the void if the cell isn't loaded
			FSN_ASSERT(currentCell->IsLoaded());
#ifdef STREAMING_USEMAP
			_where = currentCell->objects.find(entityKey);
#else
			_where = rFindEntityInCell(currentCell, entityKey);
#endif
			FSN_ASSERT( _where != currentCell->objects.end() );
			cellEntry = &_where->second;
		}
		else if (entity->GetStreamingCellIndex() == std::numeric_limits<size_t>::max())
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
			currentCell = CellAtPosition(new_x, new_y);

			currentCell_lock = Cell::mutex_t::scoped_lock(currentCell->mutex);
			currentCell->objects.push_back(std::make_pair(entity, CellEntry()));
			cellEntry = &currentCell->objects.back().second;

			entity->SetStreamingCellIndex((size_t)(currentCell - m_Cells));
		}
#endif

		FSN_ASSERT(cellEntry != nullptr);
		FSN_ASSERT(currentCell_lock && currentCell_lock.owns_lock());

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
			Cell::mutex_t::scoped_try_lock newCell_lock(newCell->mutex);
			if (!newCell_lock || !newCell->IsLoaded())
			{
				newCell = &m_TheVoid;
				
				if (currentCell != &m_TheVoid)
				{
					newCell_lock = Cell::mutex_t::scoped_try_lock(m_TheVoid.mutex);
					FSN_ASSERT(newCell_lock.owns_lock());
				}
			}
			
			{
				newCell->AddHist("Entry added from another cell");

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
				newCell->AddHist("Entry moved to another cell");
				if (cellEntry->active)
				{
					currentCell->EntryUnreferenced();
				}
			}

			currentCell = newCell;

			cellEntry->x = new_x;
			cellEntry->y = new_y;

			if (currentCell != &m_TheVoid)
				entity->SetStreamingCellIndex((size_t)(currentCell - m_Cells));
			else
				entity->SetStreamingCellIndex(std::numeric_limits<size_t>::max());
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

	void StreamingManager::OnUnreferenced(const EntityPtr& entity)
	{
		if (entity->GetStreamingCellIndex() < (m_XCellCount * m_YCellCount))
		{
			auto& currentCell = m_Cells[entity->GetStreamingCellIndex()];

			Cell::mutex_t::scoped_lock lock(currentCell.mutex);

			auto it = findEntityInCell(&currentCell, entity/*.get()*/);
			it->second.active = CellEntry::Inactive;

			currentCell.EntryUnreferenced();

			// TODO: record this entity somehow (std::set or flag) so that an assertion can be made
			//  in OnUpdated to ensure that the entity manager is honest

			if (!currentCell.IsActive() && !currentCell.inRange)
			{
				currentCell.AddHist("Store Attempted on Deactivation");
				m_Archivist->Store(&currentCell, entity->GetStreamingCellIndex());
			}
		}
		else if (entity->GetStreamingCellIndex() == std::numeric_limits<size_t>::max())
		{
			auto& currentCell = m_TheVoid;

			Cell::mutex_t::scoped_lock lock(currentCell.mutex);

			auto it = findEntityInCell(&currentCell, entity/*.get()*/);
			it->second.active = CellEntry::Inactive;

			currentCell.EntryUnreferenced();
		}

		// Update the entity-directory
		// TODO: update this in other places where StreamingCellIndex is changed?
		if (entity->IsSyncedEntity())
			m_EntityDirectory[entity->GetID()] = entity->GetStreamingCellIndex();
	}

	static bool findEntityById(EntityPtr& out, CellEntry*& out2, Cell& cell, ObjectID id)
	{
		auto found = std::find_if(cell.objects.begin(), cell.objects.end(), [id](const Cell::EntityEntryPair& entry)
		{
			return entry.first->GetID() == id;
		});
		if (found != cell.objects.end())
		{
			out = found->first;
			out2 = &found->second;
			return true;
		}
		else
			return false;
	}

	bool StreamingManager::ActivateEntity(ObjectID id)
	{
		auto it = m_EntityDirectory.find(id);
		if (it != m_EntityDirectory.end())
		{
			size_t cellIndex = it->second;
			if (cellIndex < m_XCellCount * m_YCellCount)
			{
				Cell& cell = m_Cells[cellIndex];
				Cell::mutex_t::scoped_try_lock lock(cell.mutex);
				if (!lock || !cell.IsLoaded())
				{
					m_Archivist->Retrieve(&cell, cellIndex);
					m_RequestedEntities[&cell].insert(id);
				}
				else
				{
					EntityPtr entity;
					CellEntry* entry = nullptr;
					if (findEntityById(entity, entry, cell, id))
					{
						FSN_ASSERT(entity && entry);
						if (!entry->active)
							//ActivateEntity(cell, entity, *entry);
							m_RequestedEntities[&cell].insert(id);
					}
					else
						return false;
				}
				return true;
			}
			else if (cellIndex == std::numeric_limits<size_t>::max())
			{
				Cell& cell = m_TheVoid;
				EntityPtr entity;
				CellEntry* entry = nullptr;
				if (findEntityById(entity, entry, cell, id))
				{
					FSN_ASSERT(entity && entry);
					if (!entry->active)
						ActivateEntity(cell, entity, *entry);
					return true;
				}
			}
		}
		
		return false;
	}

	void StreamingManager::activateInView(Cell *cell, CellEntry *cell_entry, const EntityPtr &entity, bool warp)
	{
		FSN_ASSERT(cell);

		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);

		Vector2 entityPosition = entity->GetPosition();
		entityPosition.x = ToGameUnits(entityPosition.x); entityPosition.y = ToGameUnits(entityPosition.y);
		if (std::any_of(m_Cameras.begin(), m_Cameras.end(),
			[&](const StreamingCamera& cam) { return (entityPosition - cam.streamPosition).length() <= m_Range; }))
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

	void StreamingManager::ActivateEntity(Cell& cell, const EntityPtr& entity, CellEntry& cell_entry)
	{
		FSN_ASSERT( !cell_entry.active );

		if (&cell != &m_TheVoid)
			entity->SetStreamingCellIndex((size_t)(&cell - m_Cells));
		//activeObject.cellObjectIndex = cell.GetCellObjectIndex( cellObject );
		cell_entry.pendingDeactivation = false;
		cell_entry.active = CellEntry::Active;

		cell.EntryReferenced();

		GenerateActivationEvent( entity );
	}

	void StreamingManager::DeactivateEntity(const EntityPtr &entity)
	{
		Cell& cell = m_Cells[entity->GetStreamingCellIndex()];
#ifdef STREAMING_USEMAP
		auto _where = cell.objects.find(entity.get());
#else
		auto _where = findEntityInCell(&cell, entity/*.get()*/);
#endif
		if (_where == cell.objects.end()) return;
		CellEntry &cellEntry = _where->second;

		DeactivateEntity(cell, entity, cellEntry);
	}

	void StreamingManager::DeactivateEntity(Cell &cell, const EntityPtr &entity, CellEntry &cell_entry)
	{
		FSN_ASSERT( cell_entry.active );
		cell_entry.active = CellEntry::Waiting;
		cell_entry.pendingDeactivation = false;

		//cell.EntryUnreferenced(); // Commented out: this doesn't get called until the entity is actually deactivated (see OnDeactivation)

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

	void StreamingManager::deactivateCells(const CL_Rect& inactiveRange)
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

				cell.inRange = false;

				if (cell.IsLoaded())
				{
					if (!cell.IsActive() && !cell.inRange)
					{
						cell.AddHist("Store Attempted due to leaving range");
						m_Archivist->Store(&cell, i-1);
					}
					else
					{
						// Attempt to access the cell (it will be locked if the archivist is in the process of loading it)
						Cell::mutex_t::scoped_try_lock lock(cell.mutex);
						if (lock)
						{
							for (auto cell_it = cell.objects.begin(), cell_end = cell.objects.end(); cell_it != cell_end; ++cell_it)
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
			}
			i += stride;
		}
	}

	void StreamingManager::processCell(Cell& cell, const std::list<Vector2>& cam_positions)
	{
		Cell::mutex_t::scoped_try_lock lock(cell.mutex);
		if (lock)
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

					// Check if the entry is in range of any cameras
					if (std::any_of(cam_positions.begin(), cam_positions.end(),
						[&](const Vector2& cam_position) { return (entityPosition - cam_position).length() <= m_Range; }))
					{
						if (!cellEntry.active)
						{
							ActivateEntity(cell, cell_it->first/*->shared_from_this()*/, cellEntry);
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
	static inline void clipRange(T& out_it, CL_Rect& inactiveRange, const CL_Rect& activeRange)
	{
		// Partial overlap
		//if (inactiveRange.is_overlapped(activeRange))
		if (inclusiveOverlap(inactiveRange, activeRange))
		{
			CL_Rect inactiveRangeY(inactiveRange);
			CL_Rect inactiveRangeX(inactiveRange);

			// + 1 / - 1 are due to the ranges being used inclusively when processed

			if (inactiveRange.top >= activeRange.top)
				inactiveRangeY.top = activeRange.bottom + 1;
			//else
			//	inactiveRangeY.bottom = activeRange.top - 1;

			if (inactiveRange.bottom <= activeRange.bottom)
				inactiveRangeY.bottom = activeRange.top - 1;
			//else
			//	inactiveRangeY.top = activeRange.bottom + 1;

			//if (inactiveRange.top > activeRange.top)
			//	inactiveRangeY.bottom = std::max(inactiveRangeY.top, inactiveRangeY.bottom);
			//else
			//	inactiveRangeY.top = std::min(inactiveRangeY.top, inactiveRangeY.bottom);

			if (inactiveRange.left >= activeRange.left)
				inactiveRangeX.left = activeRange.right + 1;
			//else
			//	inactiveRangeX.right = activeRange.left - 1;

			if (inactiveRange.right <= activeRange.right)
				inactiveRangeX.right = activeRange.left - 1;
			//else
			//	inactiveRangeX.left = activeRange.right + 1;

			//inactiveRangeX.right = std::max(inactiveRangeX.left, inactiveRangeX.right);

			// Don't include stuff in the X range that has already been included in the Y range:
			inactiveRangeX.top = activeRange.top;
			inactiveRangeX.bottom = activeRange.bottom;

			if (inactiveRangeX.get_width() >= 0 && inactiveRangeX.get_height() >= 0)
				*out_it++ = std::move(inactiveRangeX);
			if (inactiveRangeY.get_width() >= 0  && inactiveRangeY.get_height() >= 0)
				*out_it++ = std::move(inactiveRangeY);
		}
		// No overlap
		else
		{
			*out_it++ = inactiveRange;
		}
	}

	void StreamingManager::Update(const bool refresh)
	{
		// Try to clear The Void
		{
			Cell::mutex_t::scoped_try_lock lock(m_TheVoid.mutex);
			if (lock)
			{
				for (auto it = m_TheVoid.objects.begin(), end = m_TheVoid.objects.end(); it != end;/* ++it*/)
				{
					auto actualCell = CellAtPosition(it->second.x, it->second.y);

					Cell::mutex_t::scoped_try_lock lock(actualCell->mutex);
					if (lock)
					{
						if (!actualCell->IsLoaded())
						{
							// Request the cell if it isn't already (Retrieve returns true if the cell wasn't already requested)
							if (m_Archivist->Retrieve(actualCell, (size_t)(actualCell - m_Cells)))
							{
								actualCell->AddHist("Retrieved due to objects in Void");
								//FSN_ASSERT(std::find(m_CellsBeingLoaded.cbegin(), m_CellsBeingLoaded.cend(), actualCell) == m_CellsBeingLoaded.end());
								//m_CellsBeingLoaded.insert(actualCell);
							}
						}
						else
						{
							auto entity = it->first;
							// TODO:
							//changeCell(*it, m_TheVoid, *cell);
							// OR
							// m_TheVoid.RemoveEntry(it); <- one of the overloads will take an iterator, the others will take index / entry
							// cell->AddEntry(*it);

							//Cell::mutex_t::scoped_lock lock(actualCell->mutex);

							actualCell->objects.push_back( std::move(*it) );
							auto& newEntry = actualCell->objects.back().second;

							if (newEntry.active)
								actualCell->EntryReferenced();
							else if (!actualCell->IsActive())
							{
								actualCell->AddHist("Storing cell again after Retrieving it for an entity spawned in The Void");
								m_Archivist->Store(actualCell, (size_t)(actualCell - m_Cells));
							}

							// remove from current cell
							{
								it = m_TheVoid.objects.erase(it);
								end = m_TheVoid.objects.end();
								if (newEntry.active)
								{
									m_TheVoid.AddHist("Entry transferred to correct cell:");
									m_TheVoid.EntryUnreferenced();
								}
							}

							entity->SetStreamingCellIndex((size_t)(actualCell - m_Cells));
							continue;
						}
					}
					++it;
				}
			}
		}

		// Check for requested cells that have finished loading
		for (auto it = m_RequestedEntities.begin(), end = m_RequestedEntities.end(); it != end;)
		{
			auto& cell = it->first;
			FSN_ASSERT(cell && cell > m_Cells && cell < &m_Cells[m_XCellCount * m_YCellCount]);
			Cell::mutex_t::scoped_try_lock lock(cell->mutex);
			if (lock && cell->IsLoaded())
			{
				auto& requestedIds = it->second;
				for (auto eit = cell->objects.begin(), eend = cell->objects.end(); eit != eend; ++eit)
				{
					if (requestedIds.count(eit->first->GetID()) != 0)
					{
						if (!eit->second.active)
						{
							ActivateEntity(*cell, eit->first, eit->second);
							if (!cell->inRange)
								QueueEntityForDeactivation(eit->second, true);
						}
					}
				}
				it = m_RequestedEntities.erase(it);
				end = m_RequestedEntities.end();
			}
			else
				++it;
		}

		// Each vector element represents a range of cells to update - overlapping areas
		//  are split / merged into no-overlapping sub-regions (to avoid processing cells
		//  twice in a step)
		std::list<CL_Rect> inactiveRanges;
		std::list<std::pair<CL_Rect, std::list<Vector2>>> activeRanges;
		// Used to process The Void
		std::list<Vector2> localStreamPositions;
		std::list<std::pair<Vector2, PlayerID>> remoteStreamPositions;

		bool allActiveRangesStale = true;

		CamerasMutex_t::scoped_lock lock(m_CamerasMutex);
		auto it = m_Cameras.begin();
		while (it != m_Cameras.end())
		{
			StreamingCamera &cam = *it;

			// Update streaming position of this camera
			{
				CameraPtr camera = cam.camera.lock();
				if (!camera && (cam.owner == 0 || PlayerRegistry::IsLocal(cam.owner)))
				{
					it = m_Cameras.erase(it); // Remove expired (local) cameras
					continue;
				}
				++it;

				if (camera)
				{
					updateStreamingCamera(cam, std::move(camera));

					if (cam.owner != 0 && !v2Equal(cam.lastUsedPosition, cam.streamPosition, 0.5f))
					{
						RakNet::BitStream bs;
						bs.Write(cam.id);
						bs.Write(cam.streamPosition.x);
						bs.Write(cam.streamPosition.y);
						NetworkManager::GetNetwork()->Send(To::Populace(), false, MTID_REMOTECAMERA_MOVE, &bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_VIEWMANAGER);
					}
				}
			}
			const Vector2 &newPosition = cam.streamPosition;

			// Figure out the active range of this camera
			if (refresh || cam.firstUpdate || !v2Equal(cam.lastUsedPosition, cam.streamPosition, 0.5f))
			{
				Vector2 oldPosition = cam.lastUsedPosition;
				cam.lastUsedPosition = newPosition;
				cam.firstUpdate = false;

				allActiveRangesStale = false;

				if (cam.owner == 0)
					localStreamPositions.push_back(cam.streamPosition);

				CL_Rect inactiveRange;
				getCellRange(inactiveRange, oldPosition);

				CL_Rect& activeRange = cam.activeCellRange;
				getCellRange(activeRange, newPosition);
				//activeRange = range;

				if (inactiveRange != activeRange && (inactiveRange.get_width() != 0 && inactiveRange.get_height() != 0))
					inactiveRanges.push_back(std::move(inactiveRange));

				// TODO: don't create one big range for all overlapping ranges: split ranges like
				//  when an inactive range overlaps an active range
				bool merged = false;
				for (auto it = activeRanges.begin(), end = activeRanges.end(); it != end; ++it)
				{
					auto& existingRange = it->first;
					//if (activeRange.is_overlapped(existingRange))
					if (inclusiveOverlap(activeRange, existingRange))
					{
						existingRange.bounding_rect(activeRange);
						it->second.push_back(cam.streamPosition);
					}
				}
				if (!merged)
				{
					std::list<Vector2> pos; pos.push_back(cam.streamPosition);
					activeRanges.push_back(std::make_pair(activeRange, std::move(pos)));
				}
			}
			else // Camera hasn't moved
			{
				// Make sure entities from cells that just loaded are activted (even if the camera hasn't moved)
				for (auto it = m_CellsBeingLoaded.begin(), end = m_CellsBeingLoaded.end(); it != end;)
				{
					auto& cell = *it;
					Cell::mutex_t::scoped_try_lock lock(cell->mutex);
					if (lock && cell->IsLoaded())
					{
						std::list<Vector2> cams; cams.push_back(cam.streamPosition);
						processCell(*cell, cams);
						it = m_CellsBeingLoaded.erase(it);
						end = m_CellsBeingLoaded.end();
					}
					else
						++it;
				}

				// Add this cell's most recently calculated active range to make sure it doesn't get deactivated
				bool merged = false;
				for (auto it = activeRanges.begin(), end = activeRanges.end(); it != end; ++it)
				{
					auto& existingRange = it->first;
					//if (activeRange.is_overlapped(existingRange))
					if (inclusiveOverlap(cam.activeCellRange, existingRange))
					{
						existingRange.bounding_rect(cam.activeCellRange);
						it->second.push_back(cam.streamPosition);
					}
				}
				if (!merged)
				{
					std::list<Vector2> pos; pos.push_back(cam.streamPosition);
					activeRanges.push_back(std::make_pair(cam.activeCellRange, std::move(pos)));
				}
			}
		}

		// Clear loaded cells (this set is just used to make sure cells are processed if they finish loading after the camera that requested them stops moving)
		for (auto it = m_CellsBeingLoaded.begin(), end = m_CellsBeingLoaded.end(); it != end;)
		{
			auto cell = *it;
			if (cell->IsLoaded())
			{
				it = m_CellsBeingLoaded.erase(it);
				end = m_CellsBeingLoaded.end();
			}
			else
				++it;
		}
		//{
		//	auto newEnd = std::remove_if(m_CellsBeingLoaded.begin(), m_CellsBeingLoaded.end(), [](Cell* cell) { return cell->IsLoaded(); });
		//	m_CellsBeingLoaded.erase(newEnd, m_CellsBeingLoaded.end());
		//}

		// TODO: seperate activeRanges and staleActiveRanges?
		if (!allActiveRangesStale)
		{
		std::list<CL_Rect> clippedInactiveRanges;
		for (auto iit = inactiveRanges.begin(), iend = inactiveRanges.end(); iit != iend; ++iit)
		{
			for (auto ait = activeRanges.begin(), aend = activeRanges.end(); ait != aend; ++ait)
			{
				auto& inactiveRange = *iit;
				auto& activeRange = ait->first;
				clipRange(std::back_inserter(clippedInactiveRanges), inactiveRange, activeRange);
			}
		}

		// Sort to improve cache performance
		clippedInactiveRanges.sort([this](const CL_Rect& a, const CL_Rect& b)
		{
			return (a.top * m_XCellCount + a.left) < (b.top * m_XCellCount + b.left);
		});
		
		for (auto it = clippedInactiveRanges.begin(), end = clippedInactiveRanges.end(); it != end; ++it)
		{
			//std::stringstream inactivestr;
			//inactivestr << it->left << ", " << it->top << ", " << it->right << ", " << it->bottom;
			//SendToConsole("Deactivated " + inactivestr.str());
			deactivateCells(*it);
		}

		processCell(m_TheVoid, localStreamPositions);

		for (auto it = activeRanges.begin(), end = activeRanges.end(); it != end; ++it)
		{
			auto& activeRange = it->first;
			auto& streamPositions = it->second;

			if (activeRange.get_width() >= 0 && activeRange.get_height() >= 0)
			{
				//if (!clippedInactiveRanges.empty())
				//{
				//	std::stringstream activestr;
				//	activestr << activeRange.left << ", " << activeRange.top << ", " << activeRange.right << ", " << activeRange.bottom;
				//	SendToConsole("Activated " + activestr.str());
				//}

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

						cell.inRange = true;

						// Check if the cell needs to be loaded
						if (!cell.IsLoaded())
						{
							if (m_Archivist->Retrieve(&cell, i-1))
							{
								cell.AddHist("Retrieved due to entering range");
								//FSN_ASSERT(std::find(m_CellsBeingLoaded.cbegin(), m_CellsBeingLoaded.cend(), &cell) == m_CellsBeingLoaded.end());
								m_CellsBeingLoaded.insert(&cell);
							}
						}
						// Attempt to access the cell (it will be locked if the archivist is in the process of loading it)
						else
						{
							processCell(cell, streamPositions);
						}
					}
					i += stride;
				}

			}
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
