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

namespace FusionEngine
{

	static const float s_DefaultSmoothDecayRate = 0.01f;

	template <typename T>
	class Smooth
	{
	public:
		Smooth(T initial, float initial_tightness)
			: Value(initial)
			Target(initial),
			BaseTightness(initial_tightness),
			Tightness(initial_tightness),
			DecayRate(s_DefaultSmoothDecayRate)
		{
		}

		Smooth(T initial, float initial_tightness, float decay_rate)
			: Value(initial)
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

	template <typename T>
	class Interpolator
	{
	public:
		Interpolator(T* initial)
			: Value(initial),
			Initial(initial),
			Target(initial)
		{
		}

		void Update(float alpha)
		{
			*Value = Initial * (1-alpha) + Target * alpha;
		}

		T* Value;
		T Initial;
		T Target;
	};

#define INFINITE_STREAMING
//#define STREAMING_USEMAP

	struct CellEntry
	{
		bool active;
		bool pendingDeactivation;
		float pendingDeactivationTime;

		float x, y;

		CellEntry()
			: active(false),
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
#ifdef STREAMING_USEMAP
		typedef std::map<Entity*, CellEntry> CellEntryMap;
		CellEntryMap objects;
#else
		typedef std::pair<Entity*, CellEntry> EntityEntryPair;
		typedef std::vector<EntityEntryPair> CellEntryMap;
		CellEntryMap objects;
#endif
	};

	struct ActivationEvent
	{
		enum Type { Activate, Deactivate };
		Type type;
		EntityPtr entity;
	};

	/*!
	 * \brief
	 * Streams in objects within camera range.
	 *
	 * \see
	 * Entity | Camera | EntityManager
	 */
	class StreamingManager
	{
	public:
		static const float s_SmoothTightness;
		static const float s_FastTightness;

		//! Constructor
		StreamingManager();
		//! Destructor
		~StreamingManager();

		void AddCamera(const CameraPtr &cam);
		void RemoveCamera(const CameraPtr &cam);
		
		//! Sets the range within which Entities are streamed in
		void SetRange(float game_units);
		float GetRange() const;

		//CL_Rectf CalculateActiveArea(PlayerID net_idx) const;

		Cell *CellAtPosition(const Vector2 &position);
		Cell *CellAtPosition(float x, float y);

		void AddEntity(const EntityPtr &entity);
		void RemoveEntity(const EntityPtr &entity);
		//! Updates the given entity's grid position, and streams in/out
		void OnUpdated(const EntityPtr &entity, float split);

		void ActivateEntity(const EntityPtr &entity, CellEntry &entry, Cell &cell);
		void DeactivateEntity(const EntityPtr &entity);
		void DeactivateEntity(const EntityPtr &entity, CellEntry &entry);

		void QueueEntityForDeactivation(CellEntry &entry, bool warp = false);

		void GenerateActivationEvent(const EntityPtr &entity);
		void GenerateDeactivationEvent(const EntityPtr &entity);

		boost::signals2::signal<void (const ActivationEvent&)> SignalActivationEvent;

		//const std::set<EntityPtr> &GetActiveEntities() const;

		//! Calculates the active streaming area for each camera
		void Update(const bool refresh = false);

		static void Register(asIScriptEngine *engine);

	private:
		struct StreamingCamera
		{
			StreamingCamera() : tightness(0.0f), firstUpdate(true)
			{}

			std::weak_ptr<Camera> camera;

			// The current middle of the streaming area for the camera
			//  - Moves ahead based on the camera velocity
			Vector2 streamPosition;

			Vector2 lastPosition;
			Vector2 lastVelocity;
			float tightness;

			bool firstUpdate; // Will be set to true when a cam. has just been added - makes sure it gets processed

			struct IsObserver
			{
				explicit IsObserver(const CameraPtr &cam) : observedCamera(cam)
				{}

				bool operator() (const StreamingCamera& streamingCamera)
				{
					return streamingCamera.camera.lock() == observedCamera;
				}

				const CameraPtr& observedCamera;
			};
		};

		typedef std::map<PlayerID, StreamingCamera> StreamingCameraMap;
		std::vector< StreamingCamera > m_Cameras;

		float m_DeactivationTime;

		float m_Range;
		float m_RangeSquared;

		float m_CellSize;
		float m_InverseCellSize;

		unsigned int m_XCellCount;
		unsigned int m_YCellCount;

		Vector2 m_Bounds;

		Cell *m_Cells;


		void activateInView(Cell *cell, CellEntry *cell_entry, const EntityPtr &entity, bool warp);

		bool updateStreamingCamera(StreamingCamera &cam, CameraPtr camera);
	};

}

#endif
