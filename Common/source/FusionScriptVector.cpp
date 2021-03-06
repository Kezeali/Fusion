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

#include "PrecompiledHeaders.h"

#include "FusionScriptVector.h"

#include <ClanLib/Display/2D/color.h>

namespace FusionEngine { namespace Scripting
{
	//////////////////
	//// Construction
	//ScriptVector::ScriptVector(const ScriptVector &other)
	//	: m_RefCount(1)
	//{
	//	Data = other.Data;
	//}

	//ScriptVector::ScriptVector(float x, float y)
	//	: m_RefCount(1)
	//{
	//	Data.x = x; Data.y = y;
	//}

	//ScriptVector::ScriptVector(const Vector2 &other)
	//	: m_RefCount(1),
	//	Data(other)
	//{
	//}

	//ScriptVector::~ScriptVector()
	//{
	//	FSN_ASSERT( m_RefCount == 0 );
	//}

	////////////////////////
	//// Reference counting
	//void ScriptVector::AddRef()
	//{
	//	m_RefCount++;
	//}

	//void ScriptVector::Release()
	//{
	//	if ( --m_RefCount == 0 )
	//		delete this;
	//}


	/////////////////////
	//// Operations
	//// Assignment
	//ScriptVector &ScriptVector::operator=(const ScriptVector &other)
	//{
	//	Data = other.Data;

	//	// Return a ref. to this
	//	return *this;
	//}

	//ScriptVector &ScriptVector::operator+=(const ScriptVector &other)
	//{
	//	Data += other.Data;
	//	return *this;
	//}

	// Arithmatic
	static Vector2 *RefVecopAdd(const Vector2 &a, const Vector2 &b)
	{
		Vector2 *sum = new Vector2();
		v2Add(a, b, *sum);
		// Return a new object as a script handle
		return sum;
	}

	static Vector2 VecopAdd(const Vector2 &b, Vector2 &obj)
	{
		Vector2 sum;
		v2Add(obj, b, sum);
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

	// Ctors
	static void VectorDefaultCtor(void *ptr)
	{
		new (ptr) Vector2();
	}

	static void VectorInitCtor(float x, float y, void *ptr)
	{
		new (ptr) Vector2(x, y);
	}

	static void VectorCopyCtor(const Vector2 &other, void *ptr)
	{
		new (ptr) Vector2(other);
	}
	
	static void VectorDestructor(Vector2 *ptr)
	{
		ptr->~Vector2();
	}

	static void ColorfCtor(void *ptr)
	{
		new (ptr) clan::Colorf();
	}

	static void ColorfCopyCtor(const clan::Colorf &other, void *ptr)
	{
		new (ptr) clan::Colorf(other);
	}
	
	static void ColorfDtor(clan::Colorf *ptr)
	{
		ptr->~Colorf();
	}

	////////////////////////////////
	// AngelScript type registration
	int RegisterScriptVector_Native(asIScriptEngine *engine)
	{
		int r, typeId;

		r = engine->RegisterObjectType("Colour", sizeof(clan::Colorf), asOBJ_VALUE | asOBJ_APP_CLASS_CK); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Colour", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ColorfCtor), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Colour", asBEHAVE_CONSTRUCT, "void f(const Colour &in)", asFUNCTION(ColorfCopyCtor), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Colour", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(ColorfDtor), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		// Register the type
#ifdef FSN_REFCOUNTED_VECTOR
		r = engine->RegisterObjectType("Vector", sizeof(Vector2), asOBJ_REF); FSN_ASSERT( r >= 0 );
#else
		r = engine->RegisterObjectType("Vector", sizeof(Vector2), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); FSN_ASSERT( r >= 0 );
#endif
		typeId = engine->GetTypeIdByDecl("Vector");

#ifdef FSN_REFCOUNTED_VECTOR
		// Register factory methods
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f()",                 asFUNCTION(VectorDefaultFactory), asCALL_CDECL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f(float, float)",     asFUNCTION(VectorInitFactory), asCALL_CDECL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f(const Vector &in)", asFUNCTION(VectorCopyFactory), asCALL_CDECL); FSN_ASSERT( r >= 0 );
#else
		// Register constructors / destructor
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_CONSTRUCT,    "void f()",                 asFUNCTION(VectorDefaultCtor), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_CONSTRUCT,    "void f(float, float)",     asFUNCTION(VectorInitCtor), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_CONSTRUCT,    "void f(const Vector &in)", asFUNCTION(VectorCopyCtor), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_DESTRUCT,     "void f()",                 asFUNCTION(VectorDestructor), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
#endif
		
		// Register the object operator overloads
#ifdef FSN_REFCOUNTED_VECTOR
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_ADDREF,     "void f()",                    asMETHOD(Vector2, addRef), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_RELEASE,    "void f()",                    asMETHOD(Vector2, release), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
#endif
		r = engine->RegisterObjectMethod("Vector", "Vector &opAssign(const Vector &in)", asMETHODPR(Vector2, operator =, (const Vector2&), Vector2&), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "Vector &opAddAssign(const Vector &in)", asMETHODPR(Vector2, operator+=, (const Vector2&), Vector2&), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "Vector &opSubAssign(const Vector &in)", asMETHODPR(Vector2, operator-=, (const Vector2&), Vector2&), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "Vector &opMulAssign(const Vector &in)", asMETHODPR(Vector2, operator*=, (float), Vector2&), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
#ifdef FSN_REFCOUNTED_VECTOR
		r = engine->RegisterObjectMethod("Vector", "Vector@ opAdd(const Vector &in) const", asFUNCTION(RefVecopAdd), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );
#else
		r = engine->RegisterObjectMethod("Vector", "Vector opAdd(const Vector &in) const", asFUNCTION(VecopAdd), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "Vector opSub(const Vector &in) const", asMETHODPR(Vector2, operator-, (const Vector2&) const, Vector2), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "Vector opMul(const Vector &in) const", asMETHOD(Vector2, operator*), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		
		r = engine->RegisterObjectMethod("Vector", "Vector opNeg() const", asMETHODPR(Vector2, operator-, (void) const, Vector2), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
#endif

		r = engine->RegisterObjectMethod("Vector", "bool opEquals(const Vector &in) const", asMETHOD(Vector2, operator==), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		// Register the object methods
		r = engine->RegisterObjectMethod("Vector", "float length() const", asMETHOD(Vector2, length), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "float squared_length() const", asMETHOD(Vector2, squared_length), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Vector", "Vector normalised() const", asMETHOD(Vector2, normalized), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Vector", "float dot(const Vector &in) const", asMETHOD(Vector2, dot), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Vector", "float get_x() const", asMETHOD(Vector2, get_x), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "float get_y() const", asMETHOD(Vector2, get_y), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Vector", "void set_x(float)", asMETHOD(Vector2, set_x), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "void set_y(float)", asMETHOD(Vector2, set_y), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		return typeId;
	}

	int RegisterScriptVector(asIScriptEngine *engine)
	{
		int r;
		r = engine->RegisterEnum("PointOrigin");
		r = engine->RegisterEnumValue("PointOrigin", "top_left", clan::origin_top_left); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "top_center", clan::origin_top_center); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "top_right", clan::origin_top_right); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "center_left", clan::origin_center_left); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "center", clan::origin_center); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "center_right", clan::origin_center_right); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "bottom_left", clan::origin_bottom_left); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "bottom_center", clan::origin_bottom_center); FSN_ASSERT( r >= 0 );
		r = engine->RegisterEnumValue("PointOrigin", "bottom_right", clan::origin_bottom_right); FSN_ASSERT( r >= 0 );

		return RegisterScriptVector_Native(engine);
	}

}}
