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

#include "FusionAStarPathway.h"

#include <OpenSteer/Utilities.h>

namespace FusionEngine
{

	AStarPathway::AStarPathway(const int pointCount, const Vector2 points[], const float radius, const bool cyclic)
	{
		Initialize(pointCount, points, radius, cyclic);
	}

	void AStarPathway::Initialize(const int pointCount,
		                            const Vector2 points[],
		                            const float radius,
		                            const bool cyclic)
	{
		// set data members, allocate arrays
		m_Radius = radius;
		m_Cyclic = cyclic;
		m_PointCount = pointCount;
		m_TotalPathLength = 0;
		if (m_Cyclic) m_PointCount++;
		m_Lengths = new float  [m_PointCount];
		m_Points  = new Vector2[m_PointCount];
		m_Normals = new Vector2[m_PointCount];

		// loop over all points
		for (int i = 0; i < m_PointCount; i++)
		{
			// copy in point locations, closing cycle when appropriate
			const bool closeCycle = m_Cyclic && (i == m_PointCount-1);
			const int j = closeCycle ? 0 : i;
			m_Points[i] = m_Points[j];

			// for the end of each segment
			if (i > 0)
			{
				// compute the segment length
				m_Normals[i] = m_Points[i] - m_Points[i-1];
				m_Lengths[i] = m_Normals[i].length();

				// find the normalized vector parallel to the segment
				m_Normals[i] *= 1 / m_Lengths[i];

				// keep running total of segment lengths
				m_TotalPathLength += m_Lengths[i];
			}
		}
	}

	Vector2 AStarPathway::mapPointToPath(const Vector2& point, Vector2& tangent, float& outside)
	{
		float d;
		float minDistance = FLT_MAX;
		Vector2 onPath;

		// loop over all segments, find the one nearest to the given point
		for (int i = 1; i < m_PointCount; i++)
		{
			// Get the pre-calculated length and normal for this segment
			float segmentLength = m_Lengths[i];
			Vector2 segmentNormal = m_Normals[i];
			// Output
			float segmentProjection; // We won't use this, but it's outputted so meh
			Vector2 chosen;

			d = Vector2::pointToSegmentDistance(
				point, m_Points[i-1], m_Points[i], segmentLength, segmentNormal,
				// Output args:
				segmentProjection, chosen);
			if (d < minDistance)
			{
				minDistance = d;
				onPath = chosen;
				tangent = segmentNormal;
			}
		}

		// measure how far original point is outside the Pathway's "tube"
		outside = Vector2::distance(onPath, point) - m_Radius;

		// return point on path
		return onPath;
	}

	float AStarPathway::mapPointToPathDistance(const Vector2& point)
	{
		float d;
		float minDistance = FLT_MAX;
		float segmentLengthTotal = 0;
		float pathDistance = 0;

		for (int i = 1; i < m_PointCount; i++)
		{
			// Get the pre-calculated length and normal for this segment
			float segmentLength = m_Lengths[i];
			Vector2 segmentNormal = m_Normals[i];
			// Output
			float segmentProjection;
			Vector2 chosen; // We won't use this, but it's outputted so meh

			d = Vector2::pointToSegmentDistance(
				point, m_Points[i-1], m_Points[i], segmentLength, segmentNormal,
				// Output args:
				segmentProjection, chosen);
			if (d < minDistance)
			{
				minDistance = d;
				pathDistance = segmentLengthTotal + segmentProjection;
			}
			segmentLengthTotal += segmentLength;
		}

		// return distance along path of onPath point
		return pathDistance;
	}

	OpenSteer::Vec3 AStarPathway::mapPathDistanceToPoint(float pathDistance)
	{
		using namespace OpenSteer;
		// clip or wrap given path distance according to cyclic flag
		float remaining = pathDistance;
		if (m_Cyclic)
		{
			remaining = (float) fmod (pathDistance, m_TotalPathLength);
		}
		else
		{
			if (pathDistance < 0)
				return Vec3(m_Points[0].x, m_Points[0].y, 0);

			if (pathDistance >= m_TotalPathLength)
				return Vec3(m_Points[m_PointCount-1].x, m_Points[m_PointCount-1].y, 0);
		}

		// step through segments, subtracting off segment lengths until
		// locating the segment that contains the original pathDistance.
		// Interpolate along that segment to find 3d point value to return.
		Vector2 result;
		for (int i = 1; i < m_PointCount; i++)
		{
			float segmentLength = m_Lengths[i];
			if (segmentLength < remaining)
			{
				remaining -= segmentLength;
			}
			else
			{
				float ratio = remaining / segmentLength;
				result = OpenSteer::interpolate(ratio, m_Points[i-1], m_Points[i]);
				break;
			}
		}
		return Vec3(result.x, result.y, 0);
	}

}