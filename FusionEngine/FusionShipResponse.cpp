
#include "FusionShipResponse.h"

using namespace FusionEngine;

void FusionShipResponse::CollisionResponse()
{
	m_Owner->ApplyForce(-(m_Owner->GetVelocity()));
}

void FusionShipResponse::CollisionResponse(const CL_Vector2 &collision_point)
{
	m_Owner->_setPosition(collision_point);
	// Stop movement and reverse motion. Hopefully it isn't already stuck in a wall
	m_Owner->ApplyForce(-(m_Owner->GetVelocity()));
}