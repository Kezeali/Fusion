
#include "FusionPhysicsStatic.h"

using namespace FusionEngine;

FusionPhysicsStatic::FusionPhysicsStatic(FusionPhysicsWorld *world)
: m_World(world),
m_IsColliding(false),
m_Position(CL_Vector2::ZERO),
m_UsesAABB(false),
m_UsesDist(false),
m_UsesPixel(false),
m_Velocity(CL_Vector2::ZERO)
{
}

FusionPhysicsStatic::~FusionPhysicsStatic()
{
}

void FusionPhysicsStatic::SetColBitmask(const FusionEngine::FusionBitmask &bitmask)
{
	m_Bitmask = bitmask;
}

void FusionPhysicsStatic::SetColAABB(float width, float height)
{
	m_AABB = CL_Rectf(0, 0, width, height);
}

//void FusionPhysicsStatic::SetColAABB(const CL_Rectf &bbox)
//{
//	m_AABBox = bbox;
//}

void FusionPhysicsStatic::SetColDist(float dist)
{
	m_ColDist = dist;
}

FusionBitmask FusionPhysicsStatic::GetColBitmask() const
{
	return m_Bitmask;
}

bool FusionPhysicsStatic::GetColPoint(CL_Point point) const
{
	return m_Bitmask.GetBit(point);
}

CL_Rectf FusionPhysicsStatic::GetColAABB() const
{
	return m_AABB;
}

float FusionPhysicsStatic::GetColDist() const
{
	return m_ColDist;
}

void FusionPhysicsStatic::SetUsePixelCollisions(bool usePixel)
{
	m_UsesPixel = usePixel;
}

void FusionPhysicsStatic::SetUseAABBCollisions(bool useAABB)
{
	m_UsesAABB = useAABB;
}

void FusionPhysicsStatic::SetUseDistCollisions(bool useDist)
{
	m_UsesDist = useDist;
}

bool FusionPhysicsStatic::GetUsePixelCollisions()
{
	return m_UsesPixel;
}

bool FusionPhysicsStatic::GetUseAABBCollisions()
{
	return m_UsesAABB;
}

bool FusionPhysicsStatic::GetUseDistCollisions()
{
	return m_UsesDist;
}

CL_Vector2 &FusionPhysicsStatic::GetPosition() const
{
	return m_Position;
}

CL_Point &FusionPhysicsStatic::GetPositionPoint() const
{
	return CL_Point(m_Position.x, m_Position.y);
}

void FusionPhysicsStatic::_setPosition(const CL_Vector2 &position)
{
	m_Position = position;
}

