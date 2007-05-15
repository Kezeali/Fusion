/*
  Copyright (c) 2006-2007 Fusion Project Team

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

#ifndef Header_FusionEngine_AStar
#define Header_FusionEngine_AStar

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "micropather.h"
#include <OpenSteer/Pathway.h>

namespace FusionEngine
{
	/*!
	 * \brief
	 * Stores an AStar map
	 */
	class AStarPathway : public virtual OpenSteer::Pathway
	{
	public:
		//! Creates an AI module for the given script and ship.
		AStarPathway(const int pointCount, const Vector2 points[], const float radius, const bool cyclic);

	public:
		//! Util for construction
		void Initialize(const int pointCount,
		                const Vector2 points[],
		                const float radius,
		                const bool cyclic);

		//! Given an arbitrary point ("A"), returns the nearest point ("P") on this path.
		/*!
		 * Also returns, via output arguments, the path tangent at
		 * P and a measure of how far A is outside the Pathway's "tube".  Note
		 * that a negative distance indicates A is inside the Pathway.
		 */
		Vector2 mapPointToPath (const Vector2& point, Vector2& tangent, float& outside);

		//! given an arbitrary point, convert it to a distance along the path
		float mapPointToPathDistance (const Vector2& point);

		//! given a distance along the path, convert it to a point on the path
		OpenSteer::Vec3 mapPathDistanceToPoint (float pathDistance);

		//! Acssessor for total path length
		float getTotalPathLength(void) { return m_TotalPathLength; };

	public:
		int m_PointCount;
		Vector2* m_Points;
		float m_Radius;
		bool m_Cyclic;

	private:
		float* m_Lengths;
		Vector2* m_Normals;
		float m_TotalPathLength;

	};

}

#endif
