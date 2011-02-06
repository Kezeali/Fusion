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


	File Author(s):

		Elliot Hayward
*/

#include "FusionStableHeaders.h"

#include "FusionScriptVector.h"

namespace FusionEngine { namespace Scripting
{
	////////////////
	// Construction
	ScriptVector::ScriptVector(const ScriptVector &other)
		: m_RefCount(1)
	{
		Data = other.Data;
	}

	ScriptVector::ScriptVector(float x, float y)
		: m_RefCount(1)
	{
		Data.x = x; Data.y = y;
	}

	ScriptVector::ScriptVector(const Vector2 &other)
		: m_RefCount(1),
		Data(other)
	{
	}

	ScriptVector::~ScriptVector()
	{
		FSN_ASSERT( m_RefCount == 0 );
	}

	//////////////////////
	// Reference counting
	void ScriptVector::AddRef()
	{
		m_RefCount++;
	}

	void ScriptVector::Release()
	{
		if ( --m_RefCount == 0 )
			delete this;
	}


	///////////////////
	// Operations
	// Assignment
	ScriptVector &ScriptVector::operator=(const ScriptVector &other)
	{
		Data = other.Data;

		// Return a ref. to this
		return *this;
	}

	ScriptVector &ScriptVector::operator+=(const ScriptVector &other)
	{
		Data += other.Data;
		return *this;
	}

	// Arithmatic
	Vector2 *opAdd(const Vector2 &a, const Vector2 &b)
	{
		Vector2 *sum = new Vector2();
		v2Add(a, b, *sum);
		// Return a new object as a script handle
		return sum;
	}

	///////
	// Mem
	static Vector2 *VectorDefaultFactory()
	{
		return new Vector2();
	}

	static Vector2 *VectorInitFactory(float x, float y)
	{
		return new Vector2(x, y);
	}

	static Vector2 *VectorCopyFactory(const Vector2 &other)
	{
		return new Vector2(other);
	}

	////////////////////////////////
	// AngelScript type registration
	int RegisterScriptVector_Native(asIScriptEngine *engine)
	{
		int r, typeId;

		// Register the type
		r = engine->RegisterObjectType("Vector", sizeof(Vector2), asOBJ_REF); FSN_ASSERT( r >= 0 );
		typeId = engine->GetTypeIdByDecl("Vector");

		// Register factory methods
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f()",                 asFUNCTION(VectorDefaultFactory), asCALL_CDECL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f(float, float)",     asFUNCTION(VectorInitFactory), asCALL_CDECL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f(const Vector &in)", asFUNCTION(VectorCopyFactory), asCALL_CDECL); FSN_ASSERT( r >= 0 );
		
		// Register the object operator overloads
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_ADDREF,     "void f()",                    asMETHOD(Vector2, addRef), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_RELEASE,    "void f()",                    asMETHOD(Vector2, release), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "Vector &opAssign(const Vector &in)", asMETHODPR(Vector2, operator =, (const Vector2&), Vector2&), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "Vector &opAddAssign(const Vector &in)", asMETHODPR(Vector2, operator+=, (const Vector2&), Vector2&), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "Vector@ opAdd(const Vector &in)", asFUNCTION(opAdd), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );

		// Register the object methods
		r = engine->RegisterObjectMethod("Vector", "float length() const", asMETHOD(Vector2,length), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Vector", "float get_x() const", asMETHOD(Vector2,get_x), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "float get_y() const", asMETHOD(Vector2,get_y), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Vector", "void set_x(float)", asMETHOD(Vector2,get_x), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "void set_y(float)", asMETHOD(Vector2,get_y), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		return typeId;
	}

	int RegisterScriptVector(asIScriptEngine *engine)
	{
		int r;
		r = engine->RegisterEnum("PointOrigin");
		r = engine->RegisterEnumValue("PointOrigin", "top_left", origin_top_left); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "top_center", origin_top_center); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "top_right", origin_top_right); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "center_left", origin_center_left); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "center", origin_center); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "center_right", origin_center_right); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "bottom_left", origin_bottom_left); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "bottom_center", origin_bottom_center); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "bottom_right", origin_bottom_right); FSN_ASSERT( r >= 0 );

		return RegisterScriptVector_Native(engine);
	}

}}
