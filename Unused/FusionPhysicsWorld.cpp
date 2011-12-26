/*
  Copyright (c) 2006-2009 Fusion Project Team

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


	File Author(s):

		Elliot Hayward
*/

#include "Common.h"

#include "FusionPhysicsWorld.h"

/// Fusion
//#include "FusionPhysicsCollisionGrid.h"
//#include "FusionPhysicsUtils.h"
#include "FusionConsole.h"

namespace FusionEngine
{
	////! Returns true if the first address is lower than the second
	///*!
	// * This (along with CollisionAfter()) is used to sort Collision collections
	// * so that equivilant Collisions are adjcent.
	// */
	//bool CollisionBefore(Collision *lhs, Collision *rhs)
	//{
	//	if (lhs->First < rhs->First && lhs->Second < rhs->Second)
	//	{
	//		if (lhs->First < rhs->Second && lhs->Second < rhs->First)
	//		{
	//			return true;
	//		}
	//	}

	//	return false;
	//}
	////! Returns true if the first address is higher than the second
	///*!
	// * This (along with CollisionBefore()) is used to sort Collision collections
	// * so that equivilant Collisions are adjcent.
	// */
	//bool CollisionAfter(Collision *lhs, Collision *rhs)
	//{
	//	if (lhs->First < rhs->Second && lhs->Second < rhs->First)
	//	{
	//		if (lhs->First < rhs->First && lhs->Second < rhs->Second)
	//		{
	//			return true;
	//		}
	//	}

	//	return false;
	//}
	////! Returns true if the given Collisions are equal
	///*!
	// * Collisions are considered equal if both the First and Second pointers
	// * have the same address (First and Second point to physical bodies).
	// */
	//bool CollisionEqual(Collision *lhs, Collision *rhs)
	//{
	//	// Both collisions are exactly the same
	//	if (lhs->First == rhs->First && lhs->Second == rhs->Second)
	//	{
	//		return true;
	//	}

	//	// Both collisions are essentually the same, with the first and second inverted
	//	if (lhs->First == rhs->Second && lhs->Second == rhs->First)
	//	{
	//		return true;
	//	}

	//	return false;
	//}


	PhysicsWorld::PhysicsWorld()
		: m_BitmaskRes(1),
		m_Wrap(false),
		m_Width(2000.f), m_Height(2000.f),
		m_DeactivationPeriod(100),
		m_DeactivationVelocity(0.001f),
		m_MaxVelocity(100.0f),
		m_RunningSimulation(false),
		m_VelocityIterations(10),
		m_PositionIterations(8)
	{
		m_DeactivationVelocitySquared = m_DeactivationVelocity * m_DeactivationVelocity;
		m_MaxVelocitySquared = m_MaxVelocity * m_MaxVelocity;

		float widthPositive = m_Width * 0.5f;
		float heightPositive = m_Height * 0.5f;

		b2AABB worldAABB;
		worldAABB.lowerBound.Set(-widthPositive, -heightPositive);
		worldAABB.upperBound.Set(widthPositive, heightPositive);

		b2Vec2 gravity(0.0f, 0.0f);
		bool doSleep = true;

		m_BxWorld = new b2World(worldAABB, gravity, doSleep);

		m_ContactListener = new ContactListener(this);
		m_BxWorld->SetContactListener(m_ContactListener);

		m_DebugDraw = new DebugDraw();
		m_BxWorld->SetDebugDraw(m_DebugDraw);
	}

	PhysicsWorld::PhysicsWorld(float width, float height)
		: m_BitmaskRes(1),
		m_Wrap(false),
		m_Width(width), m_Height(height),
		m_DeactivationPeriod(100),
		m_DeactivationVelocity(0.001f),
		m_MaxVelocity(100.0f),
		m_RunningSimulation(false),
		m_VelocityIterations(10),
		m_PositionIterations(8)
	{
		m_DeactivationVelocitySquared = m_DeactivationVelocity * m_DeactivationVelocity;
		m_MaxVelocitySquared = m_MaxVelocity * m_MaxVelocity;

		float widthPositive = m_Width * 0.5f;
		float heightPositive = m_Height * 0.5f;

		b2AABB worldAABB;
		worldAABB.lowerBound.Set(-widthPositive, -heightPositive);
		worldAABB.upperBound.Set(widthPositive, heightPositive);

		b2Vec2 gravity(0.0f, 0.0f);
		bool doSleep = true;

		m_BxWorld = new b2World(worldAABB, gravity, doSleep);

		m_ContactListener = new ContactListener(this);
		m_BxWorld->SetContactListener(m_ContactListener);

		m_DebugDraw = new DebugDraw();
		m_BxWorld->SetDebugDraw(m_DebugDraw);
	}

	PhysicsWorld::~PhysicsWorld()
	{
		Clear();
		delete m_ContactListener;
		delete m_DebugDraw;
		delete m_BxWorld;
	}

	void PhysicsWorld::AddBody(PhysicsBodyPtr body)
	{
		b2Body* bxBody = m_BxWorld->CreateBody(body->GetBodyDef());
		body->_setB2Body(bxBody);
		body->CommitProperties();
		m_Bodies[bxBody] = body;
	}

	void PhysicsWorld::RemoveBody(PhysicsBodyPtr body)
	{
		if (m_RunningSimulation)
			m_DeleteQueue.push_back(body); // keep a reference (this this is cleared when the simulation finishes)

		m_Bodies.erase(body->GetB2Body());
	}
	
	void PhysicsWorld::_destroyBody(PhysicsBody *body)
	{
		m_BxWorld->DestroyBody(body->GetB2Body());
	}

	const PhysicsWorld::BodyMap& PhysicsWorld::GetBodies() const
	{
		return m_Bodies;
	}

	void PhysicsWorld::Clear()
	{
		// Body::Destructor calls World::BodyDeleted(self), which destroys the Box2D object
		//  so this is all that is needed to clear the world.
		m_Bodies.clear();
		m_DeleteQueue.clear();
	}

	//void PhysicsWorld::constrainBorders(void* ptr, void* data)
	//{
	//	cpBody* body = (cpBody*)ptr;
	//	PhysicsWorld* world = (PhysicsWorld*)data;

	//	if (body->p.x < 0.0f || body->p.x > world->m_Width)
	//		body->v.x = 0.0f;
	//	if (body->p.y < 0.0f || body->p.y > world->m_Height)
	//		body->v.y = 0.0f;

	//	body->p.x = fe_clamped(body->p.x, 0.0f, world->m_Width);
	//	body->p.y = fe_clamped(body->p.y, 0.0f, world->m_Height);
	//}

	//void PhysicsWorld::wrapAround(void* ptr, void* data)
	//{
	//	cpBody* body = (cpBody*)ptr;
	//	PhysicsWorld* world = (PhysicsWorld*)data;

	//	if (body->p.x > world->m_Width)
	//		body->p.x = 0.1f;
	//	else if (body->p.x < 0.0f)
	//		body->p.x = world->m_Width;

	//	if (body->p.y > world->m_Height)
	//		body->p.y = 0.1f;
	//	else if (body->p.y < 0.0f)
	//		body->p.y = world->m_Height;
	//}

	void PhysicsWorld::constrainBodies()
	{
		for (BodyMap::iterator it = m_Bodies.begin(), end = m_Bodies.end(); it != end; ++it)
		{
			PhysicsBodyPtr body = it->second;

			if (m_Wrap)
			{
				Vector2 position = body->GetPosition();
				if (position.x > m_Width)
					position.x = 0.1f;
				else if (position.x < 0.0f)
					position.x = m_Width;

				if (position.y > m_Height)
					position.y = 0.1f;
				else if (position.y < 0.0f)
					position.y = m_Height;

				body->_setPosition(position);
			}
			else
			{
				Vector2 position = body->GetPosition();
				Vector2 velocity = body->GetVelocity();

				if (position.x < 0.0f || position.x > m_Width)
					velocity.x = 0.0f;
				if (position.y < 0.0f || position.y > m_Height)
					velocity.y = 0.0f;

				body->_setVelocity(velocity);

				position.x = fe_clamped(position.x, 0.0f, m_Width);
				position.y = fe_clamped(position.y, 0.0f, m_Height);

				body->_setPosition(position);
			}
		}
	}

	void PhysicsWorld::RunSimulation(float delta_seconds)
	{
		//for (ForceList::iterator it = m_ForceQueue.begin(), end = m_ForceQueue.end(); it != end; ++it)
		//	it->ApplyForce();

		m_RunningSimulation = true; // bodies can't be deleted

		float32 dt = delta_seconds;// * 0.001f;
		if (dt > 0.0f)
			m_BxWorld->Step(dt, m_VelocityIterations, m_PositionIterations);
		else
			m_BxWorld->Step(dt, 0, 0);

		m_RunningSimulation = false; // bodies can be deleted

		// Delete bodies
		m_DeleteQueue.clear();

		//UpdateContacts();

		//constrainBodies();
	}

	void PhysicsWorld::OnBeginContact(b2Contact *contact)
	{
		// Each fixture must be fixed to a body with valid userdata (since this is how we get the
		//  pointer to the Physics::Body wrapper)
		if (contact->GetFixtureA()->GetBody()->GetUserData() == NULL || contact->GetFixtureB()->GetBody()->GetUserData() == NULL)
			return;

		PhysicsBody *body1 = static_cast<PhysicsBody*>( contact->GetFixtureA()->GetBody()->GetUserData() );
		PhysicsBody *body2 = static_cast<PhysicsBody*>( contact->GetFixtureB()->GetBody()->GetUserData() );
		//BodyMap::iterator it_body1 = m_Bodies.find(contact->shape1->GetBody());
		//BodyMap::iterator it_body2 = m_Bodies.find(contact->shape2->GetBody());
		//if (it_body1 != m_Bodies.end() && it_body2 != m_Bodies.end() && it_body1->second != NULL && it_body2->second != NULL)
		//{
		//	PhysicsBodyPtr body1 = it_body1->second;
		//	PhysicsBodyPtr body2 = it_body2->second;

			Contact fsnContact = Contact::CreateContact(contact);
			body1->ContactBegin(fsnContact);
			body2->ContactBegin(fsnContact);
		//}
	}

	void PhysicsWorld::OnEndContact(b2Contact *contact)
	{
		// Notify both bodies that the contact has stopped
		//  (if the respective bodies can be found)
		//BodyMap::iterator it_body1 = m_Bodies.find(contact->shape1->GetBody());
		//BodyMap::iterator it_body2 = m_Bodies.find(contact->shape2->GetBody());
		//if (it_body1 != m_Bodies.end() && it_body2 != m_Bodies.end() && it_body1->second != NULL && it_body2->second != NULL)
		//{
		//	PhysicsBodyPtr body1 = it_body1->second;
		//	PhysicsBodyPtr body2 = it_body2->second;

		//	Contact fsnContact(contact, body1->GetShape(contact->shape1), body2->GetShape(contact->shape2));
		//	body1->ContactEnd(fsnContact);
		//	body2->ContactEnd(fsnContact);
		//}

		if (contact->GetFixtureA()->GetBody()->GetUserData() == NULL || contact->GetFixtureB()->GetBody()->GetUserData() == NULL)
			return;

		PhysicsBody *body1 = static_cast<PhysicsBody*>( contact->GetFixtureA()->GetBody()->GetUserData() );
		PhysicsBody *body2 = static_cast<PhysicsBody*>( contact->GetFixtureB()->GetBody()->GetUserData() );

		Contact fsnContact = Contact::CreateContact(contact);
		body1->ContactEnd(fsnContact);
		body2->ContactEnd(fsnContact);
	}

	void PhysicsWorld::OnPreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
	}

	void PhysicsWorld::OnPostSolve(const b2Contact *contact, const b2ContactImpulse *impulse)
	{
		//BodyMap::iterator it_body1 = m_Bodies.find(contact->GetFixtureA()->GetBody());
		//BodyMap::iterator it_body2 = m_Bodies.find(contact->shape2->GetBody());
		//if (it_body1 != m_Bodies.end() && it_body2 != m_Bodies.end() && it_body1->second != NULL && it_body2->second != NULL)
		//{
		//	PhysicsBodyPtr body1 = it_body1->second;
		//	PhysicsBodyPtr body2 = it_body2->second;

		//	Contact fsnContact(contact, body1->GetShape(contact->shape1), body2->GetShape(contact->shape2));
		//	body1->ContactPersist(fsnContact);
		//	body2->ContactPersist(fsnContact);
		//}
	}

	//void PhysicsWorld::UpdateContacts()
	//{
	//	for (ContactList::iterator it = m_NewContacts.begin(), end = m_NewContacts.end(); it != end; ++it)
	//	{
	//		b2ContactPoint contact = (*it);

	//		BodyMap::iterator found_body1 = m_Bodies.find(contact.shape1->GetBody());
	//		BodyMap::iterator found_body2 = m_Bodies.find(contact.shape2->GetBody());
	//		if (find_body1 == m_Bodies.end() && find_body1->second != NULL)
	//		{
	//			PhysicsBody *body1 = found_body1->second;

	//			if (found_body2 != m_Bodies.end())
	//			{
	//				PhysicsBody *body2 = found_body2->second;
	//				body1->CollisionWith(body2, Contact(contact, body1->GetShape(contact.shape1), body2->GetShape(contact.shape2));
	//			}
	//		}


	//		if (found_body2 == m_Bodies.end() && found_body2->second != NULL)
	//		{
	//			PhysicsBody *body2 = found_body2->second;

	//			if (found_body1 != m_Bodies.end())
	//			{
	//				PhysicsBody *body1 = found_body2->second;
	//				body2->CollisionWith(body1, Contact(contact, body1->GetShape(contact.shape1), body2->GetShape(contact.shape2));
	//			}
	//		}
	//	}

	//	//for (ContactList::iterator it = m_ActiveContacts.begin(), end = m_ActiveContacts.end(); it != end; ++it)
	//	//{
	//	//}

	//	for (ContactList::iterator it = m_EndedContacts.begin(), end = m_EndedContacts.end(); it != end; ++it)
	//	{
	//		b2ContactPoint contact = it;

	//		BodyMap::iterator found_body = m_Bodies.find(contact.shape1->GetBody());
	//		if (found_body == m_Bodies.end() && found_body->second != NULL)
	//		{
	//				PhysicsBody *body = find_body;
	//				collider->RemoveContact(collider->GetShape(contact.shape1));
	//		}

	//		found_body = m_Bodies.find(contact.shape2->GetBody());
	//		if (found_body == m_Bodies.end() && found_body->second != NULL)
	//		{
	//				PhysicsBody *body = objects_list[contact.shape2->GetBody()];
	//				collider->RemoveContact(collider->GetShape(contact.shape2));
	//		}
	//	}

	//	ClearContacts();
	//}


	//void PhysicsWorld::ClearContacts()
	//{
	//	m_NewContacts.clear();
	//	m_ActiveContacts.clear();
	//	m_EndedContacts.clear();
	//}

	void PhysicsWorld::SetGCForDebugDraw(CL_GraphicContext gc)
	{
		m_DebugDraw->SetGraphicContext(gc);
		m_DebugDraw->SetFlags(
			b2DebugDraw::e_shapeBit |
			b2DebugDraw::e_jointBit |
			b2DebugDraw::e_coreShapeBit |
			b2DebugDraw::e_aabbBit |
			b2DebugDraw::e_centerOfMassBit |
			b2DebugDraw::e_pairBit);
	}

	//void PhysicsWorld::DebugDraw(bool fast)
	//{
	//	if (fast)
	//	{
	//		CL_Display::draw_pixel(0, 0, CL_Color::white);
	//		clBegin(GL_POLYGON);
	//		for (BodyMap::iterator it = m_Bodies.begin(), end = m_Bodies.end(); it != end; ++it)
	//		{
	//			drawBodyPoint(it->first);
	//		}
	//		clEnd();
	//	}
	//	else
	//	{
	//		for (BodyMap::iterator bodyIt = m_Bodies.begin(), bodyEnd = m_Bodies.end(); bodyIt != bodyEnd; ++bodyIt)
	//		{
	//			PhysicsBody *body = bodyIt->second;
	//			for (PhysicsBody::ShapeList::iterator shapeIt = body->GetShapes().begin(), shapeEnd = body->GetShapes().end();
	//				shapeIt != shapeEnd; ++shapeIt)
	//				drawObject(*shapeIt);
	//		}
	//	}
	//}


	void PhysicsWorld::ActivateWrapAround()
	{
		SendToConsole("World: Wrap around activated");
		m_Wrap = true;
	}

	void PhysicsWorld::DeactivateWrapAround()
	{
		SendToConsole("World: Wrap around deactivated");
		m_Wrap = false;
	}

	bool PhysicsWorld::UseWrapAround() const
	{
		return m_Wrap;
	}

	void PhysicsWorld::SetBodyDeactivationPeriod(unsigned int millis)
	{
	}

	unsigned int PhysicsWorld::GetBodyDeactivationPeriod() const
	{
		return 0;
	}

	void PhysicsWorld::SetDeactivationVelocity(float minvel)
	{
		m_DeactivationVelocity = minvel;
		m_DeactivationVelocitySquared = minvel * minvel;
	}

	float PhysicsWorld::GetDeactivationVelocity() const
	{
		return m_DeactivationVelocity;
	}


	void PhysicsWorld::SetMaxVelocity(float maxvel)
	{
		m_MaxVelocity = maxvel;
		m_MaxVelocitySquared = maxvel * maxvel;
	}

	float PhysicsWorld::GetMaxVelocity() const
	{
		return m_MaxVelocity;
	}

	void PhysicsWorld::SetDamping(float damping)
	{
	}

	float PhysicsWorld::GetDamping() const
	{
		return 0.0f;
	}

	void PhysicsWorld::SetGravity(const Vector2& grav_vector)
	{
		m_BxWorld->SetGravity(b2Vec2(grav_vector.x, grav_vector.y));
	}

	Vector2 PhysicsWorld::GetGravity() const
	{
		return b2v2(m_BxWorld->GetGravity());
	}
 

	void PhysicsWorld::SetBitmaskRes(int ppb)
	{
		m_BitmaskRes = ppb;
	}

	int PhysicsWorld::GetBitmaskRes() const
	{
		return m_BitmaskRes;
	}

	///////
	// ContactListener

	ContactListener::ContactListener(PhysicsWorld *world)
		: m_World(world)
	{
	}

	void ContactListener::BeginContact(b2Contact *contact)
	{
		m_World->OnBeginContact(contact);
	}

	void ContactListener::EndContact(b2Contact* contact)
	{
		m_World->OnEndContact(contact);
	}

	void ContactListener::PreSolve(b2Contact *contact, const b2Manifold *oldManifold)
	{
		m_World->OnPreSolve(contact, oldManifold);
	}

	void ContactListener::PostSolve(const b2Contact* contact, const b2ContactImpulse *impulse)
	{
		m_World->OnPostSolve(contact, impulse);
	}

	////////
	// ContactFilter
	ContactFilter::ContactFilter(PhysicsWorld *world)
		: m_World(world)
	{
	}

	bool ContactFilter::ShouldCollide(b2Fixture *shape1, b2Fixture *shape2)
	{
		return true;
		//m_World->OnShouldCollide(shape1, shape2);
	}

	bool ContactFilter::RayCollide(void *userData, b2Fixture *shape)
	{
		return true;
		//m_World->OnRayCollide(userData, shape);
	}

}