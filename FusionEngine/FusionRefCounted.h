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

namespace FusionEngine
{

	class RefCounted;

	//static RefCounted* RefCountedFactory();

	template <class T>
	static T* RefCountedFactory()
	{
		T* obj = new T();
		return obj;
	}

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
	class RefCounted
	{
	private:
		int m_RefCount;

	public:
		RefCounted()
		{
			m_RefCount = 1;
		}
		void addRef()
		{
			++m_RefCount;
		}
		void release()
		{
			if (--m_RefCount == 0)
				delete this;
		}

		template <class T>
		static T& Assign(const T& rhs, T* lhs)
		{
			*lhs = rhs;
			RefCounted* lhs_rc = dynamic_cast<RefCounted*>(lhs);
			lhs_rc->m_RefCount = 1;
			return *lhs;
		}

		template <class T>
		static void registerType(asIScriptEngine* engine, const std::string& name)
		{
			int r;
			
			r = engine->RegisterObjectType(name.c_str(), sizeof(T), asOBJ_REF); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_FACTORY, (name + "@ factory()").c_str(), asFUNCTION(RefCountedFactory<T>), asCALL_CDECL);

			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ADDREF, "void addref()", asMETHOD(RefCounted, addRef), asCALL_THISCALL);
			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_RELEASE, "void release()", asMETHOD(RefCounted, release), asCALL_THISCALL);

			r = engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ASSIGNMENT, (name + "& op_assign(const " + name + " &in)").c_str(), asFUNCTION(RefCounted::Assign<T>), asCALL_CDECL_OBJLAST);
		}

		//static RefCounted* RefCountedFactory()
		//{
		//	RefCounted* obj = new RefCounted();
		//	return obj;
		//}

	};

}

#endif