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

#ifndef Header_FusionEngine_RefCounted
#define Header_FusionEngine_RefCounted

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include "FusionScriptingEngine.h"
//#include <angelscirpt.h>
#include <boost/intrusive_ptr.hpp>
#include <type_traits>

namespace FusionEngine
{

	//! Normal ref-counted type
	struct normal_refcounted {};
	//! Ref-counted type with no constructor
	struct no_factory {};
	//! Ref-counted type that can't be copied by value
	struct noncopyable : public boost::noncopyable {};
	//! \see no_factory | noncopyable
	struct no_factory_noncopyable : no_factory, noncopyable {};

	//! Base class for AngelScript compatible reference counted type
	class RefCounted
	{
	private:
		int m_RefCount;

	public:
		RefCounted()
			: m_RefCount(1)
		{
		}
		virtual ~RefCounted() {}

		//! Increases reference count
		virtual void addRef()
		{
			++m_RefCount;
		}
		//! Decreases reference count
		virtual void release()
		{
			if (--m_RefCount == 0)
				OnNoReferences();
		}
		//! Override to change delete behaviour
		virtual void OnNoReferences()
		{
			delete this;
		}

	public:
		//! Factory utility
		template <class T>
		static T* Factory()
		{
			T* obj = new T();
			return obj;
		}

		//! Assign utility
		template <class T>
		static T& Assign(T* lhs, const T& rhs)
		{
			*lhs = rhs;
			RefCounted* lhs_rc = static_cast<RefCounted*>(lhs);
			lhs_rc->m_RefCount = 1;
			return *lhs;
		}

		////! Exclusion flags define things that wont be registered for this class
		//enum ExclusionFlags
		//{
		//	ex_normal       = 0x00,
		//	//! If no factory is defined, the class cannot be constructed in scripts.
		//	ex_no_factory   = 0x01,
		//	//! Is no assignment operator is defined, the class cannot be copied by value in scripts.
		//	ex_noncopyable  = 0x02
		//};

		////! Flag template type
		//template <int T>
		//struct ExclusionType
		//{
		//	enum { code = T };
		//};

		//! Registers this type as a ref-counted type with the given engine
		template <class T/*, typename FlagsT = ExclusionType<ex_normal>*/>
		static void RegisterType(asIScriptEngine* engine, const std::string& name)
		{
			int r;
			
			r = engine->RegisterObjectType(name.c_str(), sizeof(T), asOBJ_REF); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ADDREF, "void addref()", asMETHOD(RefCounted, addRef), asCALL_THISCALL);
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_RELEASE, "void release()", asMETHOD(RefCounted, release), asCALL_THISCALL);

			// Register the factory behaviour if the type allows it
			RegisterFactory_Default<T>(engine, name, std::tr1::is_base_of<no_factory, T>());
			// Register the assignment operator if the type is copyable
			RegisterAssignment<T>(engine, name, std::tr1::is_base_of<noncopyable, T>());
		}

	private:
		template <class T>
		static void RegisterFactory_Default(asIScriptEngine* engine, const std::string& name, const std::tr1::false_type&)
		{
			int r;

			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_FACTORY, (name + "@ factory()").c_str(), asFUNCTION(RefCounted::Factory<T>), asCALL_CDECL);
		}

		template <class T>
		static void RegisterAssignment(asIScriptEngine* engine, const std::string& name, const std::tr1::false_type&)
		{
			int r;

			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ASSIGNMENT, (name + "& op_assign(const " + name + " &in)").c_str(), asFUNCTION(RefCounted::Assign<T>), asCALL_CDECL_OBJFIRST);
		}

		template <class T>
		static void RegisterFactory_Default(asIScriptEngine* engine, const std::string& name, const std::tr1::true_type&)
		{}
		template <class T>
		static void RegisterAssignment(asIScriptEngine* engine, const std::string& name, const std::tr1::true_type&)
		{}
	};

	template <class _From, class _To>
	_To * convert_ref(_From * obj)
	{
		if (obj == NULL)
			return NULL;

		_To* ret = dynamic_cast<_To*>(obj);
		if (ret == NULL)
		{
			obj->release();
		}
		return ret;
	}

	//! Registers conversion operators for two ref. counted application objects
	template <class _Base, class _Derived>
	void RegisterBaseOf(asIScriptEngine *engine, const std::string &base, const std::string &derived)
	{
		int r;
		r = engine->RegisterGlobalBehaviour(asBEHAVE_REF_CAST,
			(derived+"@ f("+base+"@)").c_str(), asFUNCTIONPR(convert_ref, (_Base*), _Derived*),
			asCALL_CDECL);

		r = engine->RegisterGlobalBehaviour(asBEHAVE_IMPLICIT_REF_CAST,
			(base+"@ f("+derived+"@)").c_str(), asFUNCTIONPR(convert_ref, (_Derived*), _Base*),
			asCALL_CDECL);
	}

	void intrusive_ptr_add_ref(RefCounted *ptr);

	void intrusive_ptr_release(RefCounted *ptr);

}

#endif