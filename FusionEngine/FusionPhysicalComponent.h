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

#ifndef H_FusionPhysicalComponent
#define H_FusionPhysicalComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"
#include "FusionThreadSafeProperty.h"

#include "FusionVector2.h"
#include "FusionCommon.h"

namespace FusionEngine
{

	//template <class C>
	//static void RegisterProp(asIScriptEngine* engine, const std::string& cname, const std::string& type, const std::string& name)
	//{
	//	engine->RegisterObjectMethod(cname.c_str(), (type + " get_" + name + "() const").c_str(), asMETHOD(C, Get));
	//}

	FSN_BEGIN_COIFACE(ITransform)
	public:
		ThreadSafeProperty<Vector2> Position;
		ThreadSafeProperty<float> Angle;
		ThreadSafeProperty<int> Depth;

		void SynchroniseInterface()
		{
			FSN_SYNCH_PROP(Position);
			FSN_SYNCH_PROP(Angle);
			FSN_SYNCH_PROP(Depth);
		}

		void FireInterfaceSignals()
		{
			Position.FireSignal();
			Angle.FireSignal();
			Depth.FireSignal();
		}

		static bool IsThreadSafe() { return true; }

	public:
		virtual Vector2 GetPosition() const = 0;
		virtual void SetPosition(const Vector2& pos) = 0;

		virtual float GetAngle() const = 0;
		virtual void SetAngle(float angle) = 0;

		virtual int GetDepth() const = 0;
		virtual void SetDepth(int depth) = 0;
	};

	//! Threadsafe physical body interface
	class IRigidBody : public ITransform
	{
	public:
		static std::string GetTypeName() { return "IRigidBody"; }

		virtual ~IRigidBody()
		{}

		ThreadSafeProperty<float, NullWriter<float>> Mass;
		ThreadSafeProperty<float, NullWriter<float>> Inertia;

		ThreadSafeProperty<Vector2> Velocity;
		ThreadSafeProperty<float> AngularVelocity;

		ThreadSafeProperty<float> LinearDamping;
		ThreadSafeProperty<float> AngularDamping;

		ThreadSafeProperty<float> GravityScale;

		ThreadSafeProperty<bool> Active;
		ThreadSafeProperty<bool> SleepingAllowed;
		ThreadSafeProperty<bool, NullWriter<bool>> Awake;

		ThreadSafeProperty<bool> Bullet;
		ThreadSafeProperty<bool> FixedRotation;

		void SynchroniseInterface()
		{
			ITransform::SynchroniseInterface();
			Mass.Synchronise(GetMass()); // readonly
			Inertia.Synchronise(GetInertia()); // readonly
			FSN_SYNCH_PROP(Velocity);
			FSN_SYNCH_PROP(AngularVelocity);
			FSN_SYNCH_PROP(LinearDamping);
			FSN_SYNCH_PROP(AngularDamping);
			FSN_SYNCH_PROP(GravityScale);
			FSN_SYNCH_PROP_BOOL(Active);
			FSN_SYNCH_PROP_BOOL(SleepingAllowed);
			Awake.Synchronise(IsAwake()); // readonly
			FSN_SYNCH_PROP_BOOL(Bullet);
			FSN_SYNCH_PROP_BOOL(FixedRotation);
		}

		void FireInterfaceSignals()
		{
			ITransform::FireInterfaceSignals();
			Mass.FireSignal();
			Inertia.FireSignal();
			Velocity.FireSignal();
			AngularVelocity.FireSignal();
			LinearDamping.FireSignal();
			AngularDamping.FireSignal();
			GravityScale.FireSignal();
			Active.FireSignal();
			SleepingAllowed.FireSignal();
			Awake.FireSignal();
			Bullet.FireSignal();
			FixedRotation.FireSignal();
		}

		// Prevent simultanious access to implementation methods
		CL_Mutex m_InternalMutex;

		void ApplyForce(const Vector2& force, const Vector2& point)
		{ CL_MutexSection lock(&m_InternalMutex); ApplyForceImpl(force, point); }
		void ApplyForce(const Vector2& force)
		{ CL_MutexSection lock(&m_InternalMutex); ApplyForceImpl(force, GetCenterOfMass()); };
		void ApplyTorque(float torque)
		{ CL_MutexSection lock(&m_InternalMutex); ApplyTorqueImpl(torque); }

		void ApplyLinearImpulse(const Vector2& impulse, const Vector2& point)
		{ CL_MutexSection lock(&m_InternalMutex); ApplyLinearImpulseImpl(impulse, point); }
		void ApplyAngularImpulse(float force)
		{ CL_MutexSection lock(&m_InternalMutex); ApplyAngularImpulseImpl(force); }

		//! Returns true
		static bool IsThreadSafe() { return true; }

	public:
		//! Gets the mass
		virtual float GetMass() const = 0;
		//! Gets the inertia
		virtual float GetInertia() const = 0;

		virtual Vector2 GetCenterOfMass() const = 0;

		virtual Vector2 GetVelocity() const = 0;
		virtual void SetVelocity(const Vector2& vel) = 0;

		virtual float GetAngularVelocity() const = 0;
		virtual void SetAngularVelocity(float vel) = 0;

		//! Get the linear damping of the body.
		virtual float GetLinearDamping() const = 0;
		//! Set the linear damping of the body.
		virtual void SetLinearDamping(float linearDamping) = 0;

		//! Get the angular damping of the body.
		virtual float GetAngularDamping() const = 0;
		//! Set the angular damping of the body.
		virtual void SetAngularDamping(float angularDamping) = 0;

		//! Get the gravity scale of the body.
		virtual float GetGravityScale() const = 0;
		//! Set the gravity scale of the body.
		virtual void SetGravityScale(float scale) = 0;

		virtual bool IsActive() const = 0;
		virtual void SetActive(bool value) = 0;

		virtual bool IsSleepingAllowed() const = 0;
		virtual void SetSleepingAllowed(bool value) = 0;

		virtual bool IsAwake() const = 0;

		virtual bool IsBullet() const = 0;
		virtual void SetBullet(bool value) = 0;

		virtual bool IsFixedRotation() const = 0;
		virtual void SetFixedRotation(bool value) = 0;

		virtual void ApplyForceImpl(const Vector2& force, const Vector2& point) = 0;
		//virtual void ApplyForceImpl(const Vector2& force) = 0;
		virtual void ApplyTorqueImpl(float torque) = 0;

		virtual void ApplyLinearImpulseImpl(const Vector2& impulse, const Vector2& point) = 0;
		virtual void ApplyAngularImpulseImpl(float force) = 0;
	};

	//! Non-threadsafe physical body interface
	FSN_BEGIN_COIFACE(IPhysicalMethods)
	public:
		virtual void ApplyForce(const Vector2& force, const Vector2& point) = 0;
		virtual void ApplyForce(const Vector2& force) = 0;
		virtual void ApplyTorque(float torque) = 0;

		virtual void ApplyLinearImpulse(const Vector2& impulse, const Vector2& point) = 0;
		virtual void ApplyAngularImpulse(float force) = 0;

		//! Returns false
		static bool IsThreadSafe() { return false; }
	};

	//! Physical fixture interface
	FSN_BEGIN_COIFACE(IPhysFixture)
	public:
		ThreadSafeProperty<bool> Sensor;
		ThreadSafeProperty<float> Density;
		ThreadSafeProperty<float> Friction;
		ThreadSafeProperty<float> Restitution;
		ThreadSafeProperty<b2AABB> AABB;

		void SynchroniseInterface()
		{
			FSN_SYNCH_PROP_BOOL(Sensor);
			FSN_SYNCH_PROP(Density);
			FSN_SYNCH_PROP(Friction);
			FSN_SYNCH_PROP(Restitution);
			AABB.Synchronise(GetAABB());
		}

		void FireInterfaceSignals()
		{
			Sensor.FireSignal();
			Density.FireSignal();
			Friction.FireSignal();
			Restitution.FireSignal();
			AABB.FireSignal();
		}

		//! Returns true
		static bool IsThreadSafe() { return true; }

	public:
		//! Set if this fixture is a sensor.
		virtual void SetSensor(bool sensor) = 0;
		//! Is this fixture a sensor (non-solid)?
		virtual bool IsSensor() const = 0;

		//! Set the density of this fixture. This will _not_ automatically adjust the mass
		//! of the body. You must call b2Body::ResetMassData to update the body's mass.
		virtual void SetDensity(float density) = 0;
		//! Get the density of this fixture.
		virtual float GetDensity() const = 0;

		//! Get the coefficient of friction.
		virtual float GetFriction() const = 0;
		//! Set the coefficient of friction. This will _not_ change the friction of
		//! existing contacts.
		virtual void SetFriction(float friction) = 0;

		//! Get the coefficient of restitution.
		virtual float GetRestitution() const = 0;
		//! Set the coefficient of restitution. This will _not_ change the restitution of
		//! existing contacts.
		virtual void SetRestitution(float restitution) = 0;

		//! Get the fixture's AABB. This AABB may be enlarge and/or stale.
		//! If you need a more accurate AABB, compute it using the shape and
		//! the body transform.
		virtual const b2AABB& GetAABB() const = 0;
	};

}

#endif
