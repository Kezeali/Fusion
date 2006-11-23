
#include "FusionShipResponse.h"

using namespace FusionEngine;

void FusionShipResponse::CollisionResponse()
{
	m_Owner->ApplyForce(-(m_Owner->GetVelocity()));
}

void FusionShipResponse::CollisionResponse(const CL_Vector2 &collision_point)
{
	m_Owner->_setPosition(collision_point);
	m_Owner->ApplyForce(-(m_Owner->GetVelocity()));
}