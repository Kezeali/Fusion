
#include "Common.h"

#include "FusionPhysicsUtils.h"

/// Fusion
#include "FusionBitmask.h"
#include "FusionPhysicsBody.h"

namespace FusionEngine
{

	bool PhysUtil::GuessPointOfCollision(
			Vector2 *output,
			const Vector2 &pos_one, const Vector2 &pos_two,
			const FusionPhysicsBody *one, const FusionPhysicsBody *two)
	{

		// Check for distance collision
		// This is the preffered method, as it should be the most accurate
		if (one->GetUseDistCollisions() & two->GetUseDistCollisions())
		{
			// Find a vector between the two objects
			Vector2 d_pos = pos_two - pos_one;
			// Find the point where the vector reaches the collision distance
			d_pos	*= one->GetColDist() / d_pos.length();

			// Using overloaded assignment operator rather than memcpy, because
			//  copying two ints should be faster than copying a whole vector
			(*output) = d_pos;

			//memcpy(output, &pos, sizeof(Vector2));
			return true;
		}

		// Check for bitmask collisions
		else if (one->GetUsePixelCollisions() & two->GetUsePixelCollisions())
		{
			CL_Point offset = CL_Point(pos_one.x - pos_two.x, pos_one.y - pos_two.y);

			CL_Point out_pt;
			if ((one->GetColBitmask()->OverlapPoint(&out_pt, two->GetColBitmask(), offset)))
			{
				(*output) = Vector2(out_pt.x, out_pt.y);

				//memcpy(output, &Vector2(out_pt.x, out_pt.y), sizeof(Vector2));
				return true;
			}
		}
		
		// Check for bitmask collisons against non-bitmask objects
		// 2006/12/11: I don't know if this works...
		else if (one->GetUsePixelCollisions())
		{

			if ( one->GetColPoint( two->GetPositionPoint() ) )
			{
				(*output) = two->GetPosition();

				//memcpy(output, &Vector2(point.x, point.y), sizeof(Vector2));
				return true;
			}
		}

		else if (two->GetUsePixelCollisions())
		{
			if ( two->GetColPoint( one->GetPositionPoint() ) )
			{
				(*output) = one->GetPosition();
				//memcpy(output, one->GetPosition(), sizeof(Vector2));
				return true;
			}
		}

		// No collision found or unsupported collision methods/combination
		return false;
	}

	bool PhysUtil::FindCollisions(
		Vector2 *output_a1, Vector2 *output_a2,
		Vector2 *output_b1, Vector2 *output_b2,
		const Vector2 &vector_one, const Vector2 &vector_two,
		const FusionPhysicsBody *one, const FusionPhysicsBody *two,
		float epsilon, bool find_close)
	{
		// Positions
		Vector2 pos_one = one->GetPosition();
		Vector2 pos_two = two->GetPosition();

		// Squared movement vector lengths (speed2) (used for checking for movement)
		float speed2_one = vector_one.squared_length();
		float speed2_two = vector_two.squared_length();

		//// Objects not moving - don't use vector check
		//if (speed2_one < epsilon && speed2_two < epsilon)
		//{
		//	if (CollisionCheck(pos_one, pos_two, one, two))
		//	{
		//		(*output_a1) = pos_one;
		//		(*output_a2) = pos_two;

		//		//memcpy(output_one, &pos_one, sizeof(Vector2));
		//		//memcpy(output_two, &pos_two, sizeof(Vector2));
		//		
		//		return true;
		//	}

		//	return false;
		//}

		// These will store the start and end points while searching.
		Vector2 startpt_one, endpt_one, startpt_two, endpt_two;

		// If the search finishes, we will assume these are set to the positions of collision.
		Vector2 checkpt_one, checkpt_two;

		// This will be set to true the first time a CollisionCheck() returns true
		//  during the binary search - if collision check never returns true, we can
		//  assume there are no collisions along the given vectors.
		bool collision_found = false;

		//////////////////////////////////////////////
		// First we decide on the bounds of the search

		// Will be set to false if a point of intersection is found
		bool no_interection = true;

		// Don't bother checking for intersections on really short vectors
		if (speed2_one > epsilon)
		{

			float line_a[] = {pos_one.x, pos_one.y, vector_one.x, vector_one.y};
			float line_b[] = {pos_two.x, pos_two.y, vector_two.x, vector_two.y};
			float *line_a_ptr = line_a;
			float *line_b_ptr = line_b;

			// No point using CL_LineMath::intersects() here, because it only tells
			//  you if two /infinate/ lines intersect. So we just get a point of inter-
			//  section and check that it is in the bounds of the actual lines.
			CL_Pointf isec_point = CL_LineMath::get_intersection(line_a_ptr, line_b_ptr);

			Vector2 intersec(isec_point.x, isec_point.y);

			if (CheckBoundaries(pos_one, pos_two, vector_one, vector_two, intersec))
			{
				// If a point of intersection was found, set up the search points to
				//  find the first place where the two objects collide /before/ there

				startpt_one = pos_one;
				endpt_one   = intersec;
				startpt_two = pos_two;
				endpt_two   = intersec;

				no_interection = false;
			}

		}

		if (no_interection)
		{
			// If no point of intersection was found, set the search points to
			//  to use the whole vector.

			startpt_one = pos_one;
			startpt_two = pos_two;
			endpt_one   = pos_one + vector_one;
			endpt_two   = pos_two + vector_two;
		}

		// If the objects are already colliding, expand the search limits
		if (find_close && CollisionCheck(pos_one, pos_two, one, two))
		{
			Vector2 n;
			CalculateNormal(&n, pos_one, pos_two, one, two);
			if (two->GetCollisionFlags() & C_STATIC)
			{
				Vector2 add1 = n;

				int i = g_PhysMaxSearchItterations;
				while (CollisionCheck(startpt_one, startpt_two, one, two) && i--)
				{
					endpt_one = startpt_one;

					startpt_one += add1;
				}
			}
			else if (one->GetUseDistCollisions() && two->GetUseDistCollisions())
			{
				Vector2 d = pos_one - pos_two;
				float p = ( (one->GetColDist() + one->GetColDist()) - d.length() ) * 0.5f;
				startpt_one += n*p;
				startpt_two -= n*p;
			}
		}



		//////////////////////////////
		// Now we do the binary search

		// Distance between max/min point
		//  When this vector is sufficiantly small, we assume that it's mid-point,
		//  which will be the current 'check_pt', is the point of collision.
		//  (But I shouldn't have to tell you that. You _do_ know how a binary search
		//  works right?)
		Vector2 start_end = startpt_one - endpt_one;
		float dist_squared = start_end.squared_length();

		int i = g_PhysMaxSearchItterations;
		while ((dist_squared > epsilon) && i--)
		{

			// Find the midpoints along the check vectors
			//  (via mid = a + half distance(a to b) )
			checkpt_one = startpt_one + (endpt_one - startpt_one)*0.5f;
			checkpt_two = startpt_two + (endpt_two - startpt_two)*0.5f;

			if (CollisionCheck(checkpt_one, checkpt_two, one, two))
			{
				// Find the first point of collision
				if (find_close)
				{					
					// Collision must be before this point, so move the
					//  end bounds back
					endpt_one = checkpt_one;
					endpt_two = checkpt_two;

					// We can safely say that the objects collide /somewhere/ along the
					//  given vectors
					collision_found = true;
				}

				// Find the _last_ point of collision
				else
				{
					startpt_one = checkpt_one;
					startpt_two = checkpt_two;
					// In this case we don't set collision found to true, as we only
					//  want to do that for the last point of collision
				}
			}
			else
			{
				// Find the first point of collision
				if (find_close)
				{
					// Collision must be after this point, so move the
					//  start bounds forward
					startpt_one = checkpt_one;
					startpt_two = checkpt_two;
				}

				// Find the _last_ point of collision
				else
				{
					endpt_one = checkpt_one;
					endpt_two = checkpt_two;

					// We set collision found to true, in the assumption that if
					//  there is no collision here, the one we found previously was the last
					collision_found = true;
				}
			}


			// Check how far apart the points found are
			start_end = startpt_one - endpt_one;
			dist_squared = start_end.squared_length();
		}

		if (collision_found)
		{
			// Positions during collision
			(*output_a1) = checkpt_one;
			(*output_a2) = checkpt_two;

			// Non-colliding positions
			if (find_close)
			{
				(*output_b1) = startpt_one;
				(*output_b2) = startpt_two;
			}
			else
			{
				(*output_b1) = endpt_one;
				(*output_b2) = endpt_two;
			}

			//memcpy(output_one, &checkpt_one, sizeof(Vector2));
			//memcpy(output_two, &checkpt_two, sizeof(Vector2));
			return true;
		}
		return false;
	}

	bool PhysUtil::CollisionCheck(const Vector2 &one_pos, const Vector2 &two_pos, const FusionPhysicsBody *one, const FusionPhysicsBody *two)
	{
		// Check for distance collision
		if (one->GetUseDistCollisions() & two->GetUseDistCollisions())
		{
			Vector2 dp = one_pos - two_pos;

			// The required distance to create a collision against object one
			//  is expanded by the distance of the other.
			float dist = ( one->GetColDist() + two->GetColDist() );

			return (dp.squared_length() < (dist * dist));
		}
		
		// Check for bitmask collisions
		else if (one->GetUsePixelCollisions() & two->GetUsePixelCollisions())
		{
			CL_Point offset = CL_Point(one_pos.x - two_pos.x, one_pos.y - two_pos.y);
			return (two->GetColBitmask()->Overlap(one->GetColBitmask(), offset));
		}

		// Check for bitmask collisons against non-bitmask objects
		// Try to avoid needing to make this type of check (by making sure all objects
		//  have a bitmask) as it can throw errors if done wrong.
		else if (one->GetUsePixelCollisions())
		{
			return (one->GetColPoint( two->GetPositionPoint() ));
		}
		else if (two->GetUsePixelCollisions())
		{
			return (two->GetColPoint( one->GetPositionPoint() ));
		}

		// No collision found or unsupported collision methods/combination
		return false;
	}

	void PhysUtil::CalculateVectorIntersection(
		Vector2 *output,
		const Vector2 &st_one, const Vector2 &st_two,
		const Vector2 &vector_one, const Vector2 &vector_two)
	{
		float m1 = vector_one.y / vector_one.x;
		float d1 = st_one.y - m1*st_one.x;
		float m2 = vector_two.y / vector_two.x;
		float d2 = st_two.y - m2*st_two.x;

		float cx = ( d2 - d1 )/( m1 - m2 );
		float cy = m1*cx + d1;

		memcpy(output, &Vector2(cx, cy), sizeof(Vector2));
	}

	bool PhysUtil::CheckBoundaries(
		const Vector2 &pos_one, const Vector2 &pos_two,
		const Vector2 &vec_one, const Vector2 &vec_two,
		const Vector2 &intersec)
	{
		float cx = intersec.x;
		float cy = intersec.y;
		// Distance between the begining of the vectors and the point of intersection
		float dist_one = (pos_one.x - cx)*(pos_one.x - cx) + (pos_one.y - cy)*(pos_one.y - cy);
		float dist_two = (pos_two.x - cx)*(pos_two.x - cx) + (pos_two.y - cy)*(pos_two.y - cy);

		// If the distance between the begining of the vector and the point of intersection
		//  is greater than the length of the vector, the point is NOT on the vector
		if ( dist_one > vec_one.squared_length() ) return false;
		if ( dist_two > vec_two.squared_length() ) return false;

		return true;

	}

	void PhysUtil::CalculateNormal(
		Vector2 *output,
		const Vector2 &body_pos, const Vector2 &other_pos,
		const FusionPhysicsBody *body, const FusionPhysicsBody *other)
	{		
		// Distance (circular) object collision
		if (body->GetUseDistCollisions() & other->GetUseDistCollisions())
		{
			// Vector from center to point of collision
			Vector2 normal = body_pos - other_pos;
			normal.normalize();
			memcpy(output, &normal, sizeof(Vector2));
		}

		// Bitmask - bitmask collision
		else if (body->GetUsePixelCollisions() & other->GetUsePixelCollisions())
		{
			Vector2 normal;

			CL_Point offset = CL_Point(body_pos.x - other_pos.x, body_pos.y - other_pos.y);
			other->GetColBitmask()->CalcCollisionNormal(&normal, body->GetColBitmask(), offset);

			normal *= -1;
			normal.normalize();
			memcpy(output, &normal, sizeof(Vector2));
		}

		// AABB - bitmask collision
		/*else if (body->GetUseAABBCollisions() & other->GetUsePixelCollisions())
		{
			 AABBs aren't implimented
		}*/
		// BB - bitmask collision
		/*else if (body->GetUseBBCollisions() & other->GetUsePixelCollisions())
		{
			 BBs aren't implimented
		}*/

		// Non-bitmask - bitmask collision (probably something hitting terrain)
		else if (other->GetUsePixelCollisions())
		{
			CL_Point offset = CL_Point(body_pos.x - other_pos.x, body_pos.y - other_pos.y);
			Vector2 normal;

			FusionBitmask* bm = other->GetColBitmask();

			int ppb = bm->GetPPB();
			// Collide a new circle bitmask with the other bitmask and find the collision normal
			bm->CalcCollisionNormal(
				&normal,
				new FusionBitmask(CL_Size(ppb*2, ppb*2), ppb), offset);
			//CL_Point body_pt = CL_Point(body_pos.x, body_pos.y);
			//other->GetColPoint(body_pt);

			// The normal returned will be from the circle, we want the opposite
			normal *= -1;
			normal.normalize();
			memcpy(output, &normal, sizeof(Vector2));
		}

		// Bitmask - non-bitmask collision
		else if (body->GetUsePixelCollisions())
		{
			CL_Point offset = CL_Point(other_pos.x - body_pos.x, other_pos.y - body_pos.y);
			Vector2 normal;

			FusionBitmask* bm = body->GetColBitmask();

			// Collide the other bitmask with a new circle bitmask and find the collision normal
			bm->CalcCollisionNormal(
				&normal,
				new FusionBitmask(1.0f, bm->GetPPB()), offset);

			normal.normalize();
			memcpy(output, &normal, sizeof(Vector2));
		}

	}

}