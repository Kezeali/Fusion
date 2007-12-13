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


	File Author(s):

		Elliot Hayward
*/

#include "FusionScriptVector.h"
#include <angelscript.h>

namespace FusionEngine
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
		cl_assert( m_RefCount == 0 );
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

	// Generic
	static void VectorAddRef_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *thisPointer = (ScriptVector*)gen->GetObject();
		thisPointer->AddRef();
	}

	static void VectorRelease_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *thisPointer = (ScriptVector*)gen->GetObject();
		thisPointer->Release();
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

	static void AssignVector_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *thisPointer = (ScriptVector*)gen->GetObject();
		*thisPointer = *a;
		gen->SetReturnAddress(thisPointer);
	}

	ScriptVector &ScriptVector::operator+=(const ScriptVector &other)
	{
		Data += other.Data;
		return *this;
	}

	static void AddAssignVector_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *thisPointer = (ScriptVector*)gen->GetObject();
		*thisPointer += *a;
		gen->SetReturnAddress(thisPointer);
	}

	// Comparison
	static void VectorEqual_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *b = (ScriptVector*)gen->GetArgAddress(1);
		bool r = (*a).Data == (*b).Data;
		gen->SetReturnDWord(r);
	}

	static void VectorNotEqual_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *b = (ScriptVector*)gen->GetArgAddress(1);
		bool r = (*a).Data != (*b).Data;
		gen->SetReturnDWord(r);
	}

	// Arithmatic
	ScriptVector *operator+(const ScriptVector &a, const ScriptVector &b)
	{
		// Return a new object as a script handle
		return new ScriptVector(a.Data + b.Data);
	}

	static void AddVectors_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *b = (ScriptVector*)gen->GetArgAddress(1);
		ScriptVector *out = *a + *b;
		gen->SetReturnAddress(out);
	}

	//////////////////////
	// Properties (wrappers for generic call method)
	static void VectorLength_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *s = (ScriptVector*)gen->GetObject();
		float l = s->Data.length();
		gen->SetReturnFloat(l);
	}

	static void VectorSquaredLength_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *s = (ScriptVector*)gen->GetObject();
		float l = s->Data.squared_length();
		gen->SetReturnFloat(l);
	}


	///////
	// Mem
	/*!
	 * A wrapper for the default ScriptVector constructor, since
	 * it is not possible to take the address of the constructor directly
	 */
	static void ConstructScriptVector(ScriptVector *thisPointer)
	{
		// Construct the string in the memory received
		new(thisPointer) ScriptVector();
	}

	// This is the string factory that creates new strings for the script based on string literals
	static ScriptVector *VectorFactory(float x, float y)
	{
		return new ScriptVector(x, y);
	}

	static void VectorFactory_Generic(asIScriptGeneric *gen)
	{
		float x = gen->GetArgFloat(0);
		float y = gen->GetArgFloat(1);
		ScriptVector *ret = VectorFactory(x, y);
		gen->SetReturnAddress(ret);
	}

	// This is the default string factory, that is responsible for creating empty string objects, e.g. when a variable is declared
	static ScriptVector *VectorDefaultFactory()
	{
		// Allocate and initialize with the default constructor
		return new ScriptVector();
	}

	static void VectorDefaultFactory_Generic(asIScriptGeneric *gen)
	{
		*(ScriptVector**)gen->GetReturnPointer() = VectorDefaultFactory();
	}

	static ScriptVector *VectorCopyFactory(const ScriptVector &other)
	{
		// Allocate and initialize with the copy constructor
		return new ScriptVector(other);
	}

	static void VectorCopyFactory_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *other = (ScriptVector *)gen->GetArgObject(0);
		*(ScriptVector**)gen->GetReturnPointer() = VectorCopyFactory(*other);
	}

	////////////////////////////////
	// AngelScript type registration
	void RegisterScriptVector_Native(asIScriptEngine *engine)
	{
		int r;

		// Register the type
		r = engine->RegisterObjectType("Vector", sizeof(ScriptVector), asOBJ_REF); cl_assert( r >= 0 );

		// Register factory methods
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f()",                 asFUNCTION(VectorDefaultFactory), asCALL_CDECL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f(float, float)",     asFUNCTION(VectorFactory), asCALL_CDECL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f(const Vector &in)", asFUNCTION(VectorCopyFactory), asCALL_CDECL); assert( r >= 0 );
		
		// Note: We don't have to register the destructor, since the object uses reference counting
		//r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructScriptVector), asCALL_CDECL_OBJLAST); cl_assert( r >= 0 );

		// Register the object operator overloads
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_ADDREF,     "void f()",                    asMETHOD(ScriptVector,AddRef), asCALL_THISCALL); cl_assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_RELEASE,    "void f()",                    asMETHOD(ScriptVector,Release), asCALL_THISCALL); cl_assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_ASSIGNMENT, "Vector &f(const Vector &in)", asMETHODPR(ScriptVector, operator =, (const ScriptVector&), ScriptVector&), asCALL_THISCALL); cl_assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_ADD_ASSIGN, "Vector &f(const Vector &in)", asMETHODPR(ScriptVector, operator+=, (const ScriptVector&), ScriptVector&), asCALL_THISCALL); cl_assert( r >= 0 );

		// Register the global operator overloads
		// Note: Vector2's methods can be used directly because the
		// internal Vector2 is placed at the beginning of the class
		//r = engine->RegisterGlobalBehaviour(asBEHAVE_EQUAL,       "bool f(const Vector &in, const Vector &in)",    asMETHODPR(Vector2, operator==, (const Vector2 &), bool), asCALL_THISCALL); cl_assert( r >= 0 );
		//r = engine->RegisterGlobalBehaviour(asBEHAVE_NOTEQUAL,    "bool f(const Vector &in, const Vector &in)",    asMETHODPR(Vector2, operator!=, (const Vector2 &), bool), asCALL_THISCALL); cl_assert( r >= 0 );
		r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "Vector@ f(const Vector &in, const Vector &in)", asFUNCTIONPR(operator +, (const ScriptVector &, const ScriptVector &), ScriptVector*), asCALL_CDECL); cl_assert( r >= 0 );

		// Register the object methods
		r = engine->RegisterObjectMethod("Vector", "float length() const", asMETHOD(Vector2,length), asCALL_THISCALL); cl_assert( r >= 0 );

		r = engine->RegisterObjectMethod("Vector", "float get_x() const", asMETHOD(Vector2,getX), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Vector", "float get_y() const", asMETHOD(Vector2,getY), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectProperty("Vector", "float x", offsetof(Vector2,x)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Vector", "float y", offsetof(Vector2,y)); assert( r >= 0 );
	}

	void RegisterScriptVector_Generic(asIScriptEngine *engine)
	{
		int r;

		// Register the type
		r = engine->RegisterObjectType("Vector", sizeof(ScriptVector), asOBJ_REF); cl_assert( r >= 0 );

		// Register factory methods
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f()",                 asFUNCTION(VectorDefaultFactory_Generic), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_FACTORY,    "Vector @f(const string &in)", asFUNCTION(VectorCopyFactory_Generic), asCALL_GENERIC); assert( r >= 0 );

		// Register the object operator overloads
		// Note: We don't have to register the destructor, since the object uses reference counting
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructScriptVector), asCALL_GENERIC); cl_assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_ADDREF,     "void f()",                    asFUNCTION(VectorAddRef_Generic), asCALL_GENERIC); cl_assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_RELEASE,    "void f()",                    asFUNCTION(VectorRelease_Generic), asCALL_GENERIC); cl_assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_ASSIGNMENT, "Vector &f(const Vector &in)", asFUNCTION(AssignVector_Generic), asCALL_GENERIC); cl_assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Vector", asBEHAVE_ADD_ASSIGN, "Vector &f(const Vector &in)", asFUNCTION(AddAssignVector_Generic), asCALL_GENERIC); cl_assert( r >= 0 );

		r = engine->RegisterGlobalBehaviour(asBEHAVE_EQUAL,       "bool f(const Vector &in, const Vector &in)",    asFUNCTION(VectorEqual_Generic), asCALL_GENERIC); cl_assert( r >= 0 );
		r = engine->RegisterGlobalBehaviour(asBEHAVE_NOTEQUAL,    "bool f(const Vector &in, const Vector &in)",    asFUNCTION(VectorNotEqual_Generic), asCALL_GENERIC); cl_assert( r >= 0 );
		r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "Vector@ f(const Vector &in, const Vector &in)", asFUNCTION(AddVectors_Generic), asCALL_GENERIC); cl_assert( r >= 0 );

		// Register the object methods
		r = engine->RegisterObjectMethod("Vector", "float length() const", asFUNCTION(VectorLength_Generic), asCALL_GENERIC); cl_assert( r >= 0 );
	}

	void RegisterScriptVector(asIScriptEngine *engine)
	{
		if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
			RegisterScriptVector_Generic(engine);
		else
			RegisterScriptVector_Native(engine);
	}

}
