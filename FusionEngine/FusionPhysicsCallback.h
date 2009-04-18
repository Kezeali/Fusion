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

#include "FusionPhysicsShape.h"

namespace FusionEngine
{

	//! Collision contact point
	class Contact
	{
	private:
		Vector2 m_Velocity;
		Vector2 m_Position;
		Vector2 m_Normal;
		ShapePtr m_Shape1;
		ShapePtr m_Shape2;

	public:
		Contact(const b2ContactPoint* contact , const ShapePtr &shape1, const ShapePtr &shape2)
		{
			m_Velocity.x = contact->velocity.x;
			m_Velocity.y = contact->velocity.y;

			m_Position.x = contact->position.x;
			m_Position.y = contact->position.y;

			m_Normal.x = contact->normal.x;
			m_Normal.y = contact->normal.y;

			m_Shape1 = shape1;
			m_Shape2 = shape2;
		}

		Contact(const Vector2& v, const Vector2& p, const Vector2& n, const ShapePtr &shape1, const ShapePtr &shape2)
		{
			m_Velocity = v;
			m_Position = p;
			m_Normal = n;

			m_Shape1 = shape1;
			m_Shape2 = shape2;
		}

	public:
		void SetVelocity(const Vector2& v)
		{
			m_Velocity = v;
		}

		void SetPosition(const Vector2& p)
		{
			m_Position = p;
		}

		void SetNormal(const Vector2& n)
		{
			m_Normal = n;
		}

		void SetShape1(const ShapePtr& s1)
		{
			m_Shape1 = s1;
		}

		void SetShape2(const ShapePtr& s2)
		{
			m_Shape2 = s2;
		}

		const Vector2& GetVelocity() const
		{
			return m_Velocity;
		}

		const Vector2& GetPosition() const
		{
			return m_Position;
		}

		const Vector2& GetNormal() const
		{
			return m_Normal;
		}

		ShapePtr GetShape1() const
		{
			return m_Shape1;
		}

		ShapePtr GetShape2() const
		{
			return m_Shape2;
		}
	};

	//! Classes may impliment this to become collision handlers.
	class ICollisionHandler
	{
	public:
		//! Return true if collision checks should be preformed on the passed body.
		virtual bool CanCollideWith(const PhysicsBody *other) =0;
		//! Called on collision with the given body.
		virtual void ContactBegin(const Contact &contact) =0;
		//! Called while a contact persists
		virtual void ContactPersist(const Contact &contact) =0;
		//! Called when a collision ends
		virtual void ContactEnd(const Contact &contact) =0;
	};

}

#endif
