/*
*  Copyright (c) 2011-2013 Fusion Project Team
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

#ifndef H_FusionBox2DComponent
#define H_FusionBox2DComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"
#include "FusionPhysicalComponent.h"

#include "FusionCommon.h"
#include "FusionResourcePointer.h"

#include "FusionSerialisationHelper.h"

#include <Box2D/Box2D.h>
#include <atomic>

#define FSN_PHYS_COMPRESS_STATE

namespace FusionEngine
{

	class Box2DWorld;
	class Box2DFixture;

	class Box2DBody : public SynchronisingComponent, public ITransform, public IRigidBody
	{
		friend class Box2DWorld;
		friend class Box2DTask;
		friend class Box2DInterpolateTask;
	public:
		FSN_LIST_INTERFACES((ITransform)(IRigidBody))

		struct PropsIdx { enum Names : size_t {
			Active, SleepingAllowed, Bullet, FixedRotation,
			LinearDamping, AngularDamping,
			GravityScale,
			NumProps
		}; };
		typedef SerialisationHelper<
			bool, bool, bool, bool, // IsActive, SleepingAllowed, IsBullet, IsFixedRotation
			float, float, // Linear, angular Damping
			float> // GravityScale
			DeltaSerialiser_t;
		static_assert(PropsIdx::NumProps == DeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		struct NonDynamicPropsIdx { enum Names : size_t {
			Awake, Position, Angle, LinearVelocity, AngularVelocity,
			NumProps
		}; };
		typedef SerialisationHelper<
			bool, Vector2, float, Vector2, float> // IsAwake, Position, Angle, LinearVelocity, AngularVelocity
			NonDynamicDeltaSerialiser_t;
		static_assert(NonDynamicPropsIdx::NumProps == NonDynamicDeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		Box2DBody(b2BodyDef def);
		virtual ~Box2DBody();

		std::weak_ptr<Box2DWorld> Owner;

		void ConstructBody(b2World* world, const std::weak_ptr<Box2DWorld>& owner);
		void DestructBody(b2World* world);
		b2Body* Getb2Body() const { return m_Body; }

		void OnFixtureMassChanged() { m_FixtureMassDirty = true; }
		
		void CleanMassData();

		void CompressState();

		bool IsPinned() const { return m_PinTransform; }

		bool AddInteraction(b2Body* other);
		void ClearInteractions();
		void SetInteractingWithPlayer(const bool value) { m_InteractingWithPlayer = value; }
		bool IsInteractingWithPlayer() const;

		static Vector2 DeserialisePosition(RakNet::BitStream& in, const Vector2& origin, float radius);
		bool HasContinuousPosition() const { return GetBodyType() == Dynamic; }

	private:
		b2BodyDef m_Def;
		b2Body* m_Body;

		std::atomic<bool> m_FixtureMassDirty;
		std::set<boost::intrusive_ptr<Box2DFixture>> m_Fixtures;

		bool m_InteractingWithPlayer;
		std::set<b2Body*> m_Interacting;

		bool m_PinTransform;

		int m_Depth;

		// Interpolation data
		bool m_Interpolate;
		Vector2 m_InterpPosition;
		Vector2 m_SmoothPosition;
		Vector2 m_LastPosition;
		float m_InterpAngle;
		float m_LastAngle;
		float m_LastAngularVelocity;

		float m_SmoothTightness;

		// EntityComponent
		std::string GetType() const
		{
			switch (GetBodyType())
			{
			case BodyType::Dynamic:
				return "b2Dynamic";
			case BodyType::Kinematic:
				return "b2Kinematic";
			case BodyType::Static:
				return "b2Static";
			default:
				return "b2RigidBody";
			}
		}

		void OnSiblingAdded(const ComponentPtr& com);
		void OnSiblingRemoved(const ComponentPtr& com);

		void SerialiseTransform(RakNet::BitStream& out, const Vector2& origin, float radius);
		void DeserialiseTransform(RakNet::BitStream& in, const Vector2& position);

		void SerialiseContinuous(RakNet::BitStream& stream);
		void DeserialiseContinuous(RakNet::BitStream& stream);
		void SerialiseOccasional(RakNet::BitStream& stream);
		void DeserialiseOccasional(RakNet::BitStream& stream);

		DeltaSerialiser_t m_DeltaSerialisationHelper;
		NonDynamicDeltaSerialiser_t m_NonDynamicDeltaSerialisationHelper;

		// RigidBody interface
		BodyType GetBodyType() const override
		{
			const b2BodyType type = m_Body ? m_Body->GetType() : m_Def.type;
			switch (type)
			{
			case b2_staticBody:
				return BodyType::Static;
			case b2_kinematicBody:
				return BodyType::Kinematic;
			case b2_dynamicBody:
				return BodyType::Dynamic;
			default:
				return BodyType::Static;
			}
		}

		Vector2 GetPosition() const override { return m_Interpolate ? m_InterpPosition : (m_Body ? b2v2(m_Body->GetPosition()) : b2v2(m_Def.position)); }
		void SetPosition(const Vector2& position) override
		{
			if (m_Body) m_Body->SetTransform(b2Vec2(position.x, position.y), m_Body->GetAngle());
			else
			{
				m_Def.position.Set(position.x, position.y);
				m_InterpPosition = position;
			}
		}

		float GetAngle() const override { return m_Interpolate ? m_InterpAngle : (m_Body ? m_Body->GetAngle() : m_Def.angle); }
		void SetAngle(float angle) override
		{
			if (m_Body) m_Body->SetTransform(m_Body->GetPosition(), angle);
			else
			{
				m_Def.angle = angle;
				m_InterpAngle = angle;
			}
		}

		int GetDepth() const override { return m_Depth; }
		void SetDepth(int depth) override { m_Depth = depth; }

		bool GetInterpolate() const override { return m_Interpolate; }
		void SetInterpolate(bool value) override { m_Interpolate = value; }

		float GetMass() const override { return m_Body ? m_Body->GetMass() : 0.f; }

		float GetInertia() const override { return m_Body ? m_Body->GetInertia() : 0.f; }

		Vector2 GetCenterOfMass() const override { return m_Body ? b2v2(m_Body->GetWorldCenter()) : Vector2(); }

		Vector2 GetLocalCenterOfMass() const { return m_Body ? b2v2(m_Body->GetLocalCenter()) : Vector2(); }

		Vector2 GetVelocity() const override { return b2v2(m_Body ? m_Body->GetLinearVelocity() : m_Def.linearVelocity); }
		void SetVelocity(const Vector2& vel) override { m_Body ? m_Body->SetLinearVelocity(b2Vec2(vel.x, vel.y)) : m_Def.linearVelocity.Set(vel.x, vel.y); }

		float GetAngularVelocity() const override { return m_Body ? m_Body->GetAngularVelocity() : m_Def.angularVelocity; }
		void SetAngularVelocity(float vel) override { if (m_Body) m_Body->SetAngularVelocity(vel); else m_Def.angularVelocity = vel; }

		float GetLinearDamping() const override { return m_Body ? m_Body->GetLinearDamping() : m_Def.linearDamping; }
		void SetLinearDamping(float val) override
		{
			FSN_ASSERT(val >= 0.0f);
			if (m_Body)
				m_Body->SetLinearDamping(val);
			else
				m_Def.linearDamping = val;
			m_DeltaSerialisationHelper.markChanged(PropsIdx::LinearDamping );
		};

		float GetAngularDamping() const override { return m_Body ? m_Body->GetAngularDamping() : m_Def.angularVelocity; }
		void SetAngularDamping(float val) override
		{
			FSN_ASSERT(val >= 0.0f);
			if (m_Body)
				m_Body->SetAngularDamping(val);
			else
				m_Def.angularDamping = val;
			m_DeltaSerialisationHelper.markChanged(PropsIdx:: AngularDamping );
		};

		float GetGravityScale() const override { return m_Body ? m_Body->GetGravityScale() : m_Def.gravityScale; }
		void SetGravityScale(float val) override
		{
			if (m_Body)
				m_Body->SetGravityScale(val);
			else
				m_Def.gravityScale = val;
			m_DeltaSerialisationHelper.markChanged(PropsIdx:: GravityScale );
		};

		bool IsActive() const override { return m_Body ? m_Body->IsActive() : m_Def.active; }
		void SetActive(bool value) override
		{
			if (m_Body)
				m_Body->SetActive(value);
			else
				m_Def.active = value;
			m_DeltaSerialisationHelper.markChanged(PropsIdx::Active);
		}

		bool IsSleepingAllowed() const override { return m_Body ? m_Body->IsSleepingAllowed() : m_Def.allowSleep; }
		void SetSleepingAllowed(bool value) override
		{
			if (m_Body)
				m_Body->SetSleepingAllowed(value);
			else
				m_Def.allowSleep = value;
			m_DeltaSerialisationHelper.markChanged(PropsIdx::SleepingAllowed);
		}

		bool IsAwake() const override { return m_Body ? m_Body->IsAwake() : m_Def.awake; }

		bool IsBullet() const override { return m_Body ? m_Body->IsBullet() : m_Def.bullet; }
		void SetBullet(bool value) override
		{
			if (m_Body)
				m_Body->SetBullet(value);
			else
				m_Def.bullet = value;
			m_DeltaSerialisationHelper.markChanged(PropsIdx::Bullet);
		}

		bool IsFixedRotation() const override { return m_Body ? m_Body->IsFixedRotation() : m_Def.fixedRotation; }
		void SetFixedRotation(bool value) override
		{
			if (m_Body)
				m_Body->SetFixedRotation(value);
			else
				m_Def.fixedRotation = value;
			m_DeltaSerialisationHelper.markChanged(PropsIdx::FixedRotation);
		}

		void ApplyForceImpl(const Vector2& force, const Vector2& point) override
		{
			if (m_Body)
				m_Body->ApplyForce(b2Vec2(force.x, force.y), b2Vec2(point.x, point.y), true);
		}

		void ApplyForceToCenterImpl(const Vector2& force) override
		{
			if (m_Body)
				m_Body->ApplyForceToCenter(b2Vec2(force.x, force.y), true);
		}

		void ApplyTorqueImpl(float force) override
		{
			if (m_Body)
				m_Body->ApplyTorque(force, true);
		}

		void ApplyLinearImpulseImpl(const Vector2& impulse, const Vector2& point) override
		{
			if (m_Body)
				m_Body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y), b2Vec2(point.x, point.y), true);
		}

		void ApplyAngularImpulseImpl(float impulse) override
		{
			if (m_Body)
				m_Body->ApplyAngularImpulse(impulse, true);
		}
	};

	class Box2DFixture : public SynchronisingComponent, public IFixture
	{
		friend class Box2DWorld;
		friend class Box2DBody;
	public:
		FSN_LIST_INTERFACES((IFixture))

		struct PropsIdx { enum Names : size_t {
			Sensor,
			Density, Friction, Restitution,
			NumProps
		}; };
		typedef SerialisationHelper<
			bool, // Sensor
			float, float, float> // Density, Friction, Restitution
			DeltaSerialiser_t;
		static_assert(PropsIdx::NumProps == DeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		Box2DFixture();
		Box2DFixture(b2Fixture* fixture);
		virtual ~Box2DFixture();

		void ConstructFixture(Box2DBody* body_component);

		//! Callback: Called when any properties change that will result in a different MassData value
		std::function<void (void)> MassChanged;

	private:
		virtual b2Shape* GetShape() = 0;

		// EntityComponent
		void OnSiblingAdded(const ComponentPtr& com);
		void OnSiblingRemoved(const ComponentPtr& com);

	protected:
		virtual void SerialiseOccasional(RakNet::BitStream& stream);
		virtual void DeserialiseOccasional(RakNet::BitStream& stream);

		DeltaSerialiser_t m_DeltaSerialisationHelper;

	private:
		// IFixture

		//! Is this fixture a sensor (non-solid)?
		bool IsSensor() const;
		//! Set if this fixture is a sensor.
		void SetSensor(bool sensor);

		//! Get the density of this fixture.
		float GetDensity() const;
		//! Set the density of this fixture. This will _not_ automatically adjust the mass
		//! of the body. You must call b2Body::ResetMassData to update the body's mass.
		void SetDensity(float density);
		
		//! Get the coefficient of friction.
		float GetFriction() const;
		//! Set the coefficient of friction. This will _not_ change the friction of
		//! existing contacts.
		void SetFriction(float friction);

		//! Get the coefficient of restitution.
		float GetRestitution() const;
		//! Set the coefficient of restitution. This will _not_ change the restitution of
		//! existing contacts.
		void SetRestitution(float restitution);

		//! Get the fixture's AABB. This AABB may be enlarge and/or stale.
		//! If you need a more accurate AABB, compute it using the shape and
		//! the body transform.
		const b2AABB& GetAABB() const;

		b2FixtureDef m_Def;
	protected:
		b2Fixture* m_Fixture;

		Box2DBody* m_Body;

		std::weak_ptr<Box2DWorld> m_Owner;

		boost::signals2::scoped_connection m_BodyDestructionConnection;
	};

	class Box2DCircleFixture : public Box2DFixture, public ICircleShape
	{
		friend class Box2DWorld;
	public:
		FSN_LIST_INTERFACES((IFixture)(ICircleShape))

		struct ShapePropsIdx { enum Names : size_t {
			Radius,
			Position,
			NumProps
		}; };
		typedef SerialisationHelper<
			float, // radius
			Vector2> // Position
			ShapeDeltaSerialiser_t;
		static_assert(ShapePropsIdx::NumProps == ShapeDeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		Box2DCircleFixture();
		virtual ~Box2DCircleFixture() {}

	private:
		b2Shape* GetShape() { return &m_CircleShape; }

		// EntityComponent
		std::string GetType() const { return "b2Circle"; }

		// Box2DFixture overides
		virtual void SerialiseOccasional(RakNet::BitStream& stream);
		virtual void DeserialiseOccasional(RakNet::BitStream& stream);

		ShapeDeltaSerialiser_t m_CircleDeltaSerialisationHelper;

		b2CircleShape m_CircleShape;

		// ICircleShape
		void SetRadius(float radius);
		float GetRadius() const;
		
		void SetPosition(const Vector2& center);
		Vector2 GetPosition() const;
	};

	class Box2DPolygonFixture : public Box2DFixture, public IPolygonShape
	{
		friend class Box2DWorld;
	public:
		FSN_LIST_INTERFACES((IFixture)(IPolygonShape))

		Box2DPolygonFixture();
		~Box2DPolygonFixture();

		void Update();

		bool HotReloadEvent(ResourceDataPtr resReloading, ResourceContainer::HotReloadEvent ev);

	private:
		b2Shape* GetShape() { return m_PolygonResource.IsLoaded() ? m_PolygonResource.Get() : &m_PolyShape; }

		// EntityComponent
		std::string GetType() const { return "b2Polygon"; }

		// Box2DFixture overrides
		virtual void SerialiseOccasional(RakNet::BitStream& stream);
		virtual void DeserialiseOccasional(RakNet::BitStream& stream);

		void RefreshResource();

		bool m_ReconstructFixture;
		ResourcePointer<b2PolygonShape> m_PolygonResource;

		mutable std::vector<Vector2> m_Verts;
		b2PolygonShape m_PolyShape;

		std::string m_PolygonFile;
		bool m_ReloadPolygonResource;

		boost::signals2::connection m_PolygonLoadConnection;

		// IPolygonShape
		const std::string& GetPolygonFile() const;
		void SetPolygonFile(const std::string& path);

		const std::vector<Vector2>& GetVerts() const;
		void SetVerts(const std::vector<Vector2>& verts);

		float GetSkinThickness() const;

	};

	class Box2DEdgeChainFixture : public Box2DFixture, public IEdgeShape
	{
		friend class Box2DWorld;
	public:
		FSN_LIST_INTERFACES((IFixture)(IEdgeShape))

		Box2DEdgeChainFixture();
		~Box2DEdgeChainFixture();

		void Update();

	private:
		b2Shape* GetShape() { return m_PolygonResource.IsLoaded() ? m_PolygonResource.Get() : &m_ChainShape; }

		// EntityComponent
		std::string GetType() const { return "b2EdgeChain"; }

		// Box2DFixture overides
		virtual void SerialiseOccasional(RakNet::BitStream& stream);
		virtual void DeserialiseOccasional(RakNet::BitStream& stream);

		void RefreshResource();

		bool m_ReconstructFixture;
		ResourcePointer<b2ChainShape> m_PolygonResource;
		b2ChainShape m_ChainShape;

		std::string m_PolygonFile;
		bool m_ReloadPolygonResource;

		boost::signals2::connection m_PolygonLoadConnection;

		// IPolygonShape
		const std::string& GetPolygonFile() const;
		void SetPolygonFile(const std::string& path);

		const std::vector<Vector2>& GetVerts() const;
		void SetVerts(const std::vector<Vector2>& verts);

		bool IsLoop() const;
		void SetLoop(bool value);

		float GetSkinThickness() const;

	};

}

#endif
