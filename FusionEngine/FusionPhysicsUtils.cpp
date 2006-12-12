
#include "FusionPhysicsUtils.h"
#include "FusionBitmask.h"

namespace FusionEngine
{

	bool PhysUtil::FindPointOfCollision(CL_Vector2 *output,
		const FusionPhysicsBody *one, const FusionPhysicsBody *two, float start)
	{
		// Calculate starting positions
		CL_Vector2 one_pos = one->GetPosition();
		CL_Vector2 two_pos = two->GetPosition();
		CL_Vector2 one_veloc = one->GetVelocity();
		CL_Vector2 two_veloc = two->GetVelocity();
		// todo Calculate starting positions

		// Check for distance collision
		if (one->GetUseDistCollisions() & two->GetUseDistCollisions())
		{
			float dy = one->GetPosition().y - two->GetPosition().y;
			float dx = one->GetPosition().x - two->GetPosition().x;

			// The required distance to create a collision against object one
			//  is expanded by the distance of the other.
			float dist = (one->GetColDist() + two->GetColDist());

			if ((dx*dx + dy*dy) < (dist * dist))
			{
				// todo Make FindPointOfCollision actually do something
				memcpy(output, &one->GetPosition(), sizeof(CL_Vector2));
				return true;
			}
		}
		
		// Check for bitmask collisions
		else if (one->GetUsePixelCollisions() & two->GetUsePixelCollisions())
		{
			CL_Point offset = CL_Point(one_pos.x - two_pos.x, one_pos.y - two_pos.y);
			return (one->GetColBitmask()->OverlapPoint(two->GetColBitmask(), offset));
		}
		// Check for bitmask collisons against non-bitmask objects
		//  ATM this ignores dist colisions and AABB's; just works with a point
		else if (one->GetUsePixelCollisions() ^ two->GetUsePixelCollisions())
		{
			if (one->GetUsePixelCollisions())
			{
				return (one->GetColPoint(two->GetPositionPoint()));
			}
			else
			{
				return (two->GetColPoint(one->GetPositionPoint()));
			}
		}

		// No collision found or unsupported collision methods/combination
		return false;
	}

	bool PhysUtil::FindCollisions(CL_Vector2 *output,
		const CL_Vector2 &vector_one, const CL_Vector2 &vector_two,
		const FusionPhysicsBody *one, const FusionPhysicsBody *two
		float epsilon = 0.1f) const
	{
		// Positions
		CL_Vector2 pos_one = one->GetPosition();
		CL_Vector2 pos_two = two->GetPosition();

		// These will be set to the points which should be check for collisions along
		//  the vectors.
		CL_Vector2 check_one; // For vector/body one
		CL_Vector2 check_two; // For vector/body two
	
		// Find the point of intersection and check wheter it's valid
		CL_Vector2 inter_one = CalculateVectorIntersection(one->GetPosition(), two->GetPosition(), vector_one, vector_two);
		if (CheckBoundaries(pos_one, pos_two, vector_one, vector_two, inter_one))
		{
			// If a point of intersection was found, find the first place where the
			//  two objects collide /before/ there (binary search)

			CL_Vector2 v = pos_one - inter_one; // Used for distance between points
			float dist_squared = v.squared_length();

			int i = g_PhysMaxSearchItterations;
			while ((dist_squared > epsilon) && i--)
			{
				float m1 = vector_one.y / vector_one.x;
				float d1 = pos_one.y - m1*pos_one.x;
				float m2 = vector_two.y / vector_two.x;
				float d2 = pos_two.y - m2*pos_two.x;

				float cx = ( d2 - d1 )/( m1 - m2 );
				float cy = m1*cx + d1;

				dist_squared = v.squared_length();
			}
		}
		else
		{
			//! \todo Do a near-pass check


			// Check for collisions at the destination
			//  Set the check point to the destination of each body
			check_one = one->GetPosition() + vector_one;
			check_two = two->GetPosition() + vector_two;
			if (CollisionCheck(check_one, check_two, one, two))
			{
				memcpy(output, &check_one, sizeof(CL_Vector2));
				return true;
			}

		}

		return false;
	}

	bool PhysUtil::CollisionCheck(const CL_Vector2 &one_pos, const CL_Vector2 &two_pos, const FusionPhysicsBody *one, const FusionPhysicsBody *two)
	{
		// Check for distance collision
		if (one->GetUseDistCollisions() & two->GetUseDistCollisions())
		{
			float dy = one_pos.y - two_pos.y;
			float dx = one_pos.x - two_pos.x;

			// The required distance to create a collision against object one
			//  is expanded by the distance of the other.
			float dist = (one->GetColDist() + two->GetColDist());

			return ((dx*dx + dy*dy) < (dist * dist));
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
			if (one->GetUsePixelCollisions())
			{
				return (one->GetColPoint(two->GetPositionPoint()));
			}
			else
			{
				return (two->GetColPoint(one->GetPositionPoint()));
			}
		}

		// No collision found or unsupported collision methods/combination
		return false;
	}

	void PhysUtil::CalculateVectorIntersection(CL_Vector2 *output, const CL_Vector2 &st_one, const CL_Vector2 &st_two, const CL_Vector2 &vector_one, const CL_Vector2 &vector_two) const
	{
		float m1 = vector_one.y / vector_one.x;
		float d1 = st_one.y - m1*st_one.x;
		float m2 = vector_two.y / vector_two.x;
		float d2 = st_two.y - m2*st_two.x;

		float cx = ( d2 - d1 )/( m1 - m2 );
		float cy = m1*cx + d1;

		memcpy(output, &CL_Vector2(cx, cy), sizeof(CL_Vector2));
	}

	bool PhysUtil::CheckBoundaries(const CL_Vector2 &pos_one, const CL_Vector2 &pos_two, const CL_Vector2 &vec_one, const CL_Vector2 &vec_two, const CL_Vector2 &intersec) const
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

	bool PhysUtil::CalculateNormal(CL_Vector2 *output,
		const CL_Vector2 &point, const FusionPhysicsBody *body)
	{
		// Distance based (circular) objects
		if (body->GetUseDistCollisions())
		{
			c
		}
		
		// Bitmask based objects
		else if (body->GetUsePixelCollisions())
		{
			body->GetColBitmask()
		}

		memcpy(output, &CL_Vector2(nx, ny), sizeof(CL_Vector2));
	}

}