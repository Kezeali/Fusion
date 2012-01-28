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

#ifndef H_FusionScriptCollisionEvent
#define H_FusionScriptCollisionEvent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionRefCounted.h"

#include <Box2D/Box2D.h>

namespace FusionEngine
{

	class ScriptCollisionEvent : public RefCounted
	{
	public:
		static ScriptCollisionEvent* FromContact(b2Contact* contact)
		{
			auto e = new ScriptCollisionEvent();
			return e;
		}

		ScriptCollisionEvent()
			: m_Sensor(false),
			m_Touching(false)
		{}

		bool IsTouching() const { return m_Touching; }

		Box2DBody* GetOtherBody() const
		{
			m_OtherBody->addRef();
			return m_OtherBody.get();
		}

		Box2DFixture* GetOtherFixture() const
		{
			m_OtherFixture->addRef();
			return m_OtherFixture.get();
		}

		std::weak_ptr<Entity> GetOtherEntity() const
		{
			return m_OtherEntity;
		}

		static void Register(asIScriptEngine* engine);


		bool m_Sensor;

		bool m_Touching;

		boost::intrusive_ptr<Box2DBody> m_OtherBody;
		boost::intrusive_ptr<Box2DFixture> m_OtherFixture;
		std::weak_ptr<Entity> m_OtherEntity;

	};

	inline void ScriptCollisionEvent::Register(asIScriptEngine* engine)
	{
		RegisterType<ScriptCollisionEvent>(engine, "ScrCollisionEvent");
		int r;
		//r = engine->RegisterEnum("CollisionType"); FSN_ASSERT(r >= 0);
		//r = engine->RegisterEnumValue("CollisionType", "Sensor", ); FSN_ASSERT(r >= 0);
		//r = engine->RegisterEnumValue("CollisionType", "Solid", ); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("ScrCollisionEvent", "bool isTouching() const", asMETHOD(ScriptCollisionEvent, IsTouching), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("ScrCollisionEvent", "IRigidBody@ get_body() const", asMETHOD(ScriptCollisionEvent, GetOtherBody), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("ScrCollisionEvent", "IFixture@ get_fixture() const", asMETHOD(ScriptCollisionEvent, GetOtherFixture), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("ScrCollisionEvent", "EntityW get_entity() const", asMETHOD(ScriptCollisionEvent, GetOtherEntity), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

}

#endif
