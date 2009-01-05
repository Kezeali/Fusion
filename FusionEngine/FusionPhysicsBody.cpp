
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
		m_GotCGUpdate(false),
		m_Active(true),
		m_DeactivationCounter(0),
		m_DeactivationPeriod(100),
		m_Type(0),
		m_Mass(0.f),
		m_Radius(0),
		m_Position(Vector2::zero()),
		m_Rotation(0.f),
		m_RotationalVelocity(0.f),
		m_UsesAABB(false),
		m_UsesDist(false),
		m_UsesPixel(false),
		m_Velocity(Vector2::zero()),
		m_AppliedRelativeForce(0)
	{
		m_BxBodyDef = new b2BodyDef();
		m_BxBodyDef->allowSleep = true;
		m_BxBodyDef->linearDamping = 0.0f;
		m_BxBodyDef->angularDamping = 0.01f;

		m_BxBodyDef->position.Set(0.0f, 0.0f);
	}

	PhysicsBody::PhysicsBody(PhysicsWorld *world)
		: m_World(world),
		//m_CollisionFlags(C_NONE),
		//m_CollisionResponse(0),
		m_CollisionHandler(0),
		m_UserData(0),
		m_Acceleration(Vector2::zero()),
		m_AppliedForce(Vector2::zero()),
		m_GotCGUpdate(false),
		m_Active(true),
		m_DeactivationCounter(0),
		m_DeactivationPeriod(world->GetBodyDeactivationPeriod()),
		m_Type(0),
		m_Mass(0.f),
		m_Radius(0),
		m_Position(Vector2::zero()),
		m_Rotation(0.f),
		m_RotationalVelocity(0.f),
		m_UsesAABB(false),
		m_UsesDist(false),
		m_UsesPixel(false),
		m_Velocity(Vector2::zero()),
		m_AppliedRelativeForce(0)
	{
		m_BxBodyDef = new b2BodyDef();
		m_BxBodyDef->allowSleep = true;
		m_BxBodyDef->linearDamping = 0.0f;
		m_BxBodyDef->angularDamping = 0.01f;

		m_BxBodyDef->position.Set(0.0f, 0.0f);

		Initialize(world);
	}

	PhysicsBody::~PhysicsBody()
	{
		Clear();
		if(m_BxBody)
		{
			m_World->BodyDeleted(this);
			m_BxBody = NULL;
		}
	}

	void PhysicsBody::Initialize(PhysicsWorld *world /* = NULL */)
	{
		if (world != NULL)
			m_World = world;

		if(!m_BxBody)
		{
			m_BxBody = m_World->SubstantiateBody(this);
		}

		_setVelocity(m_InitialVelocity);
		CommitProperties();
	}

	void PhysicsBody::CommitProperties()
	{
		if(m_BxBody)
		{
			m_BxBody->SetBullet(m_Bullet);

			m_BxBody->SetUserData(m_UserData);

			b2MassData massData;
			massData.mass = m_Mass;
			massData.center.SetZero();
			massData.I = 0.0f;
			m_BxBody->SetMass(&massData);

			for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; it++)
			{
				(*it)->SetBody(m_BxBody);
				(*it)->UpdateProperties();
			}
		}
	}

	b2BodyDef *PhysicsBody::GetBodyDef() const
	{
		return m_BxBodyDef;
	}

	b2Body *PhysicsBody::GetB2Body() const
	{
		return m_BxBody;
	}

	void PhysicsBody::SetWorld(PhysicsWorld* world)
	{
		m_World = world;
		m_DeactivationPeriod = world->GetBodyDeactivationPeriod();
	}

	void PhysicsBody::SetType(int type)
	{
		m_Type = type;
	}

	void PhysicsBody::SetMass(float mass)
	{
		m_Mass = mass;
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

	void PhysicsBody::SetRadius(float radius)
	{
		m_Radius = radius;
	}

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
		//m_AppliedForce += force;

		cpBodyApplyImpulse(m_BxBody, cpv(force.x, force.y), cpvzero);//m_BxBody->p);

		_activate();
	}

	void PhysicsBody::ApplyForceRelative(const Vector2 &force)
	{
		float mag = force.length();
		Vector2 force_relative(
			sinf( GetRotation() ) * mag,
			-cosf( GetRotation() ) * mag
			);
		//m_AppliedRelativeForce += force_relative;
		//m_AppliedForce += force_relative;

		cpBodyApplyImpulse(m_BxBody, cpv(force_relative.x, force_relative.y), m_BxBody->p);

		_activate();
	}

	void PhysicsBody::ApplyForceRelative(float force)
	{
		Vector2 force_vector(
			sinf(GetRotation()) * force,
			-cosf(GetRotation()) * force
			);
		//m_AppliedRelativeForce += force_vector;
		//m_AppliedForce += force_vector;

		cpBodyApplyImpulse(m_BxBody, cpv(force_vector.x, force_vector.y), m_BxBody->p);

		_activate();
	}

	void PhysicsBody::ClearForces()
	{
	}

	void PhysicsBody::AttachShape(Shape* shape)
	{
		shape->SetBody(this);
		shape->GetShape()->collision_type = g_PhysBodyCpCollisionType;

		cpBodySetMoment(m_BxBody, m_BxBody->i + shape->GetInertia());

		// Add to world
		m_World->AddShape(shape);

		m_Shapes.push_back(ShapePtr(shape));
	}

	void PhysicsBody::DetachShape(Shape* shape)
	{
		for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
		{
			ShapePtr itpShape = (*it);
			if (itpShape->GetShape() == shape->GetShape())
			{
				cpBodySetMoment(m_BxBody, m_BxBody->i - shape->GetInertia());
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

	const ShapeList &GetShapes() const
	{
		return m_Shapes;
	}

	Shape *PhysicsBody::GetShape(b2Shape *shape)
	{
		for(ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
			if((*it)->GetShape() == shape)
				return *it;

		return NULL;
	}

	Shape *PhysicsBody::GetShape(const std::string &name)
	{
		for(ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
			if((*it)->GetName() == name)
				return *it;

		return NULL;
	}

	void PhysicsBody::AttachJoint(cpJoint* joint, bool toWorld)
	{
	}

	void PhysicsBody::DetachJoint(cpJoint* joint, bool fromWorld)
	{
	}

	void PhysicsBody::ClearJoints()
	{
	}

	void PhysicsBody::Clear()
	{
		ClearForces();
		ClearShapes();
		ClearJoints();
	}

	void PhysicsBody::SetAllShapesElasticity(float e)
	{
		for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
		{
			(*it)->GetShape()->e = e;
		}
	}

	void PhysicsBody::SetAllShapesFriction(float u)
	{
		for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
		{
			(*it)->GetShape()->u = u;
		}
	}

	void PhysicsBody::SetCoefficientOfFriction(float damping)
	{
		m_LinearDamping = damping;

		SetAllShapesFriction(damping);
	}

	void PhysicsBody::SetCoefficientOfRestitution(float bounce)
	{
		//if (bounce == 0.0f)
		//{
		//	m_CollisionFlags &= ~C_BOUNCE;
		//}
		//else
		//{
		//	m_CollisionFlags |= C_BOUNCE;
		//}

		m_Bounce = bounce;

		SetAllShapesElasticity(bounce);
	}

	void PhysicsBody::SetRotationalVelocityRad(float velocity)
	{
		m_RotationalVelocity = velocity;
		m_BxBody->w = velocity;
	}

	void PhysicsBody::SetRotationalVelocityDeg(float velocity)
	{
		m_RotationalVelocity = fe_degtorad(velocity);
		m_BxBody->w = m_RotationalVelocity;
	}

	void PhysicsBody::SetRotationalVelocity(float velocity)
	{
		m_RotationalVelocity = velocity;

		m_BxBody->w = velocity;

		// Don't activate if this was a call to stop rotation!
		if (velocity)
			_activate();
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

	void PhysicsBody::CacheBB()
	{
		for (ShapeList::iterator it = m_Shapes.begin(), end = m_Shapes.end(); it != end; ++it)
		{
			ShapePtr shape = (*it);

			m_AABB.left = fe_min( shape->GetShape()->bb.l, m_AABB.left );
			m_AABB.right = fe_max( shape->GetShape()->bb.r, m_AABB.right );
			m_AABB.top = fe_min( shape->GetShape()->bb.b, m_AABB.top );
			m_AABB.bottom = fe_max( shape->GetShape()->bb.t, m_AABB.bottom );
		}
	}

	CL_Rectf PhysicsBody::GetAABB() const
	{
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

	void PhysicsBody::AddContact(PhysicsBody *other, const Contact &contact)
	{
		if (m_CollisionHandler != 0)
			m_CollisionHandler->AddContact(other, contact);

		//if (m_CollisionResponse != 0)
		//	m_CollisionResponse(other, contacts);
	}

	void PhysicsBody::RemoveContact(const Contact &contact)
	{
		if (m_CollisionHandler != 0)
			m_CollisionHandler->RemoveContact(contact);
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
		return CheckCollisionFlag(C_STATIC);
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
		m_Position.x = m_BxBody->p.x;
		m_Position.y = m_BxBody->p.y;
		return m_Position;
	}

	CL_Point PhysicsBody::GetPositionPoint() const
	{
		return CL_Point((int)m_BxBody->p.x, (int)m_BxBody->p.y);
	}

	const Vector2 &PhysicsBody::GetForce()
	{
		m_AppliedForce.x = m_BxBody->f.x;
		m_AppliedForce.y = m_BxBody->f.y;
		return m_AppliedForce;
	}

	const Vector2& PhysicsBody::GetRelativeForce() const
	{
		return m_AppliedRelativeForce;
	}

	const Vector2 &PhysicsBody::GetAcceleration() const
	{
		return m_Acceleration;
	}

	const Vector2 &PhysicsBody::GetVelocity()
	{
		m_Velocity.x = m_BxBody->v.x;
		m_Velocity.y = m_BxBody->v.y;
		return m_Velocity;
	}

	float PhysicsBody::GetCoefficientOfFriction() const
	{
		return m_LinearDamping;
	}

	float PhysicsBody::GetCoefficientOfRestitution() const
	{
		return m_Bounce;
	}

	float PhysicsBody::GetRotationalVelocityRad() const
	{
		return m_RotationalVelocity;
	}

	float PhysicsBody::GetRotationalVelocityDeg() const
	{
		return fe_radtodeg(m_RotationalVelocity);
	}

	float PhysicsBody::GetRotationalVelocity() const
	{
		return m_RotationalVelocity;
	}

	float PhysicsBody::GetRotationRad() const
	{
		return m_BxBody->a;
	}

	float PhysicsBody::GetRotationDeg() const
	{
		return fe_radtodeg(m_BxBody->a);
	}

	float PhysicsBody::GetRotation() const
	{
		return m_BxBody->a;
	}

	bool PhysicsBody::IsActive() const
	{
		return m_Active;
	}

	void PhysicsBody::_activate()
	{
		m_Active = true;

		m_DeactivationCounter = m_DeactivationPeriod;
	}

	void PhysicsBody::_deactivate()
	{
		// Stop moving - we don't want it flying off into deep space ;)
		m_Acceleration = Vector2::zero();
		m_Velocity = Vector2::zero();

		m_Active = false;
	}

	void PhysicsBody::_deactivateAfterCountdown(unsigned int split)
	{
		m_DeactivationCounter -= split;

		if (m_DeactivationCounter <= 0)
			_deactivate();
	}

	void PhysicsBody::_setDeactivationCount(unsigned int count)
	{
		m_DeactivationCounter = count;
	}

	unsigned int PhysicsBody::GetDeactivationCount() const
	{
		return m_DeactivationCounter;
	}

	void PhysicsBody::SetDeactivationPeriod(unsigned int period)
	{
		m_DeactivationPeriod = period;
	}

	unsigned int PhysicsBody::GetDeactivationPeriod() const
	{
		return m_DeactivationPeriod;
	}


	void PhysicsBody::_setPosition(const Vector2 &position)
	{
		//m_Position = position;
		if(m_BxBody)
		{
			m_BxBody->SetXForm(b2Vec2(position.x, position.y), m_BxBody->GetAngle());
		}
		else
		{
			m_BodyDef->position.Set(position.x, position.y);
		}
	}

	void PhysicsBody::_setForce(const Vector2 &force)
	{
		m_AppliedForce = force;
	}

	void PhysicsBody::_setRelativeForce(const Vector2 &force, float direction)
	{
		m_AppliedRelativeForce = force;
	}

	void PhysicsBody::_setAcceleration(const Vector2 &acceleration)
	{
		m_Acceleration = acceleration;
	}

	void PhysicsBody::_setVelocity(const Vector2 &velocity)
	{
		m_Velocity = velocity;

		m_BxBody->v.x = velocity.x;
		m_BxBody->v.y = velocity.y;

		if(m_BxBody)
		{
			if (fe_fzero(vector.x))
				vector.x = 0;
			if (fe_fzero(vector.y))
				vector.y = 0;

			bool was_moving = IsMoving();
		}
		else
		{
			m_InitialVelocity = vector;
		}
	}

	void PhysicsBody::_setRotationRad(const float rotation)
	{
		m_Rotation = rotation;
		cpBodySetAngle(m_BxBody, rotation);
	}

	void PhysicsBody::_setRotationDeg(const float rotation)
	{
		m_Rotation = fe_degtorad(rotation);
		cpBodySetAngle(m_BxBody, fe_degtorad(rotation));
	}

	void PhysicsBody::_setRotation(const float rotation)
	{
		m_Rotation = rotation;
		cpBodySetAngle(m_BxBody, rotation);
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


	void PhysicsBody::_setCGPos(int ind)
	{
		m_CGPos = ind;
	}

	int PhysicsBody::_getCGPos() const
	{
		return m_CGPos;
	}

	void PhysicsBody::_setCCIndex(int ind)
	{
		m_CCIndex = ind;
	}

	int PhysicsBody::_getCCIndex() const
	{
		return m_CCIndex;
	}

	void PhysicsBody::_notifyCGwillUpdate()
	{
		m_GotCGUpdate = true;
	}

	bool PhysicsBody::_CGwillUpdate() const
	{
		return m_GotCGUpdate;
	}

	void PhysicsBody::_notifyCGUpdated()
	{
		m_GotCGUpdate = false;
	}

}
