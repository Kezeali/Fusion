#include "FusionPhysicsResponse.h"

namespace FusionEngine
{

	FusionPhysicsResponse::FusionPhysicsResponse()
	{
	}

	FusionPhysicsResponse::~FusionPhysicsResponse()
	{
	}

}

//void FusionPhysicsResponse::CollisionResponse()
//{
//	m_Owner->ApplyForce(-(cVel));
//}
//
//void FusionPhysicsResponse::CollisionResponse(const CL_Vector2 &collision_point)
//{
//	m_Owner->_setPosition(collision_point);
//	// Stop movement and reverse motion. Hopefully it isn't already stuck in a wall
//	m_Owner->ApplyForce(-(cVel));
//}