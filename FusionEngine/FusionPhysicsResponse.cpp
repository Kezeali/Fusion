
/// Fusion
#include "FusionPhysicsResponse.h"

using namespace FusionEngine;

FusionPhysicsResponse::FusionPhysicsResponse()
{
}

FusionPhysicsResponse::~FusionPhysicsResponse()
{
}

FusionPhysicsResponse::CollisionResponse()
{
	m_Owner->ApplyForce(-(cVel));
}

FusionPhysicsResponse::CollisionResponse(const CL_Vector2 &collision_point)
{
	m_Owner->_setPosition(collision_point);
	// Stop movement and reverse motion. Hopefully it isn't already stuck in a wall
	m_Owner->ApplyForce(-(cVel));
}