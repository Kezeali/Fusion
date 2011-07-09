/*
*  Copyright (c) 2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "FusionStableHeaders.h"

#include "FusionBox2DComponent.h"

namespace FusionEngine
{

	Box2DBody::Box2DBody(b2Body* body)
		: m_Body(body)
	{
	}

	Box2DBody::~Box2DBody()
	{
		Destruction();
	}

	void Box2DBody::OnSiblingAdded(const std::shared_ptr<IComponent>& com)
	{
	}

	void Box2DBody::SynchroniseParallelEdits()
	{
		IRigidBody::SynchroniseInterface();
		//SynchTransform();

		//Vector2 velocity;
		//if (m_Velocity.ClearWrittenValue(velocity))
		//	m_Body->SetLinearVelocity(velocity);
		//else
		//	m_Velocity.SetReadValue(ToRender(m_Body->GetLinearVelocity()));

		//if (m_AngularVelocityWritten)
		//{
		//	m_Body->SetAngularVelocity(m_AngularVelocity);
		//	m_AngularVelocityWritten = false;
		//}
		//else
		//	m_AngularVelocity = m_Body->GetAngularVelocity();

		//if (m_LinearDampingWritten)
		//{
		//	m_Body->SetLinearDamping(ToSim(m_LinearDamping));
		//	m_LinearDampingWritten = false;
		//}

		//if (m_AngularDampingWritten)
		//{
		//	m_Body->SetAngularDamping(m_AngularDamping);
		//	m_AngularDampingWritten = false;
		//}

		//if (m_GravityScaleWritten)
		//{
		//	m_Body->SetGravityScale(m_GravityScale);
		//	m_GravityScaleWritten = false;
		//}

		//// Copy flags that have been set on the threadsafe wrapper
		//if (m_Active != m_Body->IsActive())
		//	m_Body->SetActive(m_Active);

		//if (m_SleepingAllowed != m_Body->IsSleepingAllowed())
		//	m_Body->SetSleepingAllowed(m_SleepingAllowed);

		//if (m_Bullet != m_Body->IsBullet())
		//	m_Body->SetBullet(m_Bullet);

		//if (m_FixedRotation != m_Body->IsFixedRotation())
		//	m_Body->SetFixedRotation(m_FixedRotation);

		//// Copy the value of IsAwake
		//m_Awake = m_Body->IsAwake();
	}

	void Box2DBody::FireSignals()
	{
		IRigidBody::FireInterfaceSignals();
	}

	bool Box2DBody::SerialiseContinuous(RakNet::BitStream& stream)
	{
		const Vector2& pos = GetPosition();
		stream.Write(pos.x);
		stream.Write(pos.y);
		stream.Write(GetAngle());

		const Vector2& vel = GetVelocity();
		stream.Write(vel.x);
		stream.Write(vel.y);
		stream.Write(GetAngularVelocity());

		return true;
	}

	void Box2DBody::DeserialiseContinuous(RakNet::BitStream& stream)
	{
		Vector2 position;
		float angle;
		stream.Read(position.x);
		stream.Read(position.y);
		stream.Read(angle);

		SetPosition(position);
		SetAngle(angle);

		Vector2 linearVelocity;
		float angularVelocity;
		stream.Read(linearVelocity.x);
		stream.Read(linearVelocity.x);
		stream.Read(angularVelocity);

		SetVelocity(linearVelocity);
		SetAngularVelocity(angularVelocity);
	}

	bool Box2DBody::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		return m_DeltaSerialisationHelper.writeChanges(force_all, stream,
			IsActive(), IsSleepingAllowed(), IsAwake(), IsBullet(), IsFixedRotation(),
			GetLinearDamping(), GetAngularDamping(), GetGravityScale());
	}

	void Box2DBody::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		std::bitset<DeltaSerialiser_t::NumParams> changes;
		bool active, sleepingAllowed, awake, bullet, fixedrotation;
		float linearDamping, angularDamping, gravityScale;
		m_DeltaSerialisationHelper.readChanges(stream, all, changes,
			active, sleepingAllowed, awake, bullet,
			fixedrotation, linearDamping, angularDamping,
			gravityScale);

		if (changes[PropsIdx::Active])
			m_Body->SetActive(active);
		if (changes[PropsIdx::SleepingAllowed])
			m_Body->SetSleepingAllowed(sleepingAllowed);
		if (changes[PropsIdx::Awake])
			m_Body->SetAwake(awake);
		if (changes[PropsIdx::Bullet])
			m_Body->SetBullet(bullet);
		if (changes[PropsIdx::FixedRotation])
			m_Body->SetFixedRotation(fixedrotation);

		if (changes[PropsIdx::LinearDamping])
			m_Body->SetLinearDamping(linearDamping);
		if (changes[PropsIdx::AngularDamping])
			m_Body->SetAngularDamping(angularDamping);
		if (changes[PropsIdx::GravityScale])
			m_Body->SetGravityScale(gravityScale);
	}

	Box2DFixture::Box2DFixture()
		: m_Fixture(nullptr)
	{
	}

	Box2DFixture::Box2DFixture(RakNet::BitStream& stream)
		: m_Fixture(nullptr)
	{
		DeserialiseOccasional(stream, true);
	}

	Box2DFixture::Box2DFixture(const b2FixtureDef& def)
		: m_Def(def),
		m_Fixture(nullptr)
	{
	}

	Box2DFixture::Box2DFixture(b2Fixture* fixture)
		: m_Fixture(fixture)
	{
	}

	Box2DFixture::~Box2DFixture()
	{
		if (m_Fixture)
			m_Fixture->GetBody()->DestroyFixture(m_Fixture);
	}

	void Box2DFixture::OnBodyDestroyed()
	{
		m_Fixture = nullptr;
	}

	void Box2DFixture::OnSiblingAdded(const std::shared_ptr<IComponent>& com)
	{
		auto body = dynamic_cast<Box2DBody*>(com.get());
		if (body)
		{
			if (m_Fixture)
			{
				m_Fixture->GetBody()->DestroyFixture(m_Fixture);
				m_Fixture = nullptr;
				m_BodyDestructionConnection.disconnect();
			}
			b2CircleShape shape;
			shape.m_radius = 50.f;
			m_Def.shape = &shape;
			m_Fixture = body->Getb2Body()->CreateFixture(&m_Def);
			m_BodyDestructionConnection = body->Destruction.connect(std::bind(&Box2DFixture::OnBodyDestroyed, this));
		}
	}

	void Box2DFixture::OnSiblingRemoved(const std::shared_ptr<IComponent>& com)
	{
		if (m_Fixture)
		{
			auto body = dynamic_cast<Box2DBody*>(com.get());
			if (body)
			{
				m_Fixture->GetBody()->DestroyFixture(m_Fixture);
				m_Fixture = nullptr;
				m_BodyDestructionConnection.disconnect();
			}
		}
	}

	void Box2DFixture::SynchroniseParallelEdits()
	{
		IPhysFixture::SynchroniseInterface();
	}

	void Box2DFixture::FireSignals()
	{
		IPhysFixture::FireInterfaceSignals();
	}

	bool Box2DFixture::SerialiseContinuous(RakNet::BitStream& stream)
	{
		return false;
	}

	void Box2DFixture::DeserialiseContinuous(RakNet::BitStream& stream)
	{
	}

	bool Box2DFixture::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		return m_DeltaSerialisationHelper.writeChanges(force_all, stream , IsSensor(), GetDensity(), GetFriction(), GetRestitution());
	}

	void Box2DFixture::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		std::bitset<DeltaSerialiser_t::NumParams> changes;
		m_DeltaSerialisationHelper.readChanges(stream, all, changes, m_Def.isSensor, m_Def.density, m_Def.friction, m_Def.restitution);

		if (m_Fixture)
		{
			// This commented-out line works and is pretty cool, but I think it could be improved
			//m_DeltaSerialisationHelper.readChanges(stream, all, changes, m_Fixture, &b2Fixture::SetSensor, &b2Fixture::SetDensity, &b2Fixture::SetFriction, &b2Fixture::SetRestitution);

			if (changes[PropsIdx::Sensor])
				m_Fixture->SetSensor(m_Def.isSensor);
			if (changes[PropsIdx::Density])
				m_Fixture->SetDensity(m_Def.density);
			if (changes[PropsIdx::Friction])
				m_Fixture->SetFriction(m_Def.friction);
			if (changes[PropsIdx::Restitution])
				m_Fixture->SetRestitution(m_Def.restitution);
		}
	}

	bool Box2DFixture::IsSensor() const
	{
		return m_Fixture ? m_Fixture->IsSensor() : m_Def.isSensor;
	}

	void Box2DFixture::SetSensor(bool sensor)
	{
		if (m_Fixture)
			m_Fixture->SetSensor(sensor);
		else
			m_Def.isSensor = sensor;
		m_DeltaSerialisationHelper.markChanged(PropsIdx::Sensor);
	}

	float Box2DFixture::GetDensity() const
	{
		return m_Fixture ? m_Fixture->GetDensity() : m_Def.density;
	}

	void Box2DFixture::SetDensity(float density)
	{
		if (m_Fixture)
			m_Fixture->SetDensity(density);
		else
			m_Def.density = density;
		m_DeltaSerialisationHelper.markChanged(PropsIdx::Density);
	}

	float Box2DFixture::GetFriction() const
	{
		return m_Fixture ? m_Fixture->GetFriction() : m_Def.friction;
	}

	void Box2DFixture::SetFriction(float friction)
	{
		if (m_Fixture)
			m_Fixture->SetFriction(friction);
		else
			m_Def.friction = friction;
		m_DeltaSerialisationHelper.markChanged(PropsIdx::Friction);
	}

	float Box2DFixture::GetRestitution() const
	{
		return m_Fixture ? m_Fixture->GetRestitution() : m_Def.restitution;
	}

	void Box2DFixture::SetRestitution(float restitution)
	{
		if (m_Fixture)
			m_Fixture->SetRestitution(restitution);
		else
			m_Def.restitution = restitution;
		m_DeltaSerialisationHelper.markChanged(PropsIdx::Restitution);
	}

}