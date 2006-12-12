
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
		m_Active(true),
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

	FusionPhysicsBody::FusionPhysicsBody(FusionPhysicsWorld *world, CollisionCallback response)
		: m_World(world),
		m_CollisionResponse(response),
		m_Acceleration(CL_Vector2::ZERO),
		m_AppliedForce(CL_Vector2::ZERO),
		m_IsColliding(false),
		m_Active(true),
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
		m_InverseMass = 1.0f / mass;
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

	float FusionPhysicsBody::GetInverseMass()
	{
		return m_InverseMass;
	}

	float FusionPhysicsBody::GetRadius()
	{
		return m_Radius;
	}

	void FusionPhysicsBody::ApplyForce(const CL_Vector2 &force)
	{
		m_AppliedForce += force;

		_activate();
	}

	void FusionPhysicsBody::ApplyForce(float force)
	{
		CL_Vector2 force_vector(sinf(force), cosf(force));
		m_AppliedForce += force_vector;

		_activate();
	}

	void FusionPhysicsBody::SetCoefficientOfFriction(float damping)
	{
		m_LinearDamping = damping;
	}

	void FusionPhysicsBody::SetCoefficientOfRestitution(float bounce)
	{
		m_Bounce = bounce;
	}

	void FusionPhysicsBody::SetRotationalVelocity(float velocity)
	{
		m_RotationalVelocity = velocity;

		_activate();
	}

	void FusionPhysicsBody::SetColBitmask(FusionEngine::FusionBitmask *bitmask)
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

	FusionBitmask *FusionPhysicsBody::GetColBitmask() const
	{
		return m_Bitmask;
	}

	bool FusionPhysicsBody::GetColPoint(const CL_Point &point, bool auto_offset) const
	{
		if (auto_offset)
		{
			CL_Point pos = GetPositionPoint();
			// Offset
			CL_Point scaled_point = (point - pos);
			// Scale
			scaled_point.x /= m_Bitmask->GetPPB();
			scaled_point.y /= m_Bitmask->GetPPB();

			return m_Bitmask->GetBit(scaled_point);
		}
		else
			return m_Bitmask->GetBit(point);
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

	float FusionPhysicsBody::GetCoefficientOfFriction() const
	{
		return m_LinearDamping;
	}

	float FusionPhysicsBody::GetCoefficientOfRestitution() const
	{
		return m_Bounce;
	}

	float FusionPhysicsBody::GetRotationalVelocity() const
	{
		return m_RotationalVelocity;
	}

	float FusionPhysicsBody::GetRotation() const
	{
		return m_Rotation;
	}

	bool FusionPhysicsBody::IsActive() const
	{
		return m_Active;
	}

	void FusionPhysicsBody::_activate()
	{
		m_Active = true;

		m_DeactivationTime = CL_System::get_time() + m_DeactivationTimePeriod;
	}

	void FusionPhysicsBody::_deactivate()
	{
		// Stop moving - we don't want it flying off into deep space ;)
		m_Acceleration = CL_Vector2::ZERO;
		m_Velocity = CL_Vector2::ZERO;

		m_Active = false;
	}

	void FusionPhysicsBody::SetDeactivationPeriod(unsigned int period)
	{
		m_DeactivationTimePeriod = period;
	}

	unsigned int FusionPhysicsBody::GetDeactivationPeriod() const
	{
		return m_DeactivationTimePeriod;
	}

	void FusionPhysicsBody::_setDeactivationTime(unsigned int time)
	{
		m_DeactivationTime = time;
	}

	unsigned int FusionPhysicsBody::GetDeactivationTime() const
	{
		return m_DeactivationTime;
	}

	void FusionPhysicsBody::_setPosition(const CL_Vector2 &position)
	{
		m_Position = position;

		_activate();
	}

	void FusionPhysicsBody::_setForce(const CL_Vector2 &force)
	{
		m_AppliedForce = force;

		_activate();
	}

	void FusionPhysicsBody::_setAcceleration(const CL_Vector2 &acceleration)
	{
		m_Acceleration = acceleration;

		_activate();
	}

	void FusionPhysicsBody::_setVelocity(const CL_Vector2 &velocity)
	{
		m_Velocity = velocity;

		_activate();
	}

	void FusionPhysicsBody::_setRotation(const float rotation)
	{
		m_Rotation = rotation;

		_activate();
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

	void FusionPhysicsBody::_notifyCGwillUpdate()
	{
		m_GotCGUpdate = true;
	}

	bool FusionPhysicsBody::_CGwillUpdate() const
	{
		return m_GotCGUpdate;
	}

	void FusionPhysicsBody::_notifyCGUpdated()
	{
		m_GotCGUpdate = false;
	}

}
