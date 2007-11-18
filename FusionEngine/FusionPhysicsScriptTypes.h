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

	void PhysBodyConstructor(PhysicsWorld* world, PhysicsBody* obj)
	{
		new(obj) PhysicsBody(world);
	}

	void PhysBody_SetPosition(float x, float y, PhysicsBody* lhs)
	{
		lhs->_setPosition(Vector2(x, y));
	}


	static void registerPhysBodyMethods(asIScriptEngine* engine)
	{
		int r;
		r = engine->RegisterObjectBehaviour("Body", asBEHAVE_CONSTRUCT, "void f(World@)", asFUNCTIONPR(PhysBodyConstructor, (PhysicsWorld*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "void set_mass(float)", asMETHOD(PhysicsBody, SetMass), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "void attach_shape(Shape@)", asMETHOD(PhysicsBody, AttachShape), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Body", "void set_position(float, float)", asFUNCTION(PhysBody_SetPosition), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	}

	void ConstructCircleShape(PhysicsBody *o, float centre, float outer, CircleShape *obj)
	{
		new(&obj) CircleShape(o, centre, outer, Vector2::ZERO);
	}

	void ConstructCircleShape(PhysicsBody *o, float centre, float outer, float offset_x, float offset_y, CircleShape *obj)
	{
		new(&obj) CircleShape(o, centre, outer, Vector2(offset_x, offset_y));
	}

	static void registerPhysShapeMethods(asIScriptEngine* engine)
	{
		int r;
		r = engine->RegisterObjectBehaviour("CircleShape", asBEHAVE_CONSTRUCT, "void f(Body@,float,float,float,float)", asFUNCTIONPR(ConstructCircleShape, (PhysicsBody*, float, float, float, float, CircleShape*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("CircleShape", asBEHAVE_CONSTRUCT, "void f(Body@,float,float)", asFUNCTIONPR(ConstructCircleShape, (PhysicsBody*, float, float, CircleShape*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	}

	PhysicsBody* PhysWorld_CreateBody(PhysicsWorld* lhs)
	{
		return lhs->CreateBody(0);
	}

	static void registerPhysWorldMethods(asIScriptEngine* engine)
	{
		int r;
		r = engine->RegisterObjectMethod("World", "void create_body()", asFUNCTION(PhysWorld_CreateBody), asCALL_CDECL_OBJLAST);  assert( r >= 0 );
		r = engine->RegisterObjectMethod("World", "void attach_body(Body@)", asMETHOD(PhysicsWorld,AddBody), asCALL_THISCALL);  assert( r >= 0 );
	}

	static void RegisterPhysicsTypes(asIScriptEngine* engine)
	{
		PhysicsBody::registerType<PhysicsBody>(engine, "Body");
		Shape::registerType<Shape>(engine, "Shape");
		CircleShape::registerType<CircleShape>(engine, "CircleShape");
		//PhysicsWorld::registerType(engine);
		RegisterTypeNoHandle<PhysicsWorld>("World", engine);

		registerPhysBodyMethods(engine);
		registerPhysShapeMethods(engine);
		registerPhysWorldMethods(engine);
	}

}

#endif