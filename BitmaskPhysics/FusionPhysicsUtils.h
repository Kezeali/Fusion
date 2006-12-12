/*
  Copyright (c) 2006 FusionTeam

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
*/

#ifndef Header_FusionEngine_PhysUtils
#define Header_FusionEngine_PhysUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{
	//! Maximum itterations for point-of-collision binary searches
	const int g_PhysMaxSearchItterations = 50;

	static float MIN_DIFF(float a, float b, float c)
	{
		a -= b;
		a += c / 2;
		a = fmod(a, c);
		if (a < 0) a += c;
		a -= c / 2;
		return a;
	}

	//! Methods which may be useful in relation to physics and collision detection.
	/*!
	 * \todo AABB and BB collision detection
	 */
	class PhysUtil
	{
	public:
		//! Finds the actual point of collision between two objects.
		/*!
		 * This will approximate the actual point of collision of two objects
		 * known to collide, assuming the given points are the points-of-first-
		 * collision (Given by PhysUtil#FindCollision(), for example.)
		 *
		 * \param[out] output
		 * The position of collision, if one is found.
		 * \param[in] pos_one
		 * The position to consider object one to be at when checking.
		 * \param[in] pos_one 
		 * The position to consider object two to be at when checking
		 * \param[in] other
		 * The object which may be colliding against this.
		 */
		static bool GuessPointOfCollision(
			CL_Vector2 *output,
			const CL_Vector2 &pos_one, const CL_Vector2 &pos_two,
			const FusionPhysicsBody *one, const FusionPhysicsBody *two);

		//! Used to check for collisions between moving objects.
		/*!
		 * If a collision is found, output_one/output_two will be set to the point along
		 * the given vector at which the respective body collides, not the actual point
		 * of collision (i.e. the point within the two bodies where they touch.) <br>
		 * If you need the actual point of collision, use GuessPointOfCollision().
		 *
		 * \remarks
		 * If no collisions are found, <b>output</b> will not be touched.
		 *
		 * \param[out] output_one
		 * The position at which one collides.
		 * \param[in] vector_one
		 * The movement vector for object one.
		 * \param[in] vector_two
		 * The movement vector for object two.
		 * \param[in] one The object to check for collisions against.
		 * \param[in] two The object which may be colliding against 'one'.
		 * \param[in] fuzz The accuracy to which the point of collision will be found.
		 */
		static bool FindCollisions(
			CL_Vector2 *output_one, CL_Vector2 *output_two,
			const CL_Vector2 &vector_one, const CL_Vector2 &vector_two,
			const FusionPhysicsBody *one, const FusionPhysicsBody *two,
			float epsilon = 0.01f);

		//! Checks the given point for a collisino between two bodies.
		/*!
		 * FindCollisions() uses this function at various points to find collisions.
		 *
		 * \param[in] pos_one
		 * The position to consider object one to be at when checking.
		 * \param[in] pos_one 
		 * The position to consider object two to be at when checking
		 * \param[in] one
		 * The object to check for collisions against.
		 * \param[in] two
		 * The object which may be colliding against param one.
		 */
		static bool CollisionCheck(
			const CL_Vector2 &pos_one, const CL_Vector2 &two_pos,
			const FusionPhysicsBody *one, const FusionPhysicsBody *two);

		//! Gets the intersection of two vectors.
		/*!
		 * This returns a point of intersection assuming the vectors have infinate
		 * length (starting at pos_one and pos_two, and with the gradients given by
		 * vector_one and vector_two), so the point returned may not be within the
		 * bounds of the actual vectors. For this reason you should use 
		 * CheckBoundaries() to verify the returned point.
		 *
		 * \param[out] output The intersection.
		 * \param[in] st_one The starting point of the first vector.
		 * \param[in] st_two The starting point of the second vector.
		 * \param[in] vector_one The first vector.
		 * \param[in] vector_two The second vector.
		 *
		 * \returns A CL_Vector2 - The point on the vectors where an intersection is found.
		 */
		static void CalculateVectorIntersection(CL_Vector2 *output,
			const CL_Vector2 &st_one, const CL_Vector2 &st_two,
			const CL_Vector2 &vector_one, const CL_Vector2 &vector_two);

		//! Returns true if the point is within the given vectors.
		/*!
		 * If the given point resides within the rectange definded by the extremities of
		 * the given vectors (that is to say, the smallest rectange that would fully contain
		 * the given vectors) this function will return true.
		 *
		 * \param[in] pos_one The starting point (offset) of the first vector
		 * \param[in] pos_two The starting point (offset) of the second vector
		 * \param[in] vec_one The first vector
		 * \param[in] vec_two The second vector
		 * \param[in] intersec The of the point to check
		 *
		 * \retval True If the point 'intersec' is on the given lines
		 */
		static bool CheckBoundaries(
			const CL_Vector2 &pos_one, const CL_Vector2 &pos_two,
			const CL_Vector2 &vec_one, const CL_Vector2 &vec_two,
			const CL_Vector2 &intersec);

		//! Calculates the normal to the surface of an object, at a certain point.
		/*!
		 * The point used for calculation will be the point on the surface of the given
		 * object closest to the given point.
		 *
		 * \param[out] output
		 * The normal.
		 * \param[in] point
		 * The (approx.) point to find the normal at.
		 * \param[in] body
		 * The body to find the normal on.
		 * \param[in] ref
		 * The body to use if a reference body is required.
		 */
		static void CalculateNormal(
			CL_Vector2 *output,
			const CL_Vector2 &ref_pos, const CL_Vector2 &other_pos,
			const FusionPhysicsBody *ref, const FusionPhysicsBody *other);

	};

}

#endif
