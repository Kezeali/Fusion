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
		m_Width(1), m_Height(1),
		m_DeactivationPeriod(100),
		m_DeactivationVelocity(0.001f),
		m_MaxVelocity(100.0f),
		m_RunningSimulation(false)
	{
		m_DeactivationVelocitySquared = m_DeactivationVelocity * m_DeactivationVelocity;
		m_MaxVelocitySquared = m_MaxVelocity * m_MaxVelocity;

		m_WorldAABB.lowerBound.Set(-10000.0f, -10000.0f);
		m_WorldAABB.upperBound.Set(10000.0f, 10000.0f);
		b2Vec2 gravity(0.0f, 30.0f);
		bool doSleep = true;

		m_BxWorld = new b2World(m_WorldAABB, gravity, doSleep);

		//cpInitChipmunk();
		//cpResetShapeIdCounter();
		//m_ChipSpace = cpSpaceNew();

		//m_CollisionGrid = new CollisionGrid();
	}

	PhysicsWorld::~PhysicsWorld()
	{
		Clear();
		//delete m_CollisionGrid;
	}

	b2Body* PhysicsWorld::SubstantiateBody(PhysicsBody *body)
	{
		b2Body* bxBody = m_BxWorld->CreateBody(body->GetBodyDef());
		m_Bodies[bxBody] = body;
	}

	void PhysicsWorld::RemoveBody(PhysicsBodyPtr &body)
	{
		m_Bodies.erase(body->GetB2Body());
	}
	
	void PhysicsWorld::BodyDeleted(PhysicsBody *body)
	{
		m_BxWorld->DestroyBody(body->GetB2Body());
	}

	const PhysicsWorld::BodyMap& PhysicsWorld::GetBodies() const
	{
		return m_Bodies;
	}

	void PhysicsWorld::Clear()
	{
		m_Bodies.clear();

		//{
		//	BodyMap::iterator it = m_Statics.begin();
		//	for (; it != m_Statics.end(); ++it)
		//	{
		//		delete (*it);
		//	}

		//	m_Statics.clear();
		//}
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
		for (BodyMap::iterator it = m_Bodies.begin(), end = m_DeleteQueue.end(); it != end; ++it)
		{
			PhysicsBody* body = it->second;

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

	void PhysicsWorld::RunSimulation(float delta_milis)
	{
		for (ForceList::iterator it = m_ForceQueue.begin(), end = m_ForceQueue.end(); it != end; ++it)
			it->ApplyForce();

		m_RunningSimulation = true; // bodies can't be deleted

		float32 dt = delta_milis * 0.001;
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

	void PhysicsWorld::OnContactAdd(const b2ContactPoint *contact)
	{
		BodyMap::iterator found_body1 = m_Bodies.find(contact.shape1->GetBody());
		BodyMap::iterator found_body2 = m_Bodies.find(contact.shape2->GetBody());
		if (find_body1 == m_Bodies.end() && find_body1->second != NULL)
		{
			PhysicsBody *body1 = found_body1->second;

			if (found_body2 != m_Bodies.end())
			{
				PhysicsBody *body2 = found_body2->second;
				body1->CollisionWith(body2, Contact(contact, body1->GetShape(contact.shape1), body2->GetShape(contact.shape2));
			}
		}


		if (found_body2 == m_Bodies.end() && found_body2->second != NULL)
		{
			PhysicsBody *body2 = found_body2->second;

			if (found_body1 != m_Bodies.end())
			{
				PhysicsBody *body1 = found_body2->second;
				body2->CollisionWith(body1, Contact(contact, body1->GetShape(contact.shape1), body2->GetShape(contact.shape2));
			}
		}
	}

	void PhysicsWorld::OnContactPersist(const b2ContactPoint *contact)
	{
	}

	void PhysicsWorld::OnContactRemove(const b2ContactPoint *contact)
	{
		BodyMap::iterator found_body = m_Bodies.find(contact.shape1->GetBody());
		if (found_body == m_Bodies.end() && found_body->second != NULL)
		{
			PhysicsBody *body = find_body;
			body->RemoveContact(body->GetShape(contact.shape1));
		}

		found_body = m_Bodies.find(contact.shape2->GetBody());
		if (found_body == m_Bodies.end() && found_body->second != NULL)
		{
			PhysicsBody *body = objects_list[contact.shape2->GetBody()];
			body->RemoveContact(body->GetShape(contact.shape2));
		}
	}

	void PhysicsWorld::UpdateContacts()
	{
		for (ContactList::iterator it = m_NewContacts.begin(), end = m_NewContacts.end(); it != end; ++i)
		{
			b2ContactPoint contact = (*it);

			BodyMap::iterator found_body1 = m_Bodies.find(contact.shape1->GetBody());
			BodyMap::iterator found_body2 = m_Bodies.find(contact.shape2->GetBody());
			if (find_body1 == m_Bodies.end() && find_body1->second != NULL)
			{
				PhysicsBody *body1 = found_body1->second;

				if (found_body2 != m_Bodies.end())
				{
					PhysicsBody *body2 = found_body2->second;
					body1->CollisionWith(body2, Contact(contact, body1->GetShape(contact.shape1), body2->GetShape(contact.shape2));
				}
			}


			if (found_body2 == m_Bodies.end() && found_body2->second != NULL)
			{
				PhysicsBody *body2 = found_body2->second;

				if (found_body1 != m_Bodies.end())
				{
					PhysicsBody *body1 = found_body2->second;
					body2->CollisionWith(body1, Contact(contact, body1->GetShape(contact.shape1), body2->GetShape(contact.shape2));
				}
			}
		}

		//for (ContactList::iterator it = m_ActiveContacts.begin(), end = m_ActiveContacts.end(); it != end; ++it)
		//{
		//}

		for (ContactList::iterator it = m_EndedContacts.begin(), end = m_EndedContacts.end(); it != end; ++it)
		{
			b2ContactPoint contact = it;

			BodyMap::iterator found_body = m_Bodies.find(contact.shape1->GetBody());
			if (found_body == m_Bodies.end() && found_body->second != NULL)
			{
					PhysicsBody *body = find_body;
					collider->RemoveContact(collider->GetShape(contact.shape1));
			}

			found_body = m_Bodies.find(contact.shape2->GetBody());
			if (found_body == m_Bodies.end() && found_body->second != NULL)
			{
					PhysicsBody *body = objects_list[contact.shape2->GetBody()];
					collider->RemoveContact(collider->GetShape(contact.shape2));
			}
		}

		ClearContacts();
	}


	void PhysicsWorld::ClearContacts()
	{
		m_NewContacts.clear();
		m_ActiveContacts.clear();
		m_EndedContacts.clear();
	}

	void PhysicsWorld::DebugDraw(bool fast)
	{
		if (fast)
		{
			CL_Display::draw_pixel(0, 0, CL_Color::white);
			clBegin(GL_POLYGON);
			for (BodyMap::iterator it = m_Bodies.begin(), end = m_Bodies.end(); it != end; ++it)
			{
				drawBodyPoint(it->first);
			}
			clEnd();
		}
		else
		{
			for (BodyMap::iterator bodyIt = m_Bodies.begin(), bodyEnd = m_Bodies.end(); bodyIt != bodyEnd; ++bodyIt)
			{
				PhysicsBody *body = bodyIt->second;
				for (PhysicsBody::ShapeList::iterator shapeIt = body->GetShapes().begin(), shapeEnd = body->GetShapes().end();
					shapeIt != shapeEnd; ++shapeIt)
					drawObject(*shapeIt);
			}
		}
	}


	void PhysicsWorld::Initialise(int level_x, int level_y)
	{
		Clear();

		cpResetShapeIdCounter();
		cpSpaceInit(m_ChipSpace);

		//m_ChipSpace->iterations = 5;
		cpSpaceResizeStaticHash(m_ChipSpace, 10.0, 4999);
		cpSpaceResizeActiveHash(m_ChipSpace, 32.0, 999);

		cpSpaceAddCollisionPairFunc(m_ChipSpace, g_PhysBodyCpCollisionType, 0, &bodyCollFunc, this);
		cpSpaceAddCollisionPairFunc(m_ChipSpace, g_PhysBodyCpCollisionType, g_PhysBodyCpCollisionType, &bodyCollFunc, this);

		//m_CollisionGrid->SetCellSize(g_PhysGridCellW, g_PhysGridCellH, level_x, level_y);

		m_Width = level_x;
		m_Height = level_y;
	}

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
		// Only update if necessary (as this could be time consuming with a lot of bodies)
		if (m_DeactivationPeriod != millis)
		{
			BodyList::iterator it = m_Bodies.begin();
			for (; it != m_Bodies.end(); ++it)
			{
				(*it)->SetDeactivationPeriod(millis);
			}

			m_DeactivationPeriod = millis;
		}
	}

	unsigned int PhysicsWorld::GetBodyDeactivationPeriod() const
	{
		return m_DeactivationPeriod;
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
		if (damping < 1.1f)
			m_ChipSpace->damping = damping;
	}

	float PhysicsWorld::GetDamping() const
	{
		return m_ChipSpace->damping;
	}

	void PhysicsWorld::SetGravity(const Vector2& grav_vector)
	{
		m_BxWorld->SetGravity(grav_vector.x, grav_vector.y);
	}

	Vector2 PhysicsWorld::GetGravity() const
	{
		return Vector2(m_ChipSpace->gravity.x, m_ChipSpace->gravity.y);
	}
 

	void PhysicsWorld::SetBitmaskRes(int ppb)
	{
		m_BitmaskRes = ppb;
	}

	int PhysicsWorld::GetBitmaskRes() const
	{
		return m_BitmaskRes;
	}

	ContactListener::ContactListener(const PhysicsWorld *world)
		: m_World(world)
	{
	}

	void ContactListener::Add(const b2ContactPoint *point)
	{
		m_World->OnContactAdd(point);
	}

	void ContactListener::Persist(const b2ContactPoint* point)
	{
		m_World->OnContactPersist(point);
	}

	void ContactListener::Remove(const b2ContactPoint* point)
	{
		m_World->OnContactRemove(point);
	}

}