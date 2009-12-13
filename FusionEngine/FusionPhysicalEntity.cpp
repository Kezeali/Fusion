#include "FusionCommon.h"

// Class
#include "FusionPhysicalEntity.h"


namespace FusionEngine
{

	BodyDestroyer::BodyDestroyer()
	{
	}

	BodyDestroyer::BodyDestroyer(const BodyDestroyer::CallbackFn &fn)
		: m_Callback(fn)
	{}

	void BodyDestroyer::Destroy(b2Body *body) const
	{
		if (m_Callback)
			m_Callback(body);
	}

	void BodyDestroyer::SetCallback(const BodyDestroyer::CallbackFn &fn)
	{
		m_Callback = fn;
	}

	void BodyDestroyer::ClearCallback()
	{
		m_Callback = BodyDestroyer::CallbackFn();
	}

	PhysicalEntity::PhysicalEntity()
		: Entity(),
		m_Body(NULL),
		m_Angle(0.0f),
		m_AngularVelocity(0.0f)
	{}

	PhysicalEntity::PhysicalEntity(const std::string &name)
		: Entity(name),
		m_Body(NULL),
		m_Angle(0.0f),
		m_AngularVelocity(0.0f)
	{}

	PhysicalEntity::~PhysicalEntity()
	{
		if (m_Body != NULL)
			m_BodyDestroyer->Destroy(m_Body);
	}

	void PhysicalEntity::ApplyForce(const Vector2 &force, const Vector2 &point)
	{
		if (m_Body != NULL)
		{
			m_Body->ApplyForce(
				b2Vec2(force.y * s_SimUnitsPerGameUnit, force.y * s_SimUnitsPerGameUnit),
				b2Vec2(point.x * s_SimUnitsPerGameUnit, point.y * s_SimUnitsPerGameUnit)
				);
		}
	}

	void PhysicalEntity::ApplyTorque(float torque)
	{
		if (m_Body != NULL)
		{
			m_Body->ApplyTorque(torque);
		}
	}

	const Vector2 &PhysicalEntity::GetPosition()
	{
		if (m_Body != NULL)
		{
			m_Position.x = m_Body->GetPosition().x * s_GameUnitsPerSimUnit;
			m_Position.y = m_Body->GetPosition().y * s_GameUnitsPerSimUnit;
		}
		return m_Position;
	}

	const Vector2 &PhysicalEntity::GetVelocity()
	{
		if (m_Body != NULL)
		{
			m_Velocity.x = m_Body->GetLinearVelocity().x * s_GameUnitsPerSimUnit;
			m_Velocity.y = m_Body->GetLinearVelocity().y * s_GameUnitsPerSimUnit;
		}
		return m_Velocity;
	}

	float PhysicalEntity::GetAngle() const
	{
		if (m_Body != NULL)
			return m_Body->GetAngle();
		else
			return m_Angle;
	}

	float PhysicalEntity::GetAngularVelocity() const
	{
		if (m_Body != NULL)
			return m_Body->GetAngularVelocity();
		else
			return m_AngularVelocity;
	}

	void PhysicalEntity::SetPosition(const Vector2 &position)
	{
		if (m_Body != NULL)
		{
			m_Body->SetTransform(
				b2Vec2(position.x * s_SimUnitsPerGameUnit, position.y * s_SimUnitsPerGameUnit),
				m_Angle);
		}
		else
		{
			//m_BodyDef.position.Set(position.x, position.y);
			m_Position = position;
		}
	}

	void PhysicalEntity::SetVelocity(const Vector2 &velocity)
	{
		if (m_Body != NULL)
		{
			m_Body->WakeUp();
			m_Body->SetLinearVelocity(b2Vec2(velocity.x * s_SimUnitsPerGameUnit, velocity.y * s_SimUnitsPerGameUnit));
		}
		else
		{
			//m_BodyDef.linearVelocity.Set(velocity.x, velocity.y);
			m_Velocity = velocity;
		}
	}

	void PhysicalEntity::SetAngle(float angle)
	{
		if (m_Body != NULL)
		{
			m_Body->SetTransform(
				b2Vec2(m_Position.x * s_SimUnitsPerGameUnit, m_Position.y * s_SimUnitsPerGameUnit),
				angle);
		}
		else
			//m_BodyDef.angle = angle;
			m_Angle = angle;
	}

	void PhysicalEntity::SetAngularVelocity(float angular_vel)
	{
		if (m_Body != NULL)
		{
			m_Body->WakeUp();
			m_Body->SetAngularVelocity(angular_vel);
		}
		else
			//m_BodyDef.angularVelocity = angular_vel;
			m_AngularVelocity = angular_vel;
	}

	bool PhysicalEntity::IsPhysicsEnabled() const
	{
		return m_Body != NULL;
	}

	b2Body *PhysicalEntity::GetBody() const
	{
		return m_Body;
	}

	void PhysicalEntity::_setBody(b2Body *body)
	{
		m_Body = body;
	}

	const FixturePtr &PhysicalEntity::CreateFixture(const b2FixtureDef *fixture_definition, const FixtureUserDataPtr &user_data)
	{
		b2Fixture *inner = m_Body->CreateFixture(fixture_definition);
		FixturePtr fixture(new Fixture(inner));
		m_Fixtures.push_back(fixture);

		if (user_data)
			fixture->SetUserData(user_data);

		return fixture;
	}

	void PhysicalEntity::DestroyFixture(b2Fixture *inner)
	{
		DestroyFixture( Fixture::GetWrapper(inner) );
	}

	void PhysicalEntity::DestroyFixture(const FixturePtr &fixture)
	{
		for (FixtureArray::iterator it = m_Fixtures.begin(), end = m_Fixtures.end(); it != end; ++it)
		{
			if (*it == fixture)
			{
				m_Fixtures.erase(it);
				break;
			}
		}
		m_Body->DestroyFixture(fixture->GetInner());
		fixture->Invalidate();
	}


	void PhysicalEntity::SetBodyDestroyer(const BodyDestroyerPtr &destroyer)
	{
		m_BodyDestroyer = destroyer;
	}

	//PhysicalEntity::DestructorSignal &PhysicalEntity::GetDestructorSignal()
	//{
	//	return m_DestructorSignal;
	//}

	//boost::signals2::connection &PhysicalEntity::ConnectToDestructor(const PhysicalEntity::DestructorSignal::slot_type &slot)
	//{
	//	return m_DestructorSignal.connect(slot);
	//}

	void PhysicalEntity::SerialiseState(SerialisedData &state, bool local) const
	{
		std::ostringstream stateStream(std::ios::binary);

		stateStream << (m_Body != NULL); // Physics enabled

		// TODO: if bodyDef dirty send whole def (plus bool-true here to indicate that this is a full-state packet)

		if (m_Body != NULL)
		{
			const b2Vec2 &pos = m_Body->GetPosition();
			stateStream << pos.x;
			stateStream << pos.y;
			const b2Vec2 vel = m_Body->GetLinearVelocity();
			stateStream << vel.x;
			stateStream << vel.y;
			stateStream << m_Body->GetAngle();
			stateStream << m_Body->GetAngularVelocity();
		}
		else
		{
			stateStream << m_Position.x;
			stateStream << m_Position.y;

			stateStream << m_Velocity.x;
			stateStream << m_Velocity.y;

			stateStream << m_Angle;
			stateStream << m_AngularVelocity;
		}
	}

	size_t PhysicalEntity::DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser)
	{
		std::istringstream stateStream(state.data, std::ios::binary);

		bool physicsEnabled;
		stateStream >> physicsEnabled; // True if the entity has been added to the physics world on the owning peer

		float x, y;
		stateStream >> x;
		stateStream >> y;
		SetPosition(Vector2(x, y));
		
		stateStream >> x;
		stateStream >> y;
		SetVelocity(Vector2(x, y));

		stateStream >> x; SetAngle(x);
		stateStream >> x; SetAngularVelocity(x);

		// TODO: deserialise joints using entity_deserialiser?

		return stateStream.tellg();
	}

}
