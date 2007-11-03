/*
  Copyright (c) 2006 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef Header_FusionEngine_PhysicsCallback
#define Header_FusionEngine_PhysicsCallback

#if _MSC_VER > 1000
#pragma once
#endif

/*!
 * \file FusionCollisionCallback.h
 * This file defines both CollisionCallback and ICollisionHandler, either of which
 * are valid methods for allowing non-physical body classes to be aware of collisions.
 */

#include "FusionCommon.h"

// Boost
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace FusionEngine
{

	//! Collision contact point
	class Contact
	{
	private:
		Vector2 m_Position;
		Vector2 m_Normal;

	public:
		Contact(const cpContact& contact)
		{
			m_Position.x = contact.p.x;
			m_Position.y = contact.p.y;

			m_Normal.x = contact.n.x;
			m_Normal.y = contact.n.y;
		}

		Contact(const Vector2& p, const Vector2& n)
		{
			m_Position = p;
			m_Normal = n;
		}

	public:
		void SetPosioin(const Vector2& v)
		{
			m_Position = v;
		}

		void SetNormal(const Vector2& n)
		{
			m_Normal = n;
		}

		const Vector2& GetPosiion() const
		{
			return m_Position;
		}

		const Vector2& GetNormal() const
		{
			return m_Normal;
		}
	};

	//! [depreciated] Defines a function which can be used as a collision callback.
	typedef boost::function<void (const PhysicsBody*, const std::vector<Contact>&)> CollisionCallback;

	//! [depreciated] Returns a CollisionCallback corrosponding to the given member function.
	/*!
	 * \param instance The object from which to call this function.
	 * \param method The member function to call from the object given in the first arg.
	 *
	 * \remarks
	 * Remarkably, just calling boost::bind(&MyClass::MyMethod, myobject, _1, _2); is
	 * just as easy, so this function isn't very useful...
	 */
	template <class T>
	CollisionCallback CreateCCB(T *instance, void(T::*method)(const PhysicsBody*, const Vector2&))
	{
		return boost::bind(method, instance, _1, _2);
	}

	//! Classes may impliment this to become collision handlers.
	class ICollisionHandler
	{
	public:
		//! Return true if collision checks should be preformed on the passed body.
		virtual bool CanCollideWith(const PhysicsBody *other) =0;
		//! Called on collision with the given body, at the given point.
		virtual void CollisionWith(const PhysicsBody *other, const std::vector<Contact> &contacts) =0;
	};

}

///////////////////////////////////
/// Where failed callback systems go to die:

	//! Generic 2 param callback
	//template <class T, class A1, class A2>
	//class Callback2
	//{
	//public:
	//	//! Constructor
	//	Callback2(T* instance, void(T::*member)(A1, A2))
	//	{
	//		// Create a 2 param function
	//		m_Callback = boost::bind(member, instance, _1, _2);
	//	}

	//public:
	//	//! This allows the callback to be used like a normal boost::function
	//	void operator()(A1 param1, A2 param2) const
	//	{
	//		m_Callback(param1, param2);
	//	}

	//private:
	//	//! The underling boost::function to bind to and call
	//	boost::function<void (A1, A2)> m_CallBack;

	//};

	//! Physics collision response callback type
	//template <class T>
	//class CollisionCallback : public Callback2<T*, const FusionPhysicsBody*, const Vector2&>
	//{
	//	//! Constructor
	//	CollisionCallback(FusionShip* instance, void(T::*member)(const FusionPhysicsBody*, const Vector2&))
	//		: Callback2(instance, member)
	//	{
	//	}
	//};

	//template <class T>
	//struct CollisionCallback
	//{
	//	boost::function<void (T*, const FusionPhysicsBody*, const Vector2&)> m_CallBack;
	//}

	////! Abstract base class for callbacks
	//class PhysicsFunctor
	//{
	//public:
	//	//! Use like this: (*mycallback)(myparam);
	//	virtual void operator()(const FusionPhysicsBody *other)=0;
	//	virtual void operator()(const FusionPhysicsBody *other, const Vector2 &collision_point)=0;
	//};


	////! Template class for callbacks
	///*!
	// * Example code: <br>
	// * Ship *myship = new Ship; <br>
	// * PhysicsCallback<Ship> *m_CollisionResponse =
	// *                         new PhysicsCalback<Ship>(myship, &Ship::OnCollision);
	// * <br>
	// * ...a method wants to tell the ship that a collision has been detected...
	// * <br>
	// * (*m_CollisionResponse)(param); // Callback is used
	// */
	//template <class T> class PhysicsCallback : public PhysicsFunctor
	//{
	//public:
	//	//! Constructor
	//	/*!
	//	 * Takes pointer to an object and pointer to a member (of that object's class)
	//	 * and stores them in two private variables.
	//	 *
	//	 * \param instance A instance of class T (a.k.a. object with type T).
	//	 * \param method A void pointer to a method, with the param given, in class T.
	//	 */
	//	PhysicsCallback(T* instance, void(T::*method)(const FusionPhysicsBody *other))
	//	{
	//		m_Instance = instance; 
	//		m_FuncPtr = method; 
	//	};

	//	//! Use like this: (*mycallback)(myparam);
	//	virtual void operator()(const FusionPhysicsBody *other)
	//	{
	//		// Execute the callback method of the instance
	//		(*m_Instance.*m_FuncPtr)(other);
	//	};
	//	//! Use like this: (*mycallback)(myparam, mysecondparam);
	//	virtual void operator()(const FusionPhysicsBody *other, const Vector2 &collision_point)
	//	{
	//		// Execute the callback method of the instance
	//		(*m_Instance.*m_FuncPtr)(other);
	//	};

	//private:
	//	//! Pointer to member function (of class T)
	//	void (T::*m_FuncPtr)(const FusionPhysicsBody *other);
	//	//! Pointer to member function (of class T)
	//	void (T::*m_FuncPtr)(const FusionPhysicsBody *other, const Vector2 &collision_point);
	//	//! Pointer to object
	//	T *m_Instance;
	//};

#endif
