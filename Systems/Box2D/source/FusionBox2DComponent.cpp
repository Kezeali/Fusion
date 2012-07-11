/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionBox2DComponent.h"

#include "FusionMaths.h"

#include "FusionResourceManager.h"

// TEMP: auth test
#include "FusionEntity.h"
#include "FusionRender2DComponent.h"

namespace FusionEngine
{

	Box2DBody::Box2DBody(b2BodyDef def)
		: m_Def(def),
		m_Body(nullptr),
		m_Depth(0),
		m_Interpolate(false),
		m_InteractingWithPlayer(false)
	{
		m_InterpPosition.set(def.position.x, def.position.y);
		m_InterpAngle = def.angle;
		m_LastAngularVelocity = def.angularVelocity;
	}

	Box2DBody::~Box2DBody()
	{
		// If the owning Box2DWorld has been destroyed, m_Body is invalid (no need to destruct it)
		if (auto owner = Owner.lock())
		{
			if (m_Body)
				DestructBody(m_Body->GetWorld());
		}
	}

	void Box2DBody::ConstructBody(b2World* world, const std::weak_ptr<Box2DWorld>& owner)
	{
		FSN_ASSERT(world);
		FSN_ASSERT(m_Body == nullptr);

		Owner = owner;

		m_Def.userData = this;
		m_Body = world->CreateBody(&m_Def);

		const auto& tf = m_Body->GetTransform();
		m_InterpPosition = m_LastPosition = b2v2(tf.p);
		m_InterpAngle = m_LastAngle = tf.q.GetAngle();

		m_LastAngularVelocity = m_Body->GetAngularVelocity();

		for (auto it = m_Fixtures.begin(), end = m_Fixtures.end(); it != end; ++it)
		{
			auto& fixtureCom = *it;
			fixtureCom->ConstructFixture(this);
		}
	}

	void Box2DBody::DestructBody(b2World* world)
	{
		FSN_ASSERT(world);
		if (m_Body)
		{
			const auto& tf = m_Body->GetTransform();
			m_InterpPosition = m_LastPosition = b2v2(tf.p);
			m_InterpAngle = m_LastAngle = tf.q.GetAngle();

			m_LastAngularVelocity = m_Body->GetAngularVelocity();

			m_Def.active = m_Body->IsActive();
			m_Def.allowSleep = m_Body->IsSleepingAllowed();
			m_Def.angle = m_Body->GetAngle();
			m_Def.angularDamping = m_Body->GetAngularDamping();
			m_Def.angularVelocity = m_Body->GetAngularVelocity();
			m_Def.awake = m_Body->IsAwake();
			m_Def.bullet = m_Body->IsBullet();
			m_Def.fixedRotation = m_Body->IsFixedRotation();
			m_Def.gravityScale = m_Body->GetGravityScale();
			m_Def.linearDamping = m_Body->GetLinearDamping();
			m_Def.position = tf.p;
			m_Def.type = m_Body->GetType();

			for (auto it = m_Fixtures.begin(), end = m_Fixtures.end(); it != end; ++it)
			{
				auto& fixtureCom = *it;
				fixtureCom->m_Fixture = nullptr;
			}

			world->DestroyBody(m_Body);
			m_Body = nullptr;
		}
	}

	void Box2DBody::CleanMassData()
	{
		if (m_FixtureMassDirty)
			m_Body->ResetMassData();
		m_FixtureMassDirty = false;
	}

	bool Box2DBody::AddInteraction(b2Body* other)
	{
		return m_Interacting.insert(other).second;
	}

	void Box2DBody::ClearInteractions()
	{
		//for (auto it = m_Interacting.cbegin(), end = m_Interacting.cend(); it != end; ++it)
		//{
		//	auto owner = GetParent()->GetOwnerID();
		//	auto otherComponent = (Box2DBody*)(*it)->GetUserData();
		//	auto otherAuth = otherComponent->GetParent()->GetAuthority();
		//	if (owner != 0 && otherAuth == owner)
		//	{
		//		otherComponent->GetParent()->SetAuthority(0);

		//		if (auto sprite = otherComponent->GetParent()->GetComponent<ISprite>())
		//			sprite->Colour.Set(CL_Colorf(1.f, 1.f, 1.f, 1.f));
		//	}
		//}
		m_Interacting.clear();
	}

	void Box2DBody::OnSiblingAdded(const ComponentPtr& com)
	{
		if (auto fixtureCom = boost::dynamic_pointer_cast<Box2DFixture>(com))
		{
			if (m_Fixtures.insert(fixtureCom).second)
				if (m_Body)
					fixtureCom->ConstructFixture(this);
		}
	}

	void Box2DBody::OnSiblingRemoved(const ComponentPtr& com)
	{
		if (auto fixtureCom = boost::dynamic_pointer_cast<Box2DFixture>(com))
		{
			m_Fixtures.erase(fixtureCom);
		}
	}

	// 120 is assumed to be the max sim framerate (TODO: actually restrict this)
	static const float s_MaxVelocity = b2_maxTranslation * 120.f;
	static const float s_MinVelocity = -s_MaxVelocity;
	static const float s_MaxRotationalVelocity = b2_maxRotation * 120.f;
	static const float s_MinRotationalVelocity = -s_MaxRotationalVelocity;

	void Box2DBody::CompressState()
	{
#ifdef FSN_PHYS_COMPRESS_STATE
#endif
	}

	Vector2 Box2DBody::DeserialisePosition(RakNet::BitStream& stream, const Vector2& origin, float radius)
	{
		if (stream.ReadBit())
		{
			Vector2 position;
			stream.ReadFloat16(position.x, -radius, radius);
			stream.ReadFloat16(position.y, -radius, radius);
			return origin + position;
		}
		else
		{
			Vector2 position;
			stream.Read(position.x);
			stream.Read(position.y);
			return position;
		}
	}

	void Box2DBody::SerialiseTransform(RakNet::BitStream& stream, const Vector2& origin, float radius)
	{
		auto pos = m_Body ? m_Body->GetPosition() : m_Def.position;
		auto offset = pos - b2Vec2(origin.x, origin.y);
		if (offset.Length() < radius - 0.01f)
		{
			stream.Write1();
			stream.WriteFloat16(offset.x, -radius, radius);
			stream.WriteFloat16(offset.y, -radius, radius);
		}
		else
		{
			stream.Write0();
			stream.Write(pos.x);
			stream.Write(pos.y);
		}
	}

	void Box2DBody::DeserialiseTransform(RakNet::BitStream& stream, const Vector2& position)
	{
		SetPosition(position);
	}

	void Box2DBody::SerialiseContinuous(RakNet::BitStream& stream)
	{
		if (GetBodyType() == Dynamic)
		{
			//auto position = m_Body ? m_Body->GetPosition() : m_Def.position;
			//stream.Write(position.x);
			//stream.Write(position.y);
			stream.Write(m_Body ? m_Body->GetAngle() : m_Def.angle);

			const bool awake = IsAwake();
			stream.Write(awake);

			if (awake)
			{
				Vector2 vel = GetVelocity();
#ifdef FSN_PHYS_COMPRESS_STATE
				Maths::ClampThis(vel.x, s_MinVelocity, s_MaxVelocity);
				Maths::ClampThis(vel.y, s_MinVelocity, s_MaxVelocity);

				stream.WriteFloat16(vel.x, s_MinVelocity, s_MaxVelocity);
				stream.WriteFloat16(vel.y, s_MinVelocity, s_MaxVelocity);
				stream.WriteFloat16(Maths::Clamp(GetAngularVelocity(), s_MinRotationalVelocity, s_MaxRotationalVelocity), s_MinRotationalVelocity, s_MaxRotationalVelocity);
#else
				stream.Write(vel.x);
				stream.Write(vel.y);
				stream.Write(GetAngularVelocity());
#endif
			}
		}
	}

	void Box2DBody::DeserialiseContinuous(RakNet::BitStream& stream)
	{
		Vector2 position;
		float angle;
		//stream.Read(position.x);
		//stream.Read(position.y);
		stream.Read(angle);

		m_SmoothTightness = 0.2f;

		//SetPosition(position);
		SetAngle(angle);

		// If the body isn't awake, velocities are assumed to be zero
		bool awake = stream.ReadBit();

		Vector2 linearVelocity;
		float angularVelocity;
		if (awake)
		{
#ifdef FSN_PHYS_COMPRESS_STATE
			stream.ReadFloat16(linearVelocity.x, -s_MaxVelocity, s_MaxVelocity);
			stream.ReadFloat16(linearVelocity.y, -s_MaxVelocity, s_MaxVelocity);
			stream.ReadFloat16(angularVelocity, -s_MaxRotationalVelocity, s_MaxRotationalVelocity);
#else
			stream.Read(linearVelocity.x);
			stream.Read(linearVelocity.x);
			stream.Read(angularVelocity);
#endif
		}
		else
		{
			linearVelocity = Vector2::zero();
			angularVelocity = 0.f;
		}

		SetVelocity(linearVelocity);
		SetAngularVelocity(angularVelocity);

		if (m_Body)
			m_Body->SetAwake(awake);
		else
			m_Def.awake = true;

		// Prevent sleeping objects from jittering due to being slightly out of synch and colliding then being corrected repeatedly
		m_PinTransform = !awake;

		//Position.MarkChanged();
		//Angle.MarkChanged();

		//Velocity.MarkChanged();
		//AngularVelocity.MarkChanged();
	}

	void Box2DBody::SerialiseOccasional(RakNet::BitStream& stream)
	{
		m_DeltaSerialisationHelper.writeChanges(true, stream,
			IsActive(), IsSleepingAllowed(), IsBullet(), IsFixedRotation(),
			GetLinearDamping(), GetAngularDamping(), GetGravityScale());

		stream.Write(m_Depth);

		if (GetBodyType() != Dynamic)
		{
			std::bitset<NonDynamicDeltaSerialiser_t::NumParams> transformChanges;
			m_NonDynamicDeltaSerialisationHelper.writeChanges(true, stream, IsAwake(), GetPosition(), GetAngle(), GetVelocity(), GetAngularVelocity());
		}
	}

	void Box2DBody::DeserialiseOccasional(RakNet::BitStream& stream)
	{
		{
			std::bitset<DeltaSerialiser_t::NumParams> changes;
			bool active, sleepingAllowed, bullet, fixedrotation;
			float linearDamping, angularDamping, gravityScale;
			m_DeltaSerialisationHelper.readChanges(stream, true, changes,
				active, sleepingAllowed, bullet,
				fixedrotation, linearDamping, angularDamping,
				gravityScale);

			stream.Read(m_Depth);

			angularDamping = std::max(0.0f, angularDamping);
			linearDamping = std::max(0.0f, linearDamping);

			if (changes[PropsIdx::Active])
				//m_Body->SetActive(active);
				SetActive(active);
			if (changes[PropsIdx::SleepingAllowed])
				//m_Body->SetSleepingAllowed(sleepingAllowed);
				SetSleepingAllowed(sleepingAllowed);
			//if (changes[PropsIdx::Awake])
			//	if (m_Body) {/*m_Body->SetAwake(awake);*/} else m_Def.awake = awake;
			if (changes[PropsIdx::Bullet])
				//m_Body->SetBullet(bullet);
				SetBullet(bullet);
			if (changes[PropsIdx::FixedRotation])
				//m_Body->SetFixedRotation(fixedrotation);
				SetFixedRotation(fixedrotation);

			if (changes[PropsIdx::LinearDamping])
				//m_Body->SetLinearDamping(linearDamping);
				SetLinearDamping(linearDamping);
			if (changes[PropsIdx::AngularDamping])
				//m_Body->SetAngularDamping(angularDamping);
				SetAngularDamping(angularDamping);
			if (changes[PropsIdx::GravityScale])
				//m_Body->SetGravityScale(gravityScale);
				SetGravityScale(gravityScale);
		}
		
		if (GetBodyType() != Dynamic)
		{
			std::bitset<NonDynamicDeltaSerialiser_t::NumParams> changes;
			Vector2 position, linearVelocity;
			m_NonDynamicDeltaSerialisationHelper.readChanges(stream, true, changes, m_Def.awake, position, m_Def.angle, linearVelocity, m_Def.angularVelocity);

			if (changes[NonDynamicPropsIdx::Awake])
			{
				if (m_Body)
					m_Body->SetAwake(m_Def.awake);
				//Awake.MarkChanged();
			}
			// note: removed since this is serialised by standard method (in EntitySerialisationUtils):
			//if (changes[NonDynamicPropsIdx::Position])
			//{
			//	SetPosition(position);
			//	//Position.MarkChanged();
			//}
			if (changes[NonDynamicPropsIdx::Angle])
			{
				SetAngle(m_Def.angle);
				//Angle.MarkChanged();
			}

			if (changes[NonDynamicPropsIdx::LinearVelocity])
			{
				SetVelocity(linearVelocity);
				//Velocity.MarkChanged();
			}
			if (changes[NonDynamicPropsIdx::AngularVelocity])
			{
				SetAngularVelocity(m_Def.angularVelocity);
				//AngularVelocity.MarkChanged();
			}
		}
	}

	Box2DFixture::Box2DFixture()
		: m_Fixture(nullptr),
		m_Body(nullptr)
	{
	}

	Box2DFixture::Box2DFixture(b2Fixture* fixture)
		: m_Fixture(fixture),
		m_Body(nullptr)
	{
	}

	Box2DFixture::~Box2DFixture()
	{
		if (auto lock = m_Owner.lock())
		{
			if (m_Fixture)
				m_Fixture->GetBody()->DestroyFixture(m_Fixture);
		}
	}

	void Box2DFixture::ConstructFixture(Box2DBody* body_component)
	{
		FSN_ASSERT(body_component->Getb2Body());

		if (m_Fixture)
			body_component->Getb2Body()->DestroyFixture(m_Fixture);

		m_Owner = body_component->Owner;
		m_Body = body_component;

		m_Def.shape = GetShape();
		m_Fixture = body_component->Getb2Body()->CreateFixture(&m_Def);
	}

	void Box2DFixture::OnSiblingAdded(const ComponentPtr& com)
	{
		//auto body = dynamic_cast<Box2DBody*>(com.get());
		//if (body)
		//{
		//	if (m_Fixture)
		//	{
		//		m_Fixture->GetBody()->DestroyFixture(m_Fixture);
		//		m_Fixture = nullptr;
		//		//m_BodyDestructionConnection.disconnect();
		//	}
		//	if (body->Getb2Body())
		//	{
		//		m_Def.shape = GetShape();
		//		m_Fixture = body->Getb2Body()->CreateFixture(&m_Def);
		//		//m_BodyDestructionConnection = body->Destruction.connect(std::bind(&Box2DFixture::OnBodyDestroyed, this));
		//	}

		//	m_Body = body;
		//}
	}

	void Box2DFixture::OnSiblingRemoved(const ComponentPtr& com)
	{
		//if (m_Fixture)
		//{
		//	auto body = dynamic_cast<Box2DBody*>(com.get());
		//	if (body)
		//	{
		//		m_Fixture->GetBody()->DestroyFixture(m_Fixture);
		//		m_Fixture = nullptr;
		//		//m_BodyDestructionConnection.disconnect();

		//		m_Body = nullptr;
		//	}
		//}
	}

	void Box2DFixture::SerialiseOccasional(RakNet::BitStream& stream)
	{
		m_DeltaSerialisationHelper.writeChanges(true, stream , IsSensor(), GetDensity(), GetFriction(), GetRestitution());
	}

	void Box2DFixture::DeserialiseOccasional(RakNet::BitStream& stream)
	{
		std::bitset<DeltaSerialiser_t::NumParams> changes;
		m_DeltaSerialisationHelper.readChanges(stream, true, changes, m_Def.isSensor, m_Def.density, m_Def.friction, m_Def.restitution);

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

	const b2AABB& Box2DFixture::GetAABB() const
	{
		if (m_Fixture)
			return m_Fixture->GetAABB(0);
		else
		{
			static b2AABB empty;
			return empty;
		}
	}

	Box2DCircleFixture::Box2DCircleFixture()
	{
	}

	void Box2DCircleFixture::SerialiseOccasional(RakNet::BitStream& stream)
	{
		Box2DFixture::SerialiseOccasional(stream);
		m_CircleDeltaSerialisationHelper.writeChanges(true, stream , GetRadius(), GetPosition());
	}

	void Box2DCircleFixture::DeserialiseOccasional(RakNet::BitStream& stream)
	{
		Box2DFixture::DeserialiseOccasional(stream);

		//float radius;
		Vector2 position;
		
		std::bitset<ShapeDeltaSerialiser_t::NumParams> changes;
		m_CircleDeltaSerialisationHelper.readChanges(stream, true, changes, m_CircleShape.m_radius, position);

		m_CircleShape.m_p.Set(position.x, position.y);

		if (m_Fixture)
		{
			FSN_ASSERT(m_Fixture->GetShape()->m_type == b2Shape::e_circle);
			auto shape = static_cast<b2CircleShape*>(m_Fixture->GetShape());

			shape->m_radius = m_CircleShape.m_radius;
			//SetPosition(position);
			shape->m_p.Set(position.x, position.y);

			if (m_Body)
				m_Body->OnFixtureMassChanged();
		}
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

		if (m_Body)
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
		if (m_Fixture)
		{
			FSN_ASSERT(m_Fixture->GetShape()->m_type == b2Shape::e_circle);
			auto shape = static_cast<b2CircleShape*>(m_Fixture->GetShape());
			shape->m_p.Set(position.x, position.y);

			m_CircleDeltaSerialisationHelper.markChanged(ShapePropsIdx::Position);

			if (m_Body)
				m_Body->OnFixtureMassChanged();
		}
		else
			m_CircleShape.m_p.Set(position.x, position.y);
	}

	Vector2 Box2DCircleFixture::GetPosition() const
	{
		//FSN_ASSERT(m_Fixture->GetShape()->m_type == b2Shape::e_circle);
		//auto shape = static_cast<b2CircleShape*>(m_Fixture->GetShape());
		//return b2v2(shape->m_p);
		return b2v2(m_CircleShape.m_p);
	}

	Box2DPolygonFixture::Box2DPolygonFixture()
		: m_ReconstructFixture(false)
	{
	}

	Box2DPolygonFixture::~Box2DPolygonFixture()
	{
		m_PolygonLoadConnection.disconnect();
	}

	void Box2DPolygonFixture::RefreshResource()
	{
		if (!m_PolygonFile.empty())
		{
			m_PolygonLoadConnection.disconnect();
			m_PolygonLoadConnection = ResourceManager::getSingleton().GetResource("POLYGON", m_PolygonFile, [this](ResourceDataPtr data)
			{
				m_PolygonResource.SetTarget(data); m_ReconstructFixture = true;
			});
		}
		else
		{
			m_PolygonResource.Release();
		}
	}

	void Box2DPolygonFixture::Update()
	{
		if (m_ReloadPolygonResource)
		{
			RefreshResource();

			m_ReloadPolygonResource = false;
		}

		if (m_ReconstructFixture && m_Body)
		{
			m_ReconstructFixture = false;
			SkinThickness.MarkChanged();
			ConstructFixture(m_Body);
			m_Body->OnFixtureMassChanged();
		}
	}

	void Box2DPolygonFixture::SerialiseOccasional(RakNet::BitStream& stream)
	{
		Box2DFixture::SerialiseOccasional(stream);

		SerialisationUtils::write(stream, m_PolygonFile);
		
		if (m_PolygonFile.empty())
		{
			stream.Write(m_PolyShape.GetVertexCount());
			for (int i = 0; i < m_PolyShape.GetVertexCount(); ++i)
			{
				const b2Vec2& vert = m_PolyShape.GetVertex(i);
				stream.Write(vert.x);
				stream.Write(vert.y);
			}
		}
	}

	void Box2DPolygonFixture::DeserialiseOccasional(RakNet::BitStream& stream)
	{
		Box2DFixture::DeserialiseOccasional(stream);

		SerialisationUtils::read(stream, m_PolygonFile);
		if (m_PolygonFile != PolygonFile.Get())
		{
			m_ReloadPolygonResource = true;
			PolygonFile.MarkChanged();
		}

		if (m_PolygonFile.empty())
		{
			int32 vertexCount;
			stream.Read(vertexCount);
			std::vector<b2Vec2> verts;
			verts.resize(vertexCount);
			bool newShape = vertexCount != m_PolyShape.GetVertexCount();
			for (int i = 0; i < vertexCount; ++i)
			{
				auto& vert = verts[i];
				stream.Read(vert.x);
				stream.Read(vert.y);

				if (!newShape && !(vert == m_PolyShape.GetVertex(i)))
					newShape = true;
			}

			if (newShape)
			{
				m_PolyShape.Set(verts.data(), verts.size());

				RefreshResource();
				m_ReconstructFixture = true;
			}

			if (m_Body)
				m_Body->OnFixtureMassChanged();
		}
	}

	const std::string& Box2DPolygonFixture::GetPolygonFile() const
	{
		return m_PolygonFile;
	}

	void Box2DPolygonFixture::SetPolygonFile(const std::string& filename)
	{
		m_PolygonFile = filename;
		m_ReloadPolygonResource = true;
	}

	const std::vector<Vector2>& Box2DPolygonFixture::GetVerts() const
	{
		//std::vector<Vector2> verts;
		m_Verts.reserve(m_PolyShape.GetVertexCount());
		for (int i = 0; i < m_PolyShape.GetVertexCount(); ++i)
		{
			const b2Vec2& vert = m_PolyShape.GetVertex(i);
			m_Verts.push_back(Vector2(vert.x, vert.y));
		}
		return m_Verts;
	}

	void Box2DPolygonFixture::SetVerts(const std::vector<Vector2>& verts)
	{
		m_Verts = verts;

		std::vector<b2Vec2> b2Verts(verts.size());
		std::transform(verts.begin(), verts.end(), b2Verts.begin(), [](const Vector2& v)->b2Vec2 { return b2Vec2(v.x, v.y); });
		m_PolyShape.Set(b2Verts.data(), b2Verts.size());

		RefreshResource();
		m_ReconstructFixture = true;
	}

	float Box2DPolygonFixture::GetSkinThickness() const
	{
		if (m_Fixture)
			return m_Fixture->GetShape()->m_radius;
		else if (m_PolygonResource.IsLoaded())
			return m_PolygonResource->m_radius;
		else
			return SkinThickness.Get();
	}

}