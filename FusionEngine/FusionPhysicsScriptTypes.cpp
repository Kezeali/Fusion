/*
  Copyright (c) 2009 Fusion Project Team

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

		
	File Author(s):

		Elliot Hayward
*/

#include "Common.h"

#include "FusionPhysicsScriptTypes.h"

#include "FusionPhysicsShape.h"
#include "FusionScriptedEntity.h"

#include "FusionRefCounted.h"

#include "FusionScriptTypeRegistrationUtils.h"


namespace FusionEngine
{

	class ScriptFixtureDef : public RefCounted
	{
	public:
		b2FixtureDef *fixtureDefinition;

		ScriptFixtureDef(b2FixtureDef *def)
			: fixtureDefinition(def)
		{}

		virtual ~ScriptFixtureDef()
		{}

		void OnNoReferences()
		{
			delete fixtureDefinition;
			delete this;
		}

		void SetFriction(float friction)
		{
			fixtureDefinition->friction = friction;
		}

		float GetFriction()
		{
			return fixtureDefinition->friction;
		}

		void SetRestitution(float restitution)
		{
			fixtureDefinition->restitution = restitution;
		}

		float GetRestitution()
		{
			return fixtureDefinition->restitution;
		}

		void SetDensity(float density)
		{
			fixtureDefinition->density = density;
		}

		float GetDensity()
		{
			return fixtureDefinition->density;
		}

		void SetSensor(bool isSensor)
		{
			fixtureDefinition->isSensor = isSensor;
		}

		bool IsSensor()
		{
			return fixtureDefinition->isSensor;
		}
	};

	class ScriptCircleFixtureDef : public ScriptFixtureDef
	{
	public:
		ScriptCircleFixtureDef(b2CircleDef *obj)
			: ScriptFixtureDef(obj)
		{}

		virtual ~ScriptCircleFixtureDef()
		{}

		void SetSimLocalPosition(const Vector2 &position)
		{
			static_cast<b2CircleDef*>( fixtureDefinition )->localPosition.Set(position.x, position.y);
		}

		Vector2* GetSimLocalPosition()
		{
			const b2Vec2 &pos = static_cast<b2CircleDef*>( fixtureDefinition )->localPosition;
			return new Vector2(pos.x, pos.y);
		}

		void SetLocalPosition(const Vector2 &position)
		{
			static_cast<b2CircleDef*>( fixtureDefinition )->localPosition.Set(ToSimUnits(position.x) , ToSimUnits(position.y));
		}

		Vector2* GetLocalPosition()
		{
			const b2Vec2 &pos = static_cast<b2CircleDef*>( fixtureDefinition )->localPosition;
			return new Vector2(ToGameUnits(pos.x), ToGameUnits(pos.y));
		}

		void SetSimRadius(float radius)
		{
			static_cast<b2CircleDef*>( fixtureDefinition )->radius = radius;
		}

		float GetSimRadius()
		{
			return static_cast<b2CircleDef*>( fixtureDefinition )->radius;
		}

		void SetRadius(float radius)
		{
			static_cast<b2CircleDef*>( fixtureDefinition )->radius = ToSimUnits(radius);
		}

		float GetRadius()
		{
			return ToGameUnits( static_cast<b2CircleDef*>( fixtureDefinition )->radius );
		}
	};

	ScriptCircleFixtureDef * ScriptCircleFixtureDef_Constructor()
	{
		return new ScriptCircleFixtureDef(new b2CircleDef());
	}

	//void PhysBodyConstructor(PhysicsBody* obj)
	//{
	//	new(obj) PhysicsBody();
	//}

	//PhysicsBody* Body_Factory()
	//{
	//	return new PhysicsBody();
	//}

	// Wrapper methods provider easier debugging
	//void PhysBody_SetPosition(float x, float y, PhysicsBody* lhs)
	//{
	//	lhs->_setPosition(Vector2(x, y));
	//}

	//void PhysBody_GetPosition(Vector2 &out, PhysicsBody* lhs)
	//{
	//	ToGameUnits(out, lhs->GetSimulationPosition());
	//}

	//void PhysBody_ApplyForceRelative(float force, PhysicsBody* lhs)
	//{
	//	lhs->ApplyForceRelative(Vector2(0.f, force));
	//}

	void ScriptedEntity_ApplyForce(const Vector2 &force, ScriptedEntity *obj)
	{
		b2Body *body = obj->GetBody();
		b2Vec2 worldForce = body->GetWorldVector(b2Vec2(force.x, force.y));
		body->ApplyForce(worldForce, body->GetWorldCenter());
	}

	void ScriptedEntity_ApplyTorque(float torque, ScriptedEntity *obj)
	{
		obj->GetBody()->ApplyTorque(torque);
	}

	b2Fixture *ScriptedEntity_CreateFixture(ScriptFixtureDef *def, ScriptedEntity *obj)
	{
		b2Fixture *fixture = obj->GetBody()->CreateFixture(def->fixtureDefinition);
		return fixture;
	}

	b2Fixture *ScriptedEntity_CreateCircleFixture(const Vector2 &offset, float radius, ScriptedEntity *obj)
	{
		return obj->GetBody()->CreateFixture( DefineCircleFixture(radius, offset).get() );
	}

	void registerPhysBodyMethods(asIScriptEngine* engine)
	{
		int r;
		//r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_FACTORY, "Body@ f()", asFUNCTION(Body_Factory), asCALL_CDECL); FSN_ASSERT( r >= 0 );

		//r = engine->RegisterObjectMethod("ScriptedEntity", "void setMass(float)", asFUNCTION(PhysBody_SetMass), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("ScriptedEntity", "void applyForce(Vector &in)", asFUNCTION(ScriptedEntity_ApplyForce), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		//r = engine->RegisterObjectMethod("ScriptedEntity", "void applyThrust(float)", asFUNCTION(PhysBody_ApplyForceRelative), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("ScriptedEntity", "void applyTorque(float)", asFUNCTION(ScriptedEntity_ApplyTorque), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("ScriptedEntity", "Fixture@ createFixture(FixtureDef@)", asFUNCTION(ScriptedEntity_CreateFixture), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("ScriptedEntity", "Fixture@ createCircleFixture(const Vector &in, float)", asFUNCTION(ScriptedEntity_CreateCircleFixture), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

	void FixtureDefinition_Constructor(FixtureDefinition *ptr)
	{
		new(&ptr) FixtureDefinition(new b2FixtureDef());
	}

	void FixtureDefinition_Destructor(FixtureDefinition *obj)
	{
		obj->~shared_ptr();
	}

	FixtureDefinition* FixtureDefinition_Assign(const FixtureDefinition& copy, FixtureDefinition *obj)
	{
		*obj = copy;
		return obj;
	}

	void FixtureDefinition_SetFriction(float friction, FixtureDefinition *obj)
	{
		(*obj)->friction = friction;
	}

	void FixtureDefinition_SetRestitution(float restitution, FixtureDefinition *obj)
	{
		(*obj)->restitution = restitution;
	}

	void FixtureDefinition_SetDensity(float density, FixtureDefinition *obj)
	{
		(*obj)->density = density;
	}

	float FixtureDefinition_GetFriction(FixtureDefinition *obj)
	{
		return (*obj)->friction;
	}

	float FixtureDefinition_GetRestitution(FixtureDefinition *obj)
	{
		return (*obj)->restitution;
	}

	float FixtureDefinition_GetDensity(FixtureDefinition *obj)
	{
		return (*obj)->density;
	}

	FixtureDefinition CastToFixtureDefinition(FixtureDefinition *obj)
	{
		b2FixtureDef *copy = new b2FixtureDef(*obj->get());
		return FixtureDefinition(copy);
	}

	void CircleFixtureDefinition_Constructor(FixtureDefinition *ptr)
	{
		new(&ptr) FixtureDefinition(new b2CircleDef());
	}

	void CircleFixtureDefinition_Destructor(FixtureDefinition *obj)
	{
		obj->~shared_ptr();
	}

	FixtureDefinition* CircleFixtureDefinition_Assign(const FixtureDefinition& copy, FixtureDefinition *obj)
	{
		FSN_ASSERT(dynamic_cast<b2CircleDef*>(copy.get()) != NULL);
		*obj = copy;
		return obj;
	}

	void CircleFixtureDefinition_SetLocalPosition(const Vector2 &position, FixtureDefinition *obj)
	{
		b2CircleDef *circleDef = static_cast<b2CircleDef*>( (*obj).get() );

		circleDef->localPosition.x = position.x;
		circleDef->localPosition.y = position.y;
	}

	void CircleFixtureDefinition_SetRadius(float radius, FixtureDefinition *obj)
	{
		b2CircleDef *circleDef = static_cast<b2CircleDef*>( (*obj).get() );

		circleDef->radius = radius;
	}

	Vector2* CircleFixtureDefinition_GetLocalPosition(FixtureDefinition *obj)
	{
		b2CircleDef *circleDef = static_cast<b2CircleDef*>( (*obj).get() );

		return new Vector2(circleDef->localPosition.x, circleDef->localPosition.y);
	}

	float CircleFixtureDefinition_GetRadius(FixtureDefinition *obj)
	{
		b2CircleDef *circleDef = static_cast<b2CircleDef*>( (*obj).get() );

		return circleDef->radius;
	}

	void registerFixtureDefMethods(asIScriptEngine* engine, const char *class_name)
	{
		int r;
		r = engine->RegisterObjectMethod(class_name, "void setFriction(float)", asMETHOD(ScriptFixtureDef,SetFriction), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "void setRestitution(float)", asMETHOD(ScriptFixtureDef,SetRestitution), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "void setDensity(float)", asMETHOD(ScriptFixtureDef,SetDensity), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "void setSensor(bool)", asMETHOD(ScriptFixtureDef,SetSensor), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "float getFriction()", asMETHOD(ScriptFixtureDef,GetFriction), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "float getRestitution()", asMETHOD(ScriptFixtureDef,GetRestitution), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "float getDensity()", asMETHOD(ScriptFixtureDef,GetDensity), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "bool isSensor()", asMETHOD(ScriptFixtureDef,IsSensor), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	}

	void registerCircleFixtureDefMethods(asIScriptEngine* engine, const char *class_name)
	{
		int r;
		r = engine->RegisterObjectMethod(class_name, "void setSimLocalPosition(const Vector &in)", asMETHOD(ScriptCircleFixtureDef,SetLocalPosition), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "void setLocalPosition(const Vector &in)", asMETHOD(ScriptCircleFixtureDef,SetLocalPosition), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "void setSimRadius(float)", asMETHOD(ScriptCircleFixtureDef,SetSimRadius), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "void setRadius(float)", asMETHOD(ScriptCircleFixtureDef,SetRadius), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "Vector@ getSimLocalPosition()", asMETHOD(ScriptCircleFixtureDef,GetSimLocalPosition), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "Vector@ getLocalPosition()", asMETHOD(ScriptCircleFixtureDef,GetLocalPosition), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "float getSimRadius()", asMETHOD(ScriptCircleFixtureDef,GetSimRadius), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(class_name, "float getRadius()", asMETHOD(ScriptCircleFixtureDef,GetRadius), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	}

	void registerFixtureDef(asIScriptEngine* engine)
	{
		int r;

		r = engine->RegisterObjectType("FixtureDef", 0, asOBJ_REF); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("FixtureDef", asBEHAVE_ADDREF, "void f()", asMETHOD(RefCounted, addRef), asCALL_THISCALL);
		r = engine->RegisterObjectBehaviour("FixtureDef", asBEHAVE_RELEASE, "void f()", asMETHOD(RefCounted, release), asCALL_THISCALL);
		r = engine->RegisterObjectBehaviour("FixtureDef", asBEHAVE_ASSIGNMENT, "FixtureDef& op_assign(const FixtureDef &in)", asFUNCTION(RefCounted::Assign<ScriptFixtureDef>), asCALL_CDECL_OBJFIRST);

		registerFixtureDefMethods(engine, "FixtureDef");

		r = engine->RegisterObjectType("CircleFixtureDef", 0, asOBJ_REF); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("CircleFixtureDef", asBEHAVE_ADDREF, "void f()", asMETHOD(RefCounted, addRef), asCALL_THISCALL);
		r = engine->RegisterObjectBehaviour("CircleFixtureDef", asBEHAVE_RELEASE, "void f()", asMETHOD(RefCounted, release), asCALL_THISCALL);

		r = engine->RegisterObjectBehaviour("CircleFixtureDef", asBEHAVE_FACTORY, "CircleFixtureDef@ f()", asFUNCTION(ScriptCircleFixtureDef_Constructor), asCALL_CDECL);
		r = engine->RegisterObjectBehaviour("CircleFixtureDef", asBEHAVE_ASSIGNMENT, "CircleFixtureDef& op_assign(const CircleFixtureDef &in)", asFUNCTION(RefCounted::Assign<ScriptCircleFixtureDef>), asCALL_CDECL_OBJFIRST);

		registerFixtureDefMethods(engine, "CircleFixtureDef");
		registerCircleFixtureDefMethods(engine, "CircleFixtureDef");

		RegisterBaseOf<ScriptFixtureDef, ScriptCircleFixtureDef>(engine, "FixtureDef", "CircleFixtureDef");
	}

	void Fixture_AddRef(b2Fixture *obj)
	{
		Entity *entity = (Entity*)obj->GetBody()->GetUserData();
		SendToConsole("Fixture referenced: (TYPE) on " + entity->ToString());
	}

	void Fixture_Release(b2Fixture *obj)
	{
		Entity *entity = (Entity*)obj->GetBody()->GetUserData();
		SendToConsole("Fixture de-referenced: (TYPE) on " + entity->ToString());
	}

	void RegisterPhysicsTypes(asIScriptEngine* engine)
	{
		int r;
		r = engine->RegisterEnum("ShapeType");
		r = engine->RegisterEnumValue("ShapeType", "unknown", b2_unknownShape);
		r = engine->RegisterEnumValue("ShapeType", "circle", b2_circleShape);
		r = engine->RegisterEnumValue("ShapeType", "polygon", b2_polygonShape);
		r = engine->RegisterEnumValue("ShapeType", "edge", b2_edgeShape);

		r = engine->RegisterObjectType("Fixture", 0, asOBJ_REF); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("Fixture", asBEHAVE_ADDREF, "void f()", asFUNCTION(Fixture_AddRef), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("Fixture", asBEHAVE_RELEASE, "void f()", asFUNCTION(Fixture_Release), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Fixture", "ShapeType getType() const", asMETHOD(b2Fixture, GetType), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Fixture", "bool isSensor() const", asMETHOD(b2Fixture, IsSensor), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Fixture", "void setSensor(bool)", asMETHOD(b2Fixture, SetSensor), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Fixture", "ShapeType getType() const", asMETHOD(b2Fixture, GetType), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		registerFixtureDef(engine);
	}

}
