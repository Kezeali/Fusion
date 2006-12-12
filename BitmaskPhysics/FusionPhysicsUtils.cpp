
#include "FusionPhysicsUtils.h"

/// Fusion
#include "FusionBitmask.h"
#include "FusionPhysicsBody.h"

namespace FusionEngine
{

	bool PhysUtil::GuessPointOfCollision(
			CL_Vector2 *output,
			const CL_Vector2 &pos_one, const CL_Vector2 &pos_two,
			const FusionPhysicsBody *one, const FusionPhysicsBody *two)
	{

		// Check for bitmask collisions
		//  These are by far the easiest type of collisions to find a point for
		//  from a coding stand point (I'm not sure how it performs)
		if (one->GetUsePixelCollisions() & two->GetUsePixelCollisions())
		{
			CL_Point offset = CL_Point(pos_one.x - pos_two.x, pos_one.y - pos_two.y);

			CL_Point out_pt;
			if ((one->GetColBitmask()->OverlapPoint(&out_pt, two->GetColBitmask(), offset)))
			{
				memcpy(output, &CL_Vector2(out_pt.x, out_pt.y), sizeof(CL_Vector2));
				return true;
			}
		}

		// Check for distance collision
		else if (one->GetUseDistCollisions() & two->GetUseDistCollisions())
		{
			// Find the 'CoM'... where mass is the collision distance :P
			CL_Vector2 pos = (pos_one * one->GetColDist() + pos_two * two->GetColDist());

			pos	*= 1/(one->GetColDist() + two->GetColDist());


			memcpy(output, &pos, sizeof(CL_Vector2));
			return true;
		}
		
		// Check for bitmask collisons against non-bitmask objects
		// 2006/12/11: I don't know if this works...
		else
		{
			CL_Point pospt_one = one->GetPositionPoint();
			CL_Point pospt_two = two->GetPositionPoint();

			if (one->GetUsePixelCollisions())
			{
				CL_Point point = pospt_one - pospt_two;

				if (one->GetColPoint(point))
				{
					memcpy(output, &CL_Vector2(point.x, point.y), sizeof(CL_Vector2));
					return true;
				}

			}
			else
			{
				CL_Point point = pospt_two - pospt_one;

				if (two->GetColPoint(one->GetPositionPoint()))
				{
					memcpy(output, &CL_Vector2(point.x, point.y), sizeof(CL_Vector2));
					return true;
				}

			}
		}

		// No collision found or unsupported collision methods/combination
		return false;
	}

	bool PhysUtil::FindCollisions(
		CL_Vector2 *output_one, CL_Vector2 *output_two,
		const CL_Vector2 &vector_one, const CL_Vector2 &vector_two,
		const FusionPhysicsBody *one, const FusionPhysicsBody *two,
		float epsilon)
	{
		// Positions
		CL_Vector2 pos_one = one->GetPosition();
		CL_Vector2 pos_two = two->GetPosition();

		// Objects not moving - don't use vector check
		if (vector_one.squared_length() == 0 || vector_two.squared_length() == 0)
		{
			if (CollisionCheck(pos_one, pos_two, one, two))
			{
				memcpy(output_one, &pos_one, sizeof(CL_Vector2));
				memcpy(output_two, &pos_two, sizeof(CL_Vector2));
				
				return true;
			}

			return false;
		}

		// These temps will store the start and end points while searching.
		//  The ones prefaced with 'orig' are used in the case that there 
		//  are no collisions along the given vector
		//  2006/12/10: The 'orig' var.s aren't used anymore. See 'collision_found' for the replacement
		CL_Vector2 startpt_one, endpt_one, startpt_two, endpt_two;

		// If the search finishes, we will assume these are set to the points of collision.
		CL_Vector2 checkpt_one, checkpt_two;

		// This will be set to true the first time a CollisionCheck() returns true
		//  during the binary search - if collision check never returns true, we can
		//  assume there are no collisions along the given vectors.
		bool collision_found = false;


		//////////////////////////////////////////////
		// First we decide on the bounds of the search

		// Find the point of intersection and check wheter it's valid
		/*CL_Vector2 intersec;
		CalculateVectorIntersection(
			&intersec,
			pos_one, pos_two,
			vector_one, vector_two);*/

		float line_a[] = {pos_one.x, pos_one.y, vector_one.x, vector_one.y};
		float line_b[] = {pos_two.x, pos_two.y, vector_two.x, vector_two.y};
		float *line_a_ptr = line_a;
		float *line_b_ptr = line_b;

		//if (CheckBoundaries(pos_one, pos_two, vector_one, vector_two, intersec))
		if (CL_LineMath::intersects(line_a_ptr, line_b_ptr))
		{
			// If a point of intersection was found, find the first place where the
			//  two objects collide /before/ there
			CL_Pointf isec_point = CL_LineMath::get_intersection(line_a_ptr, line_b_ptr);

			CL_Vector2 intersec; intersec.x = isec_point.x; intersec.y = isec_point.y;

			startpt_one = pos_one;
			endpt_one   = intersec;
			startpt_two = pos_two;
			endpt_two   = intersec;
		}
		else
		{
			// If no point of intersection was found, find the first place where the
			//  objects collide anywhere along the vector.

			startpt_one = pos_one;
			startpt_two = pos_two;
			endpt_one   = pos_one + vector_one;
			endpt_two   = pos_two + vector_two;

		}

		//////////////////////////////
		// Now we do the binary search

		// Distance between max/min point
		//  When this vector is sufficiantly small, we assume that it's mid-point,
		//  which will be the current 'check_pt', is the point of collision.
		//  (But I shouldn't have to tell you that. You _do_ know how a binary search
		//  works right?)
		CL_Vector2 start_end = startpt_one - endpt_one;
		float dist_squared = start_end.squared_length();

		int i = g_PhysMaxSearchItterations;
		while ((dist_squared > epsilon) && i--)
		{
			// Find the midpoints along the check vectors
			//  (via mid = a + half distance(a to b) )
			checkpt_one = startpt_one + (endpt_one - startpt_one)/2;
			checkpt_two = startpt_two + (endpt_two - startpt_two)/2;

			if (CollisionCheck(checkpt_one, checkpt_two, one, two))
			{
				// Collision must be before this point, so move the
				//  end bounds back
				endpt_one = checkpt_one;
				endpt_two = checkpt_two;

				// We can safely say that the objects collide /somewhere/ along the
				//  given vectors
				collision_found = true;
			}
			else
			{
				// Collision must be after this point, so move the
				//  start bounds forward
				startpt_one = checkpt_one;
				startpt_two = checkpt_two;
			}

			//// Start point has reached the original end point, or vice versa.
			////  This usually means that no collisions were found on the first itteration
			//if (startpt_one == origend_one || startpt_two == origend_two ||
			//	  endpt_one == origstart_one || endpt_two == origstart_two)
			//	return false; // There are no collisions!

			// Check how far apart the points found are
			start_end = startpt_one - endpt_one;
			dist_squared = start_end.squared_length();
		}

		if (collision_found)
		{
			memcpy(output_one, &checkpt_one, sizeof(CL_Vector2));
			memcpy(output_two, &checkpt_two, sizeof(CL_Vector2));
			return true;
		}
		return false;
	}

	bool PhysUtil::CollisionCheck(const CL_Vector2 &one_pos, const CL_Vector2 &two_pos, const FusionPhysicsBody *one, const FusionPhysicsBody *two)
	{
		// Check for distance collision
		if (one->GetUseDistCollisions() & two->GetUseDistCollisions())
		{
			CL_Vector2 dp = one_pos - two_pos;

			// The required distance to create a collision against object one
			//  is expanded by the distance of the other.
			float dist = (one->GetColDist() + two->GetColDist());

			return (dp.squared_length() < (dist * dist));
		}
		
		// Check for bitmask collisions
		else if (one->GetUsePixelCollisions() & two->GetUsePixelCollisions())
		{
			CL_Point offset = CL_Point(one_pos.x - two_pos.x, one_pos.y - two_pos.y);
			return (one->GetColBitmask()->Overlap(two->GetColBitmask(), offset));
		}

		// Check for bitmask collisons against non-bitmask objects
		//  ATM this ignores dist colisions and AABB's; just works with a point
		else if (one->GetUsePixelCollisions() ^ two->GetUsePixelCollisions())
		{
			// [removed] auto_offset now does this work
			/*CL_Point pospt_one = one->GetPositionPoint();
			CL_Point pospt_two = two->GetPositionPoint();*/

			if (one->GetUsePixelCollisions())
			{
				//CL_Point point = pospt_two - pospt_one;
				return (one->GetColPoint( two->GetPositionPoint() ));
			}
			else
			{
				//CL_Point point = pospt_one - pospt_two;
				return (two->GetColPoint( one->GetPositionPoint() ));
			}
		}

		// No collision found or unsupported collision methods/combination
		return false;
	}

	void PhysUtil::CalculateVectorIntersection(
		CL_Vector2 *output,
		const CL_Vector2 &st_one, const CL_Vector2 &st_two,
		const CL_Vector2 &vector_one, const CL_Vector2 &vector_two)
	{
		float m1 = vector_one.y / vector_one.x;
		float d1 = st_one.y - m1*st_one.x;
		float m2 = vector_two.y / vector_two.x;
		float d2 = st_two.y - m2*st_two.x;

		float cx = ( d2 - d1 )/( m1 - m2 );
		float cy = m1*cx + d1;

		memcpy(output, &CL_Vector2(cx, cy), sizeof(CL_Vector2));
	}

	bool PhysUtil::CheckBoundaries(
		const CL_Vector2 &pos_one, const CL_Vector2 &pos_two,
		const CL_Vector2 &vec_one, const CL_Vector2 &vec_two,
		const CL_Vector2 &intersec)
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
		CL_Vector2 *output,
		const CL_Vector2 &body_pos, const CL_Vector2 &other_pos,
		const FusionPhysicsBody *body, const FusionPhysicsBody *other)
	{		
		// Bitmask - bitmask collision
		if (body->GetUsePixelCollisions() & other->GetUsePixelCollisions())
		{
			CL_Vector2 normal;

			CL_Point offset = CL_Point(body_pos.x - other_pos.x, body_pos.y - other_pos.y);
			body->GetColBitmask()->CalcCollisionNormal(&normal, other->GetColBitmask(), offset);

			normal.unitize();
			memcpy(output, &normal, sizeof(CL_Vector2));
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
			CL_Vector2 normal;

			FusionBitmask* bm = other->GetColBitmask();

			// Collide a new circle bitmask with the other bitmask and find the collision normal
			bm->CalcCollisionNormal(
				&normal,
				new FusionBitmask(CL_Size(50, 50), bm->GetPPB()), offset);
			//CL_Point body_pt = CL_Point(body_pos.x, body_pos.y);
			//other->GetColPoint(body_pt);

			// The normal returned will be from the circle, we want the opposite
			normal *= -1;
			normal.unitize();
			memcpy(output, &normal, sizeof(CL_Vector2));
		}

		// Bitmask - non-bitmask collision
		else if (body->GetUsePixelCollisions())
		{
			CL_Point offset = CL_Point(other_pos.x - body_pos.x, other_pos.y - body_pos.y);
			CL_Vector2 normal;

			FusionBitmask* bm = body->GetColBitmask();

			// Collide the other bitmask with a new circle bitmask and find the collision normal
			bm->CalcCollisionNormal(
				&normal,
				new FusionBitmask(1.0f, bm->GetPPB()), offset);

			normal.unitize();
			memcpy(output, &normal, sizeof(CL_Vector2));
		}

		// Assume distance (circular) object collision
		else
		{
			// Vector from center to point of collision
			CL_Vector2 normal = body_pos - other_pos;
			normal.unitize();
			memcpy(output, &normal, sizeof(CL_Vector2));
		}

	}

}