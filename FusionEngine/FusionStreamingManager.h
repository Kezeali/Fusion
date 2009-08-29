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

		void SetPlayerCamera(ObjectID net_idx, const CameraPtr &cam);
		void RemovePlayerCamera(ObjectID net_idx);
		
		//! Sets the range within which Entities are streamed in
		void SetRange(float game_units);

		CL_Rectf CalculateActiveArea(ObjectID net_idx) const;

		//! Streams the entity in if it is within range
		void ProcessEntity(const EntityPtr &entity) const;

		//! Calculates the active streaming area for each camera
		void Update();

	private:
		struct StreamingCamera
		{
			CameraPtr Camera;
			Vector2 LastPosition;
			// The current middle of the streaming area for the camera
			//  - Moves ahead based on the camera velocity
			Vector2 StreamPoint;

			Vector2 LastVelocity;
			float Tightness;
		};

		//! Returns true if the entity is within the streaming area of the camera
		bool processEntity(const StreamingCamera &cam, const EntityPtr &entity) const;

		typedef std::map<ObjectID, StreamingCamera> StreamingCameraMap;
		StreamingCameraMap m_Cameras;

		float m_Range;
	};

}

#endif
