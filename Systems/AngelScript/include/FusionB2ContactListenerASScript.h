/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionB2ContactListenerASScript
#define H_FusionB2ContactListenerASScript

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionBox2DContactListener.h"

#include "FusionAngelScriptComponent.h"
#include "FusionBox2DComponent.h"
#include "FusionScriptCollisionEvent.h"

#include <Box2D/Box2D.h>

namespace FusionEngine
{

	class ASScriptB2ContactListener : public Box2DContactListener
	{
	public:
		ASScriptB2ContactListener(ASScript* script)
			: m_Script(script)
		{}

		tbb::concurrent_queue<boost::intrusive_ptr<ScriptCollisionEvent>> m_CollisionEnterEvents;
		tbb::concurrent_queue<boost::intrusive_ptr<ScriptCollisionEvent>> m_CollisionExitEvents;

	private:
		void BeginContact(b2Contact* contact)
		{
			auto bodyComA = static_cast<Box2DBody*>(contact->GetFixtureA()->GetBody()->GetUserData());
			auto bodyComB = static_cast<Box2DBody*>(contact->GetFixtureB()->GetBody()->GetUserData());

			auto fixtureA = static_cast<Box2DFixture*>(contact->GetFixtureA()->GetUserData());
			auto fixtureB = static_cast<Box2DFixture*>(contact->GetFixtureB()->GetUserData());

			if (bodyComA && bodyComB)
			{
				if (bodyComA->GetParent() != m_Script->GetParent() && bodyComB->GetParent() != m_Script->GetParent())
					return;
				else
				{
					if (bodyComB->GetParent() == m_Script->GetParent())
					{
						std::swap(bodyComA, bodyComB);
						std::swap(fixtureA, fixtureB);
					}

					auto ev = boost::intrusive_ptr<ScriptCollisionEvent>(ScriptCollisionEvent::FromContact(contact), false);
					ev->m_OtherBody = bodyComB;
					ev->m_OtherFixture = fixtureB;
					ev->m_OtherEntity = bodyComB->GetParent()->shared_from_this();

					ev->m_Sensor = contact->GetFixtureA()->IsSensor() || contact->GetFixtureB()->IsSensor();

					m_CollisionEnterEvents.push(ev);
				}
			}
		}
		void EndContact(b2Contact* contact)
		{
			auto bodyComA = static_cast<Box2DBody*>(contact->GetFixtureA()->GetBody()->GetUserData());
			auto bodyComB = static_cast<Box2DBody*>(contact->GetFixtureB()->GetBody()->GetUserData());

			auto fixtureA = static_cast<Box2DFixture*>(contact->GetFixtureA()->GetUserData());
			auto fixtureB = static_cast<Box2DFixture*>(contact->GetFixtureB()->GetUserData());

			if (bodyComA && bodyComB)
			{
				if (bodyComA->GetParent() != m_Script->GetParent() && bodyComB->GetParent() != m_Script->GetParent())
					return;
				else
				{
					if (bodyComB->GetParent() == m_Script->GetParent())
					{
						std::swap(bodyComA, bodyComB);
						std::swap(fixtureA, fixtureB);
					}

					auto ev = boost::intrusive_ptr<ScriptCollisionEvent>(ScriptCollisionEvent::FromContact(contact), false);
					ev->m_OtherBody = bodyComB;
					ev->m_OtherFixture = fixtureB;
					ev->m_OtherEntity = bodyComB->GetParent()->shared_from_this();

					ev->m_Sensor = contact->GetFixtureA()->IsSensor() || contact->GetFixtureB()->IsSensor();

					m_CollisionExitEvents.push(ev);
				}
			}
		}
		//void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
		//{}
		//void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
		//{}

		ASScript* m_Script;
	};

}

#endif