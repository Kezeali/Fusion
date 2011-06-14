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

	class Box2DBody : public IComponent, public IPhysicalProperties, public IPhysicalMethods
	{
		friend class Box2DWorld;
	public:
		typedef boost::mpl::vector<ITransform, IPhysicalProperties, IPhysicalMethods>::type Interfaces;

		static void MergeDelta(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data);

	private:
		Box2DBody(b2Body* body)
			: m_Body(body)
		{
		}

		b2Body* m_Body;

		// IComponent
		std::string GetType() const { return "Box2DBody"; }

		void SynchroniseParallelEdits();
		void SynchTransform();

		bool SerialiseContinuous(RakNet::BitStream& stream);
		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		void DeserialiseContinuous(RakNet::BitStream& stream);
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		// Threadsafe interface
		float m_Mass;
		float GetMass() const { return m_Mass; }

		float m_Inertia;
		float GetInertia() const { return m_Inertia; }

		ValueBuffer<Vector2> m_Position;
		const Vector2& GetPosition() const { return m_Position.Read(); }
		void SetPosition(const Vector2& position) { m_Position.Write(position); }

		tbb::atomic<float> m_Angle;
		tbb::atomic<bool> m_AngleWritten;
		float GetAngle() const { return m_Angle; }
		void SetAngle(float angle) { m_Angle = angle; m_AngleWritten = true; }

		ValueBuffer<Vector2> m_Velocity;
		const Vector2& GetVelocity() const { return m_Velocity.Read(); }
		void SetVelocity(const Vector2& vel) { m_Velocity.Write(vel); }

		tbb::atomic<float> m_AngularVelocity;
		tbb::atomic<bool> m_AngularVelocityWritten;
		float GetAngularVelocity() const { return m_AngularVelocity; }
		void SetAngularVelocity(float vel) { m_AngularVelocity = vel; m_AngularVelocityWritten = true; }

#define FSN_ATOMIC_FLOAT_PROP(name) \
	float Get##name() const { return m_##name; }\
	void Set##name(float vel) { m_##name = vel; m_##name##Written = true; }\
	float m_Serialised##name##Value;\
	tbb::atomic<float> m_##name;\
	tbb::atomic<bool> m_##name##Written;

		//! Get linear damping
		FSN_ATOMIC_FLOAT_PROP(LinearDamping);

		//! Get the angular damping of the body.
		FSN_ATOMIC_FLOAT_PROP(AngularDamping);

		//! Get the gravity scale of the body.
		FSN_ATOMIC_FLOAT_PROP(GravityScale);

#undef FSN_ATOMIC_FLOAT_PROP

		bool m_SerialisedActiveValue;
		tbb::atomic<bool> m_Active;
		bool IsActive() const { return m_Body->IsActive(); } // nothing else writes this value, so this is safe
		void SetActive(bool value) { m_Active = value; }

		bool m_SerialisedSleepingAllowedValue;
		tbb::atomic<bool> m_SleepingAllowed;
		bool IsSleepingAllowed() const { return m_SleepingAllowed; }
		void SetSleepingAllowed(bool value) { m_SleepingAllowed = value; }

		bool m_SerialisedAwakeValue;
		bool m_Awake;
		bool IsAwake() const { return m_Awake; }

		bool m_SerialisedBulletValue;
		tbb::atomic<bool> m_Bullet;
		bool IsBullet() const { return m_Bullet; }
		void SetBullet(bool value) { m_Bullet = value; }

		bool m_SerialisedFixedRotationValue;
		tbb::atomic<bool> m_FixedRotation;
		bool IsFixedRotation() const { return m_FixedRotation; }
		void SetFixedRotation(bool value) { m_FixedRotation = value; }

		// Non-threadsafe interface
		void ApplyForce(const Vector2& force, const Vector2& point)
		{
			m_Body->ApplyForce(b2Vec2(force.x, force.y), b2Vec2(point.x, point.y));
		}

		void ApplyForce(const Vector2& force)
		{
			m_Body->ApplyForce(b2Vec2(force.x, force.y), m_Body->GetWorldCenter());
		}

		void ApplyTorque(float force)
		{
			m_Body->ApplyTorque(force);
		}
	};

	template <typename T>
	struct AtomicFlagTypeProp
	{
		T serialised;
		tbb::atomic<T> written;

		void Set(const T& v) { written = v; }
	};

	template <>
	struct AtomicFlagTypeProp<float>
	{
		float serialised;
		tbb::atomic<float> written;
		tbb::atomic<bool> changed;

		void Set(const float& v) { written = v; changed = true; }
	};

	class Box2DFixture : public IComponent, public IFixtureProperties
	{
		friend class Box2DWorld;
	public:
		typedef boost::mpl::vector<IFixtureProperties>::type Interfaces;

		static void MergeDelta(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data);

	private:
		// IComponent
		std::string GetType() const { return "Box2DFixture"; }

		void SynchroniseParallelEdits();

		bool SerialiseContinuous(RakNet::BitStream& stream);
		void DeserialiseContinuous(RakNet::BitStream& stream);
		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		// IFixtureProperties

		AtomicFlagTypeProp<bool> m_SensorProp;
		//! Is this fixture a sensor (non-solid)?
		bool IsSensor() const { return m_Fixture->IsSensor(); }
		//! Set if this fixture is a sensor.
		void SetSensor(bool sensor) { m_SensorProp.Set(sensor); }

		AtomicFlagTypeProp<float> m_DensityProp;
		//! Get the density of this fixture.
		float GetDensity() const { return m_Fixture->GetDensity(); }
		//! Set the density of this fixture. This will _not_ automatically adjust the mass
		//! of the body. You must call b2Body::ResetMassData to update the body's mass.
		void SetDensity(float density) { m_DensityProp.Set(density); }
		
		AtomicFlagTypeProp<float> m_FrictionProp;
		//! Get the coefficient of friction.
		float GetFriction() const { return m_Fixture->GetFriction();}
		//! Set the coefficient of friction. This will _not_ change the friction of
		//! existing contacts.
		void SetFriction(float friction) { m_FrictionProp.Set(friction); }

		AtomicFlagTypeProp<float> m_RestitutionProp;
		//! Get the coefficient of restitution.
		float GetRestitution() const { m_Fixture->GetRestitution(); }
		//! Set the coefficient of restitution. This will _not_ change the restitution of
		//! existing contacts.
		void SetRestitution(float restitution) { m_RestitutionProp.Set(restitution); }

		//! Get the fixture's AABB. This AABB may be enlarge and/or stale.
		//! If you need a more accurate AABB, compute it using the shape and
		//! the body transform.
		const b2AABB& GetAABB(int childIndex) const { return m_Fixture->GetAABB(childIndex); }

		b2Fixture* m_Fixture;
	};

}

#endif
