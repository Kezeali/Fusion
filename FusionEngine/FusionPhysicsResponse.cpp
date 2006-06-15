
/// Fusion
#include "FusionPhysicsResponse.h"

using namespace FusionEngine;

FusionPhysicsResponse::FusionPhysicsResponse()
: m_Mass(0),
m_AppliedForce(0),
m_Acceleration(0),
m_Velocity(0),
m_Position(0)
{
}

FusionPhysicsResponse::FusionPhysicsResponse(const CL_Vector2 &position)
: m_Mass(0),
m_AppliedForce(0),
m_Acceleration(0),
m_Velocity(0),
m_Position(position)
{
}

FusionPhysicsResponse::~FusionPhysicsResponse()
{
}