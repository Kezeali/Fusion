
#include "FusionPhysicsWorld.h"

#include "FusionPhysicsCollisionGrid.h"

using namespace FusionEngine;

FusionPhysicsWorld::FusionPhysicsWorld()
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

void FusionPhysicsWorld::RunSimulation(unsigned int split)
{
	// Move bodies

	PhysicsBodyList::iterator it = m_Bodies.begin();
	for (;it != m_Bodies.end(); ++it)
	{
		FusionPhysicsBody *cBod = (*it);

		// Collision detection
		CL_Vector2 cVel = cBod->GetVelocity();
		if (cVel > 0)
		{
			PhysicsBodyList bodies = m_CollisionGrid->FindAdjacentBodies(cBod);
			PhysicsBodyList::iterator it = bodies.begin();

			for (; it != bodies.end(); ++it)
			{
				_checkCollision(cBod, (*it));
			}
		}
		// Check for collisions of moving objects.
		if (cVel.squared_length() > (cBod->GetColDist() * cBod->GetColDist()))
		{
			PhysicsBodyList bodies = m_CollisionGrid->FindAdjacentBodies(cBod);
			PhysicsBodyList::iterator it = bodies.begin();

			for (; it != bodies.end(); ++it)
			{
				CL_Vector2 point_collision = 
					_checkVectorForCollisions(cVel, (*it)->GetVelocity(), cBod, (*it));

				if (point_collision != CL_Vector2::ZERO)
				{
					cBod->m_Position = point_collision;
					// Stop movement and reverse motion. Hopefully it isn't already stuck in a wall
					cBod->ApplyForce(-(cVel));
				}
			}
		}

		// Movement
		CL_Vector2 accel = cBod->m_AppliedForce / cBod->m_Mass;
		cBod->m_Velocity += cBod->m_Acceleration;
	}
}

bool FusionPhysicsWorld::_checkCollision(const FusionPhysicsBody *one, const FusionPhysicsBody *two)
{
	// Check for distance collision
	if (one->GetUseDistCollisions())
	{
		int dy = one->GetPosition().y - two->GetPosition().y;
		int dx = one->GetPosition().x - two->GetPosition().x;

		// The required distance to create a collision against object one
		//  is expanded by the distance of the other.
		float dist = (one->GetColDist() + two->GetColDist());

		return ((dx*dx + dy*dy) < (dist * dist));
	}
}

CL_Vector2 &FusionPhysicsWorld::_checkVectorForCollisions(const CL_Vector2 &vector_one, const CL_Vector2 &vector_two, const FusionPhysicsBody *one, const FusionPhysicsBody *two) const
{
	// destination
	CL_Vector2 dest_one = one->GetPosition() + vector_one;
	CL_Vector2 dest_two = two->GetPosition() + vector_two;

	// Check for distance collision
	if (one->GetUseDistCollisions() & two->GetUseDistCollisions())
	{
		int dy = dest_one.y - dest_two.y;
		int dx = dest_one.x - dest_two.x;

		// The required distance to create a collision against object one
		//  is expanded by the distance of the other.
		float dist = (one->GetColDist() + two->GetColDist());

		return ((dx*dx + dy*dy) < (dist * dist));
	}
	// Check for bitmask collisions
	else if (one->GetUsePixelCollisions() & two->GetUsePixelCollisions())
	{
		CL_Point offset = one->GetPositionPoint() - two->GetPositionPoint();
		one->GetColBitmask().Overlap(two->GetColBitmask(), offset);
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
}
