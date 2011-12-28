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

#define FSN_TSP_SIGNALS2

#include <boost/preprocessor.hpp>
#ifdef FSN_TSP_SIGNALS2
#include <boost/signals2/signal.hpp>
#else
#include <boost/signal.hpp>
#include <tbb/mutex.h>
#endif

#include <boost/any.hpp>

#include <tbb/enumerable_thread_specific.h>

#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionScriptedSlots.h"

#define FSN_SYNCH_PROP_C(prop, get, set) \
	if ((prop.m_Changed && prop.Synchronise( get() )) || prop.SynchroniseExternalOnly())\
	set(prop.Get());

#define FSN_SYNCH_PROP_BOOL(prop) FSN_SYNCH_PROP_C(prop, Is ## prop, Set ## prop)
#define FSN_SYNCH_PROP(prop) FSN_SYNCH_PROP_C(prop, Get ## prop, Set ## prop)

#define FSN_PROPA(type, prop) \
	FSN_PROP_X(ThreadSafeProperty<type>, prop)

//! Prop
#define FSN_PROP_X(propType, member) \
	std::pair<std::string, propType> &get_ ## member() const { return std::make_pair(#member, member); }

#define FSN_PROP(type, prop) ;
	//ThreadSafeProperty<type> prop; \
	//std::pair<std::string, ThreadSafeProperty<type>*> &get_ ## prop() const { return std::make_pair(#prop, &prop); }

//! Readonly prop
#define FSN_PROP_R(type, prop) \
	ThreadSafeProperty<type, NullWriter<type>> &get_ ## prop() { return prop; }


#define FSN_INIT_PROP(prop) \
	prop.SetCallbacks(this, &iface::Get ## prop, &iface::Set ## prop)

#define FSN_INIT_PROP_R(prop) \
	prop.SetCallbacks<iface>(this, &iface::Get ## prop, nullptr)

#define FSN_INIT_PROP_BOOL(prop) \
	prop.SetCallbacks(this, &iface::Is ## prop, &iface::Set ## prop)

#define FSN_INIT_PROP_BOOL_R(prop) \
	prop.SetCallbacks<iface>(this, &iface::Is ## prop, nullptr)

#define FSN_INIT_PROP_W(prop) \
	prop.SetCallbacks<iface>(this, nullptr, &iface::Set ## prop)

#define FSN_GET_SET(prop) FSN_INIT_PROP(prop)
#define FSN_GET(prop) FSN_INIT_PROP_R(prop)
#define FSN_SET(prop) FSN_INIT_PROP_W(prop)
#define FSN_IS_SET(prop) FSN_INIT_PROP_BOOL(prop)
#define FSN_IS(prop) FSN_INIT_PROP_BOOL_R(prop)

#define FSN_INIT_PROPS(r, data, v) BOOST_PP_SEQ_ELEM(0, v)( BOOST_PP_SEQ_ELEM(1, v) );

namespace FusionEngine
{
	class IComponentProperty
	{
	public:
		virtual void FireSignal() = 0;
		virtual void Synchronise() = 0;
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
			if (m_WriteBuffers.empty() == false)
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
		DefaultStaticWriter() : m_Written(false) {}\
		void Write(const t& value) { m_WriteBuffer = value; m_Written = true; }\
		bool DumpWrittenValue(t& into)\
		{\
			if (m_Written && into != m_WriteBuffer)\
			{\
				into = m_WriteBuffer;\
				m_Written = false;\
				return true;\
			}\
			return false;\
		}\
		tbb::atomic<t> m_WriteBuffer;\
		bool m_Written;\
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

#ifdef FSN_TSP_SIGNALS2
	typedef boost::signals2::connection ThreadSafePropertyConnection;
#else
	typedef boost::signals::connection ThreadSafePropertyConnection;
#endif

	// Here is some complicated bullshit that saves maybe 1 ptr's worth of memory
	//  (over using 2 std::function objects)
	// Altho, it does allow the handy GetObjectAsComponent method
	template <class GetT, class SetT>
	class IGetSetCallback
	{
	public:
		virtual ~IGetSetCallback() {}
		virtual void Set(SetT) = 0;
		virtual GetT Get() const = 0;
		
		virtual IComponent* GetObjectAsComponent() const = 0;
	};

	template <class C, class GetT, class SetT>
	class GetSetCallback : public IGetSetCallback<GetT, SetT>
	{
	public:
		typedef SetT value_type_for_set;
		typedef GetT value_type_for_get;

		typedef void (C::*set_fn_t)(value_type_for_set);
		typedef value_type_for_get (C::*get_fn_t)(void) const;

		C *m_Object;
		set_fn_t m_SetFn;
		get_fn_t m_GetFn;

		GetSetCallback(C *obj, get_fn_t get_fn, set_fn_t set_fn)
			: m_Object(obj),
			m_SetFn(set_fn),
			m_GetFn(get_fn)
		{}

		void Set(SetT value)
		{
			FSN_ASSERT(m_Object); FSN_ASSERT(m_SetFn);
			(m_Object->*m_SetFn)(value);
		}
		GetT Get() const
		{
			FSN_ASSERT(m_Object); FSN_ASSERT(m_GetFn);
			return (m_Object->*m_GetFn)();
		}
		
		IComponent* GetObjectAsComponent() const
		{
			FSN_ASSERT(m_Object);
			return dynamic_cast<IComponent*>(m_Object);
		}
	};

	//! Threadsafe property wrapper
	template <class T, class Writer = DefaultStaticWriter<T>>
	class ThreadSafeProperty : public IComponentProperty
	{
	private:
		ThreadSafeProperty(const ThreadSafeProperty& other)
		{}

	public:
#ifndef FSN_TSP_SIGNALS2
		tbb::mutex m_Mutex;
#endif

		ThreadSafeProperty()
			: m_Changed(true),
			m_Owner(nullptr),
			m_GetSetCallbacks(nullptr),
			m_Refs(1)
		{}

		// Fundimental and enum types are passed to "Set" by value
		typedef typename std::conditional<
				std::is_fundamental<T>::value || std::is_enum<T>::value,
			T, const T &>::type value_type_for_set;
		// Fundimental, enum and Vector types are returned from "Get" by value
		typedef typename std::conditional<
			std::is_fundamental<T>::value || std::is_enum<T>::value || std::is_same<T, Vector2>::value || std::is_same<T, Vector2i>::value,
			T, const T &>::type value_type_for_get;

		template <class C>
		void SetCallbacks(C* obj, value_type_for_get (C::*get_fn)(void) const, void (C::*set_fn)(value_type_for_set))
		{
			typedef void (C::*set_fn_t)(value_type_for_set);
			typedef value_type_for_get (C::*get_fn_t)(void) const;

			m_GetSetCallbacks = new GetSetCallback<C, value_type_for_get, value_type_for_set>(obj, get_fn, set_fn);

			GetOwner();

			//m_Object = obj;

			//m_Get = get_fn;
			//m_Set = set_fn;
		}

		explicit ThreadSafeProperty(const T& value)
			: m_Changed(true),
			m_Owner(nullptr),
			m_Value(value),
			m_Refs(1)
		{}

		~ThreadSafeProperty()
		{
			FSN_ASSERT(m_Refs <= 1);
			m_Connection.disconnect();
		}
		
		ThreadSafeProperty& operator= (const ThreadSafeProperty& copy)
		{
			Set(copy.m_Value);
			return *this;
		}

		ThreadSafeProperty& operator= (const T& value)
		{
			Set(value);
			return *this;
		}

		//! Connect an observer to this property
		ThreadSafePropertyConnection Connect(const std::function<void (const T&)>& callback)
		{
#ifndef FSN_TSP_SIGNALS2
			tbb::mutex::scoped_lock lock(m_Mutex);
#endif
			return m_Signal.connect(callback);
		}

		//! Bind this property's value to another property
		void BindProperty(ThreadSafeProperty& other)
		{
#ifndef FSN_TSP_SIGNALS2
			tbb::mutex::scoped_lock lock(m_Mutex);
#endif
			//using namespace std::placeholders;
			if (m_Connection.connected())
				m_Connection.disconnect();
			m_Connection = other.m_Signal.connect(boost::bind(&ThreadSafeProperty<T, Writer>::Set, this, boost::arg<1>()));//std::bind(&Set, this, _1));
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

		bool SynchroniseExternalOnly()
		{
			if (m_Writer.DumpWrittenValue(m_Value))
			{
				m_Changed = true;
				return true;
			}
			else
				return false;
		}

		void Synchronise()
		{
			FSN_ASSERT(m_GetSetCallbacks);

			if (m_Writer.DumpWrittenValue(m_Value))
			{
				m_Changed = true;
				m_GetSetCallbacks->Set(m_Value);
			}
			else if (m_Changed)
			{
				m_Value = m_GetSetCallbacks->Get();
			}
		}

		//! Fire signal
		void FireSignal()
		{
			if (m_Changed)
			{
				m_Signal(m_Value);
				m_Changed = false;
			}
			else
			{
				m_Changed = false;
			}
		}

		//! Mark changed
		void MarkChanged()
		{
			GetOwner()->OnPropertyChanged(this);
			m_Changed = true;
		}

		const T& Get() const { return m_Value; }
		void Set(const T& value)
		{
			m_Writer.Write(value);

			GetOwner()->OnPropertyChanged(this);
		}

	private:
		static bool registered; // Gets to true when this type has been registered as a script type
	public:
		static void RegisterProp(asIScriptEngine* engine, const std::string& type)
		{
			if (registered)
				return;
			registered = true;

			std::string cname = "Property_" + type + "_";
			if (std::is_same<Writer, NullWriter<T>>::value)
				cname = "ReadonlyProperty_" + type + "_";

			typedef ThreadSafeProperty<T, Writer> this_type;
			typedef Scripting::Registation::ValueTypeHelper<this_type> helper_type;

			int r;
			//r = engine->RegisterObjectType(cname.c_str(), sizeof(ThreadSafeProperty<T, Writer>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA); FSN_ASSERT(r >= 0);
			//r = engine->RegisterObjectBehaviour(cname.c_str(), 
			//	asBEHAVE_CONSTRUCT, 
			//	"void f()", 
			//	asFUNCTION(helper_type::Construct), 
			//	asCALL_CDECL_OBJLAST);
			//FSN_ASSERT(r >= 0);
			////r = engine->RegisterObjectBehaviour(cname.c_str(), 
			////	asBEHAVE_CONSTRUCT, 
			////	("void f(const " + type + " &in)").c_str(), 
			////	asFUNCTIONPR(helper_type::Construct, (const T&, this_type*), void),
			////	asCALL_CDECL_OBJLAST);
			////FSN_ASSERT(r >= 0);
			//r = engine->RegisterObjectBehaviour(cname.c_str(),
			//	asBEHAVE_DESTRUCT,
			//	"void f()",
			//	asFUNCTION(helper_type::Destruct),
			//	asCALL_CDECL_OBJLAST);
			//FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectType(cname.c_str(), 0, asOBJ_REF); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectBehaviour(cname.c_str(), asBEHAVE_ADDREF, "void f()", asMETHOD(this_type, AddRef), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectBehaviour(cname.c_str(), asBEHAVE_RELEASE, "void f()", asMETHOD(this_type, Release), asCALL_THISCALL); FSN_ASSERT(r >= 0);

			//int subTypeId = engine->GetTypeIdByDecl(type.c_str());
			//bool referenceSubtype = engine->GetObjectTypeById(subTypeId)->GetFlags() & asOBJ_REF;
			bool referenceSubtype = false;

			r = engine->RegisterObjectMethod(cname.c_str(), ("void set_value(const " + type + " &in)").c_str(), asMETHOD(this_type, Set), asCALL_THISCALL); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod(cname.c_str(), (type + " get_value() const").c_str(), asMETHOD(this_type, ImplicitCast), asCALL_THISCALL); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod(cname.c_str(), (cname + " &opShl(const " + type + "&in)").c_str(), asMETHOD(this_type, opAssign), asCALL_THISCALL); FSN_ASSERT(r >= 0);

			//r = engine->RegisterObjectMethod(cname.c_str(), (cname + " &opAssign(const " + cname + " &in)").c_str(), asMETHODPR(this_type, operator=, (const ThreadSafeProperty&), ThreadSafeProperty&), asCALL_THISCALL); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod(cname.c_str(), (cname + " &opAssign(const " + type + " &in)").c_str(), asMETHOD(this_type, opAssign), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod(cname.c_str(), (cname + " &opAddAssign(const " + type + " &in)").c_str(), asMETHOD(this_type, opAddAssign), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			if (!std::is_same<T, bool>::value)
			{
				if (!referenceSubtype)
				{
					r = engine->RegisterObjectBehaviour(cname.c_str(), asBEHAVE_IMPLICIT_VALUE_CAST, (type + " f() const").c_str(), asMETHOD(this_type, ImplicitCast), asCALL_THISCALL); FSN_ASSERT(r >= 0);
				}
				else
				{
					r = engine->RegisterObjectBehaviour(type.c_str(), asBEHAVE_FACTORY, (type + " @f(const " + cname + " &in)").c_str(), asFUNCTION(this_type::TeaFactory), asCALL_CDECL); FSN_ASSERT(r >= 0);
				}
			}

			r = engine->RegisterObjectMethod(cname.c_str(), "SignalConnection @connect(const string &in)", asMETHOD(this_type, connect), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod(cname.c_str(), ("void bindProperty(" + cname + " @)").c_str(), asMETHOD(this_type, BindProperty), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		}

		void AddRef()
		{
			++m_Refs;
		}

		void Release()
		{
			--m_Refs;
		}

		ThreadSafeProperty &opAssign(const T& value) { Set(value); return *this; }
		ThreadSafeProperty &opAddAssign(const T& value)
		{
			// Only arithemtic types, other than bool, can be used with the += operator
			typedef std::integral_constant<bool,
				!std::is_same<T, bool>::value &&
				std::is_arithmetic<T>::value> canAssign_t;
			return opAddAssignImpl(value, canAssign_t());
		}
		ThreadSafeProperty &opAddAssignImpl(const T& value, const std::true_type&) { Set(Get() + value); return *this; }
		ThreadSafeProperty &opAddAssignImpl(const T& value, const std::false_type&) { asGetActiveContext()->SetException("opAddAssign is invalid for this type"); return *this; }

		T ImplicitCast() const { return Get(); }
		static T *TeaFactory(const ThreadSafeProperty& prop)
		{
			return new T(prop.Get());
		}

		ScriptedSlotWrapper* connect(const std::string& script_fn)
		{
			using namespace std::placeholders;
			auto wrapper = ScriptedSlotWrapper::CreateWrapperFor(asGetActiveContext(), script_fn);
#ifdef FSN_TSP_SIGNALS2
			wrapper->HoldConnection(Connect(std::bind(&ScriptedSlotWrapper::CallbackRef<T>, wrapper, _1)));
#endif
			return wrapper;
		};

		void bindProperty(ThreadSafeProperty<T, Writer>* other)
		{
#ifndef FSN_TSP_SIGNALS2
			tbb::mutex::scoped_lock lock(m_Mutex);
#endif
			//using namespace std::placeholders;
			if (m_Connection.connected())
				m_Connection.disconnect();
			m_Connection = other->m_Signal.connect(boost::bind(&ThreadSafeProperty<T, Writer>::Set, this, boost::arg<1>()));//std::bind(&Set, this, _1));
		}

	private:
		unsigned int m_Refs;

		IComponent* GetOwner()
		{
			// The unusual ordering is to (hopefully) avoid branch misprediction:
			if (m_Owner)
				return m_Owner;
			else
			{
				FSN_ASSERT(m_GetSetCallbacks);
				m_Owner = m_GetSetCallbacks->GetObjectAsComponent();
				m_Owner->AddProperty(this);
				return m_Owner;
			}
		}

		IComponent* m_Owner;
		IGetSetCallback<value_type_for_get, value_type_for_set>* m_GetSetCallbacks;

		//std::function<void (value_type_for_set)> m_SetFn;
		//std::function<value_type_for_get (void) const> m_GetFn;

#ifdef FSN_TSP_SIGNALS2
		boost::signals2::signal<void (const T&)> m_Signal;
#else
		boost::signal<void (const T&)> m_Signal;
#endif
		ThreadSafePropertyConnection m_Connection; // Properties can bind directly to other properties

		bool m_Changed;
	public:
		T m_Value;
	private:

		Writer m_Writer;
	};

	template <class T, class Writer>
	bool ThreadSafeProperty<T, Writer>::registered = false;

}

#endif
