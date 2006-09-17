/*
  Copyright (c) 2006 Elliot Hayward

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#include "FusionPhysicsBody.h"

#include "FusionPhysicsResponse.h"

using namespace FusionEngine;

FusionPhysicsBody::FusionPhysicsBody(FusionPhysicsWorld *world)
: m_World(world),
m_Acceleration(CL_Vector2::ZERO),
m_AppliedForce(CL_Vector2::ZERO),
m_IsColliding(false),
m_Mass(0.f),
m_Position(CL_Vector2::ZERO),
m_Rotation(0.f),
m_RotationalVelocity(0.f),
m_UsesAABB(false),
m_UsesDist(false),
m_UsesPixel(false),
m_Velocity(CL_Vector2::ZERO)
{
}

FusionPhysicsBody::FusionPhysicsBody(FusionPhysicsWorld *world, const FusionPhysicsResponse &response)
: m_World(world),
m_CollisionResponse(response),
m_Acceleration(CL_Vector2::ZERO),
m_AppliedForce(CL_Vector2::ZERO),
m_IsColliding(false),
m_Mass(0.f),
m_Position(CL_Vector2::ZERO),
m_Rotation(0.f),
m_RotationalVelocity(0.f),
m_UsesAABB(false),
m_UsesDist(false),
m_UsesPixel(false),
m_Velocity(CL_Vector2::ZERO)
{
}

FusionPhysicsBody::~FusionPhysicsBody()
{
}

void FusionPhysicsBody::SetMass(float mass)
{
	m_Mass = mass;
}

void FusionPhysicsBody::ApplyForce(const CL_Vector2 &force)
{
	m_AppliedForce = force;
}

void FusionPhysicsBody::SetRotationalVelocity(const float velocity)
{
	m_Velocity = velocity;
}

void FusionPhysicsBody::SetColBitmask(const FusionEngine::FusionBitmask &bitmask)
{
	m_Bitmask = bitmask;
}

void FusionPhysicsBody::SetColAABB(float width, float height)
{
	m_AABB = CL_Rectf(0, 0, width, height);
}

//void FusionPhysicsBody::SetColAABB(const CL_Rectf &bbox)
//{
//	m_AABBox = bbox;
//}

void FusionPhysicsBody::SetColDist(float dist)
{
	m_ColDist = dist;
}

FusionBitmask FusionPhysicsBody::GetColBitmask() const
{
	return m_Bitmask;
}

bool FusionPhysicsBody::GetColPoint(CL_Point point) const
{
	return m_Bitmask.GetBit(point);
}

CL_Rectf FusionPhysicsBody::GetColAABB() const
{
	return m_AABB;
}

float FusionPhysicsBody::GetColDist() const
{
	return m_ColDist;
}

void FusionPhysicsBody::SetUsePixelCollisions(bool usePixel)
{
	m_UsesPixel = usePixel;
}

void FusionPhysicsBody::SetUseAABBCollisions(bool useAABB)
{
	m_UsesAABB = useAABB;
}

void FusionPhysicsBody::SetUseDistCollisions(bool useDist)
{
	m_UsesDist = useDist;
}

bool FusionPhysicsBody::GetUsePixelCollisions()
{
	return m_UsesPixel;
}

bool FusionPhysicsBody::GetUseAABBCollisions()
{
	return m_UsesAABB;
}

bool FusionPhysicsBody::GetUseDistCollisions()
{
	return m_UsesDist;
}

CL_Vector2 &FusionPhysicsBody::GetPosition() const
{
	return m_Position;
}

CL_Point &FusionPhysicsBody::GetPositionPoint() const
{
	return CL_Point(m_Position.x, m_Position.y);
}

CL_Vector2 &FusionPhysicsBody::GetAcceleration() const
{
	return m_Acceleration;
}

CL_Vector2 &FusionPhysicsBody::GetVelocity() const
{
	return m_Velocity;
}

float FusionPhysicsBody::GetRotationalVelocity() const
{
	return m_RotationalVelocity;
}

float FusionPhysicsBody::GetRotation() const
{
	return m_Rotation;
}

void FusionPhysicsBody::_setPosition(const CL_Vector2 &position)
{
	m_Position = position;
}

void FusionPhysicsBody::_setAcceleration(const CL_Vector2 &acceleration)
{
	m_Acceleration = acceleration;
}

void FusionPhysicsBody::_setVelocity(const CL_Vector2 &velocity)
{
	m_Velocity = velocity;
}

void FusionPhysicsBody::_setRotation(const float rotation)
{
	m_Rotation = rotation;
}

void FusionPhysicsBody::_setCGPos(int ind)
{
	m_CGPos = ind;
}

int FusionPhysicsBody::_getCGPos() const
{
	return m_CGPos;
}

void FusionPhysicsBody::_setCCIndex(int ind)
{
	m_CCIndex = ind;
}

void FusionPhysicsBody::_getCCIndex() const
{
	return m_CCIndex;
}
