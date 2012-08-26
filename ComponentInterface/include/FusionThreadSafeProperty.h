/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#include "FusionAppType.h"
#include "FusionComponentProperty.h"
#include "FusionSerialisationHelper.h"

#define FSN_SYNCH_PROP_BOOL(prop) FSN_SYNCH_PROP_C(prop, Is ## prop, Set ## prop)
#define FSN_SYNCH_PROP(prop) FSN_SYNCH_PROP_C(prop, Get ## prop, Set ## prop)

#define FSN_PROP_ADDPROPERTY(prop) \
	prop.SetInterfaceObject(component->AddProperty(BOOST_PP_STRINGIZE(prop), &prop));

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

#define FSN_GET_SET (1, 1, FSN_INIT_PROP)
#define FSN_GET     (1, 0, FSN_INIT_PROP_R)
#define FSN_SET     (0, 1, FSN_INIT_PROP_W)
#define FSN_IS_SET  (1, 1, FSN_INIT_PROP_BOOL)
#define FSN_IS      (1, 0, FSN_INIT_PROP_BOOL_R)

// Accessors for the above tuple macros
#define FSN_PROP_IS_GETABLE(v) BOOST_PP_TUPLE_ELEM(3, 0, BOOST_PP_SEQ_ELEM(0, v))
#define FSN_PROP_IS_SETABLE(v) BOOST_PP_TUPLE_ELEM(3, 1, BOOST_PP_SEQ_ELEM(0, v))
#define FSN_PROP_EXEC_INIT_MACRO(v, p) BOOST_PP_TUPLE_ELEM(3, 2, BOOST_PP_SEQ_ELEM(0, v))(p)
#define FSN_PROP_IS_READONLY(v) BOOST_PP_NOT(FSN_PROP_IS_SETABLE(v))

#define FSN_PROP_GET_NAME(v) BOOST_PP_SEQ_ELEM(1, v)
#define FSN_PROP_GET_TYPE(v) BOOST_PP_SEQ_ELEM(2, v)
#define FSN_PROP_GET_SERIALISER(v) BOOST_PP_SEQ_ELEM(3, v)
#define FSN_PROP_HAS_SERIALISER(v) BOOST_PP_EQUAL( 4, BOOST_PP_SEQ_SIZE(v) )

#define FSN_INIT_PROPS(r, data, v) \
	FSN_PROP_EXEC_INIT_MACRO(v, BOOST_PP_SEQ_ELEM(1, v)); \
	FSN_PROP_ADDPROPERTY(BOOST_PP_SEQ_ELEM(1, v));
// Generates lines like:
//  ThreadSafeProperty<T, DefaultStaticWriter<T, GenericPropertySerialiser<T>>> prop_name;
#define FSN_DEFAULTWRITER_TYPENAME(v) BOOST_PP_CAT(FSN_PROP_GET_NAME(v),Writer_t)
#define FSN_SERIALISER_PARAM(v) BOOST_PP_IF(FSN_PROP_HAS_SERIALISER(v), FSN_PROP_GET_SERIALISER(v), GenericPropertySerialiser< FSN_PROP_GET_TYPE(v) >)
#define FSN_NULLWRITER_PARAM(v) NullWriter< ## FSN_PROP_GET_TYPE(v) ## >
#define FSN_DEFAULTWRITER_typedef(v) typedef DefaultStaticWriter< ## FSN_PROP_GET_TYPE(v) BOOST_PP_COMMA() FSN_SERIALISER_PARAM(v) ## > FSN_DEFAULTWRITER_TYPENAME(v);
#define FSN_DEFINE_PROPS(r, data, v) \
	ThreadSafeProperty<FSN_PROP_GET_TYPE(v)\
	BOOST_PP_COMMA() BOOST_PP_IF(FSN_PROP_IS_READONLY(v), FSN_NULLWRITER_PARAM(v), DefaultStaticWriter<FSN_PROP_GET_TYPE(v)>)\
	BOOST_PP_COMMA() BOOST_PP_IF(FSN_PROP_HAS_SERIALISER(v), FSN_PROP_GET_SERIALISER(v), GenericPropertySerialiser<FSN_PROP_GET_TYPE(v)>)\
	> FSN_PROP_GET_NAME(v);

#define FSN_COIFACE_PROPS(iface_name, properties) \
	void InitProperties()\
	{\
	typedef iface_name iface;\
	auto component = dynamic_cast<EntityComponent*>(this);\
	FSN_ASSERT(component);\
	BOOST_PP_SEQ_FOR_EACH(FSN_INIT_PROPS, _, properties) \
	}\
	BOOST_PP_SEQ_FOR_EACH(FSN_DEFINE_PROPS, _, properties)

#define FSN_COIFACE_CTOR FSN_COIFACE_PROPS

#define FSN_COIFACE(interface_name, properties)\
	FSN_BEGIN_COIFACE(interface_name)\
	FSN_COIFACE_PROPS(interface_name, properties)\
	FSN_END_COIFACE()

namespace FusionEngine
{

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

	//! Generic serialiser
	template <typename T, bool Continuous = false>
	struct GenericPropertySerialiser
	{
		static bool IsContinuous() { return Continuous; }
		static void Serialise(RakNet::BitStream& stream, const T& value)
		{
			SerialisationUtils::write(stream, value);
		}
		static void Deserialise(RakNet::BitStream& stream, T& value)
		{
			SerialisationUtils::read(stream, value);
		}
	};

	//! Container serialiser
	template <typename T>
	struct ContainerPropertySerialiser
	{
		static bool IsContinuous() { return false; }
		static void Serialise(RakNet::BitStream& stream, const T& value)
		{
			stream.Write(value.size());
			for (auto it = value.begin(); it != value.end(); ++it)
				stream.Write(value);
		}
		static void Deserialise(RakNet::BitStream& stream, T& value)
		{
			auto size = value.size();
			if (stream.Read(size))
			{
				value.resize(size);
				for (size_t i = 0; i < size; ++i)
				{
					if (stream.Read(value[i])) {} else break;
				}
			}
		}
	};

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
		
		virtual EntityComponent* GetObjectAsComponent() const = 0;
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
		
		EntityComponent* GetObjectAsComponent() const
		{
			FSN_ASSERT(m_Object);
			return dynamic_cast<EntityComponent*>(m_Object);
		}
	};

	//! Threadsafe property wrapper
	template <class T, class Writer = DefaultStaticWriter<T>, class Serialiser = GenericPropertySerialiser<T>>
	class ThreadSafeProperty : public IComponentProperty
	{
	private:
		// Non-copyable
		ThreadSafeProperty(const ThreadSafeProperty& other)
		{}

	public:
		typedef ThreadSafeProperty<T, Writer, Serialiser> This_t;

		ThreadSafeProperty()
			: m_PropertyID(-1),
			m_GetSetCallbacks(nullptr),
			m_Changed(true)
		{
			m_SubscriptionAgent.SetHandlerFn(std::bind(&This_t::Set, this, std::placeholders::_1));
		}
		
		explicit ThreadSafeProperty(const T& value)
			: m_PropertyID(-1),
			m_GetSetCallbacks(nullptr),
			m_Changed(true),
			m_Value(value)
		{
			m_SubscriptionAgent.SetHandlerFn(std::bind(&This_t::Set, this, std::placeholders::_1));
		}

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
			m_GetSetCallbacks = new GetSetCallback<C, value_type_for_get, value_type_for_set>(obj, get_fn, set_fn);
		}

		~ThreadSafeProperty()
		{
			if (m_GetSetCallbacks)
				delete m_GetSetCallbacks;
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

		void SetInterfaceObject(const boost::intrusive_ptr<ComponentProperty>& obj) { m_InterfaceObject = obj; }

		ComponentProperty* GetInterfaceObject() const
		{
			if (m_InterfaceObject)
				m_InterfaceObject->addRef();
			return m_InterfaceObject.get();
		}

		PropertyID GetID() const { return m_PropertyID; }

		void AquireSignalGenerator(PropertySignalingSystem_t& system, PropertyID own_id)
		{
			//m_ChangedCallback = system.MakeGenerator<const T&>(own_id, std::bind(&This_t::Get, this));
			m_ChangedCallback = system.MakeGenerator<const T&>(own_id, [this]()->const T& { this->Synchronise(); return this->Get(); });
			m_PropertyID = own_id;
			m_SubscriptionAgent.ActivateSubscription(system);
		}

		void Follow(PropertySignalingSystem_t& system, PropertyID, PropertyID id)
		{
			if (!std::is_same<Writer, NullWriter<T>>::value)
			{
				m_SubscriptionAgent.Subscribe(system, id);
			}
		}

		void Synchronise()
		{
			FSN_ASSERT(m_GetSetCallbacks);

			if (m_Writer.DumpWrittenValue(m_Value))
			{
				m_GetSetCallbacks->Set(m_Value);
			}
			else if (m_Changed)
			{
				m_Value = m_GetSetCallbacks->Get();
				m_Changed = false;
			}
		}

		void Serialise(RakNet::BitStream& stream)
		{
			// Non-writable properties don't need to be synched
			//  (one assumes that they represent some dependent
			//  or constant value)
			if (!std::is_same<Writer, NullWriter<T>>::value)
			{
				m_SubscriptionAgent.SaveSubscription(stream);
				Serialiser::Serialise(stream, m_Value);
			}
		}

		void Deserialise(RakNet::BitStream& stream)
		{
			if (!std::is_same<Writer, NullWriter<T>>::value)
			{
				m_SubscriptionAgent.LoadSubscription(stream);
				T temp;
				Serialiser::Deserialise(stream, temp);
				m_Writer.Write(temp);
			}
		}

		void DeserialiseNonConcurrent(RakNet::BitStream& stream)
		{
			if (!std::is_same<Writer, NullWriter<T>>::value)
			{
				Synchronise();
				m_SubscriptionAgent.LoadSubscription(stream);
				Serialiser::Deserialise(stream, m_Value);
				m_GetSetCallbacks->Set(m_Value);
			}
		}

		bool IsContinuous() const
		{
			return Serialiser::IsContinuous();
		}

		template <typename T>
		void SetSpecific(void* value)
		{
			m_Writer.Write(*static_cast<T*>(value));
		}
		template <>
		void SetSpecific<bool>(void* value)
		{
			m_Writer.Write(reinterpret_cast<bool>(value));
		}

		void* GetRef()
		{
			return &m_Value;
		}
		void Set(void* value, int type_id)
		{
			if (Scripting::AppType<T>::type_id == type_id)
			{
				SetSpecific<T>(value);

				if (m_ChangedCallback)
					m_ChangedCallback();
			}
		}

		int GetTypeId() const
		{
			return Scripting::AppType<T>::type_id;
		}

		//bool IsEqual(IComponentProperty* other) const
		//{
		//	if (auto sametype = dynamic_cast<ThreadSafeProperty*>(other))
		//		return sametype->m_Value == this->m_Value;
		//	else
		//		return false;
		//}

		//! Mark changed
		void MarkChanged()
		{
			m_Changed = true;
			if (m_ChangedCallback)
				m_ChangedCallback();
		}

		const T& Get() const { return m_Value; }
		void Set(const T& value)
		{
			m_Writer.Write(value);

			if (m_ChangedCallback)
				m_ChangedCallback();
		}

	private:
		typename PropertySignalingSystem_t::GeneratorDetail_t::Impl<const T&>::GeneratorFn_t m_ChangedCallback;

		PersistentConnectionAgent<const T&> m_SubscriptionAgent;

		PropertyID m_PropertyID;

		IGetSetCallback<value_type_for_get, value_type_for_set>* m_GetSetCallbacks;

		boost::intrusive_ptr<ComponentProperty> m_InterfaceObject;

		bool m_Changed;
	public:
		T m_Value;
	private:

		Writer m_Writer;
	};

}

#endif
