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

#ifndef Header_FusionEngine_TypeRegistrationUtils
#define Header_FusionEngine_TypeRegistrationUtils

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include "FusionResource.h"

#ifdef _DEBUG
#include "FusionConsole.h"
#endif


namespace FusionEngine
{

	template <typename T>
	class TypeRegisterHelper
	{
	public:
		static void Construct(T* in)
		{
			new (in) T();
		}

		static void Destruct(T* in)
		{
			in->~T();
		}

		static void CopyConstruct(const T& rhs, T* in)
		{
			new (in) T(rhs);
		}

		static T& Assign(const T& rhs, T* lhs)
		{
			*lhs = rhs;
			return *lhs;
		}

	};

	template <typename T>
	void RegisterType(const std::string& type_name, asIScriptEngine* engine)
	{
		FSN_ASSERT(engine && "Passed NULL engine pointer to registerVector");

		int error_code = 0;

		error_code = engine->RegisterObjectType(type_name.c_str(), sizeof(T), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);
		FSN_ASSERT(error_code >= 0 && "Failed to register object type");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(), 
			asBEHAVE_CONSTRUCT, 
			"void f()", 
			asFUNCTION(TypeRegisterHelper<T>::Construct), 
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register constructor");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
			asBEHAVE_DESTRUCT,
			"void f()",
			asFUNCTION(TypeRegisterHelper<T>::Destruct),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register destructor");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
			asBEHAVE_CONSTRUCT,
			(std::string("void f(")+type_name+"&in)").c_str(),
			asFUNCTION(TypeRegisterHelper<T>::CopyConstruct),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register copy constructor");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
			asBEHAVE_ASSIGNMENT,
			(type_name+"& f(const "+type_name+"&in)").c_str(),
			asFUNCTION(TypeRegisterHelper<T>::Assign),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register assignment operator");

	}

	//! Registers a POD type (no constructor)
	template <typename T>
	void RegisterTypePOD(const std::string& type_name, asIScriptEngine* engine)
	{
		FSN_ASSERT(engine && "Passed NULL engine pointer to registerVector");

		int error_code = 0;

		error_code = engine->RegisterObjectType(type_name.c_str(), sizeof(T), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
		FSN_ASSERT(error_code >= 0 && "Failed to register object type");

	}

	//! Registers a REF NOHANDLE type (no constructor)
	template <typename T>
	void RegisterTypeNoHandle(const std::string& type_name, asIScriptEngine* engine)
	{
		FSN_ASSERT(engine && "Passed NULL engine pointer to registerVector");

		int error_code = 0;

		error_code = engine->RegisterObjectType(type_name.c_str(), sizeof(T), asOBJ_REF | asOBJ_NOHANDLE);
		FSN_ASSERT(error_code >= 0 && "Failed to register object type");

	}

}

#endif