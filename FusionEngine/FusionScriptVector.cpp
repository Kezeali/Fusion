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
		: m_RefCount(1),
		Vector2(other)
	{
	}

	ScriptVector::ScriptVector(float x, float y)
		: m_RefCount(1),
		Vector2(x,y)
	{
	}

	ScriptVector::~ScriptVector()
	{
		cl_assert( refCount == 0 );
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
	// Operations (wrappers for generic call method)
	// Assignment
	static void AssignVector_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *thisPointer = (ScriptVector*)gen->GetObject();
		*thisPointer = *a;
		gen->SetReturnAddress(thisPointer);
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
		bool r = *a == *b;
		gen->SetReturnDWord(r);
	}

	static void VectorNotEqual_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *b = (ScriptVector*)gen->GetArgAddress(1);
		bool r = *a != *b;
		gen->SetReturnDWord(r);
	}

	static void VectorLesserOrEqual_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *b = (ScriptVector*)gen->GetArgAddress(1);
		bool r = *a <= *b;
		gen->SetReturnDWord(r);
	}

	static void VectorGreaterOrEqual_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *b = (ScriptVector*)gen->GetArgAddress(1);
		bool r = *a >= *b;
		gen->SetReturnDWord(r);
	}

	static void VectorLesser_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *b = (ScriptVector*)gen->GetArgAddress(1);
		bool r = *a < *b;
		gen->SetReturnDWord(r);
	}

	static void VectorGreater_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *a = (ScriptVector*)gen->GetArgAddress(0);
		ScriptVector *b = (ScriptVector*)gen->GetArgAddress(1);
		bool r = *a > *b;
		gen->SetReturnDWord(r);
	}

	// Arithmatic
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
		float l = s->length();
		gen->SetReturnDWord(l);
	}

	static void VectorSquaredLength_Generic(asIScriptGeneric *gen)
	{
		ScriptVector *s = (ScriptVector*)gen->GetObject();
		float l = s->squared_length();
		gen->SetReturnDWord(l);
	}

}
