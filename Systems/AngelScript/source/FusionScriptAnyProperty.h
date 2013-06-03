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

#ifndef H_FusionScriptAnyProperty
#define H_FusionScriptAnyProperty

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentProperty.h"

// FusionThreadSafeProperty.h: Just for DefaultStaticWriter :(
#include "FusionThreadSafeProperty.h"
#include "FusionTypes.h"

class CScriptAny;

namespace FusionEngine
{

	namespace ASScriptSerialisaiton
	{
		// Since these aren't built-in, a static type-id (for use when serialising) has to be defined for them
		static const int s_EntityTypeID = -1;
		static const int s_CompressedStringTypeID = -2;

		static const size_t s_MaxStringLength = 2048;
	}
	
	//! Like ThreadSafeProperty
	class ScriptAnyTSP : public IComponentProperty
	{
		friend class SignalGeneratorAquisitionFactory;
		friend class PropertyFollowerFactory;
	public:
		ScriptAnyTSP(boost::intrusive_ptr<asIScriptObject> obj, size_t index, int entity_wrapper_type_id);

		~ScriptAnyTSP()
		{
			//m_Value->Release();
		}

		const std::string& GetName() const { return m_Name; }

		void AquireSignalGenerator(PropertySignalingSystem_t& system, PropertyID own_id);

		void GeneratePersistentFollower();

		void Follow(PropertySignalingSystem_t& system, PropertyID, PropertyID id);

		bool IsDirty();

		bool MarkForSerialisationIfDirty();

		void Unmark()
		{
			m_ChangedSinceSerialised = false;
		}

		bool HasChangedSinceSerialised() const
		{
			return m_ChangedSinceSerialised;
		}

		void Synchronise();

		void Serialise(RakNet::BitStream& stream);

		void Deserialise(RakNet::BitStream& stream);

		bool IsContinuous() const
		{
			return false;
		}

		int GetTypeId() const { return m_TypeId; }
		void* GetRef();
		void SetAny(void* ref, int typeId);

		CScriptAny* Get() const;
		void Set(const boost::intrusive_ptr<CScriptAny>& any);

	protected:
		asIScriptObject* m_Object;
		unsigned int m_Index;
		std::string m_Name;

		int m_TypeId;

		int m_EntityWrapperTypeId;

		//PropertySignalingSystem_t::GeneratorDetail_t::Impl<boost::intrusive_ptr<CScriptAny>>::GeneratorFn_t m_ChangedCallback;

		//PersistentFollowerPtr m_PersistentFollower;

		// Stores the pointer-id if this property is an entity pointer which needs to be resolved
		std::uint32_t m_PointerID;
		boost::intrusive_ptr<CScriptAny> m_Value;
		DefaultStaticWriter<boost::intrusive_ptr<CScriptAny>> m_Writer;

		bool m_ChangedSinceSerialised;
	};

}

#endif