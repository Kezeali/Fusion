/*
  Copyright (c) 2006 FusionTeam

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

	//! Abstract base class for callbacks
	class PhysicsFunctor
	{
	public:
		//! Use like this: (*mycallback)(myparam);
		virtual void operator()(const FusionPhysicsBody *other)=0;
		virtual void operator()(const FusionPhysicsBody *other, const CL_Vector2 &collision_point)=0;
	};


	//! Template class for callbacks
	/*!
	 * Example code: <br>
	 * Ship *myship = new Ship; <br>
	 * PhysicsCallback<Ship> *m_CollisionResponse =
	 *                         new PhysicsCalback<Ship>(myship, &Ship::OnCollision);
	 * <br>
	 * ...a method wants to tell the ship that a collision has been detected...
	 * <br>
	 * (*m_CollisionResponse)(param); // Callback is used
	 */
	template <class T> class PhysicsCallback : public PhysicsFunctor
	{
	public:
		//! Constructor
		/*!
		 * Takes pointer to an object and pointer to a member (of that object's class)
		 * and stores them in two private variables.
		 *
		 * \param instance A instance of class T (a.k.a. object with type T).
		 * \param method A void pointer to a method, with the param given, in class T.
		 */
		PhysicsCallback(T* instance, void(T::*method)(const FusionPhysicsBody *other))
		{
			m_Instance = instance; 
			m_FuncPtr = method; 
		};

		//! Use like this: (*mycallback)(myparam);
		virtual void operator()(const FusionPhysicsBody *other)
		{
			// Execute the callback method of the instance
			(*m_Instance.*m_FuncPtr)(other);
		};
		//! Use like this: (*mycallback)(myparam, mysecondparam);
		virtual void operator()(const FusionPhysicsBody *other, const CL_Vector2 &collision_point)
		{
			// Execute the callback method of the instance
			(*m_Instance.*m_FuncPtr)(other);
		};

	private:
		//! Pointer to member function (of class T)
		void (T::*m_FuncPtr)(const FusionPhysicsBody *other);
		//! Pointer to member function (of class T)
		void (T::*m_FuncPtr)(const FusionPhysicsBody *other, const CL_Vector2 &collision_point);
		//! Pointer to object
		T *m_Instance;
	};

}

#endif
