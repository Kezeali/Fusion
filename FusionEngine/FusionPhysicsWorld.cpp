
#include "FusionEngineCommon.h"

/// STL

/// Fusion

/// Class
#include "FusionPhysicsWorld.h"

using namespace FusionEngine;

FusionPhysicsWorld::FusionPhysicsWorld()
{
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
}

bool FusionPhysicsWorld::_CheckCollision(const FusionPhysicsBody &one, const FusionPhysicsBody &two)
{
	// Check for distance collision
	if (one.GetUseDistCollisions())
	{
	}
}

bool FusionPhysicsWorld::_CheckVectorForCollisions(const CL_Vector2 &vector, const FusionPhysicsBody &one, const FusionPhysicsBody &two)
{
	// Work out the interval at which to check collisions.
	float interval = ;
}
