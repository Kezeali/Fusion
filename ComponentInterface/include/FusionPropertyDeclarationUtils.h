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

#ifndef H_FusionPropertyDeclarationUtils
#define H_FusionPropertyDeclarationUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <boost/preprocessor.hpp>

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

namespace FusionEngine
{

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

	template <class GetT, class SetT>
	class IGetSetCallback
	{
	public:
		virtual ~IGetSetCallback() {}
		virtual void Set(SetT) = 0;
		virtual GetT Get() const = 0;

		virtual bool HasSet() const = 0;
		
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

		void Set(SetT value) override
		{
			FSN_ASSERT(m_Object); FSN_ASSERT(m_SetFn);
			(m_Object->*m_SetFn)(value);
		}
		GetT Get() const override
		{
			FSN_ASSERT(m_Object); FSN_ASSERT(m_GetFn);
			return (m_Object->*m_GetFn)();
		}

		bool HasSet() const override
		{
			return m_SetFn != nullptr;
		}
		
		EntityComponent* GetObjectAsComponent() const override
		{
			FSN_ASSERT(m_Object);
			return dynamic_cast<EntityComponent*>(m_Object);
		}
	};


}

#endif
