/*
*  Copyright (c) 2013 Fusion Project Team
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

#ifndef H_FusionStandardProperty
#define H_FusionStandardProperty

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionPropertyDeclarationUtils.h"

#include "FusionComponentProperty.h"

#include <boost/intrusive_ptr.hpp>

// Generates lines like:
//  ThreadSafeProperty<T, GenericPropertySerialiser<T>> prop_name;
#define FSN_DEFINE_PROPS(r, data, v) \
	StandardProperty<FSN_PROP_GET_TYPE(v)\
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

namespace FusionEngine
{

	template <class T, class Serialiser = GenericPropertySerialiser<T>>
	class StandardProperty : public IComponentProperty
	{
	public:
		typedef typename StandardProperty<T, Serialiser> This_t;

		StandardProperty()
			: m_PropertyID(-1),
			m_GetSetCallbacks(nullptr)
		{
			m_SubscriptionAgent.SetHandlerFn(std::bind(&This_t::Set, this, std::placeholders::_1));
		}

		~StandardProperty()
		{
			if (m_GetSetCallbacks)
				delete m_GetSetCallbacks;
		}

		// Fundimental and enum types are passed to "Set" by value
		typedef typename std::conditional<
				std::is_fundamental<T>::value || std::is_enum<T>::value,
			T, const T &>::type type_for_set;
		// Fundimental, enum and Vector types are returned from "Get" by value
		typedef typename std::conditional<
			std::is_fundamental<T>::value || std::is_enum<T>::value || std::is_same<T, Vector2>::value || std::is_same<T, Vector2i>::value,
			T, const T &>::type type_for_get;

		template <class C>
		void SetCallbacks(C* obj, type_for_get (C::*get_fn)(void) const, void (C::*set_fn)(type_for_set))
		{
			m_GetSetCallbacks = new GetSetCallback<C, type_for_get, type_for_set>(obj, get_fn, set_fn);
		}

		void SetInterfaceObject(const boost::intrusive_ptr<ComponentProperty>& obj)
		{
			m_InterfaceObject = obj;
		}

		ComponentProperty* GetInterfaceObject() const
		{
			if (m_InterfaceObject)
				m_InterfaceObject->addRef();
			return m_InterfaceObject.get();
		}

		void AquireSignalGenerator(PropertySignalingSystem_t& system, PropertyID own_id) override
		{
			m_ChangedCallback = system.MakeGenerator<T>(own_id, [this]()->T { return this->Get(); });
			m_PropertyID = own_id;
			m_SubscriptionAgent.ActivateSubscription(system);
		}

		void Follow(PropertySignalingSystem_t& system, PropertyID own_id, PropertyID id) override
		{
		}

		void Synchronise() override
		{
		}

		void Serialise(RakNet::BitStream& stream) override
		{
			if (m_GetSetCallbacks->HasSet())
			{
				m_SubscriptionAgent.SaveSubscription(stream);
				Serialiser::Serialise(stream, m_GetSetCallbacks->Get());
			}
		}
		void Deserialise(RakNet::BitStream& stream) override
		{
			if (m_GetSetCallbacks->HasSet())
			{
				m_SubscriptionAgent.LoadSubscription(stream);
				T temp;
				Serialiser::Deserialise(stream, temp);
				m_GetSetCallbacks->Set(temp);
			}
		}
		bool IsContinuous() const override
		{
			return Serialiser::IsContinuous();
		}

		template <typename T>
		void SetSpecific(void* value)
		{
			m_GetSetCallbacks->Set(*static_cast<T*>(value));
		}
		template <>
		void SetSpecific<bool>(void* value)
		{
			m_GetSetCallbacks->Set(reinterpret_cast<bool>(value));
		}

		int GetTypeId() const override
		{
			return Scripting::RegisteredAppType<T>::type_id;
		}
		void* GetRef() override
		{
			m_CachedValue = m_GetSetCallbacks->Get();
			return &m_CachedValue;
		}
		void SetAny(void* value, int type_id) override
		{
			if (Scripting::RegisteredAppType<T>::type_id == type_id)
			{
				SetSpecific<T>(value);

				if (m_ChangedCallback)
					m_ChangedCallback();
			}
		}

		type_for_get Get() const { return m_GetSetCallbacks->Get(); }
		void Set(type_for_set value)
		{
			m_GetSetCallbacks->Set(value);

			if (m_ChangedCallback)
				m_ChangedCallback();
		}

		void MarkChanged()
		{
			if (m_ChangedCallback)
				m_ChangedCallback();
		}

	protected:
		typename PropertySignalingSystem_t::GeneratorDetail_t::Impl<const T&>::GeneratorFn_t m_ChangedCallback;

		PersistentConnectionAgent<const T&> m_SubscriptionAgent;

		PropertyID m_PropertyID;

		IGetSetCallback<type_for_get, type_for_set>* m_GetSetCallbacks;
		T m_CachedValue;

		boost::intrusive_ptr<ComponentProperty> m_InterfaceObject;
	};

}

#endif