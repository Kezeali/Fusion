/*
*  Copyright (c) 2006-2007 Fusion Project Team
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

#ifndef H_FusionScriptTypeRegistrationUtils
#define H_FusionScriptTypeRegistrationUtils

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
	namespace Scripting
	{
		namespace Registation
		{

			template <typename T>
			class ValueTypeHelper
			{
			public:
				static void Construct(T* ptr)
				{
					new (ptr) T();
				}

				static void Destruct(T* obj)
				{
					obj->~T();
				}
			};

			template <typename T>
			class CopyableValueTypeHelper : public ValueTypeHelper<T>
			{
			public:
				static void CopyConstruct(const T& other, T* ptr)
				{
					new (ptr) T(other);
				}

				static T& Assign(const T& other, T* _this)
				{
					*_this = other;
					return *_this;
				}
			};

		}

	}

	template <typename T>
	void RegisterValueType(const std::string& type_name, asIScriptEngine* engine, asDWORD type_flags)
	{
		FSN_ASSERT(engine && "Need a valid engine pointer");

		using namespace Scripting::Registation;

		typedef CopyableValueTypeHelper<T> helper_type;

		int error_code;

		error_code = engine->RegisterObjectType(type_name.c_str(), sizeof(T), asOBJ_VALUE | type_flags);
		FSN_ASSERT(error_code >= 0 && "Failed to register object type");

		if (type_flags & asOBJ_APP_CLASS_CONSTRUCTOR)
		{
			error_code = engine->RegisterObjectBehaviour(type_name.c_str(), 
				asBEHAVE_CONSTRUCT, 
				"void f()", 
				asFUNCTION(helper_type::Construct), 
				asCALL_CDECL_OBJLAST);
			FSN_ASSERT(error_code >= 0 && "Failed to register constructor");
		}

		if (type_flags & asOBJ_APP_CLASS_DESTRUCTOR)
		{
			error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
				asBEHAVE_DESTRUCT,
				"void f()",
				asFUNCTION(helper_type::Destruct),
				asCALL_CDECL_OBJLAST);
			FSN_ASSERT(error_code >= 0 && "Failed to register destructor");
		}

		if (type_flags & asOBJ_APP_CLASS_COPY_CONSTRUCTOR)
		{
			error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
				asBEHAVE_CONSTRUCT,
				(std::string("void f(")+type_name+"&in)").c_str(),
				asFUNCTION(helper_type::CopyConstruct),
				asCALL_CDECL_OBJLAST);
			FSN_ASSERT(error_code >= 0 && "Failed to register copy constructor");
		}

		if (type_flags & asOBJ_APP_CLASS_ASSIGNMENT)
		{
			error_code = engine->RegisterObjectMethod(type_name.c_str(),
				(type_name+"& f(const "+type_name+"&in)").c_str(),
				asFUNCTION(helper_type::Assign),
				asCALL_CDECL_OBJLAST);
			FSN_ASSERT(error_code >= 0 && "Failed to register assignment operator");
		}
	}
	
	template <typename T>
	void RegisterSharedPtrType(const std::string& name, asIScriptEngine *engine)
	{
		using namespace Scripting::Registation;
		typedef ValueTypeHelper<std::shared_ptr<T>> helper_type;

		int r;
		r = engine->RegisterObjectType(name.c_str(), sizeof(std::shared_ptr<T>), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(helper_type::Construct), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_DESTRUCT, "void f()", asFUNCTION(helper_type::Destruct), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod(name.c_str(), name + "& opAssign(const " + name + " &in other)",
			asMETHODPR(std::shared_ptr<T>, operator=, (const std::shared_ptr<T> &), std::shared_ptr<T> &), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

	//! Registers a POD type (no constructor)
	template <typename T>
	void RegisterTypePOD(const std::string& type_name, asIScriptEngine* engine)
	{
		FSN_ASSERT(engine && "Passed NULL engine pointer to registerVector");

		int error_code = 0;

		error_code = engine->RegisterObjectType(type_name.c_str(), sizeof(T), asOBJ_VALUE | asOBJ_POD);
		FSN_ASSERT(error_code >= 0 && "Failed to register object type");
	}

	//! Registers a singleton (REF & NOHANDLE) type
	template <typename T>
	void RegisterSingletonType(const std::string& type_name, asIScriptEngine* engine)
	{
		FSN_ASSERT_MSG(engine, "Passed NULL engine pointer to RegisterSingletonType");

		int error_code = 0;

		error_code = engine->RegisterObjectType(type_name.c_str(), sizeof(T), asOBJ_REF | asOBJ_NOHANDLE);
		FSN_ASSERT_MSG(error_code >= 0, "Failed to register object type");
	}

}

#endif