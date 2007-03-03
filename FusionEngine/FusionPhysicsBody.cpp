
#include "FusionPhysicsBody.h"

/// Fusion
#include "FusionPhysicsWorld.h"

namespace FusionEngine
{

	FusionPhysicsBody::FusionPhysicsBody(FusionPhysicsWorld *world)
		: m_World(world),
		m_CollisionFlags(C_NONE),
		m_CollisionResponse(0),
		m_CollisionHandler(0),
		m_UserData(0),
		m_Acceleration(Vector2::ZERO),
		m_AppliedForce(Vector2::ZERO),
		m_GotCGUpdate(false),
		m_Active(true),
		m_DeactivationCounter(0),
		m_DeactivationPeriod(world->GetBodyDeactivationPeriod()),
		m_Type(0),
		m_Mass(0.f),
		m_Radius(0),
		m_Position(Vector2::ZERO),
		m_Rotation(0.f),
		m_RotationalVelocity(0.f),
		m_UsesAABB(false),
		m_UsesDist(false),
		m_UsesPixel(false),
		m_Velocity(Vector2::ZERO),
		m_AppliedEngineForce(0)
	{
	}

	FusionPhysicsBody::FusionPhysicsBody(FusionPhysicsWorld *world, CollisionHandler *handler)
		: m_World(world),
		m_CollisionFlags(C_NONE),
		m_CollisionResponse(0),
		m_CollisionHandler(handler),
		m_UserData(0),
		m_Acceleration(Vector2::ZERO),
		m_AppliedForce(Vector2::ZERO),
		m_GotCGUpdate(false),
		m_Active(true),
		m_DeactivationPeriod(world->GetBodyDeactivationPeriod()),
		m_DeactivationCounter(0),
		m_Type(0),
		m_Mass(0.f),
		m_Radius(0),
		m_Position(Vector2::ZERO),
		m_Rotation(0.f),
		m_RotationalVelocity(0.f),
		m_UsesAABB(false),
		m_UsesDist(false),
		m_UsesPixel(false),
		m_Velocity(Vector2::ZERO),
		m_AppliedEngineForce(0)
	{
	}

	FusionPhysicsBody::FusionPhysicsBody(FusionPhysicsWorld *world, const CollisionCallback &response)
		: m_World(world),
		m_CollisionFlags(C_NONE),
		m_CollisionResponse(response),
		m_CollisionHandler(0),
		m_UserData(0),
		m_Acceleration(Vector2::ZERO),
		m_AppliedForce(Vector2::ZERO),
		m_GotCGUpdate(false),
		m_Active(true),
		m_DeactivationPeriod(world->GetBodyDeactivationPeriod()),
		m_DeactivationCounter(0),
		m_Type(0),
		m_Mass(0.f),
		m_Radius(0),
		m_Position(Vector2::ZERO),
		m_Rotation(0.f),
		m_RotationalVelocity(0.f),
		m_UsesAABB(false),
		m_UsesDist(false),
		m_UsesPixel(false),
		m_Velocity(Vector2::ZERO),
		m_AppliedEngineForce(0)
	{
	}

	void FusionPhysicsBody::SetType(int type)
	{
		m_Type = type;
	}

	void FusionPhysicsBody::SetMass(float mass)
	{
		if (mass == 0.0f)
		{
			m_CollisionFlags |= C_STATIC;

			m_Mass = 0.0f;
			m_InverseMass = 0.0f;
		}
		else
		{
			m_Mass = mass;
			m_InverseMass = 1.0f / mass;
		}
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

	void FusionPhysicsBody::ApplyForce(const Vector2 &force)
	{
		m_AppliedForce += force;

		_activate();
	}

	void FusionPhysicsBody::ApplyEngineForce(float force)
	{
		// A force vector will be calculated during the simulation step
		//  to make sure rotational velocity & step time are taken into account
		m_AppliedEngineForce += force;

		_activate();
	}

	void FusionPhysicsBody::SetCoefficientOfFriction(float damping)
	{
		m_LinearDamping = damping;
	}

	void FusionPhysicsBody::SetCoefficientOfRestitution(float bounce)
	{
		if (bounce == 0.0f)
		{
			m_CollisionFlags ^= C_BOUNCE;
		}
		else
		{
			m_CollisionFlags |= C_BOUNCE;
		}

		m_Bounce = bounce;
	}

	void FusionPhysicsBody::SetRotationalVelocity(float velocity)
	{
		m_RotationalVelocity = velocity;

		// Don't activate if this was a call to stop rotation!
		if (velocity != 0)
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

	CL_Rectf FusionPhysicsBody::GetColAABB() const
	{
		return m_AABB;
	}

	float FusionPhysicsBody::GetColDist() const
	{
		return m_ColDist;
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

	void *FusionPhysicsBody::GetUserData() const
	{
		return m_UserData;
	}


	void FusionPhysicsBody::SetCollisionCallback(const CollisionCallback &method)
	{
		m_CollisionResponse = method;
	}

	void FusionPhysicsBody::SetCollisionHandler(CollisionHandler *handler)
	{
		m_CollisionHandler = handler;
	}

	bool FusionPhysicsBody::CanCollideWith(FusionPhysicsBody *other)
	{
		if (m_CollisionHandler != 0)
			return m_CollisionHandler->CanCollideWith(other);

		return true;
	}

	void FusionPhysicsBody::CollisionWith(FusionPhysicsBody *other, const Vector2 &collision_point)
	{
		if (m_CollisionHandler != 0)
			m_CollisionHandler->CollisionWith(other, collision_point);

		if (m_CollisionResponse != 0)
			m_CollisionResponse(other, collision_point);
	}

	void FusionPhysicsBody::CollisionResponse(FusionPhysicsBody *other, const Vector2 &collision_point)
	{
		if (m_CollisionResponse != 0)
			m_CollisionResponse(other, collision_point);

		if (m_CollisionHandler != 0)
			m_CollisionHandler->CollisionWith(other, collision_point);
	}

	int FusionPhysicsBody::GetCollisionFlags() const
	{
		return m_CollisionFlags;
	}

	bool FusionPhysicsBody::CheckCollisionFlag(int flag)
	{
		return (m_CollisionFlags & flag) ? true : false;
	}

	void FusionPhysicsBody::_setCollisionFlags(int flags)
	{
		m_CollisionFlags = flags;
	}

	const Vector2 &FusionPhysicsBody::GetPosition() const
	{
		return m_Position;
	}

	CL_Point FusionPhysicsBody::GetPositionPoint() const
	{
		return CL_Point((int)m_Position.x, (int)m_Position.y);
	}

	const Vector2 &FusionPhysicsBody::GetForce() const
	{
		return m_AppliedForce;
	}

	float FusionPhysicsBody::GetEngineForce() const
	{
		return m_AppliedEngineForce;
	}

	const Vector2 &FusionPhysicsBody::GetAcceleration() const
	{
		return m_Acceleration;
	}

	const Vector2 &FusionPhysicsBody::GetVelocity() const
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

		m_DeactivationCounter = m_DeactivationPeriod;
	}

	void FusionPhysicsBody::_deactivate()
	{
		// Stop moving - we don't want it flying off into deep space ;)
		m_Acceleration = Vector2::ZERO;
		m_Velocity = Vector2::ZERO;

		m_Active = false;
	}

	void FusionPhysicsBody::_deactivateAfterCountdown(unsigned int split)
	{
		m_DeactivationCounter -= split;

		if (m_DeactivationCounter <= 0)
			_deactivate();
	}

	void FusionPhysicsBody::_setDeactivationCount(unsigned int count)
	{
		m_DeactivationCounter = count;
	}

	unsigned int FusionPhysicsBody::GetDeactivationCount() const
	{
		return m_DeactivationCounter;
	}

	void FusionPhysicsBody::SetDeactivationPeriod(unsigned int period)
	{
		m_DeactivationPeriod = period;
	}

	unsigned int FusionPhysicsBody::GetDeactivationPeriod() const
	{
		return m_DeactivationPeriod;
	}


	void FusionPhysicsBody::_setPosition(const Vector2 &position)
	{
		m_Position = position;
	}

	void FusionPhysicsBody::_setForce(const Vector2 &force)
	{
		m_AppliedForce = force;
	}

	void FusionPhysicsBody::_setEngineForce(float force)
	{
		m_AppliedEngineForce = force;
	}

	void FusionPhysicsBody::_setAcceleration(const Vector2 &acceleration)
	{
		m_Acceleration = acceleration;
	}

	void FusionPhysicsBody::_setVelocity(const Vector2 &velocity)
	{
		m_Velocity = velocity;
	}

	void FusionPhysicsBody::_setRotation(const float rotation)
	{
		m_Rotation = rotation;
	}


	//void FusionPhysicsBody::_notifyCollisionWith(FusionEngine::FusionPhysicsBody *other)
	//{
	//	m_CollidingBodies.push_back(other);
	//}

	//bool FusionPhysicsBody::IsCollidingWith(FusionEngine::FusionPhysicsBody *other) const
	//{
	//	BodyList::const_iterator it = m_CollidingBodies.begin();
	//	for (; it != m_CollidingBodies.end(); ++it)
	//	{
	//		if ((*it) == other)
	//			return true;
	//	}

	//	return false;
	//}

	//void FusionPhysicsBody::ClearCollisions()
	//{
	//	m_CollidingBodies.clear();
	//}


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
