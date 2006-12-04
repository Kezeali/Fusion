
#include "FusionPhysicsBody.h"

/// Fusion
#include "FusionPhysicsWorld.h"

namespace FusionEngine
{

	FusionPhysicsBody::FusionPhysicsBody(FusionPhysicsWorld *world)
		: m_World(world),
		m_CollisionResponse(0),
		m_Acceleration(CL_Vector2::ZERO),
		m_AppliedForce(CL_Vector2::ZERO),
		m_IsColliding(false),
		m_Type(0),
		m_Mass(0.f),
		m_Radius(0),
		m_Position(CL_Vector2::ZERO),
		m_Rotation(0.f),
		m_RotationalVelocity(0.f),
		m_UsesAABB(false),
		m_UsesDist(false),
		m_UsesPixel(false),
		m_Velocity(CL_Vector2::ZERO)
	{
	}

	FusionPhysicsBody::FusionPhysicsBody(FusionPhysicsWorld *world, const CollisionCallback &response)
		: m_World(world),
		m_CollisionResponse(response),
		m_Acceleration(CL_Vector2::ZERO),
		m_AppliedForce(CL_Vector2::ZERO),
		m_IsColliding(false),
		m_Type(0),
		m_Mass(0.f),
		m_Radius(0),
		m_Position(CL_Vector2::ZERO),
		m_Rotation(0.f),
		m_RotationalVelocity(0.f),
		m_UsesAABB(false),
		m_UsesDist(false),
		m_UsesPixel(false),
		m_Velocity(CL_Vector2::ZERO)
	{
	}

	void FusionPhysicsBody::SetType(int type)
	{
		m_Type = type;
	}

	void FusionPhysicsBody::SetMass(float mass)
	{
		m_Mass = mass;
	}

	void FusionPhysicsBody::SetRadius(float radius)
	{
		m_Radius = radius;
	}

	int FusionPhysicsBody::GetType()
	{
		return m_Type;
	}

	float FusionPhysicsBody::GetMass()
	{
		return m_Mass;
	}

	float FusionPhysicsBody::GetRadius()
	{
		return m_Radius;
	}

	void FusionPhysicsBody::ApplyForce(const CL_Vector2 &force)
	{
		m_AppliedForce += force;
	}

	void FusionPhysicsBody::SetFrictionConstant(float constant)
	{
		m_FrictionConst = constant;
	}

	void FusionPhysicsBody::SetRotationalVelocity(float velocity)
	{
		m_RotationalVelocity = velocity;
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

	bool FusionPhysicsBody::GetUsePixelCollisions() const
	{
		return m_UsesPixel;
	}

	bool FusionPhysicsBody::GetUseAABBCollisions() const
	{
		return m_UsesAABB;
	}

	bool FusionPhysicsBody::GetUseDistCollisions() const
	{
		return m_UsesDist;
	}

	void FusionPhysicsBody::SetUserData(void *userdata)
	{
		m_UserData = userdata;
	}

	const void *FusionPhysicsBody::GetUserData() const
	{
		return m_UserData;
	}

	void FusionPhysicsBody::SetCollisionCallback(const CollisionCallback &method)
	{
		m_CollisionResponse = method;
	}

	void FusionPhysicsBody::CollisionResponse(FusionPhysicsBody *other)
	{
		if (m_CollisionResponse != 0)
			m_CollisionResponse(other, CL_Vector2::ZERO);
	}

	void FusionPhysicsBody::CollisionResponse(FusionPhysicsBody *other, const CL_Vector2 &collision_point)
	{
		if (m_CollisionResponse != 0)
			m_CollisionResponse(other, collision_point);
	}

	const CL_Vector2 &FusionPhysicsBody::GetPosition() const
	{
		return m_Position;
	}

	CL_Point FusionPhysicsBody::GetPositionPoint() const
	{
		return CL_Point(m_Position.x, m_Position.y);
	}

	const CL_Vector2 &FusionPhysicsBody::GetForce() const
	{
		return m_AppliedForce;
	}

	const CL_Vector2 &FusionPhysicsBody::GetAcceleration() const
	{
		return m_Acceleration;
	}

	const CL_Vector2 &FusionPhysicsBody::GetVelocity() const
	{
		return m_Velocity;
	}

	float FusionPhysicsBody::GetFrictionConstant() const
	{
		return m_FrictionConst;
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

	void FusionPhysicsBody::_setForce(const CL_Vector2 &force)
	{
		m_AppliedForce = force;
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

	int FusionPhysicsBody::_getCCIndex() const
	{
		return m_CCIndex;
	}

}
