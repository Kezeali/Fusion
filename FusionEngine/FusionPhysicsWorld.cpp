
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

		CL_Vector2 cVel = cBod->GetVelocity();
		if (cVel > 0)
		{
			PhysicsBodyList bodies = m_CollisionGrid->FindAdjacentBodies(cBod);
			PhysicsBodyList::iterator it = bodies.begin();

			for (; it != bodies.end(); ++it)
			{
				_CheckCollision(cBod, (*it));
			}
		}
		// If there could be anything, outside the collision distance, along the movement
		// path, check.
		if (cVel.squared_length > (cBod->GetColDist() ^2))
		{
			_CheckVectorForCollisions(cVel, cBod, cOther);
		}
	}
}

bool FusionPhysicsWorld::_CheckCollision(const FusionPhysicsBody *one, const FusionPhysicsBody *two)
{
	// Check for distance collision
	if (one->GetUseDistCollisions())
	{
		int dy = one->GetPosition().y - two->GetPosition().y;
		int dx = one->GetPosition().x - two->GetPosition().x;

		return ((dx ^2 + dy ^2) < (one->GetColDist() - two->GetColDist()) ^2);
	}
}

bool FusionPhysicsWorld::_CheckVectorForCollisions(const CL_Vector2 &vector, const FusionPhysicsBody *one, const FusionPhysicsBody *two)
{
	// destination
	CL_Vector2 dest = one->GetPosition() + vector;

	PhysicsBodyList bodies = m_CollisionGrid->FindAdjacentBodies(
}
