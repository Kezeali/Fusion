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
*  Many improvements to this streaming implementation were
*   inspired by Fiedler's Cubes (including some algorithms)
*  http://www.gafferongames.com/fiedlers-cubes
*
*  File Author:
*    Elliot Hayward
*/

#ifndef H_FusionStreamingManager
#define H_FusionStreamingManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <boost/signals2.hpp>

#include "FusionEntity.h"
#include "FusionCamera.h"
#include "FusionCameraManager.h"
#include "FusionIDStack.h"

#include "FusionHashable.h"

//#include <tbb/concurrent_unordered_map.h>
//#include <tbb/mutex.h>
#include <boost/thread/recursive_mutex.hpp>

namespace FusionEngine
{

	class CellDataSource;

	struct CellHandleGreater
	{
		bool operator() (const CellHandle& l, const CellHandle& r)
		{
			if (l.y == r.y)
				return l.x < r.x;
			else
				return l.y < r.y;
		}
	};

	static const float s_DefaultSmoothDecayRate = 0.01f;

	template <typename T>
	class Smooth
	{
	public:
		Smooth(T initial, float initial_tightness)
			: Value(initial),
			Target(initial),
			BaseTightness(initial_tightness),
			Tightness(initial_tightness),
			DecayRate(s_DefaultSmoothDecayRate)
		{
		}

		Smooth(T initial, float initial_tightness, float decay_rate)
			: Value(initial),
			Target(initial),
			BaseTightness(initial_tightness),
			Tightness(initial_tightness),
			DecayRate(decay_rate)
		{
		}

		void Update()
		{
			Value = Value + (Target-Value) * Tightness;

			if (fe_fequal(Value, Target))
				Tightness = BaseTightness;
			else
				Tightness = Tightness + (BaseTightness - Tightness) * DecayRate;
		}

		T Value;
		T Target;

		float Tightness;
		float BaseTightness;

		float DecayRate;
	};

	template <typename T>
	T fe_interpolate(T previous, T current, float alpha)
	{
		return previous * (1-alpha) + current * alpha;
	}

#define INFINITE_STREAMING
//#define STREAMING_USEMAP
//#define FSN_CELL_HISTORY

	struct CellEntry
	{
		enum State { Inactive = 0, Active = 1, Waiting = 2 };
		//bool active;
		State active;
		bool pendingDeactivation;
		float pendingDeactivationTime;

		float x, y;

		CellEntry()
			: active(Inactive),
			pendingDeactivation(false),
			pendingDeactivationTime(0.0f),
			x(0.0f),
			y(0.0f)
		{}
	};
	//typedef EntityPtr CellEntry;
	class Cell
	{
	public:
		Cell()
			: inRange(false),
			objects(0)
		{
			loaded = false;
			active_entries = 0;
			waiting = Ready;
		}

		void Reset()
		{
			inRange = false;
			objects.clear();
			loaded = false;
			active_entries = 0;
			waiting = Ready;
		}

	private:
		Cell(const Cell& other)
			: objects(other.objects),
			active_entries(other.active_entries),
			loaded(other.loaded)
		{
			waiting = Ready;
		}

		Cell(Cell&& other)
			: objects(std::move(other.objects)),
			active_entries(other.active_entries),
			loaded(other.loaded)
		{}

		Cell& operator= (const Cell& other)
		{
			objects = other.objects;
			active_entries = other.active_entries;
			loaded = other.loaded;
			return *this;
		}

		Cell& operator= (Cell&& other)
		{
			objects = std::move(other.objects);
			active_entries = other.active_entries;
			loaded = other.loaded;
			return *this;
		}
	public:
#ifdef STREAMING_USEMAP
		typedef std::map<Entity*, CellEntry> CellEntryMap;
		CellEntryMap objects;
#else
		typedef std::pair<EntityPtr, CellEntry> EntityEntryPair;
		typedef std::vector<EntityEntryPair> CellEntryMap;
		CellEntryMap objects;
#endif
		tbb::atomic<unsigned int> active_entries;
		void EntryUnreferenced() { FSN_ASSERT(active_entries > 0); --active_entries; AddHist("EntryUnreferenced", active_entries); }
		void EntryReferenced() { ++active_entries; AddHist("EntryReferenced", active_entries); }
		bool IsActive() const { return active_entries > 0; }

		tbb::atomic<bool> loaded;
		bool IsLoaded() const { return loaded; }

		bool inRange;

		enum WaitingState { Ready, Retrieve, Store };
		tbb::atomic<WaitingState> waiting;

		bool IsRetrieved() const { return loaded && waiting != Store; }

		typedef boost::recursive_mutex mutex_t;
		mutex_t mutex;

#ifdef FSN_CELL_HISTORY
		mutex_t historyMutex;
		std::vector<std::string> history;
#endif
		void AddHist(const std::string& hist, unsigned int num = -1);
	};

	struct StreamingHandle
	{
		size_t cellIndex;
	};

	struct ActivationEvent
	{
		enum Type { Activate, Deactivate, DeactivateAll };
		Type type;
		EntityPtr entity;
	};

	struct RemoteActivationEvent
	{
		enum Type { Activate, Deactivate, DeactivateAll };
		Type type;
		PlayerID viewer;
		ObjectID entity;
		std::shared_ptr<RakNet::BitStream> state;
	};

	/*!
	 * \brief
	 * Streams in objects within camera range.
	 *
	 * \see
	 * Entity | Camera | EntityManager
	 */
	class StreamingManager : public CameraManager
	{
	public:
		static const float s_SmoothTightness;
		static const float s_FastTightness;

		//! Constructor
		StreamingManager(CellDataSource* archivist);
		//! Destructor
		~StreamingManager();

		//! Initialises the manager
		void Initialise(float cell_size);
		//! Clears out all held cells
		void Reset();

		//! Adds the given camera
		void AddCamera(const CameraPtr &cam, float range = -1.f);
		//! Removes the given camera
		void RemoveCamera(const CameraPtr &cam);
		//! Adds an owned camera (for network sync)
		void AddOwnedCamera(PlayerID owner, const CameraPtr& cam, float range = -1.f);
		
		//! Sets the default range within which Entities are streamed in
		/*
		* Note that this is just the default; individual cameras can have their
		* own range setting.
		*/
		void SetRange(float sim_units);
		//! Returns the default range
		float GetRange() const;

		//CL_Rectf CalculateActiveArea(PlayerID net_idx) const;

		//unsigned int GetNumCellsAcross() const { return m_XCellCount; }
		//float GetMapWidth() const { return m_Bounds.x * 2.f; }
		float GetCellSize() const { return m_CellSize; }

		inline CellHandle ToCellLocation(float x, float y) const;
		inline CellHandle ToCellLocation(const Vector2 &position) const;

		inline Cell *CellAtCellLocation(const CellHandle& cell_location);

		inline Cell *CellAtPosition(float x, float y);
		inline Cell *CellAtPosition(const Vector2 &position);

		inline std::pair<CellHandle, Cell*> CellAndLocationAtPosition(const Vector2 &position);

		//! Pass all loaded cells to the archivist to be stored
		/*!
		* \param setup_refresh
		* Setting this to true will make the next call to Update() do a full refresh
		* (process all cameras) so any cells that were being retrieved before this was
		* called will continue to be loaded.
		*/
		void StoreAllCells(bool setup_refresh = true);

		//! Writes info needed when loading saves
		void Save(std::ostream& stream);
		//! Loads saved info
		void Load(std::istream& stream);

		void AddEntity(const EntityPtr &entity);
		void RemoveEntity(const EntityPtr &entity);
		//! Updates the given entity's grid position, and streams in/out
		void OnUpdated(const EntityPtr &entity, float dt);
		void OnUnreferenced(const EntityPtr& entity);

		//! Move & update archived entity data
		void UpdateInactiveEntity(ObjectID id, const Vector2& position, const std::shared_ptr<RakNet::BitStream>& continuous_data, const std::shared_ptr<RakNet::BitStream>& occasional_data);

		bool ActivateEntity(ObjectID id);
		void DeactivateEntity(const EntityPtr &entity);

		boost::signals2::signal<void (const ActivationEvent&)> SignalActivationEvent;

		boost::signals2::signal<void (const RemoteActivationEvent&)> SignalRemoteActivationEvent;

		//const std::set<EntityPtr> &GetActiveEntities() const;

		//! Calculates the active streaming area for each camera
		void Update(const bool refresh = false);

		static void Register(asIScriptEngine *engine);

	private:
		//! Streaming camera data
		struct StreamingCamera
		{
			//! Ctor
			StreamingCamera()
				: tightness(0.0f), firstUpdate(true), range(0.0f), rangeSquared(0.0f), defaultRange(true), owner(0), id(0)
			{}
			//! Move ctor
			StreamingCamera(StreamingCamera&& other)
				: tightness(other.tightness),
				firstUpdate(other.firstUpdate),
				range(other.range),
				rangeSquared(other.rangeSquared),
				defaultRange(other.defaultRange),
				owner(other.owner),
				id(other.id),
				camera(std::move(other.camera)),
				streamPosition(other.streamPosition),
				lastUsedPosition(other.lastUsedPosition),
				activeCellRange(other.activeCellRange),
				lastPosition(other.lastPosition),
				lastVelocity(other.lastVelocity)
			{}

			//! Owner for owned cameras
			PlayerID owner;
			//! How this is identified to other systems
			uint8_t id;

			//! Render-camera object (if this is a local camera)
			/*!
			* This may not be available for remote cameras (cameras owned by
			* a player on another peer.) In that case, the position is sync'd
			* over the network, rather than taken from an actual local camera.
			*/
			std::weak_ptr<Camera> camera;

			//! The current middle of the streaming area for the camera
			//! Moves ahead based on the camera velocity
			Vector2 streamPosition;
			//! The streamPosition that was most recently actualy processed
			Vector2 lastUsedPosition; 

			//! Activation range of this camera, it it isn't default
			float range;
			//! range squared
			float rangeSquared;
			//! True if this camera should use the default range (m_Range)
			bool defaultRange;

			//! The rect. of cells activated by this camera last time it was processed
			CL_Rect activeCellRange;

			//! Position last time this camera was updated
			Vector2 lastPosition;
			//! Velocity (size of jump) last time this camera was updated
			Vector2 lastVelocity;
			//! Smoothing tightness (increases over time as the camera moves)
			float tightness;

			//! Will be set to true when a camera has just been added - makes sure it gets processed
			bool firstUpdate; 

			//! Functor: returns true if the given StreamingCamera uses a specific render Camera
			struct HasSameCamera
			{
				explicit HasSameCamera(const CameraPtr &cam) : newCamera(cam)
				{}

				bool operator() (const StreamingCamera& streamingCamera)
				{
					return streamingCamera.camera.lock() == newCamera;
				}

				const CameraPtr& newCamera;
			};

		private:
			//! Copy ctor (Disabled)
			StreamingCamera(const StreamingCamera& other)
				: tightness(other.tightness),
				firstUpdate(other.firstUpdate),
				range(other.range),
				rangeSquared(other.range),
				defaultRange(other.defaultRange),
				owner(other.owner),
				camera(other.camera),
				streamPosition(other.streamPosition),
				lastUsedPosition(other.lastUsedPosition),
				activeCellRange(other.activeCellRange),
				lastPosition(other.lastPosition),
				lastVelocity(other.lastVelocity)
			{
			}
		};

		typedef boost::recursive_mutex CamerasMutex_t;
		CamerasMutex_t m_CamerasMutex;

		StreamingCamera& createStreamingCamera(PlayerID owner, const CameraPtr& controller, float range);

		std::vector<StreamingCamera> m_Cameras;

		IDSet<uint8_t> m_CamIds;

		float m_DeactivationTime;

		float m_Range;
		float m_RangeSquared;

		float m_CellSize;
		float m_InverseCellSize;

		//unsigned int m_XCellCount;
		//unsigned int m_YCellCount;

		//Vector2 m_Bounds;

		// Cells are sorted by y then x, so they can be iterated over linearly when processing
		std::map<CellHandle, std::shared_ptr<Cell>, CellHandleGreater> m_Cells;
		// Where entities are stored until the correct cell is loaded for them
		Cell m_TheVoid;
		std::map<CellHandle, std::shared_ptr<Cell>, CellHandleGreater> m_CellsBeingLoaded;
		// Entities that have been requested useing the public method ActivateEntity(ObjectID)
		std::map<CellHandle, std::set<ObjectID>, CellHandleGreater> m_RequestedEntities;

		std::unordered_map<ObjectID, size_t> m_EntityDirectory;

		CellDataSource* m_Archivist;


		std::shared_ptr<Cell>& RetrieveCell(const CellHandle &location);
		void StoreCell(const CellHandle& location);
		//! Makes sure that the given cell is in either Ready or Retrieve state
		bool ConfirmRetrieval(const CellHandle &location, Cell* cell);

		void ActivateEntity(const CellHandle& cell_location, Cell &cell, const EntityPtr &entity, CellEntry &entry);
		void DeactivateEntity(Cell &cell, const EntityPtr &entity, CellEntry &entry);

		void RemoteActivateEntity(CellEntry& entry, ObjectID entity, PlayerID viewer, std::shared_ptr<RakNet::BitStream> state);

		void QueueEntityForDeactivation(CellEntry &entry, bool warp = false);

		void GenerateActivationEvent(const EntityPtr &entity);
		void GenerateDeactivationEvent(const EntityPtr &entity);

		void GenerateRemoteActivationEvent(ObjectID entity, PlayerID viewer, std::shared_ptr<RakNet::BitStream> state);
		void GenerateRemoteDeactivationEvent(ObjectID entity, PlayerID viewer);


		void changeCell(Cell::EntityEntryPair& entry, Cell& current_cell, Cell& new_cell);

		void getCellRange(CL_Rect& out, const Vector2& pos, const float camera_range);

		void deactivateCells(const CL_Rect& inactiveRange);

		// TODO: replace the pair here with a struct CamToProcess { Vec2, float }
		void processCell(const CellHandle& cell_location, Cell& cell, const std::list<std::pair<Vector2, float>>& cam_position, const std::list<std::pair<std::pair<Vector2, float>, PlayerID>>& remote_positions);

		void activateInView(const CellHandle& cell_location, Cell *cell, CellEntry *cell_entry, const EntityPtr &entity, bool warp);

		bool updateStreamingCamera(StreamingCamera &cam, CameraPtr camera);
	};

}

#endif
