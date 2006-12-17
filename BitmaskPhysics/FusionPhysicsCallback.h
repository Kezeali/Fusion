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

#include "FusionEngineCommon.h"

namespace FusionEngine
{
	typedef boost::function<void (const FusionPhysicsBody*, const CL_Vector2&)> CollisionCallback;

	//! Returns a CollisionCallback corrosponding to the given member function
	/*!
	 * \param instance The object from which to call this function.
	 * \param method The member function to call from the object given in the first arg.
	 *
	 * \remarks
	 * Generally, just calling boost::bind(&MyClass::MyMethod, myobject, _1, _2); is
	 * just as easy, so this function isn't very useful...
	 */
	template <class T>
	CollisionCallback CreateCCB(T *instance, void(T::*method)(const FusionPhysicsBody*, const CL_Vector2&))
	{
		return boost::bind(method, instance, _1, _2);
	}

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
	//class CollisionCallback : public Callback2<T*, const FusionPhysicsBody*, const CL_Vector2&>
	//{
	//	//! Constructor
	//	CollisionCallback(FusionShip* instance, void(T::*member)(const FusionPhysicsBody*, const CL_Vector2&))
	//		: Callback2(instance, member)
	//	{
	//	}
	//};

	//template <class T>
	//struct CollisionCallback
	//{
	//	boost::function<void (T*, const FusionPhysicsBody*, const CL_Vector2&)> m_CallBack;
	//}

	////! Abstract base class for callbacks
	//class PhysicsFunctor
	//{
	//public:
	//	//! Use like this: (*mycallback)(myparam);
	//	virtual void operator()(const FusionPhysicsBody *other)=0;
	//	virtual void operator()(const FusionPhysicsBody *other, const CL_Vector2 &collision_point)=0;
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
	//	virtual void operator()(const FusionPhysicsBody *other, const CL_Vector2 &collision_point)
	//	{
	//		// Execute the callback method of the instance
	//		(*m_Instance.*m_FuncPtr)(other);
	//	};

	//private:
	//	//! Pointer to member function (of class T)
	//	void (T::*m_FuncPtr)(const FusionPhysicsBody *other);
	//	//! Pointer to member function (of class T)
	//	void (T::*m_FuncPtr)(const FusionPhysicsBody *other, const CL_Vector2 &collision_point);
	//	//! Pointer to object
	//	T *m_Instance;
	//};

#endif
