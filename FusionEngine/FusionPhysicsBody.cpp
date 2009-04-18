
#include "Common.h"

#include "FusionPhysicsBody.h"

/// Fusion
#include "FusionPhysicsWorld.h"

namespace FusionEngine
{

	PhysicsBody::PhysicsBody()
		: m_World(0),
		//m_CollisionFlags(C_NONE),
		//m_CollisionResponse(0),
		m_CollisionHandler(0),
		m_UserData(0),
		m_Acceleration(Vector2::zero()),
		m_AppliedForce(Vector2::zero()),
		//m_GotCGUpdate(false),
		m_Active(true),
		m_DeactivationCounter(0),
		m_DeactivationPeriod(100),
		m_Type(0),
		m_Mass(0.f),
		m_Radius(0),
		m_CachedPosition(Vector2::zero()),
		m_Rotation(0.f),
		m_RotationalVelocity(0.f),
		//m_UsesAABB(false),
		//m_UsesDist(false),
		//m_UsesPixel(false),
		m_CachedVelocity(Vector2::zero()),
		m_AppliedRelativeForce(0)
	{
		m_BxBodyDef = new b2BodyDef();
		m_BxBodyDef->allowSleep = true;
		m_BxBodyDef->linearDamping = 0.0f;
		m_BxBodyDef->angularDamping = 0.01f;

		m_BxBodyDef->position.Set(0.0f, 0.0f);
	}

	//PhysicsBody::PhysicsBody(PhysicsWorld *world)
	//	: m_World(world),
	//	//m_CollisionFlags(C_NONE),
	//	//m_CollisionResponse(0),
	//	m_CollisionHandler(0),
	//	m_UserData(0),
	//	m_Acceleration(Vector2::zero()),
	//	m_AppliedForce(Vector2::zero()),
	//	//m_GotCGUpdate(false),
	//	m_Active(true),
	//	m_DeactivationCounter(0),
	//	m_DeactivationPeriod(world->GetBodyDeactivationPeriod()),
	//	m_Type(0),
	//	m_Mass(0.f),
	//	m_Radius(0),
	//	m_CachedPosition(Vector2::zero()),
	//	m_Rotation(0.f),
	//	m_RotationalVelocity(0.f),
	//	//m_UsesAABB(false),
	//	//m_UsesDist(false),
	//	//m_UsesPixel(false),
	//	m_CachedVelocity(Vector2::zero()),
	//	m_AppliedRelativeForce(0)
	//{
	//	m_BxBodyDef = new b2BodyDef();
	//	m_BxBodyDef->allowSleep = true;
	//	m_BxBodyDef->linearDamping = 0.0f;
	//	m_BxBodyDef->angularDamping = 0.01f;

	//	m_BxBodyDef->position.Set(0.0f, 0.0f);

	//	Initialize(world);
	//}

	PhysicsBody::~PhysicsBody()
	{
		Clear();
		if(m_BxBody)
		{
			m_World->_destroyBody(this);
			m_BxBody = NULL;
		}
	}

	//void PhysicsBody::Initialize(PhysicsWorld *world)
	//{
	//	if (world == NULL)
	//		return;

	//	m_World = world;

	//	_setVelocity(m_InitialVelocity);
	//	CommitProperties();
	//}

	void PhysicsBody::CommitProperties()
	{
		if(m_BxBody)
		{
			m_BxBody->SetBullet(m_Bullet);

			m_BxBody->SetUserData(m_UserData);

			if (!fe_fzero(m_Mass))
			{
				b2MassData massData;
				massData.mass = m_Mass;
				massData.center.SetZero();
				massData.I = 0.0f;
				m_BxBody->SetMass(&massData);
			}

			for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; it++)
			{
				(*it)->SetBody(m_BxBody);
				(*it)->Generate();
			}
		}
	}

	b2BodyDef *PhysicsBody::GetBodyDef() const
	{
		return m_BxBodyDef;
	}

	void PhysicsBody::_setB2Body(b2Body *b2body)
	{
		m_BxBody = b2body;
	}

	b2Body *PhysicsBody::GetB2Body() const
	{
		return m_BxBody;
	}

	void PhysicsBody::SetWorld(PhysicsWorld* world)
	{
		m_World = world;
		//m_DeactivationPeriod = world->GetBodyDeactivationPeriod();
	}

	void PhysicsBody::SetType(int type)
	{
		m_Type = type;
	}

	void PhysicsBody::SetMass(float mass)
	{
		m_Mass = mass;
	}

	void PhysicsBody::SetCanSleep(bool canSleep)
	{
		m_BxBodyDef->allowSleep = canSleep;
	}

	void PhysicsBody::SetInitialSleepState(bool isSleeping)
	{
		m_BxBodyDef->isSleeping = isSleeping;
	}


//	void PhysicsBody::RecalculateInertia()
//	{
//		float moment = 0;
//#ifdef PHYSBODY_USE_BOOST_PTRLIST
//		for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
//		{
//#else
//		for (ShapeList::iterator p_it = m_Shapes.begin(), end = m_Shapes.end(); p_it != end; ++p_it)
//		{
//			ShapeList::value_type it = (*p_it);
//#endif
//			moment += it->GetInertia();
//		}
//		cpBodySetMoment(m_BxBody, moment);
//	}

	//void PhysicsBody::SetRadius(float radius)
	//{
	//	m_Radius = radius;
	//}

	int PhysicsBody::GetType()
	{
		return m_Type;
	}

	float PhysicsBody::GetMass()
	{
		return m_Mass;
	}

	float PhysicsBody::GetInverseMass()
	{
		return m_InverseMass;
	}

	float PhysicsBody::GetRadius()
	{
		return m_Radius;
	}

	void PhysicsBody::ApplyForce(const Vector2 &force)
	{
		b2Vec2 worldForce = m_BxBody->GetWorldVector(b2Vec2(force.x, force.y));
		m_BxBody->ApplyForce(worldForce, m_BxBody->GetWorldCenter());
	}

	void PhysicsBody::ApplyForceRelative(const Vector2 &force)
	{
		float x = cosf( GetAngle() ) * force.getX() - sinf( GetAngle() ) * force.getY();
		float y = sinf( GetAngle() ) * force.getX() + cosf( GetAngle() ) * force.getY();

		m_BxBody->ApplyForce(b2Vec2(x, y), m_BxBody->GetWorldCenter());

		//_activate();
	}

	void PhysicsBody::ApplyForceRelative(float force)
	{
		float x = sinf(GetAngle()) * force;
		float y = -cosf(GetAngle()) * force;

		m_BxBody->ApplyImpulse(b2Vec2(x, y), m_BxBody->GetWorldCenter());

		//_activate();
	}

	void PhysicsBody::ApplyImpulse(const Vector2 &force)
	{
		//m_AppliedForce += force;

		m_BxBody->ApplyImpulse(b2Vec2(force.x, force.y), m_BxBody->GetWorldCenter());

		//_activate();
	}

	void PhysicsBody::ApplyImpulseRelative(const Vector2 &force)
	{
		float x = cosf( GetAngle() ) * force.getX() - sinf( GetAngle() ) * force.getY();
		float y = sinf( GetAngle() ) * force.getX() + cosf( GetAngle() ) * force.getY();

		m_BxBody->ApplyImpulse(b2Vec2(x, y), m_BxBody->GetWorldCenter());

		//_activate();
	}

	void PhysicsBody::ApplyImpulseRelative(float force)
	{
		float x = sinf(GetAngle()) * force;
		float y = -cosf(GetAngle()) * force;

		m_BxBody->ApplyImpulse(b2Vec2(x, y), m_BxBody->GetWorldCenter());

		//_activate();
	}

	void PhysicsBody::ApplyTorque(float torque)
	{
		m_BxBody->ApplyTorque(torque);
	}

	//void PhysicsBody::ClearForces()
	//{
	//}

	void PhysicsBody::AttachShape(ShapePtr shape)
	{
		shape->SetBody(m_BxBody);

		m_Shapes.push_back(shape);
	}

	void PhysicsBody::DetachShape(ShapePtr shape)
	{
		for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
		{
			ShapePtr itpShape = (*it);
			if (itpShape->GetShape() == shape->GetShape())
			{
				shape->SetBody(NULL);
				m_Shapes.erase(it);
			}
		}
	}

	void PhysicsBody::ClearShapes()
	{
		for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
		{
			ShapePtr shape = (*it);
			shape->SetBody(NULL);
		}
		m_Shapes.clear();
	}

	const PhysicsBody::ShapeList &PhysicsBody::GetShapes()
	{
		return m_Shapes;
	}

	ShapePtr PhysicsBody::GetShape(b2Shape *shape)
	{
		for(ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
			if((*it)->GetShape() == shape)
				return *it;

		return ShapePtr();
	}

	ShapePtr PhysicsBody::GetShape(b2Shape *shape) const
	{
		for(ShapeList::const_iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
			if((*it)->GetShape() == shape)
				return *it;

		return ShapePtr();
	}

	ShapePtr PhysicsBody::GetShape(const std::string &name)
	{
		for(ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
			if((*it)->GetName() == name)
				return *it;

		return ShapePtr();
	}

	ShapePtr PhysicsBody::GetShape(const std::string &name) const
	{
		for(ShapeList::const_iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
			if((*it)->GetName() == name)
				return *it;

		return ShapePtr();
	}

	void PhysicsBody::AddJoint(b2Joint* joint)
	{
	}

	void PhysicsBody::RemoveJoint(b2Joint* joint)
	{
	}

	void PhysicsBody::ClearJoints()
	{
	}

	void PhysicsBody::Clear()
	{
		//ClearForces();
		ClearShapes();
		ClearJoints();
	}

	//void PhysicsBody::SetAllShapesElasticity(float e)
	//{
	//	for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
	//	{
	//		(*it)->GetShape()->e = e;
	//	}
	//}

	//void PhysicsBody::SetAllShapesFriction(float u)
	//{
	//	for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
	//	{
	//		(*it)->GetShape()->u = u;
	//	}
	//}

	//void PhysicsBody::SetCoefficientOfFriction(float damping)
	//{
	//	m_LinearDamping = damping;

	//	SetAllShapesFriction(damping);
	//}

	//void PhysicsBody::SetCoefficientOfRestitution(float bounce)
	//{
	//	//if (bounce == 0.0f)
	//	//{
	//	//	m_CollisionFlags &= ~C_BOUNCE;
	//	//}
	//	//else
	//	//{
	//	//	m_CollisionFlags |= C_BOUNCE;
	//	//}

	//	m_Bounce = bounce;

	//	SetAllShapesElasticity(bounce);
	//}

	void PhysicsBody::SetAngularVelocityRad(float velocity)
	{
		m_RotationalVelocity = velocity;
		m_BxBody->SetAngularVelocity(velocity);
	}

	void PhysicsBody::SetAngularVelocityDeg(float velocity)
	{
		m_RotationalVelocity = fe_degtorad(velocity);
		m_BxBody->SetAngularVelocity(m_RotationalVelocity);
	}

	void PhysicsBody::SetAngularVelocity(float velocity)
	{
		m_RotationalVelocity = velocity;
		m_BxBody->SetAngularVelocity(velocity);
	}

	//void PhysicsBody::SetColBitmask(FusionEngine::FusionBitmask *bitmask)
	//{
	//	m_Bitmask = bitmask;
	//}

	//void PhysicsBody::SetColAABB(float width, float height)
	//{
	//	m_AABB = CL_Rectf(0, 0, width, height);
	//}

	//void PhysicsBody::SetColAABB(const CL_Rectf &bbox)
	//{
	//	m_AABBox = bbox;
	//}

	//void PhysicsBody::SetRadius(float dist)
	//{
	//	m_ColDist = dist;
	//}

	//FusionBitmask *PhysicsBody::GetColBitmask() const
	//{
	//	return m_Bitmask;
	//}

	//void PhysicsBody::CacheBB()
	//{
	//	for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
	//	{
	//		ShapePtr shape = (*it);

	//		m_AABB.left = fe_min( shape->GetShape()->bb.l, m_AABB.left );
	//		m_AABB.right = fe_max( shape->GetShape()->bb.r, m_AABB.right );
	//		m_AABB.top = fe_min( shape->GetShape()->bb.b, m_AABB.top );
	//		m_AABB.bottom = fe_max( shape->GetShape()->bb.t, m_AABB.bottom );
	//	}
	//}

	CL_Rectf PhysicsBody::GetAABB() const
	{
		//if (not active)
		//	CacheBB();
		return m_AABB;
	}

	//float PhysicsBody::GetColDist() const
	//{
	//	return m_ColDist;
	//}

	//bool PhysicsBody::GetColPoint(const CL_Point &point, bool auto_offset) const
	//{
	//	if (auto_offset)
	//	{
	//		CL_Point pos = GetPositionPoint();
	//		// Offset
	//		CL_Point scaled_point = (point - pos);
	//		// Scale
	//		scaled_point.x /= m_Bitmask->GetPPB();
	//		scaled_point.y /= m_Bitmask->GetPPB();

	//		return m_Bitmask->GetBit(scaled_point);
	//	}
	//	else
	//		return m_Bitmask->GetBit(point);
	//}

	//void PhysicsBody::SetUsePixelCollisions(bool usePixel)
	//{
	//	m_UsesPixel = usePixel;
	//}

	//void PhysicsBody::SetUseAABBCollisions(bool useAABB)
	//{
	//	m_UsesAABB = useAABB;
	//}

	//void PhysicsBody::SetUseDistCollisions(bool useDist)
	//{
	//	m_UsesDist = useDist;
	//}

	//bool PhysicsBody::GetUsePixelCollisions() const
	//{
	//	return m_UsesPixel;
	//}

	//bool PhysicsBody::GetUseAABBCollisions() const
	//{
	//	return m_UsesAABB;
	//}

	//bool PhysicsBody::GetUseDistCollisions() const
	//{
	//	return m_UsesDist;
	//}

	void PhysicsBody::SetUserData(void *userdata)
	{
		m_UserData = userdata;
	}

	void *PhysicsBody::GetUserData() const
	{
		return m_UserData;
	}


	//void PhysicsBody::SetCollisionCallback(const CollisionCallback &method)
	//{
	//	m_CollisionResponse = method;
	//}

	void PhysicsBody::SetCollisionHandler(ICollisionHandler *handler)
	{
		m_CollisionHandler = handler;
	}

	bool PhysicsBody::CanCollideWith(PhysicsBody *other)
	{
		if (m_CollisionHandler != 0)
			return m_CollisionHandler->CanCollideWith(other);

		return true;
	}

	void PhysicsBody::ContactBegin(const Contact &contact)
	{
		if (m_CollisionHandler != 0)
			m_CollisionHandler->ContactBegin(contact);

		//if (m_CollisionResponse != 0)
		//	m_CollisionResponse(other, contacts);
	}

	void PhysicsBody::ContactPersist(const Contact &contact)
	{
		if (m_CollisionHandler != 0)
			m_CollisionHandler->ContactPersist(contact);
	}

	void PhysicsBody::ContactEnd(const Contact &contact)
	{
		if (m_CollisionHandler != 0)
			m_CollisionHandler->ContactEnd(contact);
	}

	//void PhysicsBody::CollisionResponse(PhysicsBody *other, const std::vector<Contact> &contacts)
	//{
	//	if (m_CollisionResponse != 0)
	//		m_CollisionResponse(other, contacts);

	//	if (m_CollisionHandler != 0)
	//		m_CollisionHandler->CollisionWith(other, contacts);
	//}

	bool PhysicsBody::IsStatic() const
	{
		return m_BxBody->IsStatic();
	}

	//int PhysicsBody::GetCollisionFlags()
	//{
	//	return m_CollisionFlags;
	//}

	//int PhysicsBody::GetCollisionFlags() const
	//{
	//	return m_CollisionFlags;
	//}

	//bool PhysicsBody::CheckCollisionFlag(int flag)
	//{
	//	return (m_CollisionFlags & flag) ? true : false;
	//}

	//bool PhysicsBody::CheckCollisionFlag(int flag) const
	//{
	//	return (m_CollisionFlags & flag) ? true : false;
	//}

	//void PhysicsBody::_setCollisionFlags(int flags)
	//{
	//	m_CollisionFlags = flags;
	//}

	const Vector2 &PhysicsBody::GetPosition()
	{
		m_CachedPosition = b2v2(m_BxBody->GetPosition());
		return m_CachedPosition;
	}

	CL_Point PhysicsBody::GetPositionPoint() const
	{
		const b2Vec2& pos = m_BxBody->GetPosition();
		return CL_Point((int)fe_round(pos.x), (int)fe_round(pos.y));
	}

	//const Vector2 &PhysicsBody::GetForce()
	//{
	//	m_AppliedForce.x = m_BxBody->f.x;
	//	m_AppliedForce.y = m_BxBody->f.y;
	//	return m_AppliedForce;
	//}

	//const Vector2& PhysicsBody::GetRelativeForce() const
	//{
	//	return m_AppliedRelativeForce;
	//}

	//const Vector2 &PhysicsBody::GetAcceleration() const
	//{
	//	return m_Acceleration;
	//}

	const Vector2 &PhysicsBody::GetVelocity()
	{
		m_CachedVelocity = b2v2(m_BxBody->GetLinearVelocity());
		return m_CachedVelocity;
	}

	//float PhysicsBody::GetFriction() const
	//{
	//	return m_LinearDamping;
	//}

	//float PhysicsBody::GetRestitution() const
	//{
	//	return m_Bounce;
	//}

	float PhysicsBody::GetAngularVelocityRad() const
	{
		return m_BxBody->GetAngularVelocity();
	}

	float PhysicsBody::GetAngularVelocityDeg() const
	{
		return fe_radtodeg(GetAngularVelocityRad());
	}

	float PhysicsBody::GetAngularVelocity() const
	{
		return GetAngularVelocityRad();
	}

	float PhysicsBody::GetAngleRad() const
	{
		return m_BxBody->GetAngle();
	}

	float PhysicsBody::GetAngleDeg() const
	{
		return fe_radtodeg(GetAngleRad());
	}

	float PhysicsBody::GetAngle() const
	{
		return GetAngleRad();
	}

	bool PhysicsBody::IsActive() const
	{
		return !m_BxBody->IsSleeping();
	}

	void PhysicsBody::_activate()
	{
		m_BxBody->WakeUp();
	}

	void PhysicsBody::_deactivate()
	{
		m_BxBody->PutToSleep();
	}

	//void PhysicsBody::_deactivateAfterCountdown(unsigned int split)
	//{
	//	m_DeactivationCounter -= split;

	//	if (m_DeactivationCounter <= 0)
	//		_deactivate();
	//}

	//void PhysicsBody::_setDeactivationCount(unsigned int count)
	//{
	//	m_DeactivationCounter = count;
	//}

	//unsigned int PhysicsBody::GetDeactivationCount() const
	//{
	//	return m_DeactivationCounter;
	//}

	//void PhysicsBody::SetDeactivationPeriod(unsigned int period)
	//{
	//	m_DeactivationPeriod = period;
	//}

	//unsigned int PhysicsBody::GetDeactivationPeriod() const
	//{
	//	return m_DeactivationPeriod;
	//}


	void PhysicsBody::_setPosition(const Vector2 &position)
	{
		//m_CachedPosition = position;
		if(m_BxBody)
		{
			m_BxBody->SetXForm(b2Vec2(position.x, position.y), m_BxBody->GetAngle());
		}
		else
		{
			m_BxBodyDef->position.Set(position.x, position.y);
		}
	}

	//void PhysicsBody::_setForce(const Vector2 &force)
	//{
	//	m_AppliedForce = force;
	//}

	//void PhysicsBody::_setRelativeForce(const Vector2 &force, float direction)
	//{
	//	m_AppliedRelativeForce = force;
	//}

	//void PhysicsBody::_setAcceleration(const Vector2 &acceleration)
	//{
	//	m_Acceleration = acceleration;
	//}

	void PhysicsBody::_setVelocity(const Vector2 &velocity)
	{
		m_CachedVelocity = velocity;

		if(m_BxBody)
		{
			if (fe_fzero(velocity.x))
				m_CachedVelocity.x = 0;
			if (fe_fzero(velocity.y))
				m_CachedVelocity.y = 0;

			m_BxBody->SetLinearVelocity(b2Vec2(m_CachedVelocity.x, m_CachedVelocity.y));
		}
		else
		{
			m_InitialVelocity = velocity;
		}
	}

	void PhysicsBody::_setAngleRad(const float rotation)
	{
		m_Rotation = rotation;
		m_BxBody->SetXForm(m_BxBody->GetPosition(), rotation);
	}

	void PhysicsBody::_setAngleDeg(const float rotation)
	{
		m_Rotation = fe_degtorad(rotation);
		_setAngleRad(fe_degtorad(rotation));
	}

	void PhysicsBody::_setAngle(const float rotation)
	{
		m_Rotation = rotation;
		_setAngleRad(rotation);
	}


	//void PhysicsBody::_notifyCollisionWith(FusionEngine::PhysicsBody *other)
	//{
	//	m_CollidingBodies.push_back(other);
	//}

	//bool PhysicsBody::IsCollidingWith(FusionEngine::PhysicsBody *other) const
	//{
	//	BodyList::const_iterator it = m_CollidingBodies.begin();
	//	for (; it != m_CollidingBodies.end(); ++it)
	//	{
	//		if ((*it) == other)
	//			return true;
	//	}

	//	return false;
	//}

	//void PhysicsBody::ClearCollisions()
	//{
	//	m_CollidingBodies.clear();
	//}


	//void PhysicsBody::_setCGPos(int ind)
	//{
	//	m_CGPos = ind;
	//}

	//int PhysicsBody::_getCGPos() const
	//{
	//	return m_CGPos;
	//}

	//void PhysicsBody::_setCCIndex(int ind)
	//{
	//	m_CCIndex = ind;
	//}

	//int PhysicsBody::_getCCIndex() const
	//{
	//	return m_CCIndex;
	//}

	//void PhysicsBody::_notifyCGwillUpdate()
	//{
	//	m_GotCGUpdate = true;
	//}

	//bool PhysicsBody::_CGwillUpdate() const
	//{
	//	return m_GotCGUpdate;
	//}

	//void PhysicsBody::_notifyCGUpdated()
	//{
	//	m_GotCGUpdate = false;
	//}

}
