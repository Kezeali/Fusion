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
		void DeserialiseContinuous(RakNet::BitStream& stream);
		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		// Threadsafe interface
		float GetMass() const { return m_Body->GetMass(); }

		float GetInertia() const { return m_Body->GetInertia(); }

		Vector2 GetPosition() const { return b2v2(m_Body->GetPosition()); }
		void SetPosition(const Vector2& position) { m_Body->SetTransform(b2Vec2(position.x, position.y), m_Body->GetAngle()); }

		float GetAngle() const { return m_Body->GetAngle(); }
		void SetAngle(float angle) { m_Body->SetTransform(m_Body->GetPosition(), angle); }

		Vector2 GetVelocity() const { return b2v2(m_Body->GetLinearVelocity()); }
		void SetVelocity(const Vector2& vel) { m_Body->SetLinearVelocity(b2Vec2(vel.x, vel.y)); }

		float GetAngularVelocity() const { return m_Body->GetAngularVelocity(); }
		void SetAngularVelocity(float vel) { m_Body->SetAngularVelocity(vel); }

#define BODY_PROP(name) \
	float Get ## name() const { return m_Body->Get ## name(); }\
	void Set ## name(float val) { m_Body->Set ## name(val); }

		//! Get linear damping
		BODY_PROP(LinearDamping);

		//! Get the angular damping of the body.
		BODY_PROP(AngularDamping);

		//! Get the gravity scale of the body.
		BODY_PROP(GravityScale);

#undef BODY_PROP

		bool IsActive() const { return m_Body->IsActive(); }
		void SetActive(bool value) { m_Body->SetActive(value); }

		bool IsSleepingAllowed() const { return m_Body->IsSleepingAllowed(); }
		void SetSleepingAllowed(bool value) { m_Body->SetSleepingAllowed(value); }

		bool IsAwake() const { return m_Body->IsAwake(); }

		bool IsBullet() const { return m_Body->IsBullet(); }
		void SetBullet(bool value) { m_Body->SetBullet(value); }

		bool IsFixedRotation() const { return m_Body->IsFixedRotation(); }
		void SetFixedRotation(bool value) { m_Body->SetFixedRotation(value); }

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
		Box2DFixture(b2Fixture* fixture);

		// IComponent
		std::string GetType() const { return "Box2DFixture"; }

		void SynchroniseParallelEdits();

		bool SerialiseContinuous(RakNet::BitStream& stream);
		void DeserialiseContinuous(RakNet::BitStream& stream);
		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		// IFixtureProperties

		//! Is this fixture a sensor (non-solid)?
		bool IsSensor() const { return m_Fixture->IsSensor(); }
		//! Set if this fixture is a sensor.
		void SetSensor(bool sensor) { m_Fixture->SetSensor(sensor); }

		//! Get the density of this fixture.
		float GetDensity() const { return m_Fixture->GetDensity(); }
		//! Set the density of this fixture. This will _not_ automatically adjust the mass
		//! of the body. You must call b2Body::ResetMassData to update the body's mass.
		void SetDensity(float density) { m_Fixture->SetDensity(density); }
		
		//! Get the coefficient of friction.
		float GetFriction() const { return m_Fixture->GetFriction();}
		//! Set the coefficient of friction. This will _not_ change the friction of
		//! existing contacts.
		void SetFriction(float friction) { m_Fixture->SetFriction(friction); }

		//! Get the coefficient of restitution.
		float GetRestitution() const { m_Fixture->GetRestitution(); }
		//! Set the coefficient of restitution. This will _not_ change the restitution of
		//! existing contacts.
		void SetRestitution(float restitution) { m_Fixture->SetRestitution(restitution); }

		//! Get the fixture's AABB. This AABB may be enlarge and/or stale.
		//! If you need a more accurate AABB, compute it using the shape and
		//! the body transform.
		const b2AABB& GetAABB(int childIndex) const { return m_Fixture->GetAABB(childIndex); }

		b2Fixture* m_Fixture;
	};

}

#endif
