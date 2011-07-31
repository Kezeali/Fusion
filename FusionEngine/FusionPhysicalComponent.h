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

#include <tbb/spin_mutex.h>

namespace FusionEngine
{

	//template <class C>
	//static void RegisterProp(asIScriptEngine* engine, const std::string& cname, const std::string& type, const std::string& name)
	//{
	//	engine->RegisterObjectMethod(cname.c_str(), (type + " get_" + name + "() const").c_str(), asMETHOD(C, Get));
	//}

	FSN_BEGIN_COIFACE(ITransform)
	public:
		void InitProperties()
		{
			Position.SetCallbacks(this, &ITransform::GetPosition, &ITransform::SetPosition);
			Angle.SetCallbacks(this, &ITransform::GetAngle, &ITransform::SetAngle);
			Depth.SetCallbacks(this, &ITransform::GetDepth, &ITransform::SetDepth);
		}

		ThreadSafeProperty<Vector2> Position;
		ThreadSafeProperty<float> Angle;
		ThreadSafeProperty<int> Depth;

		static void RegisterScriptInterface(asIScriptEngine* engine);

		static bool IsThreadSafe() { return true; }

	protected:
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

		FSN_COIFACE_CTOR(IRigidBody,
			((FSN_GET_SET)(Interpolate))
			((FSN_GET)(Mass))
			((FSN_GET)(Inertia))
			((FSN_GET)(CenterOfMass))

			((FSN_GET_SET)(Velocity))
			((FSN_GET_SET)(AngularVelocity))

			((FSN_GET_SET)(LinearDamping))
			((FSN_GET_SET)(AngularDamping))

			((FSN_GET_SET)(GravityScale))

			((FSN_IS_SET)(Active))
			((FSN_IS_SET)(SleepingAllowed))
			((FSN_IS)(Awake))

			((FSN_IS_SET)(Bullet))
			((FSN_IS_SET)(FixedRotation)) )

		ThreadSafeProperty<bool> Interpolate;

		ThreadSafeProperty<float, NullWriter<float>> Mass;
		ThreadSafeProperty<float, NullWriter<float>> Inertia;
		ThreadSafeProperty<Vector2, NullWriter<Vector2>> CenterOfMass;

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

		static void RegisterScriptInterface(asIScriptEngine* engine);

		enum BodyType { Static, Kinematic, Dynamic };

		virtual BodyType GetBodyType() const = 0;

		// Prevent simultanious access to implementation methods
		tbb::spin_mutex m_InternalMutex;

		void ApplyForce(const Vector2& force, const Vector2& point)
		{
			tbb::spin_mutex::scoped_lock lock(m_InternalMutex);
			ApplyForceImpl(force, point);
		}
		void ApplyForce(const Vector2& force)
		{
			tbb::spin_mutex::scoped_lock lock(m_InternalMutex);
			ApplyForceImpl(force, GetCenterOfMass());
		};
		void ApplyTorque(float torque)
		{
			tbb::spin_mutex::scoped_lock lock(m_InternalMutex);
			ApplyTorqueImpl(torque);
		}

		void ApplyLinearImpulse(const Vector2& impulse, const Vector2& point)
		{
			tbb::spin_mutex::scoped_lock lock(m_InternalMutex);
			ApplyLinearImpulseImpl(impulse, point);
		}
		void ApplyAngularImpulse(float force)
		{
			tbb::spin_mutex::scoped_lock lock(m_InternalMutex);
			ApplyAngularImpulseImpl(force);
		}

		//! Returns true
		static bool IsThreadSafe() { return true; }

	protected:
		virtual bool GetInterpolate() const = 0;
		virtual void SetInterpolate(bool interpolate) = 0;

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

	//! Physical fixture interface
	FSN_BEGIN_COIFACE(IFixture)
	public:

		FSN_COIFACE_CTOR(IFixture,
			((FSN_IS_SET)(Sensor))
			((FSN_GET_SET)(Density))
			((FSN_GET_SET)(Friction))
			((FSN_GET_SET)(Restitution))
			((FSN_GET)(AABB)) )

		ThreadSafeProperty<bool> Sensor;
		ThreadSafeProperty<float> Density;
		ThreadSafeProperty<float> Friction;
		ThreadSafeProperty<float> Restitution;
		ThreadSafeProperty<b2AABB, NullWriter<b2AABB>> AABB;

		//ThreadSafeProperty<b2MassData> MassData;

		static void RegisterScriptInterface(asIScriptEngine* engine);

		//! Returns true
		static bool IsThreadSafe() { return true; }

	protected:
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

		//virtual b2MassData GetMassData() const = 0;
	};

	class ICircleShape
	{
	public:
		static std::string GetTypeName() { return "ICircleShape"; }
		virtual ~ICircleShape() {}

		FSN_COIFACE_CTOR(ICircleShape,
			((FSN_GET_SET)(Position))
			((FSN_GET_SET)(Radius)) )

		ThreadSafeProperty<Vector2> Position;
		ThreadSafeProperty<float> Radius;

		//! Returns true
		static bool IsThreadSafe() { return true; }

	protected:
		virtual void SetPosition(const Vector2& center) = 0;
		virtual Vector2 GetPosition() const = 0;

		virtual void SetRadius(float radius) = 0;
		virtual float GetRadius() const = 0;
	};

	class IPolygonShape
	{
	public:
		static std::string GetTypeName() { return "IPolygonShape"; }
		virtual ~IPolygonShape() {}

		FSN_COIFACE_CTOR(IPolygonShape,
			((FSN_GET)(Radius)) )

		ThreadSafeProperty<float, NullWriter<float>> Radius;

		//! Returns true
		static bool IsThreadSafe() { return true; }

		void SetAsBox(float half_width, float half_height)
		{
			CL_MutexSection lock(&m_InternalMutex);
			SetAsBoxImpl(half_width, half_height);
		}
		void SetAsBox(float half_width, float half_height, const Vector2& center, float angle)
		{
			CL_MutexSection lock(&m_InternalMutex);
			SetAsBoxImpl(half_width, half_height, center, angle);
		}
		void SetAsEdge(const Vector2 &v1, const Vector2 &v2)
		{
			CL_MutexSection lock(&m_InternalMutex);
			SetAsEdgeImpl(v1, v2);
		}

	private:
		CL_Mutex m_InternalMutex;

	protected:
		virtual float GetRadius() const = 0;

		virtual void SetAsBoxImpl(float half_width, float half_height) = 0;
		virtual void SetAsBoxImpl(float half_width, float half_height, const Vector2& center, float angle) = 0;
		virtual void SetAsEdgeImpl(const Vector2 &v1, const Vector2 &v2) = 0;
	};

}

#endif
