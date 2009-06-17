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

#ifndef Header_FusionEngine_RefCounted
#define Header_FusionEngine_RefCounted

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include "FusionScriptingEngine.h"
//#include <angelscirpt.h>
#include <boost/intrusive_ptr.hpp>

namespace FusionEngine
{

	//class RefCounted;

	//static RefCounted* RefCountedFactory();

	//template <class T>
	//class RefCountedHelper
	//{
	//public:
	//	static T& Assign(const T& rhs, T* lhs)
	//	{
	//		*lhs = rhs;
	//		(RefCounted*)lhs->ResetRefCount();
	//		return *lhs;
	//	}
	//}

	//template <class T>

	//! Base class for AngelScript compatible ReferenceCounted type
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
			RefCounted* lhs_rc = dynamic_cast<RefCounted*>(lhs);
			lhs_rc->m_RefCount = 1;
			return *lhs;
		}

		//! Registers this type as a ref-counted type with the given engine
		template <class T>
		static void RegisterType(asIScriptEngine* engine, const std::string& name)
		{
			int r;
			
			r = engine->RegisterObjectType(name.c_str(), sizeof(T), asOBJ_REF); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_FACTORY, (name + "@ factory()").c_str(), asFUNCTION(RefCounted::Factory<T>), asCALL_CDECL);

			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ADDREF, "void addref()", asMETHOD(RefCounted, addRef), asCALL_THISCALL);
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_RELEASE, "void release()", asMETHOD(RefCounted, release), asCALL_THISCALL);

			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ASSIGNMENT, (name + "& op_assign(const " + name + " &in)").c_str(), asFUNCTION(RefCounted::Assign<T>), asCALL_CDECL_OBJFIRST);
		}

		//static RefCounted* RefCountedFactory()
		//{
		//	RefCounted* obj = new RefCounted();
		//	return obj;
		//}

	};

	void intrusive_ptr_add_ref(RefCounted *ptr);

	void intrusive_ptr_release(RefCounted *ptr);

}

#endif