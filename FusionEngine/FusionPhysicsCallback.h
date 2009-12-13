/*
  Copyright (c) 2006-2009 Fusion Project Team

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
//#include <boost/function.hpp>
//#include <boost/bind.hpp>

#include "FusionPhysicsShape.h"

namespace FusionEngine
{

	//! Collision contact point
	class Contact
	{
	public:
		Vector2Array m_Points;
		Vector2 m_Normal;

		FixturePtr m_FixtureA;
		FixturePtr m_FixtureB;

	public:
		Contact()
		{
		}

		Contact(const Vector2Array& p, const Vector2& n, const FixturePtr &shape1, const FixturePtr &shape2)
		{
			m_Points = p;
			m_Normal = n;

			m_FixtureA = shape1;
			m_FixtureB = shape2;
		}

		static Contact CreateContact(b2Contact *contact)
		{
			b2Manifold *manifold = contact->GetManifold();

			b2WorldManifold worldManifold;
			contact->GetWorldManifold(&worldManifold);

			//PhysicalEntity *entityA = static_cast<PhysicalEntity*>( contact->GetFixtureA()->GetBody()->GetUserData() );
			//PhysicalEntity *entityB = static_cast<PhysicalEntity*>( contact->GetFixtureB()->GetBody()->GetUserData() );

			Fixture *fixtureA = static_cast<Fixture*>( contact->GetFixtureA()->GetUserData() );
			Fixture *fixtureB = static_cast<Fixture*>( contact->GetFixtureB()->GetUserData() );

			Contact fsnContact;

			FixturePtr fa;
			FixturePtr fb;
			if (fixtureA != NULL)
				fa.reset(fixtureA);
			if (fixtureB != NULL)
				fa.reset(fixtureB);
				
			fsnContact.SetFixtureA( fa );
			fsnContact.SetFixtureB( fb );

			fsnContact.SetNormal( b2v2(worldManifold.m_normal) );

			fsnContact.m_Points.reserve(manifold->m_pointCount);
			for (int32 i = 0; i < manifold->m_pointCount; ++i)
			{
				fsnContact.m_Points.push_back( b2v2(worldManifold.m_points[i]) );
			}

			return fsnContact;
		}

	public:
		void SetPoints(const Vector2Array& p)
		{
			m_Points = p;
		}

		void SetNormal(const Vector2& n)
		{
			m_Normal = n;
		}

		void SetFixtureA(const FixturePtr& fixture)
		{
			m_FixtureA = fixture;
		}

		void SetFixtureB(const FixturePtr& fixture)
		{
			m_FixtureB = fixture;
		}

		const Vector2Array& GetPoints() const
		{
			return m_Points;
		}

		const Vector2& GetNormal() const
		{
			return m_Normal;
		}

		FixturePtr GetFixtureA() const
		{
			return m_FixtureA;
		}

		FixturePtr GetFixtureB() const
		{
			return m_FixtureB;
		}


		void SetShape1(const FixturePtr& s1)
		{
			m_FixtureA = s1;
		}

		void SetShape2(const FixturePtr& s2)
		{
			m_FixtureB = s2;
		}

		FixturePtr GetShape1() const
		{
			return m_FixtureA;
		}

		FixturePtr GetShape2() const
		{
			return m_FixtureB;
		}
	};

	//! Classes may impliment this to become collision handlers.
	class ICollisionHandler
	{
	public:
		//! Return true if collision checks should be preformed on the passed body.
		//virtual bool CanCollideWith(PhysicsBodyPtr other) =0;
		//! Called on collision with the given body.
		virtual void ContactBegin(const Contact &contact) =0;
		//! Called while a contact persists
		virtual void ContactPersist(const Contact &contact) =0;
		//! Called when a collision ends
		virtual void ContactEnd(const Contact &contact) =0;
	};

}

#endif
