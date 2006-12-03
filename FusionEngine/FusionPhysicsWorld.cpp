
#include "FusionPhysicsWorld.h"

#include "FusionPhysicsCollisionGrid.h"

namespace FusionEngine
{

	FusionPhysicsWorld::FusionPhysicsWorld()
		: m_BitmaskRes(1)
	{
		m_CollisionGrid = new FusionPhysicsCollisionGrid();
	}

	FusionPhysicsWorld::~FusionPhysicsWorld()
	{
	}

	void FusionPhysicsWorld::AddBody(FusionPhysicsBody *body)
	{
		m_Bodies.push_back(body);
	}
	
	void FusionPhysicsWorld::RemoveBody(FusionPhysicsBody *body)
	{
		PhysicsBodyList::iterator it = m_Bodies.begin();
		for (; it != m_Bodies.end(); ++it)
		{
			if ((*it) == body)
			{
				m_Bodies.erase(it);
				break;
			}
		}
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(int type)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this);
		body->SetType(type);

		m_Bodies.push_back(body);
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this);
		body->SetType(type);
		body->SetMass(props.mass);
		body->SetRadius(props.radius);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		m_Bodies.push_back(body);
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(FusionPhysicsResponse *response, int type)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this, response);
		body->SetType(type);

		m_Bodies.push_back(body);
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(FusionPhysicsResponse *response, int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this, response);
		body->SetType(type);
		body->SetMass(props.mass);
		body->SetRadius(props.radius);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		m_Bodies.push_back(body);
	}

	void FusionPhysicsWorld::DestroyBody(FusionPhysicsBody *body)
	{
		PhysicsBodyList::iterator it = m_Bodies.begin();
		for (; it != m_Bodies.end(); ++it)
		{
			if ((*it) == body)
			{
				m_Bodies.erase(it);
				break;
			}
		}

		delete body;
	}

	void FusionPhysicsWorld::Clear()
	{	
		PhysicsBodyList::iterator it = m_Bodies.begin();
		for (; it != m_Bodies.end(); ++it)
		{
			delete (*it);
		}

		m_Bodies.clear();
	}

	void FusionPhysicsWorld::RunSimulation(unsigned int split)
	{
		// Move bodies

		PhysicsBodyList::iterator it = m_Bodies.begin();
		for (;it != m_Bodies.end(); ++it)
		{
			FusionPhysicsBody *cBod = (*it);

			// Collision detection
			CL_Vector2 cVel = cBod->GetVelocity();
			if (cVel.squared_length() != 0)
			{
				PhysicsBodyList bodies = m_CollisionGrid->FindAdjacentBodies(cBod);
				PhysicsBodyList::iterator it = bodies.begin();

				for (; it != bodies.end(); ++it)
				{
					if (_checkCollision(cBod, (*it)))
						cBod->CollisionResponse();
				}
			}
			// Check for collisions of moving objects.
			if (cVel.squared_length() > (cBod->GetColDist() * cBod->GetColDist()))
			{
				PhysicsBodyList bodies = m_CollisionGrid->FindAdjacentBodies(cBod);
				PhysicsBodyList::iterator it = bodies.begin();

				for (; it != bodies.end(); ++it)
				{
					CollisionPoint collision = 
						_checkVectorForCollisions(cVel, (*it)->GetVelocity(), cBod, (*it));

					// This makes the assumption that nothing will ever collide at (0,0)
					//  Hopefully this wont be a problem...
					//! \todo Look into making a collision structure for _checkVectorForCollisions
					//! return value.
					if (collision.experianced)
					{
						cBod->CollisionResponse(collision.collision_point);
					}
				}
			}

			// Movement
			//  Linear motion
			float friction = -(cBod->m_FrictionConstant * cBod->m_Velocity);
			CL_Vector2 accel = (cBod->m_AppliedForce - friction) / cBod->m_Mass;
			CL_Vector2 veloc = cBod->m_Velocity + accel;

			// m_MaxVelocity is used to prevent objects from reaching crazy speeds
			//  because a lazy level designer didn't put friction triggers in the level.
			if (veloc.squared_length() > m_MaxVelocitySquared)
			{
				cBod->m_Acceleration = 0;
				// Calculate the maximum x and y velocities for this angle
				double a = atan(veloc.x/veloc.y);
				veloc.x = m_MaxVelocity * sin(a);
				veloc.y = m_MaxVelocity * cos(a);
			}
			else
				cBod->m_Acceleration = accel;

			cBod->m_Velocity = veloc;

			cBod->m_AppliedForce = 0;
			//  Rotation
			cBod->m_Rotation += cBod->m_RotationalVelocity;
		}
	}

	void FusionPhysicsWorld::LevelChange(int level_x, int level_y)
	{
		m_Width = level_x;
		m_Height = level_y;
	}

	void FusionPhysicsWorld::SetMaxVelocity(float maxvel)
	{
		m_MaxVelocity = maxvel;
		m_MaxVelocitySquared = maxvel * maxvel;
	}

	float FusionPhysicsWorld::GetMaxVelocity() const
	{
		return m_MaxVelocity;
	}

	void FusionPhysicsWorld::SetBitmaskRes(int ppb)
	{
		m_BitmaskRes = ppb;
	}

	int FusionPhysicsWorld::GetBitmaskRes() const
	{
		return m_BitmaskRes;
	}

	bool FusionPhysicsWorld::_checkCollision(const FusionPhysicsBody *one, const FusionPhysicsBody *two)
	{
		// Check for distance collisions
		if (one->GetUseDistCollisions() & two->GetUseDistCollisions())
		{
			float dy = one->GetPosition().y - two->GetPosition().y;
			float dx = one->GetPosition().x - two->GetPosition().x;

			// The required distance to create a collision against object one
			//  is expanded by the distance of the other.
			float dist = (one->GetColDist() + two->GetColDist());

			return ((dx*dx + dy*dy) < (dist * dist));
		}
		// Check for bitmask collisions
		else if (one->GetUsePixelCollisions() & two->GetUsePixelCollisions())
		{
			CL_Point offset = one->GetPositionPoint() - two->GetPositionPoint();
			return one->GetColBitmask().Overlap(two->GetColBitmask(), offset);
		}
		// Check for bitmask collisons against non-bitmask objects
		//  ATM this ignores dist colisions and AABB's; just works with a point
		else if (one->GetUsePixelCollisions() ^ two->GetUsePixelCollisions())
		{
			if (one->GetUsePixelCollisions())
				return one->GetColPoint(two->GetPositionPoint());
			else
				return two->GetColPoint(one->GetPositionPoint());
		}

		// Objects use a unsupported collision method
		return false;
	}

	FusionPhysicsWorld::CollisionPoint FusionPhysicsWorld::_checkVectorForCollisions(const CL_Vector2 &vector_one, const CL_Vector2 &vector_two, const FusionPhysicsBody *one, const FusionPhysicsBody *two) const
	{
		CollisionPoint retvel;

		// Positions
		CL_Vector2 pos_one = one->GetPosition();
		CL_Vector2 pos_two = two->GetPosition();
		// Destination
		CL_Vector2 dest_one = one->GetPosition() + vector_one;
		CL_Vector2 dest_two = two->GetPosition() + vector_two;

		
				//// If the vectors intersect, we assume there is a collision
				//retvel.experianced = true;
				//retvel.collision_point = inter_one;
				//return retvel;

		// Find the point of intersection and check wheter it's valid
		CL_Vector2 inter_one = _getVectorIntersection(one->GetPosition(), two->GetPosition(), vector_one, vector_two);
		if (_checkBoundaries(pos_one, pos_two, vector_one, vector_two, inter_one))
		{

			}
		else
		{
		}
		// Check for distance collision
		if (one->GetUseDistCollisions() & two->GetUseDistCollisions())
		{
			float dy = dest_one.y - dest_two.y;
			float dx = dest_one.x - dest_two.x;

			// The required distance to create a collision against object one
			//  is expanded by the distance of the other.
			float dist = (one->GetColDist() + two->GetColDist());

			if ((dx*dx + dy*dy) < (dist * dist))
			{
				retvel.experianced = true;
				retvel.collision_point = dest_one;
				return retvel;
			}
			// Check for bitmask collisions
			else if (one->GetUsePixelCollisions() & two->GetUsePixelCollisions())
			{
				CL_Point offset = one->GetPositionPoint() - two->GetPositionPoint();
				CL_Point col_point = one->GetColBitmask().OverlapPoint(two->GetColBitmask(), offset);

				retvel.experianced = true;
				retvel.collision_point = CL_Vector2(col_point.x, col_point.y);
				return retvel;
			}
			// Check for bitmask collisons against non-bitmask objects
			//  ATM this ignores dist colisions and AABB's; just works with a point
			else if (one->GetUsePixelCollisions() ^ two->GetUsePixelCollisions())
			{
				if (one->GetUsePixelCollisions())
				{
					if (one->GetColPoint(two->GetPositionPoint()))
					{
						retvel.experianced = true;
						// Since we check for collisions using two's current position,
						//  that must be the point of collision!
						retvel.collision_point = two->GetPosition();
						return retvel;
					}
				}
				else
				{
					if (two->GetColPoint(one->GetPositionPoint()))
					{
						retvel.experianced = true;
						// Since we check for collisions using one's current position,
						//  that must be the point of collision!
						retvel.collision_point = one->GetPosition();
						return retvel;
					}
				}
			}

		// Unsupported collision method/combination
		retvel.experianced = false;
		retvel.collision_point = CL_Vector2::ZERO;
		return retvel;
	}

	CL_Vector2 FusionPhysicsWorld::_getVectorIntersection(const CL_Vector2 &pos_one, const CL_Vector2 &pos_two, const CL_Vector2 &vector_one, const CL_Vector2 &vector_two) const
	{
		float m1 = vector_one.y / vector_one.x;
		float d1 = pos_one.y - m1*pos_one.x;
		float m2 = vector_two.y / vector_two.x;
		float d2 = pos_two.y - m2*pos_two.x;

		float cx = ( d2 - d1 )/( m1 - m2 );
		float cy = m1*cx + d1;

		return CL_Vector2(cx, cy);
	}

	bool FusionPhysicsWorld::_checkBoundaries(const CL_Vector2 &pos_one, const CL_Vector2 &pos_two, const CL_Vector2 &vec_one, const CL_Vector2 &vec_two, const CL_Vector2 &intersec) const
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

}
