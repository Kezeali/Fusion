/*
*  Copyright (c) 2011 Fusion Project Team
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

#ifndef H_FusionThreadSafeProperty
#define H_FusionThreadSafeProperty

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <angelscript.h>

#include <vector>
#include <functional>
#include <type_traits>

#include <BitStream.h>

#include <boost/mpl/vector.hpp>
#include <boost/preprocessor.hpp>
#include <boost/signals2/signal.hpp>

#include <tbb/enumerable_thread_specific.h>

#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionScriptedSlots.h"

#define FSN_SYNCH_PROP_C(prop, get, set) \
	if (prop.Synchronise( get() ))\
	set(prop.Get());

#define FSN_SYNCH_PROP_BOOL(prop) FSN_SYNCH_PROP_C(prop, Is ## prop, Set ## prop)
#define FSN_SYNCH_PROP(prop) FSN_SYNCH_PROP_C(prop, Get ## prop, Set ## prop)

namespace FusionEngine
{
	class IComponentProperty
	{
	public:
		virtual void FireSignal() = 0;
	};

	template <typename T>
	struct RefReader
	{
		RefReader(const T& ref_)
			: ref(ref_)
		{}

		const T& Read() { return ref; }

		const T& ref;
	};

	template <typename T>
	struct ValueReader
	{
		const T& Read() { return value; }

		T value;
	};

	template <typename T>
	struct FunctionReader
	{
		FunctionReader(const std::function<const T& (void)>& fn)
			: readFunction(fn)
		{}

		const T& Read() { return readFunction(); }

		std::function<const T& (void)> readFunction;
	};

	template <typename T>
	struct DefaultStaticWriter
	{
		void Write(const T& value) { m_WriteBuffers.local() = value; }
		bool DumpWrittenValue(T& into)
		{
			if (m_WriteBuffers.size() > 0)
			{
				into = *m_WriteBuffers.begin();
				m_WriteBuffers.clear();
				return true;
			}
			return false;
		}
		tbb::enumerable_thread_specific<T> m_WriteBuffers;
	};

#define INTEGRAL_TYPES (bool) (char) (signed char) (unsigned char) (short) (unsigned short) (int) (unsigned) (long) (unsigned long) (long long)

#define INTEGRAL_WRITER_SPECIALISATION(r,data,t)\
	template <>\
	struct DefaultStaticWriter<t>\
	{\
		void Write(const t& value) { m_WriteBuffer = value; }\
		bool DumpWrittenValue(t& into)\
		{\
			if (into != m_WriteBuffer)\
			{\
				into = m_WriteBuffer;\
				return true;\
			}\
			return false;\
		}\
		tbb::atomic<t> m_WriteBuffer;\
	};

	BOOST_PP_SEQ_FOR_EACH(INTEGRAL_WRITER_SPECIALISATION, _, INTEGRAL_TYPES)

#undef INTEGRAL_WRITER_SPECIALISATION
#undef INTEGRAL_TYPES

	//! For properties that can't be written
	template <typename T>
	struct NullWriter
	{
		void Write(const T&) { FSN_ASSERT_FAIL("Can't set this property"); }
		bool DumpWrittenValue(T&) { return false; }
	};

	//static void RegisterPropConnection(asIScriptEngine* engine)
	//{
	//	RegisterValueType<boost::signals2::connection>("SigConnection", engine, asOBJ_APP_CLASS_CAK);
	//	int r;
	//	r = engine->RegisterObjectMethod("PropConnection", "void disconnect()", asMETHOD(boost::signals2::connection, disconnect), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	//}

	//! Threadsafe property wrapper
	template <typename T, class Writer = DefaultStaticWriter<T>>
	class ThreadSafeProperty : public IComponentProperty
	{
	public:
		typedef boost::signals2::connection Connection;

		ThreadSafeProperty()
		{}

		~ThreadSafeProperty()
		{
			m_Connection.disconnect();
		}
		
		ThreadSafeProperty<T>& operator= (const ThreadSafeProperty<T>& copy)
		{
			m_Value = copy.m_Value;
			return *this;
		}

		//! Connect an observer to this property
		Connection Connect(const std::function<void (const T&)>& callback)
		{
			return m_Signal.connect(callback);
		}

		//! Bind this property's value to another property
		void BindProperty(ThreadSafeProperty<T>& prop)
		{
			using namespace std::placeholders;
			if (m_Connection.connected())
				m_Connection.disconnect();
			m_Connection = prop.Connect(std::bind(&Set, this, _1));
		}

		//! Synchronise parallel (thread) writes
		bool Synchronise(const T& sim_value)
		{
			// With NullWriter this first conditional is optimised away (in MSVC, at least)
			if (m_Writer.DumpWrittenValue(m_Value))
			{
				m_Changed = true;
				return true;
			}
			else if (m_Changed)
			{
				m_Value = sim_value;
			}
			return false;
		}

		bool Synchronise(void)
		{
			if (m_Writer.DumpWrittenValue(m_Value))
			{
				m_Changed = true;
				return true;
			}
			else
				return false;
		}

		//template <class C>
		//void Synchronise(const T& sim_value, C set_obj, void (C::*set_fn)(const T&))
		//{
		//	if (m_Writer.DumpWrittenValue(m_Value))
		//	{
		//		m_ChangedSinceSerialised = m_Changed = true;
		//		(set_obj->*set_fn)(m_Value);
		//	}
		//	else if (m_Changed)
		//	{
		//		m_Value = sim_value;
		//	}
		//}

		//template <class C>
		//void Synchronise(const T& sim_value, C set_obj, void (C::*set_fn)(T))
		//{
		//	if (m_Writer.DumpWrittenValue(m_Value))
		//	{
		//		m_ChangedSinceSerialised = m_Changed = true;
		//		(set_obj->*set_fn)(m_Value);
		//	}
		//	else if (m_Changed)
		//	{
		//		m_Value = sim_value;
		//	}
		//}

		//! Fire signal
		void FireSignal()
		{
			if (m_Changed)
			{
				m_Signal(m_Value);
				m_Changed = false;
			}
		}

		//! Mark changed
		void MarkChanged()
		{
			m_Changed = true;
		}

		const T& Get() const { return m_Value; }
		void Set(const T& value) { m_Writer.Write(value); }

		static void RegisterProp(asIScriptEngine* engine, const std::string& cname, const std::string& type)
		{
			int r;
			//RegisterValueType<ThreadSafeProperty<T>>(cname.c_str(), engine, asOBJ_APP_CLASS_DK);
			r = engine->RegisterObjectType(cname.c_str(), sizeof(ThreadSafeProperty<T>), asOBJ_VALUE | asOBJ_APP_CLASS_DA); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod(cname.c_str(), (type + " opAssign(const " + type + " &in)").c_str(), asMETHOD(ThreadSafeProperty<T>, opAssign), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod(cname.c_str(), (type + " opAddAssign(const " + type + " &in)").c_str(), asMETHOD(ThreadSafeProperty<T>, opAddAssign), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectBehaviour(cname.c_str(), asBEHAVE_IMPLICIT_VALUE_CAST, (type + " f() const").c_str(), asMETHOD(ThreadSafeProperty<T>, ImplicitCast), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod(cname.c_str(), ("SignalConnection connect(const string &in)").c_str(), asMETHOD(ThreadSafeProperty<T>, connect), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod(cname.c_str(), ("void bindProperty(" + cname + " &in)").c_str(), asMETHOD(ThreadSafeProperty<T>, BindProperty), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		}

	private:
		T opAssign(const T& value) { Set(value); return Get(); }
		T opAddAssign(const T& value) { opAddAssignImpl(value, typename std::is_arithmetic<T>()); }
		T opAddAssignImpl(const T& value, const std::true_type&) { Set(Get() + value); return Get(); }
		T opAddAssignImpl(const T& value, const std::false_type&) { asGetActiveContext()->SetException("opAddAssign is invalid for this type"); }

		T ImplicitCast() const { return Get(); }

		ScriptedSlotWrapper* connect(const std::string& script_fn)
		{
			using namespace std::placeholders;
			auto wrapper = ScriptedSlotWrapper::CreateWrapperFor(asGetActiveContext(), script_fn);
			wrapper->HoldConnection(Connect(std::bind(&ScriptedSlotWrapper::CallbackRef<T>, wrapper, _1)));
			return wrapper;
		};

	private:
		boost::signals2::signal<void (const T&)> m_Signal;
		boost::signals2::connection m_Connection; // Properties can bind directly to other properties

		bool m_Changed;
		T m_Value;

		Writer m_Writer;
	};

}

#endif
