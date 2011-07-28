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

#ifndef H_FusionBox2DComponent
#define H_FusionBox2DComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"
#include "FusionPhysicalComponent.h"

#include "FusionCommon.h"

#include "FusionSerialisationHelper.h"

#include <boost/mpl/for_each.hpp>
#include <Box2D/Box2D.h>
#include <tbb/atomic.h>

namespace FusionEngine
{

	class Box2DBodySerialiser
	{
	public:
		static void Deserialise(RakNet::BitStream& input);
	};

	template <typename T>
	class DeltaValueBuffer : public ValueBuffer<T>
	{
	public:
		T m_SerialisedValue;
	};

	class Box2DBody : public IComponent, public IRigidBody
	{
		friend class Box2DWorld;
		friend class Box2DTask;
		friend class Box2DInterpolateTask;
	public:
		FSN_LIST_INTERFACES((ITransform)(IRigidBody))

		struct PropsIdx { enum Names : size_t {
			Active, SleepingAllowed, Awake, Bullet, FixedRotation,
			LinearDamping, AngularDamping,
			GravityScale,
			NumProps
		}; };
		typedef SerialisationHelper<
			bool, bool, bool, bool, bool, // active, SleepingAllowed, IsAwake, IsBullet, IsFixedRotation
			float, float, // Linear, angular Damping
			float> // GravityScale
			DeltaSerialiser_t;
		static_assert(PropsIdx::NumProps == DeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		Box2DBody(b2Body* body);
		virtual ~Box2DBody();

		boost::signals2::signal<void (void)> Destruction;

		b2Body* Getb2Body() const { return m_Body; }

		void OnFixtureMassChanged() { m_FixtureMassDirty = true; }
		
		void CleanMassData();

	private:
		b2Body* m_Body;

		tbb::atomic<bool> m_FixtureMassDirty;

		int m_Depth;

		// Interpolation data
		bool m_Interpolate;
		Vector2 m_InterpPosition;
		Vector2 m_LastPosition;
		float m_InterpAngle;
		float m_LastAngle;
		float m_LastAngularVelocity;

		// IComponent
		std::string GetType() const { return "Box2DBody"; }

		void OnSiblingAdded(const std::shared_ptr<IComponent>& com);
		void OnSiblingRemoved(const std::shared_ptr<IComponent>& com);

		void SynchroniseParallelEdits();
		void FireSignals();

		bool SerialiseContinuous(RakNet::BitStream& stream);
		void DeserialiseContinuous(RakNet::BitStream& stream);
		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		DeltaSerialiser_t m_DeltaSerialisationHelper;

		// RigidBody interface
		BodyType GetBodyType() const
		{
			switch (m_Body->GetType())
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

		Vector2 GetPosition() const { return m_Interpolate ? m_InterpPosition : b2v2(m_Body->GetPosition()); }
		void SetPosition(const Vector2& position) { m_Body->SetTransform(b2Vec2(position.x, position.y), m_Body->GetAngle()); }

		float GetAngle() const { return m_Interpolate ? m_InterpAngle : m_Body->GetAngle(); }
		void SetAngle(float angle) { m_Body->SetTransform(m_Body->GetPosition(), angle); }

		int GetDepth() const { return m_Depth; }
		void SetDepth(int depth) { m_Depth = depth; }

		bool GetInterpolate() const { return m_Interpolate; }
		void SetInterpolate(bool value) { m_Interpolate = value; }

		float GetMass() const { return m_Body->GetMass(); }

		float GetInertia() const { return m_Body->GetInertia(); }

		Vector2 GetCenterOfMass() const { return b2v2(m_Body->GetWorldCenter()); }

		Vector2 GetLocalCenterOfMass() const { return b2v2(m_Body->GetLocalCenter()); }

		Vector2 GetVelocity() const { return b2v2(m_Body->GetLinearVelocity()); }
		void SetVelocity(const Vector2& vel) { m_Body->SetLinearVelocity(b2Vec2(vel.x, vel.y)); }

		float GetAngularVelocity() const { return m_Body->GetAngularVelocity(); }
		void SetAngularVelocity(float vel) { m_Body->SetAngularVelocity(vel); }

		float GetLinearDamping() const { return m_Body->GetLinearDamping(); }
		void SetLinearDamping(float val)
		{
			m_Body->SetLinearDamping(val);
			m_DeltaSerialisationHelper.markChanged(PropsIdx:: LinearDamping );
		};

		float GetAngularDamping() const { return m_Body->GetAngularDamping(); }
		void SetAngularDamping(float val)
		{
			m_Body->SetAngularDamping(val);
			m_DeltaSerialisationHelper.markChanged(PropsIdx:: AngularDamping );
		};

		float GetGravityScale() const { return m_Body->GetGravityScale(); }
		void SetGravityScale(float val)
		{
			m_Body->SetGravityScale(val);
			m_DeltaSerialisationHelper.markChanged(PropsIdx:: GravityScale );
		};

		bool IsActive() const { return m_Body->IsActive(); }
		void SetActive(bool value)
		{
			//m_Body->SetActive(value);
			m_DeltaSerialisationHelper.markChanged(PropsIdx::Active);
		}

		bool IsSleepingAllowed() const { return m_Body->IsSleepingAllowed(); }
		void SetSleepingAllowed(bool value)
		{
			m_Body->SetSleepingAllowed(value);
			m_DeltaSerialisationHelper.markChanged(PropsIdx::SleepingAllowed);
		}

		bool IsAwake() const { return m_Body->IsAwake(); }

		bool IsBullet() const { return m_Body->IsBullet(); }
		void SetBullet(bool value)
		{
			m_Body->SetBullet(value);
			m_DeltaSerialisationHelper.markChanged(PropsIdx::Bullet);
		}

		bool IsFixedRotation() const { return m_Body->IsFixedRotation(); }
		void SetFixedRotation(bool value)
		{
			m_Body->SetFixedRotation(value);
			m_DeltaSerialisationHelper.markChanged(PropsIdx::FixedRotation);
		}

		void ApplyForceImpl(const Vector2& force, const Vector2& point)
		{
			m_Body->ApplyForce(b2Vec2(force.x, force.y), b2Vec2(point.x, point.y));
		}

		void ApplyTorqueImpl(float force)
		{
			m_Body->ApplyTorque(force);
		}

		void ApplyLinearImpulseImpl(const Vector2& impulse, const Vector2& point)
		{
			m_Body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y), b2Vec2(point.x, point.y));
		}

		void ApplyAngularImpulseImpl(float impulse)
		{
			m_Body->ApplyAngularImpulse(impulse);
		}
	};

	class Box2DFixture : public IComponent, public IFixture
	{
		friend class Box2DWorld;
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
		Box2DFixture(RakNet::BitStream& stream);
		Box2DFixture(b2Fixture* fixture);
		virtual ~Box2DFixture();

		void OnBodyDestroyed();

		// Called when any properties change that will result in a different MassData value
		std::function<void (void)> MassChanged;

	private:
		virtual b2Shape* GetShape() = 0;

		// IComponent
		void OnSiblingAdded(const std::shared_ptr<IComponent>& com);
		void OnSiblingRemoved(const std::shared_ptr<IComponent>& com);

	protected:
		virtual void SynchroniseParallelEdits();
		virtual void FireSignals();

		virtual bool SerialiseContinuous(RakNet::BitStream& stream);
		virtual void DeserialiseContinuous(RakNet::BitStream& stream);
		virtual bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		virtual void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

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
		Box2DCircleFixture(RakNet::BitStream& stream);
		virtual ~Box2DCircleFixture() {}

		static void CopyChanges(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta);

	private:
		b2Shape* GetShape() { return &m_CircleShape; }

		// IComponent
		std::string GetType() const { return "Box2DCircleFixture"; }

		// Box2DFixture overides
		virtual void SynchroniseParallelEdits();
		virtual void FireSignals();

		virtual bool SerialiseContinuous(RakNet::BitStream& stream);
		virtual void DeserialiseContinuous(RakNet::BitStream& stream);
		virtual bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		virtual void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

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

		//struct ShapePropsIdx { enum Names : size_t {
		//	Radius,
		//	Position,
		//	NumProps
		//}; };
		//typedef SerialisationHelper<
		//	float, // radius
		//	Vector2> // Position
		//	ShapeDeltaSerialiser_t;
		//static_assert(ShapePropsIdx::NumProps == ShapeDeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		Box2DPolygonFixture();
		Box2DPolygonFixture(RakNet::BitStream& stream);

		static void CopyChanges(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta);

	private:
		b2Shape* GetShape() { return &m_PolygonShape; }

		// IComponent
		std::string GetType() const { return "Box2DPolygonFixture"; }

		// Box2DFixture overides
		virtual void SynchroniseParallelEdits();
		virtual void FireSignals();

		virtual bool SerialiseContinuous(RakNet::BitStream& stream);
		virtual void DeserialiseContinuous(RakNet::BitStream& stream);
		virtual bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		virtual void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		//ShapeDeltaSerialiser_t m_CircleDeltaSerialisationHelper;
		bool m_VerticiesChanged;
		b2PolygonShape m_PolygonShape;

		// IPolygonShape
		float GetRadius() const;

		void SetAsBoxImpl(float half_width, float half_height);
		void SetAsBoxImpl(float half_width, float half_height, const Vector2& center, float angle);
		void SetAsEdgeImpl(const Vector2 &v1, const Vector2 &v2);
	};

}

#endif
