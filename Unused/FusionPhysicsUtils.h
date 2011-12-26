/*
  Copyright (c) 2006 Fusion Project Team

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

#include "FusionCommon.h"

namespace FusionEngine
{
	//! Maximum itterations for point-of-collision binary searches
	const int g_PhysMaxSearchItterations = 50;
	//! Accuracy of comparasons
	const float g_PhysGenericFuzz = 0.001f;

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
	 * \todo AABB and BB collision detection.
	 *
	 * \todo Get rid of memcpy where Vector2::operator= could be used, because
	 * copying two (int)s should be faster than copying a whole Vector2...
	 */
	namespace PhysUtil
	{
		//! Finds the actual point of collision between two objects.
		/*!
		 * This will approximate the actual point of collision of two objects
		 * known to collide, assuming the given points are the positions-of-first-
		 * collision (Given by PhysUtil#FindCollision(), for example.)
		 *
		 * \param[out] output
		 * The position of collision, if one is found.
		 *
		 * \param[in] pos_one
		 * The position to consider object (one) to be at when checking.
		 *
		 * \param[in] pos_one 
		 * The position to consider object (two) to be at when checking.
		 *
		 * \param[in] one
		 * The the first object involved the collision.
		 *
		 * \param[in] two
		 * The second object involved in the collision.
		 */
		bool GuessPointOfCollision(
			Vector2 *output,
			const Vector2 &pos_one, const Vector2 &pos_two,
			const PhysicsBody *one, const PhysicsBody *two);

		//! Used to check for collisions between moving objects.
		/*!
		 * <p>
		 * If a collision is found, output_a1/output_a2 will be set to the point along
		 * the given vector at which the respective body collides, not the actual point
		 * of collision (i.e. the point within the two bodies where they touch.) <br>
		 * If you need the actual point of collision, use GuessPointOfCollision().
		 * </p>
		 * output_b1 and output_b2 will be set to the final positions at which each
		 * body <b>does not</b> collide - this is a safe position to pop back to.
		 *
		 * \remarks
		 * If no collisions are found, <b>output</b> will not be touched.
		 *
		 * \param[out] output_a1
		 * The position at which one collides. Use this point when finding a normal
		 * for bitmask collisions.
		 * \param[out] output_a2
		 * The position at which two collides. Use this point when finding a normal
		 * for bitmask collisions.
		 *
		 * \param[out] output_b1
		 * A position just before one collides.
		 * \param[out] output_b2
		 * A position just before two collides.
		 *
		 *
		 * \param[in] vector_one
		 * The movement vector for object one.
		 * \param[in] vector_two
		 * The movement vector for object two.
		 *
		 * \param[in] one 
		 * The object to check for collisions against.
		 * \param[in] two
		 * The object which may be colliding against 'one'.
		 *
		 * \param[in] epsilon
		 * The accuracy to which the point of collision will be found.
		 *
		 * \param[in] find_close
		 * Whether to return the closest colliding point to the beginning of
		 * the vector (true), or the furthermost point (false)
		 */
		bool FindCollisions(
			Vector2 *output_a1, Vector2 *output_a2,
			Vector2 *output_b1, Vector2 *output_b2,
			const Vector2 &vector_one, const Vector2 &vector_two,
			const PhysicsBody *one, const PhysicsBody *two,
			float epsilon = 0.01f, bool find_close = true);

		//! Checks the given point for a collision between two bodies.
		/*!
		 * FindCollisions() uses this function at various points to find collisions.
		 *
		 * \remarks
		 * If at all possible, try to avoid needing to make bitmask against non-bitmask
		 * collision checks (by making sure all objects have a bitmask) as it can throw 
		 * errors if done wrong.
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
		bool CollisionCheck(
			const Vector2 &pos_one, const Vector2 &two_pos,
			const PhysicsBody *one, const PhysicsBody *two);

		//! [depreciated] Use the ClanLib line math function.
		/*!
		 * Gets the intersection of two vectors.
		 *
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
		 * \returns A Vector2 - The point on the vectors where an intersection is found.
		 */
		void CalculateVectorIntersection(Vector2 *output,
			const Vector2 &st_one, const Vector2 &st_two,
			const Vector2 &vector_one, const Vector2 &vector_two);

		//! Returns true if the point is within the given vectors.
		/*!
		 * <p>
		 * If the given point resides within the rectange definded by the extremities of
		 * the given vectors (that is to say, the smallest rectange that would fully contain
		 * the given vectors) this function will return true.
		 * </p>
		 * Only use this if you already know the point lies on an infinate line from
		 * the given vectors. (e.g. if the point was returned by 
		 * CL_LineMath#get_intersection())
		 *
		 * \param[in] pos_one The starting point (offset) of the first vector
		 * \param[in] pos_two The starting point (offset) of the second vector
		 * \param[in] vec_one The first vector
		 * \param[in] vec_two The second vector
		 * \param[in] intersec The of the point to check
		 *
		 * \retval True If the point 'intersec' is on the given lines
		 */
		bool CheckBoundaries(
			const Vector2 &pos_one, const Vector2 &pos_two,
			const Vector2 &vec_one, const Vector2 &vec_two,
			const Vector2 &intersec);

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
		void CalculateNormal(
			Vector2 *output,
			const Vector2 &ref_pos, const Vector2 &other_pos,
			const PhysicsBody *ref, const PhysicsBody *other);

	};

}

#endif