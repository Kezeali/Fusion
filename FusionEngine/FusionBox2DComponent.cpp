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

	void Box2DBody::CleanMassData()
	{
		if (m_FixtureMassDirty)
			m_Body->ResetMassData();
		m_FixtureMassDirty = false;
	}

	void Box2DBody::OnSiblingAdded(const std::shared_ptr<IComponent>& com)
	{
		//if (auto fixtureCom = dynamic_cast<Box2DFixture*>(com.get()))
		//{
		//	fixtureCom->MassChanged = std::bind(&Box2DBody::OnFixtureMassChanged, this);
		//}
	}

	void Box2DBody::OnSiblingRemoved(const std::shared_ptr<IComponent>& com)
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
			m_Def.shape = GetShape();
			m_Fixture = body->Getb2Body()->CreateFixture(&m_Def);
			m_BodyDestructionConnection = body->Destruction.connect(std::bind(&Box2DFixture::OnBodyDestroyed, this));

			m_Body = body;
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

				m_Body = nullptr;
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

	Box2DCircleFixture::Box2DCircleFixture()
	{
		m_CircleShape.m_radius = 0.5f;
	}

	// Make sure this calls Box2DCircleFixture::DeserialiseOccasional
	Box2DCircleFixture::Box2DCircleFixture(RakNet::BitStream& stream)
		: Box2DFixture(stream)
	{
	}

	void Box2DCircleFixture::CopyChanges(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta)
	{
		Box2DFixture::DeltaSerialiser_t::copyChanges(result, current_data, delta);

		ShapeDeltaSerialiser_t::copyChanges(result, current_data, delta);
	}

	void Box2DCircleFixture::SynchroniseParallelEdits()
	{
		ICircleShape::SynchroniseInterface();
		Box2DFixture::SynchroniseParallelEdits();
	}

	void Box2DCircleFixture::FireSignals()
	{
		ICircleShape::FireInterfaceSignals();
		Box2DFixture::FireSignals();
	}

	bool Box2DCircleFixture::SerialiseContinuous(RakNet::BitStream& stream)
	{
		return false;
	}

	void Box2DCircleFixture::DeserialiseContinuous(RakNet::BitStream& stream)
	{
	}

	bool Box2DCircleFixture::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		bool changesWritten;
		changesWritten = Box2DFixture::SerialiseOccasional(stream, force_all);
		changesWritten |= m_CircleDeltaSerialisationHelper.writeChanges(force_all, stream , GetRadius(), GetPosition());
		return changesWritten;
	}

	void Box2DCircleFixture::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		Box2DFixture::DeserialiseOccasional(stream, all);

		//float radius;
		Vector2 position;
		
		FSN_ASSERT(m_Fixture->GetShape()->m_type == b2Shape::e_circle);
		auto circleShape = static_cast<b2CircleShape*>(m_Fixture->GetShape());

		std::bitset<ShapeDeltaSerialiser_t::NumParams> changes;
		m_CircleDeltaSerialisationHelper.readChanges(stream, all, changes, circleShape->m_radius, position);

		if (changes[ShapePropsIdx::Position])
			circleShape->m_p.Set(position.x, position.y);

		if (changes.any())
			m_Body->OnFixtureMassChanged();
	}

	void Box2DCircleFixture::SetRadius(float radius)
	{
		m_CircleShape.m_radius = radius;

		if (m_Fixture)
		{
			auto shape = m_Fixture->GetShape();
			shape->m_radius = radius;
		}

		m_CircleDeltaSerialisationHelper.markChanged(ShapePropsIdx::Radius);

		m_Body->OnFixtureMassChanged();
	}

	float Box2DCircleFixture::GetRadius() const
	{
		if (m_Fixture)
			return m_Fixture->GetShape()->m_radius;
		else
			return m_CircleShape.m_radius;
	}

	void Box2DCircleFixture::SetPosition(const Vector2& position)
	{
		FSN_ASSERT(m_Fixture->GetShape()->m_type == b2Shape::e_circle);
		auto shape = static_cast<b2CircleShape*>(m_Fixture->GetShape());
		shape->m_p.Set(position.x, position.y);

		m_CircleDeltaSerialisationHelper.markChanged(ShapePropsIdx::Position);

		m_Body->OnFixtureMassChanged();
	}

	Vector2 Box2DCircleFixture::GetPosition() const
	{
		FSN_ASSERT(m_Fixture->GetShape()->m_type == b2Shape::e_circle);
		auto shape = static_cast<b2CircleShape*>(m_Fixture->GetShape());
		return b2v2(shape->m_p);
	}

	Box2DPolygonFixture::Box2DPolygonFixture()
	{
	}

	Box2DPolygonFixture::Box2DPolygonFixture(RakNet::BitStream& stream)
		: Box2DFixture(stream)
	{
	}

	void Box2DPolygonFixture::CopyChanges(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta)
	{
		Box2DFixture::DeltaSerialiser_t::copyChanges(result, current_data, delta);

		if (delta.ReadBit())
		{
			int32 numVerts;
			delta.Read(numVerts);

			std::vector<unsigned char> verts(numVerts);
			if (delta.ReadBits(verts.data(), sizeof(float) * 2 * numVerts * 8))
				FSN_EXCEPT(Exception, "Failed to copy changes");

			result.Write(numVerts);
			result.WriteBits(verts.data(), sizeof(float) * 2 * numVerts * 8);
		}
	}

	void Box2DPolygonFixture::SynchroniseParallelEdits()
	{
		IPolygonShape::SynchroniseInterface();
		Box2DFixture::SynchroniseParallelEdits();
	}

	void Box2DPolygonFixture::FireSignals()
	{
		IPolygonShape::FireInterfaceSignals();
		Box2DFixture::FireSignals();
	}

	bool Box2DPolygonFixture::SerialiseContinuous(RakNet::BitStream& stream)
	{
		return false;
	}

	void Box2DPolygonFixture::DeserialiseContinuous(RakNet::BitStream& stream)
	{
	}

	bool Box2DPolygonFixture::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		bool changesWritten;

		changesWritten = Box2DFixture::SerialiseOccasional(stream, force_all);
		
		if (m_VerticiesChanged || force_all)
		{
			if (!force_all)
				stream.Write1();
			stream.Write(m_PolygonShape.GetVertexCount());
			for (int i = 0; i < m_PolygonShape.GetVertexCount(); ++i)
			{
				const b2Vec2& vert = m_PolygonShape.GetVertex(i);
				stream.Write(vert.x);
				stream.Write(vert.y);
			}

			changesWritten = true;
		}
		else if (!force_all)
			stream.Write0();

		return changesWritten;
	}

	void Box2DPolygonFixture::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		FSN_EXCEPT(NotImplementedException, "this aint done (need to update the actual fixture)");

		Box2DFixture::DeserialiseOccasional(stream, all);

		//float radius;
		Vector2 position;
		
		FSN_ASSERT(m_Fixture->GetShape()->m_type == b2Shape::e_polygon);
		auto circleShape = static_cast<b2PolygonShape*>(m_Fixture->GetShape());

		if (all || stream.ReadBit())
		{
			int32 vertexCount;
			stream.Read(vertexCount);
			std::vector<b2Vec2> verts;
			verts.resize(vertexCount);
			for (int i = 0; i < vertexCount; ++i)
			{
				auto& vert = verts[i];
				stream.Read(vert.x);
				stream.Read(vert.y);
			}
			m_PolygonShape.Set(verts.data(), verts.size());

			m_Body->OnFixtureMassChanged();
		}
	}

	float Box2DPolygonFixture::GetRadius() const
	{
		if (m_Fixture)
			return m_Fixture->GetShape()->m_radius;
		else
			return m_PolygonShape.m_radius;
	}

	void Box2DPolygonFixture::SetAsBoxImpl(float half_width, float half_height)
	{
		m_PolygonShape.SetAsBox(half_width, half_height);

		m_VerticiesChanged = true;
	}

	void Box2DPolygonFixture::SetAsBoxImpl(float half_width, float half_height, const Vector2& center, float angle)
	{
		m_PolygonShape.SetAsBox(half_width, half_height, b2Vec2(center.x, center.y), angle);

		m_VerticiesChanged = true;
	}

	void Box2DPolygonFixture::SetAsEdgeImpl(const Vector2 &v1, const Vector2 &v2)
	{
		std::array<b2Vec2, 2> verts = { b2Vec2(v1.x, v1.y), b2Vec2(v2.x, v2.y) };
		m_PolygonShape.Set(verts.data(), 2);

		m_VerticiesChanged = true;
	}

}