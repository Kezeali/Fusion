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
		m_Body(NULL)
	{}

	PhysicalEntity::PhysicalEntity(const std::string &name)
		: Entity(name),
		m_Body(NULL)
	{}

	PhysicalEntity::~PhysicalEntity()
	{
		if (m_Body != NULL)
			m_BodyDestroyer->Destroy(m_Body);
	}

	void PhysicalEntity::ApplyForce(const Vector2 &point, const Vector2 &force)
	{
		if (m_Body != NULL)
		{
			m_Body->ApplyForce(b2Vec2(point.x, point.y), b2Vec2(force.y, force.y));
		}
	}

	const Vector2 &PhysicalEntity::GetPosition()
	{
		if (m_Body != NULL)
		{
			m_Position.x = m_Body->GetPosition().x;
			m_Position.y = m_Body->GetPosition().y;
		}
		return m_Position;
	}

	const Vector2 &PhysicalEntity::GetVelocity()
	{
		if (m_Body != NULL)
		{
			m_Velocity.x = m_Body->GetLinearVelocity().x;
			m_Velocity.y = m_Body->GetLinearVelocity().y;
		}
		return m_Velocity;
	}

	float PhysicalEntity::GetAngle()
	{
		if (m_Body != NULL)
			return m_Body->GetAngle();
		else
			return m_Angle;
	}

	float PhysicalEntity::GetAngularVelocity()
	{
		if (m_Body != NULL)
			return m_Body->GetAngularVelocity();
		else
			return m_AngularVelocity;
	}

	void PhysicalEntity::SetPosition(const Vector2 &position)
	{
		if (m_Body != NULL)
			m_Body->SetPosition(b2Vec2(position.x, position.y));
		else
		{
			//m_BodyDef.position.Set(position.x, position.y);
			m_Position = position;
		}
	}

	void PhysicalEntity::SetVelocity(const Vector2 &velocity)
	{
		if (m_Body != NULL)
			m_Body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
		else
		{
			//m_BodyDef.linearVelocity.Set(velocity.x, velocity.y);
			m_Velocity = velocity;
		}
	}

	void PhysicalEntity::SetAngle(float angle)
	{
		if (m_Body != NULL)
			m_Body->SetAngle(angle);
		else
			//m_BodyDef.angle = angle;
			m_Angle = angle;
	}

	void PhysicalEntity::SetAngularVelocity(float angular_vel)
	{
		if (m_Body != NULL)
			m_Body->SetAngularVelocity(angular_vel);
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


	//const b2BodyDef &PhysicalEntity::GetBodyDef() const
	//{
	//	return m_BodyDef;
	//}

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
