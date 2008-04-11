/*
  Copyright (c) 2006-2007 Fusion Project Team

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

#ifndef Header_FusionEngine_PhysicsScriptTypes
#define Header_FusionEngine_PhysicsScriptTypes

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include "FusionPhysicsShape.h"
#include "FusionPhysicsBody.h"
#include "FusionPhysicsWorld.h"

#include "FusionRefCounted.h"

#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{

	void PhysBodyConstructor(PhysicsBody* obj)
	{
		new(obj) PhysicsBody();
	}

	void PhysBodyConstructor_World(PhysicsWorld* world, PhysicsBody* obj)
	{
		new(obj) PhysicsBody(world);
	}

	// Wrapper methods provider easier debugging
	void PhysBody_SetMass(float m, PhysicsBody* lhs)
	{
		lhs->SetMass(m);
	}

	void PhysBody_SetPosition(float x, float y, PhysicsBody* lhs)
	{
		lhs->_setPosition(Vector2(x, y));
	}

	void PhysBody_GetPosition(Vector2 &out, PhysicsBody* lhs)
	{
		out = lhs->GetPosition();
	}

	void PhysBody_ApplyForceRelative(float f, PhysicsBody* lhs)
	{
		lhs->ApplyForceRelative(f);
	}


	static void registerPhysBodyMethods(asIScriptEngine* engine)
	{
		int r;
		//r = engine->RegisterObjectBehaviour("Body", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(PhysBodyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		//r = engine->RegisterObjectMethod("Body", "void set_world(World)", asMETHOD(PhysicsBody, SetWorld), asCALL_THISCALL); assert( r >= 0 );
		//r = engine->RegisterObjectMethod("Body", "void set_mass(float)", asMETHOD(PhysicsBody, SetMass), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "void set_mass(float)", asFUNCTION(PhysBody_SetMass), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "void attach_shape(CircleShape@)", asMETHOD(PhysicsBody, AttachShape), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "void set_position(float, float)", asFUNCTION(PhysBody_SetPosition), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "void get_position(Vector &out)", asFUNCTIONPR(PhysBody_GetPosition,(void),Vector2), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "const Vector& get_position()", asMETHOD(PhysicsBody,GetPosition), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "void apply_force(Vector &in)", asMETHOD(PhysicsBody, ApplyForce), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Body", "const Vector& get_force()", asMETHOD(PhysicsBody,GetForce), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "void apply_thrust(float)", asFUNCTION(PhysBody_ApplyForceRelative), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("Body", "const Vector& get_velocity()", asMETHOD(PhysicsBody,GetVelocity), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Body", "void set_rotational_velocity(float)", asMETHOD(PhysicsBody, SetRotationalVelocity), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Body", "float get_rotational_velocity()", asMETHOD(PhysicsBody, GetRotationalVelocity), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Body", "float get_angle()", asMETHOD(PhysicsBody, GetRotation), asCALL_THISCALL); assert(r >= 0);
	}

	void ConstructCircleShape(PhysicsBody *o, float centre, float outer, CircleShape *obj)
	{
		new(&obj) CircleShape(o, centre, outer, Vector2::ZERO);
	}

	void ConstructCircleShapeOffset(PhysicsBody *o, float centre, float outer, float offset_x, float offset_y, CircleShape *obj)
	{
		new(&obj) CircleShape(o, centre, outer, Vector2(offset_x, offset_y));
	}

	CircleShape* CircleShapeFactory(PhysicsBody *body, float centre, float outer)
	{
		return new CircleShape(body, centre, outer, Vector2::ZERO);
	}

	static void registerPhysShapeMethods(asIScriptEngine* engine)
	{
		int r;
		//r = engine->RegisterObjectBehaviour("CircleShape", asBEHAVE_CONSTRUCT, "void f(Body@,float,float,float,float)", asFUNCTIONPR(ConstructCircleShape, (PhysicsBody*, float, float, float, float), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		//r = engine->RegisterObjectBehaviour("CircleShape", asBEHAVE_CONSTRUCT, "void f(Body@,float,float)", asFUNCTION(ConstructCircleShape), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("CircleShape", asBEHAVE_FACTORY, "CircleShape@ factory(Body@,float,float)", asFUNCTION(CircleShapeFactory), asCALL_CDECL); assert( r >= 0 );
	}

	PhysicsBody* PhysWorld_CreateBody(PhysicsWorld* lhs)
	{
		return lhs->CreateBody(0);
	}

	void PhysicsWorld_ListBodies(PhysicsWorld* lhs)
	{
		const BodyList& bodies = lhs->GetBodies();

		std::string output = CL_String::format("Active bodies: (%1)\n", (int)bodies.size());
		for (BodyList::const_iterator it = bodies.begin(), end = bodies.end(); it != end; ++it)
		{
			PhysicsBody* body = (*it);
			const Vector2& p = body->GetPosition();
			
			output += CL_String::format("\t ( %1 , %2 )", p.x, p.y);
		}
		SendToConsole(output);
	}

	static void registerPhysWorldMethods(asIScriptEngine* engine)
	{
		int r;
		//r = engine->RegisterObjectMethod("World", "Body create_body()", asFUNCTION(PhysWorld_CreateBody), asCALL_CDECL_OBJLAST);  assert( r >= 0 );
		r = engine->RegisterObjectMethod("World", "void attach_body(Body@)", asMETHOD(PhysicsWorld,AddBody), asCALL_THISCALL);  assert( r >= 0 );
		r = engine->RegisterObjectMethod("World", "void list_bodies()", asFUNCTION(PhysicsWorld_ListBodies), asCALL_CDECL_OBJLAST);  assert( r >= 0 );
		r = engine->RegisterObjectMethod("World", "void enable_wraparound()", asMETHOD(PhysicsWorld,ActivateWrapAround), asCALL_THISCALL);  assert( r >= 0 );
		r = engine->RegisterObjectMethod("World", "void disable_wraparound()", asMETHOD(PhysicsWorld,DeactivateWrapAround), asCALL_THISCALL);  assert( r >= 0 );
	}

	// NB: Virtual functions may cause errors
	static void RegisterPhysicsTypes(asIScriptEngine* engine)
	{
		//RegisterType<PhysicsBody>("Body", engine);
		PhysicsBody::registerType<PhysicsBody>(engine, "Body");
		//RegisterTypePOD<Shape>("Shape", engine);
		//Shape::registerType<Shape>(engine, "Shape");
		//RegisterType<CircleShape>("CircleShape", engine);
		CircleShape::registerType<CircleShape>(engine, "CircleShape");
		//PhysicsWorld::registerType(engine);
		//RegisterTypeNoHandle<PhysicsWorld>("World", engine);
		RegisterTypePOD<PhysicsWorld>("World", engine);

		registerPhysBodyMethods(engine);
		registerPhysShapeMethods(engine);
		registerPhysWorldMethods(engine);
	}

}

#endif