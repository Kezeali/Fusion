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

#include <RakNet/BitStream.h>

#define FSN_TSP_SIGNALS2

#include <boost/preprocessor.hpp>
#ifdef FSN_TSP_SIGNALS2
#include <boost/signals2/signal.hpp>
#else
#include <boost/signal.hpp>
#include <tbb/mutex.h>
#endif

#include <boost/any.hpp>
#include <boost/intrusive_ptr.hpp>

#include <tbb/enumerable_thread_specific.h>

#include "FusionAppType.h"
#include "FusionComponentProperty.h"
#include "FusionSerialisationHelper.h"

#include "FusionPropertyDeclarationUtils.h"

// Generates lines like:
//  ThreadSafeProperty<T, DefaultStaticWriter<T>, GenericPropertySerialiser<T>> prop_name;
#define FSN_BUFFERED_DEFAULTWRITER_TYPENAME(v) BOOST_PP_CAT(FSN_PROP_GET_NAME(v),Writer_t)
#define FSN_BUFFERED_SERIALISER_PARAM(v) BOOST_PP_IF(FSN_PROP_HAS_SERIALISER(v), FSN_PROP_GET_SERIALISER(v), GenericPropertySerialiser< FSN_PROP_GET_TYPE(v) >)
#define FSN_BUFFERED_NULLWRITER_PARAM(v) NullWriter< ## FSN_PROP_GET_TYPE(v) ## >
#define FSN_BUFFERED_DEFAULTWRITER_typedef(v) typedef DefaultStaticWriter< ## FSN_PROP_GET_TYPE(v) BOOST_PP_COMMA() FSN_BUFFERED_SERIALISER_PARAM(v) ## > FSN_BUFFERED_DEFAULTWRITER_TYPENAME(v);
#define FSN_DEFINE_BUFFERED_PROPS(r, data, v) \
	ThreadSafeProperty<FSN_PROP_GET_TYPE(v)\
	BOOST_PP_COMMA() BOOST_PP_IF(FSN_PROP_IS_READONLY(v), FSN_BUFFERED_NULLWRITER_PARAM(v), DefaultStaticWriter<FSN_PROP_GET_TYPE(v)>)\
	BOOST_PP_COMMA() BOOST_PP_IF(FSN_PROP_HAS_SERIALISER(v), FSN_PROP_GET_SERIALISER(v), GenericPropertySerialiser<FSN_PROP_GET_TYPE(v)>)\
	> FSN_PROP_GET_NAME(v);

#define FSN_PROP_SET_SYNCH_CALLBACK(prop) \
	prop.SetRequestSynchonisationCallback(std::bind(&PropertySynchronsier::Enqueue, component->GetSynchroniser(), std::placeholders::_1));

#define FSN_INIT_BUFFERED_PROPS(r, data, v) \
	FSN_PROP_EXEC_INIT_MACRO(v, BOOST_PP_SEQ_ELEM(1, v)); \
	FSN_PROP_ADDPROPERTY(BOOST_PP_SEQ_ELEM(1, v)); \
	FSN_PROP_SET_SYNCH_CALLBACK(BOOST_PP_SEQ_ELEM(1, v));

#define FSN_COIFACE_BUFFERED_PROPS(iface_name, properties) \
	void InitProperties()\
	{\
	typedef iface_name iface;\
	auto component = dynamic_cast<SynchronisingComponent*>(this);\
	FSN_ASSERT(component);\
	BOOST_PP_SEQ_FOR_EACH(FSN_INIT_BUFFERED_PROPS, _, properties) \
	}\
	BOOST_PP_SEQ_FOR_EACH(FSN_DEFINE_BUFFERED_PROPS, _, properties)

#define FSN_COIFACE_BUFFERED(interface_name, properties)\
	FSN_BEGIN_COIFACE(interface_name)\
	FSN_COIFACE_BUFFERED_PROPS(interface_name, properties)\
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

	//! Threadsafe property wrapper
	template <class T, class Writer = DefaultStaticWriter<T>, class Serialiser = GenericPropertySerialiser<T>>
	class ThreadSafeProperty : public IComponentProperty
	{
	private:
		// Non-copyable
		ThreadSafeProperty(const ThreadSafeProperty& other)
		{}

	public:
		typedef typename ThreadSafeProperty<T, Writer, Serialiser> This_t;

		ThreadSafeProperty()
			: m_PropertyID(-1),
			m_GetSetCallbacks(nullptr),
			m_Changed(true)
		{
			m_ReadA = false;
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
		
		ThreadSafeProperty& operator= (const ThreadSafeProperty& other)
		{
			Set(other.Value());
			return *this;
		}

		ThreadSafeProperty& operator= (const T& value)
		{
			Set(value);
			return *this;
		}

		void SetInterfaceObject(const boost::intrusive_ptr<ComponentProperty>& obj)
		{
			m_InterfaceObject = obj;
		}

		// Used to generate objects used by the script interface
		ComponentProperty* GetInterfaceObject() const
		{
			if (m_InterfaceObject)
				m_InterfaceObject->addRef();
			return m_InterfaceObject.get();
		}

		PropertyID GetID() const { return m_PropertyID; }

		typedef std::function<void (IComponentProperty*)> ChangedCallback_t;

		void SetRequestSynchonisationCallback(const ChangedCallback_t& function)
		{
			m_RequestSynchronisationCallback = function;
		}

		void AquireSignalGenerator(PropertySignalingSystem_t& system, PropertyID own_id) override
		{
			m_PropertyID = own_id;
		}

		void Follow(PropertySignalingSystem_t& system, PropertyID, PropertyID id) override
		{
		}

		void Synchronise() override
		{
			FSN_ASSERT(m_GetSetCallbacks);

			if (m_Writer.DumpWrittenValue(Value()))
			{
				m_GetSetCallbacks->Set(Get());
			}
			else if (m_Changed)
			{
				Value() = m_GetSetCallbacks->Get();
				m_Changed = false;
			}
		}

		void Serialise(RakNet::BitStream& stream) override
		{
			// Non-writable properties don't need to be synched
			//  (one assumes that they represent some dependent
			//  or constant value)
			if (!std::is_same<Writer, NullWriter<T>>::value)
			{
				Serialiser::Serialise(stream, Get());
			}
		}

		void Deserialise(RakNet::BitStream& stream) override
		{
			if (!std::is_same<Writer, NullWriter<T>>::value)
			{
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
				Serialiser::Deserialise(stream, Value());
				m_GetSetCallbacks->Set(Get());
			}
		}

		bool IsContinuous() const override
		{
			return Serialiser::IsContinuous();
		}

		template <typename X>
		void SetSpecific(void* value)
		{
			Set(*static_cast<X*>(value));
		}
		template <>
		void SetSpecific<bool>(void* value)
		{
			Set(reinterpret_cast<bool>(value));
		}

		void* GetRef() override
		{
			return &(Value());
		}
		void SetAny(void* value, int type_id) override
		{
			if (Scripting::RegisteredAppType<T>::type_id == type_id)
			{
				SetSpecific<T>(value);
			}
		}

		int GetTypeId() const override
		{
			return Scripting::RegisteredAppType<T>::type_id;
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
			if (m_RequestSynchronisationCallback)
				m_RequestSynchronisationCallback(this);
		}

		const T& Get() const
		{
			return Value();
		}
		void Set(const T& value)
		{
			m_Writer.Write(value);
			if (m_RequestSynchronisationCallback)
				m_RequestSynchronisationCallback(this);
		}

	private:
		PropertyID m_PropertyID;

		IGetSetCallback<value_type_for_get, value_type_for_set>* m_GetSetCallbacks;

		boost::intrusive_ptr<ComponentProperty> m_InterfaceObject;

		bool m_Changed;
		T m_ValueA, m_ValueB;
		tbb::atomic<bool> m_ReadA;

		Writer m_Writer;

		ChangedCallback_t m_RequestSynchronisationCallback;

		T& Value()
		{
			if (m_ReadA)
				return m_ValueA;
			else
				return m_ValueB;
		}
		
		const T& Value() const
		{
			if (m_ReadA)
				return m_ValueA;
			else
				return m_ValueB;
		}

	};

}

#endif
