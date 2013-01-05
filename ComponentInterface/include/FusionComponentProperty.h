/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionComponentProperty
#define H_FusionComponentProperty

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionPropertySignalingSystem.h"
#include "FusionRefCounted.h"

#include <RakNet/BitStream.h>

namespace FusionEngine
{
	class ComponentProperty;

	//! Entity Component Property implementation interface
	class IComponentProperty
	{
	public:
		virtual ~IComponentProperty() {}

		virtual void AquireSignalGenerator(PropertySignalingSystem_t& system, PropertyID own_id) = 0;

		virtual void Follow(PropertySignalingSystem_t& system, PropertyID own_id, PropertyID id) = 0;

		virtual void Synchronise() = 0;

		virtual void Serialise(RakNet::BitStream& stream) = 0;
		virtual void Deserialise(RakNet::BitStream& stream) = 0;
		virtual bool IsContinuous() const = 0;

		virtual int GetTypeId() const { return -1; }
		virtual void* GetRef() = 0;
		virtual void SetAny(void* value, int type_id) = 0;

		virtual bool IsEqual(IComponentProperty*) const { return false; }
	};

	//! Component Property front-end
	/*!
	* \see IComponentProperty
	*/
	class ComponentProperty : public RefCounted
	{
		friend class EntityComponent;
	public:
		//! CTOR
		ComponentProperty()
			: m_Impl(nullptr),
			m_ID(-1)
		{}

		ComponentProperty(IComponentProperty* impl, PropertyID id)
			: m_Impl(impl),
			m_ID(id)
		{
			m_ScriptType = ScriptManager::getSingleton().GetEnginePtr()->GetObjectTypeById(m_Impl->GetTypeId());
		}

		//! Called to allow the component implementation to connect to any signal generators from properties that it needs to follow
		void AquireSignalGenerator(PropertySignalingSystem_t& system)
		{
			m_Impl->AquireSignalGenerator(system, GetID());
		}

		//! Make this property follow the property with the given ID
		void Follow(PropertySignalingSystem_t& system, PropertyID id)
		{
			m_Impl->Follow(system, GetID(), id);
		}

		//! Call when it is safe to synchronise thread-local data
		void Synchronise() { m_Impl->Synchronise(); }

		//! Save this property's state
		void Serialise(RakNet::BitStream& stream) { m_Impl->Serialise(stream); }
		//! Deserialise this property's state
		void Deserialise(RakNet::BitStream& stream) { m_Impl->Deserialise(stream); }
		//! Returns true if this property should be written to the continuous stream
		bool IsContinuous() const { return m_Impl->IsContinuous(); }

		void Set(void* ref)
		{
			FSN_ASSERT(GetImpl()->GetTypeId() > 0);
			GetImpl()->SetAny(ref, GetImpl()->GetTypeId());
		}
		void* Get()
		{
			FSN_ASSERT(GetImpl()->GetTypeId() > 0);
			return GetImpl()->GetRef();
		}

		//! Returns this property's globally unique ID
		PropertyID GetID() const { return m_ID; }

		//! Operator equal
		bool operator==(IComponentProperty* other) const
		{
			return m_Impl->IsEqual(other);
		}
		//! Operator not equal
		bool operator!=(IComponentProperty* other) const
		{
			return !m_Impl->IsEqual(other);
		}

		IComponentProperty* GetImpl() const { return m_Impl; }

		//! Register this script type
		static void Register(asIScriptEngine* engine);

	private:
		IComponentProperty* m_Impl;
		PropertyID m_ID;
		asIObjectType* m_ScriptType;
	};

}

#endif
