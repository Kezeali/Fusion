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

#include "FusionScriptManager.h"
#include <angelscript.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>
#include <type_traits>

namespace FusionEngine
{

	//! Normal ref-counted type
	struct normal_refcounted {};
	//! Type that can't be copied by value
	struct noncopyable : public boost::noncopyable {};

	//! Base class for AngelScript compatible reference counted type
	class RefCounted
	{
	protected:
		volatile int m_RefCount;

	public:
		RefCounted()
			: m_RefCount(1)
		{}
		RefCounted(int initial_reference_count)
			: m_RefCount(initial_reference_count)
		{}
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
		//! Assign utility
		template <class T>
		static T& Assign(T* lhs, const T& rhs)
		{
			*lhs = rhs;
			RefCounted* lhs_rc = static_cast<RefCounted*>(lhs);
			lhs_rc->m_RefCount = 1;
			return *lhs;
		}

		//! Registers this type as a ref-counted type with the given engine
		template <class T>
		static void RegisterType(asIScriptEngine* engine, const std::string& name)
		{
			int r;
			
			r = engine->RegisterObjectType(name.c_str(), sizeof(T), asOBJ_REF); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ADDREF, "void addref()", asMETHOD(RefCounted, addRef), asCALL_THISCALL);
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_RELEASE, "void release()", asMETHOD(RefCounted, release), asCALL_THISCALL);

			// Register the assignment operator if the type is copyable
			RegisterAssignment<T>(engine, name, std::is_base_of<noncopyable, T>());
		}

	protected:
		template <class T>
		static void RegisterAssignment(asIScriptEngine* engine, const std::string& name, const std::tr1::false_type&)
		{
			int r;

			r = engine->RegisterObjectMethod(name.c_str(), (name + "& opAssign(const " + name + " &in)").c_str(), asFUNCTION(RefCounted::Assign<T>), asCALL_CDECL_OBJFIRST);
		}

		template <class T>
		static void RegisterAssignment(asIScriptEngine* engine, const std::string& name, const std::tr1::true_type&)
		{}
	};

	//! Base class for implementing a Garbage Collected, Reference Counted class
	template <class T>
	class GarbageCollected : public RefCounted
	{
	protected:
		static int s_TypeId;
		bool m_GCFlag;

	public:
		//! Constructor
		GarbageCollected()
			: m_GCFlag(false)
		{
			ScriptManager *manager = ScriptManager::getSingletonPtr();
			if (manager != NULL && manager->GetEnginePtr() != NULL)
				manager->GetEnginePtr()->NotifyGarbageCollectorOfNewObject(this, s_TypeId);
		}
		//! Constructor
		GarbageCollected(asIScriptEngine *engine)
			: m_GCFlag(false)
		{
			engine->NotifyGarbageCollectorOfNewObject(this, s_TypeId);
		}
		virtual ~GarbageCollected()
		{
		}
		//! Calls RefCounted#addRef() then sets the GC-flag to false
		virtual void addRef()
		{
			RefCounted::addRef();
			m_GCFlag = false;
		}
		//! Calls RefCounted#release() then sets the GC-flag to false
		virtual void release()
		{
			m_GCFlag = false;
			RefCounted::release();
		}

		//! Sets the GC flag
		void SetGCFlag()
		{
			m_GCFlag = true;
		}
		//! Returns true if this object has been marked by the GC
		bool GetGCFlag()
		{
			return m_GCFlag;
		}
		//! Returns the reference-count
		int GetRefCount()
		{
			return m_RefCount;
		}

		//! Should call asIScriptEngine::GCEnumCallback() on every GCed object held by this object
		virtual void EnumReferences(asIScriptEngine *engine) =0;
		//! Should release every GCed object held by this object
		virtual void ReleaseAllReferences(asIScriptEngine *engine) =0;

		static void RegisterGCType(asIScriptEngine* engine, const std::string& name)
		{
			int r;
			
			r = engine->RegisterObjectType(name.c_str(), 0, asOBJ_REF | asOBJ_GC); FSN_ASSERT(r >= 0);

			// Note that the TypeId is recorded here
			s_TypeId = engine->GetTypeIdByDecl(name.c_str());

			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ADDREF, "void f()", asMETHOD(GarbageCollected, addRef), asCALL_THISCALL);
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_RELEASE, "void f()", asMETHOD(GarbageCollected, release), asCALL_THISCALL);

			// GC behaviours
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_SETGCFLAG,
				"void f()", asMETHOD(GarbageCollected, SetGCFlag), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_GETGCFLAG,
				"bool f()", asMETHOD(GarbageCollected, GetGCFlag), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_GETREFCOUNT,
				"int f()", asMETHOD(GarbageCollected, GetRefCount), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ENUMREFS,
				"void f(int&in)", asMETHOD(GarbageCollected, EnumReferences), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_RELEASEREFS,
				"void f(int&in)", asMETHOD(GarbageCollected, ReleaseAllReferences), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

			// Register the assignment operator if the type is copyable
			RegisterAssignment<T>(engine, name, std::tr1::is_base_of<noncopyable, T>());
		}
	};

	template <typename T> int GarbageCollected<T>::s_TypeId = -1;

	template <class _From, class _To>
	_To * convert_ref(_From * obj)
	{
		if (obj == NULL)
			return NULL;

		_To* ret = dynamic_cast<_To*>(obj);
		if (ret != NULL)
			ret->addRef();
		return ret;
	}

	//! Registers conversion operators for two ref. counted application objects
	template <class _Base, class _Derived>
	void RegisterBaseOf(asIScriptEngine *engine, const std::string &base, const std::string &derived)
	{
		int r;
		r = engine->RegisterObjectBehaviour(base.c_str(), asBEHAVE_REF_CAST,
			(derived+"@ f()").c_str(), asFUNCTION((convert_ref<_Base, _Derived>)),
			asCALL_CDECL_OBJLAST);

		r = engine->RegisterObjectBehaviour(derived.c_str(), asBEHAVE_IMPLICIT_REF_CAST,
			(base+"@ f()").c_str(), asFUNCTION((convert_ref<_Derived, _Base>)),
			asCALL_CDECL_OBJLAST);
	}

	void intrusive_ptr_add_ref(RefCounted *ptr);

	void intrusive_ptr_release(RefCounted *ptr);

}

#endif