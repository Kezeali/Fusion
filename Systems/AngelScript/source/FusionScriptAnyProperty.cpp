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

#include "PrecompiledHeaders.h"

#include "FusionScriptAnyProperty.h"

#include "FusionCommonAppTypes.h"
#include "FusionProfiling.h"
#include "FusionSerialisationError.h"
#include "scriptany.h"

#include <boost/mpl/for_each.hpp>

#include <RakNet/StringCompressor.h>

#include <ScriptUtils/Inheritance/TypeTraits.h>

namespace FusionEngine
{

	//! Used with boost::mpl::for_each to detect the type of a given property and make a signal generator for it
	class SignalGeneratorAquisitionFactory
	{
	public:
		SignalGeneratorAquisitionFactory(PropertySignalingSystem_t& sys, ScriptAnyTSP* this_prop_, PropertyID own_id_)
			: system(sys),
			this_prop(this_prop_),
			own_id(own_id_)
		{}

		template <typename T>
		void operator() (T)
		{
			if (Scripting::RegisteredAppType<T>::type_id == this_prop->GetTypeId())
			{
				// Copying these values because lambdas can't capture member vars by value (bah)
				auto propVar = this_prop;
				int typeId = Scripting::RegisteredAppType<T>::type_id;
				auto getter = [propVar, typeId]()->T
				{
					FSN_ASSERT(propVar->GetTypeId() == typeId); // Type ID shouldn't have changed

					propVar->Synchronise();

					auto scriptAny = propVar->Get();
					T value;
					scriptAny->Retrieve(&value, typeId);
					return value;
				};
				this_prop->m_ChangedCallback = system.MakeGenerator<T>(own_id, getter);
			}
		}

		PropertySignalingSystem_t& system;
		ScriptAnyTSP* this_prop;
		PropertyID own_id;
	};

	//! Used with boost::mpl::for_each to detect the type of a given property and make a follower agent for it
	class PropertyFollowerFactory
	{
	public:
		PropertyFollowerFactory(ScriptAnyTSP* this_prop_)
			: this_prop(this_prop_)
		{}

		template <typename T>
		void operator() (T)
		{
			if (Scripting::RegisteredAppType<T>::type_id == this_prop->GetTypeId())
			{
				// Copying these values because lambdas can't capture member vars by value (bah)
				auto propVar = this_prop;
				int typeId = Scripting::RegisteredAppType<T>::type_id;

				auto setter = [propVar, typeId](T value)
				{
					propVar->SetAny(static_cast<void*>(&value), typeId);
				};
				this_prop->m_PersistentFollower = std::make_shared<PersistentConnectionAgent<T>>(setter);
			}
		}

		ScriptAnyTSP* this_prop;
	};


	ScriptAnyTSP::ScriptAnyTSP(boost::intrusive_ptr<asIScriptObject> obj, size_t index, int entity_wrapper_type_id) : m_Object(obj.get()),
		m_Index(index),
		m_EntityWrapperTypeId(entity_wrapper_type_id)
	{
		m_TypeId = m_Object->GetPropertyTypeId(m_Index);
		m_Name = m_Object->GetPropertyName(m_Index);

		//auto any = static_cast<CScriptAny*>(scalable_malloc(sizeof(CScriptAny))); FSN_ASSERT(any);
		//new (any) CScriptAny(m_Object->GetAddressOfProperty(m_Index), m_TypeId, obj->GetEngine());
		auto any = new CScriptAny(m_Object->GetAddressOfProperty(m_Index), m_TypeId, obj->GetEngine());
		m_Value = any;
		any->Release(); // Assigning the intrusive-ptr above increments the ref-count
		//m_Value = new CScriptAny(m_Object->GetAddressOfProperty(m_Index), m_Object->GetPropertyTypeId(m_Index), obj->GetEngine());

		GeneratePersistentFollower();
	}

	void ScriptAnyTSP::AquireSignalGenerator(PropertySignalingSystem_t& system, PropertyID own_id)
	{
		FSN_PROFILE("ScriptPropAquireSignalGenerator");
		// Build generators for the app types listed in CommonAppTypes
		SignalGeneratorAquisitionFactory factory(system, this, own_id);
		boost::mpl::for_each<Scripting::CommonAppTypes>(factory);
		// Fall back on the any callback (works for script types)
		if (!m_ChangedCallback)
		{
			m_ChangedCallback = system.MakeGenerator<boost::intrusive_ptr<CScriptAny>>(own_id,
				[this]()->boost::intrusive_ptr<CScriptAny> { this->Synchronise(); return boost::intrusive_ptr<CScriptAny>(this->Get()); });
		}
	}

	void ScriptAnyTSP::GeneratePersistentFollower()
	{
		PropertyFollowerFactory factory(this);
		boost::mpl::for_each<Scripting::CommonAppTypes>(factory);
		// Fall back on the any callback (works for script types)
		if (!m_PersistentFollower)
		{
			using namespace std::placeholders;
			m_PersistentFollower = std::make_shared<PersistentConnectionAgent<boost::intrusive_ptr<CScriptAny>>>(std::bind(&ScriptAnyTSP::Set, this, _1));
		}
	}

	void ScriptAnyTSP::Follow(PropertySignalingSystem_t& system, PropertyID, PropertyID id)
	{
		m_PersistentFollower->Subscribe(system, id);
	}

	bool ScriptAnyTSP::IsDirty()
	{
		if (m_Value->value.typeId == m_TypeId)
		{
			if (m_TypeId & asTYPEID_OBJHANDLE)
			{
				return (*(void**)m_Object->GetAddressOfProperty(m_Index)) != (/**(void**)*/m_Value->value.valueObj);
				//memcmp(/**(void**)*/m_Object->GetAddressOfProperty(m_Index), /**(void**)*/ref, sizeof(uintptr_t));
			}
			else
			{
				if (m_TypeId & asTYPEID_MASK_OBJECT)
				{
					size_t size = m_Object->GetEngine()->GetObjectTypeById(m_TypeId)->GetSize();
					return memcmp(m_Object->GetAddressOfProperty(m_Index), m_Value->value.valueObj, size) != 0;
				}
				else
				{
					size_t size = m_Object->GetEngine()->GetSizeOfPrimitiveType(m_TypeId);
					return memcmp(m_Object->GetAddressOfProperty(m_Index), &m_Value->value.valueInt, size) != 0;
				}
			}
		}
		else
			return true;
	}

	bool ScriptAnyTSP::MarkForSerialisationIfDirty()
	{
		if (!IsDirty())
			return false;
		else
		{
			m_ChangedSinceSerialised = true;
			if (m_ChangedCallback)
				m_ChangedCallback();
			return true;
		}
	}

	void ScriptAnyTSP::Synchronise()
	{
		if (m_Writer.DumpWrittenValue(m_Value))
		{
			m_Value->Retrieve(m_Object->GetAddressOfProperty(m_Index), m_TypeId);

			m_ChangedSinceSerialised = true; // The property was changed using Set(...)
		}
		else
		{
			m_Value->Store(m_Object->GetAddressOfProperty(m_Index), m_TypeId);
		}
	}

	void ScriptAnyTSP::Serialise(RakNet::BitStream& stream)
	{
		m_PersistentFollower->SaveSubscription(stream);

		// Save prop data
		FSN_ASSERT(m_Value);

		auto& val = m_Value->value;
		FSN_ASSERT((m_TypeId & asTYPEID_OBJHANDLE) == 0 || m_TypeId == m_EntityWrapperTypeId);

		if (m_TypeId == m_EntityWrapperTypeId)
		{
			auto temp = m_TypeId;
			m_TypeId = ASScriptSerialisaiton::s_EntityTypeID;
			stream.Write(m_TypeId);
			m_TypeId = temp;

			if (val.valueObj)
			{
				auto propAsScriptObj = static_cast<asIScriptObject*>(val.valueObj);
				//FSN_ASSERT(propAsScriptObj && std::string(propAsScriptObj->GetPropertyName(0)) == "app_obj");
				//auto app_obj = *static_cast<EntityPtr*>(propAsScriptObj->GetAddressOfProperty(0));
				//if (app_obj)
				//	stream.Write(app_obj->GetID());
				//else
				//	stream.Write(ObjectID(0));

				FSN_ASSERT(propAsScriptObj && std::string(propAsScriptObj->GetPropertyName(2)) == "pointer_id");
				auto pointer_id = *static_cast<uint32_t*>(propAsScriptObj->GetAddressOfProperty(2));
				//FSN_ASSERT(pointer_id != 0);
				stream.Write(pointer_id);
			}
			else
			{
				//stream.Write(ObjectID(0));
				stream.Write(uint32_t(0));
			}
		}
		else if (m_TypeId == ScriptManager::getSingleton().GetStringTypeId())
		{
			stream.Write(ASScriptSerialisaiton::s_CompressedStringTypeID);

			std::string strVal;
			m_Value->Retrieve(&strVal, val.typeId);
			if (strVal.length() > ASScriptSerialisaiton::s_MaxStringLength)
			{
				FSN_EXCEPT(SerialisationError, "String value too long");
			}
			RakNet::StringCompressor::Instance()->EncodeString(strVal.c_str(), strVal.length() + 1, &stream);
		}
		else if ((m_TypeId & asTYPEID_SCRIPTOBJECT) != 0)
		{
			auto objectType = ScriptManager::getSingleton().GetEnginePtr()->GetObjectTypeById(m_TypeId);

			asIScriptObject* object;
			if (m_Value->Retrieve(&object, m_TypeId))
			{
				auto serialisableInterfaceType = ScriptManager::getSingleton().GetEnginePtr()->GetObjectTypeByName("ISerialisable");
				if (ScriptUtils::Inheritance::base_implements(objectType, serialisableInterfaceType))
				{
					// TODO: replace this with static ISerialiseable::getSingleton().Serialise(obj, stream);
					auto serialiseMethod = objectType->GetMethodByDecl("void Serialise(BitStream@)");
					auto ser = ScriptUtils::Calling::Caller::CallerForMethodFuncId(object, serialiseMethod->GetId());
					ser(&stream);
				}
				else // Not ISerialiseable
				{
				}
			}
		}
		else if ((val.typeId & asTYPEID_APPOBJECT) != 0)
		{
			FSN_EXCEPT(SerialisationError, "Can't serialise app objects");
			// TODO: Scripting::AppTypeSerialiser (singleton) ::RegististerSerialiser(type_id, function<void (BitStream, void*)>)
			// Scripting::AppTypeSerialiser::getSingleton().Serialise(m_TypeId, val.valueObj);
		}
		else
		{
			FSN_ASSERT((val.typeId & asTYPEID_MASK_OBJECT) == 0);

			stream.Write(val.typeId);
			stream.Write(val.valueInt);
		}
	}

	void ScriptAnyTSP::Deserialise(RakNet::BitStream& stream)
	{
		// Load subscription
		m_PersistentFollower->LoadSubscription(stream);

		// Load prop data
		stream.Read(m_TypeId);

		if (m_TypeId == ASScriptSerialisaiton::s_EntityTypeID)
		{
			m_Value->value.valueObj = 0;

			//stream.Read(m_EntityID);
			stream.Read(m_PointerID);
		}
		else if (m_TypeId == ASScriptSerialisaiton::s_CompressedStringTypeID)
		{
			RakNet::RakString rakStringVal;
			if (RakNet::StringCompressor::Instance()->DecodeString(&rakStringVal, ASScriptSerialisaiton::s_MaxStringLength, &stream))
			{
				auto actualTypeId = ScriptManager::getSingleton().GetStringTypeId();
				std::string strVal(rakStringVal.C_String());

				m_Value->Store(&strVal, actualTypeId);
			}
			else
			{
				FSN_EXCEPT(SerialisationError, "Compressed string value seems to be too long");
			}
		}
		else
		{
			auto v = m_Value->value.valueInt;
			stream.Read(v);
			m_Value->Store(&v, m_Value->value.typeId);
		}
	}

	void* ScriptAnyTSP::GetRef()
	{
		if ((m_Value->GetTypeId() & asTYPEID_OBJHANDLE) || (m_Value->GetTypeId() & asTYPEID_MASK_OBJECT))
			return m_Value->value.valueObj;
		else
		{
			switch (m_Value->GetTypeId())
			{
			case asTYPEID_FLOAT:
			case asTYPEID_DOUBLE:
				return &m_Value->value.valueFlt;
			default:
				return &m_Value->value.valueInt;
			}
		}
	}

	CScriptAny* ScriptAnyTSP::Get() const
	{
		return m_Value.get();
	}

	void ScriptAnyTSP::SetAny(void* ref, int typeId)
	{
		if (typeId == m_TypeId)
		{
			//auto any = static_cast<CScriptAny*>(scalable_malloc(sizeof(CScriptAny))); FSN_ASSERT(any);
			//m_Writer.Write(new (any) CScriptAny(ref, typeId, m_Object->GetEngine()));
			auto any = new CScriptAny(ref, typeId, m_Object->GetEngine());
			m_Writer.Write(any);
			any->Release();

			if (m_ChangedCallback)
				m_ChangedCallback();
		}
		else
			FSN_EXCEPT(InvalidArgumentException, "Tried to assign a value of incorrect type to a script property");
	}

	void ScriptAnyTSP::Set(const boost::intrusive_ptr<CScriptAny>& any)
	{
		if (any->GetTypeId() == m_TypeId)
		{
			m_Writer.Write(any);

			if (m_ChangedCallback)
				m_ChangedCallback();
		}
		else
			FSN_EXCEPT(InvalidArgumentException, "Tried to assign a value of incorrect type to a script property");
	}

}