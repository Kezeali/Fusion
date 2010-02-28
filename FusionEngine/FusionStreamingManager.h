/*
  Copyright (c) 2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward

*/

#ifndef Header_FusionEngine_StreamingManager
#define Header_FusionEngine_StreamingManager

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
		typedef std::map<EntityPtr, CellEntry> CellEntryMap;
		CellEntryMap objects;
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

		void SetPlayerCamera(PlayerID net_idx, const CameraPtr &cam);
		void RemovePlayerCamera(PlayerID net_idx);
		
		//! Sets the range within which Entities are streamed in
		void SetRange(float game_units);

		//CL_Rectf CalculateActiveArea(PlayerID net_idx) const;

		Cell *CellAtPosition(const Vector2 &position);
		Cell *CellAtPosition(float x, float y);

		void AddEntity(const EntityPtr &entity);
		void RemoveEntity(const EntityPtr &entity);
		//! Updates the given entity's grid position, and streams in/out
		void OnMoved(const EntityPtr &entity);

		void ActivateEntity(const EntityPtr &entity, CellEntry &entry, Cell &cell);
		void DeactivateEntity(const EntityPtr &entity);

		void QueueEntityForDeactivation(CellEntry &entry, bool warp = false);

		void GenerateActivationEvent(const EntityPtr &entity);
		void GenerateDeactivationEvent(const EntityPtr &entity);

		boost::signals2::signal<void (ActivationEvent)> SignalActivationEvent;

		//const std::set<EntityPtr> &GetActiveEntities() const;

		//! Calculates the active streaming area for each camera
		void Update();

		static void Register(asIScriptEngine *engine);

	private:
		struct StreamingCamera
		{
			CameraPtr Camera;
			// The current middle of the streaming area for the camera
			//  - Moves ahead based on the camera velocity
			Vector2 StreamPosition;

			Vector2 LastPosition;
			Vector2 LastVelocity;
			float Tightness;
		};

		typedef std::map<PlayerID, StreamingCamera> StreamingCameraMap;
		StreamingCameraMap m_Cameras;

		float m_DeactivationTime;

		float m_Range;
		float m_RangeSquared;

		float m_CellSize;
		float m_InverseCellSize;

		unsigned int m_XCellCount;
		unsigned int m_YCellCount;

		Vector2 m_Bounds;

		Cell *m_Cells;

		//std::set<EntityPtr> m_ActiveEntities;

		//! Returns true if the entity is within the streaming area of the camera
		bool activateWithinRange(const StreamingCamera &cam, const EntityPtr &entity, CellEntry &cell_entry);

		bool updateStreamingCamera(StreamingCamera &cam);
	};

}

#endif
