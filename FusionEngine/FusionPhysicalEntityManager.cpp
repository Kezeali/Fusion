#include "FusionCommon.h"

#include "FusionPhysicalEntityManager.h"

namespace FusionEngine
{

	PhysicalWorld::PhysicalWorld()
		: m_VelocityIterations(s_PhysicsVelocityIterations),
		m_PositionIterations(s_PhysicsPositionIterations)
	{
		initialise();
	}

	PhysicalWorld::~PhysicalWorld()
	{
		//for (ConnectionList::iterator it = m_Connections.begin(), end = m_Connections.end(); it != end; ++it)
		//	it->disconnect();
		//m_Connections.clear();

		m_BodyDestroyer->ClearCallback();

		delete m_DebugDraw;
		delete m_ContactListener;
		delete m_World;
	}

	void PhysicalWorld::SetGraphicContext(const CL_GraphicContext &context)
	{
		m_DebugDraw->SetGraphicContext(context);
	}

	void PhysicalWorld::SetDebugDrawViewport(const ViewportPtr &viewport)
	{
		m_DebugDraw->SetViewport(viewport);
	}

	void PhysicalWorld::EnableDebugDraw(bool enable)
	{
		if (enable)
		{
			m_World->SetDebugDraw(m_DebugDraw);

			m_DebugDraw->SetFlags(
				b2DebugDraw::e_shapeBit |
				b2DebugDraw::e_jointBit |
				b2DebugDraw::e_aabbBit |
				b2DebugDraw::e_centerOfMassBit |
				b2DebugDraw::e_pairBit);
		}
		else
			m_World->SetDebugDraw(NULL);
	}

	b2World *PhysicalWorld::GetB2World() const
	{
		return m_World;
	}

	void PhysicalWorld::Step(float dt)
	{
		m_DebugDraw->SetupView();
		if (dt > 0.0f)
			m_World->Step(dt, m_VelocityIterations, m_PositionIterations);
		else
			m_World->Step(dt, 0, 0);
		m_DebugDraw->ResetView();
	}

	void PhysicalWorld::OnEntityDestruction(b2Body *body)
	{
		m_World->DestroyBody(body);
	}

	void PhysicalWorld::PrepareEntity(PhysicalEntity *entity)
	{
		entity->SetBodyDestroyer(m_BodyDestroyer);
	}

	//void PhysicalWorld::AddEntity(const PhysicalEntityPtr &entity)
	//{
	//	//m_Physicals.insert(entity);
	//}

	//void PhysicalWorld::RemoveEntity(const PhysicalEntityPtr &entity)
	//{
	//	//m_Physicals.erase(entity);
	//}

	//void PhysicalWorld::Clear()
	//{
	//	//m_Physicals.clear();
	//}

	void PhysicalWorld::OnBeginContact(b2Contact *contact)
	{
		// Each fixture must be fixed to a body with valid userdata (since this is how we get the
		//  pointer to the Entity)
		if (contact->GetFixtureA()->GetBody()->GetUserData() == NULL || contact->GetFixtureB()->GetBody()->GetUserData() == NULL)
			return;

		PhysicalEntity *entity1 = static_cast<PhysicalEntity*>( contact->GetFixtureA()->GetBody()->GetUserData() );
		PhysicalEntity *entity2 = static_cast<PhysicalEntity*>( contact->GetFixtureB()->GetBody()->GetUserData() );

		Contact fsnContact = Contact::CreateContact(contact);
		entity1->ContactBegin(fsnContact);
		entity2->ContactBegin(fsnContact);
	}

	void PhysicalWorld::OnEndContact(b2Contact *contact)
	{
		if (contact->GetFixtureA()->GetBody()->GetUserData() == NULL || contact->GetFixtureB()->GetBody()->GetUserData() == NULL)
			return;

		PhysicalEntity *entity1 = static_cast<PhysicalEntity*>( contact->GetFixtureA()->GetBody()->GetUserData() );
		PhysicalEntity *entity2 = static_cast<PhysicalEntity*>( contact->GetFixtureB()->GetBody()->GetUserData() );

		Contact fsnContact = Contact::CreateContact(contact);
		entity1->ContactEnd(fsnContact);
		entity2->ContactEnd(fsnContact);
	}

	void PhysicalWorld::OnPreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
	}

	void PhysicalWorld::OnPostSolve(const b2Contact *contact, const b2ContactImpulse *impulse)
	{
	}

	void PhysicalWorld::initialise()
	{
		b2Vec2 gravity(0.0f, 0.0f);

		m_World = new b2World(gravity, true);

		m_ContactListener = new ContactListener();
		m_World->SetContactListener(m_ContactListener);

		m_DebugDraw = new DebugDraw();
		m_World->SetDebugDraw(m_DebugDraw);

		m_BodyDestroyer.reset( new BodyDestroyer(boost::bind(&PhysicalWorld::OnEntityDestruction, this, _1)) );
	}


	///////////////////
	// ContactListener
	ContactListener::ContactListener()
	{
	}

	void ContactListener::BeginContact(b2Contact *contact)
	{
		// Each fixture must be fixed to a body with valid userdata (since this is how we get the
		//  pointer to the Entity)
		if (contact->GetFixtureA()->GetBody()->GetUserData() == NULL || contact->GetFixtureB()->GetBody()->GetUserData() == NULL)
			return;

		PhysicalEntity *entity1 = static_cast<PhysicalEntity*>( contact->GetFixtureA()->GetBody()->GetUserData() );
		PhysicalEntity *entity2 = static_cast<PhysicalEntity*>( contact->GetFixtureB()->GetBody()->GetUserData() );

		Contact fsnContact = Contact::CreateContact(contact);
		entity1->ContactBegin(fsnContact);
		entity2->ContactBegin(fsnContact);
	}

	void ContactListener::EndContact(b2Contact* contact)
	{
		if (contact->GetFixtureA()->GetBody()->GetUserData() == NULL || contact->GetFixtureB()->GetBody()->GetUserData() == NULL)
			return;

		PhysicalEntity *entity1 = static_cast<PhysicalEntity*>( contact->GetFixtureA()->GetBody()->GetUserData() );
		PhysicalEntity *entity2 = static_cast<PhysicalEntity*>( contact->GetFixtureB()->GetBody()->GetUserData() );

		Contact fsnContact = Contact::CreateContact(contact);
		entity1->ContactEnd(fsnContact);
		entity2->ContactEnd(fsnContact);
	}

	void ContactListener::PreSolve(b2Contact *contact, const b2Manifold *oldManifold)
	{
	}

	void ContactListener::PostSolve(const b2Contact* contact, const b2ContactImpulse *impulse)
	{
	}

	////////////////
	// ContactFilter
	ContactFilter::ContactFilter(PhysicalWorld *mgr)
		: m_Manager(mgr)
	{
	}

	bool ContactFilter::ShouldCollide(b2Fixture *shape1, b2Fixture *shape2)
	{
		return true;
		//m_Manager->OnShouldCollide(shape1, shape2);
	}

	bool ContactFilter::RayCollide(void *userData, b2Fixture *shape)
	{
		return true;
		//m_Manager->OnRayCollide(userData, shape);
	}

}
