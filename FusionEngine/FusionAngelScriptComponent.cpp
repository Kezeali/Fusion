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

#include "FusionStableHeaders.h"

#include "FusionAngelScriptComponent.h"

#include "scriptany.h"

namespace FusionEngine
{

	template <typename T>
	class ScriptTSP : public IComponentProperty
	{
	public:
		ScriptTSP(boost::intrusive_ptr<asIScriptObject> obj, size_t index)
			: m_Object(obj),
			m_Index(index)
		{
		}

		void Synchronise()
		{
			if (m_Writer.DumpWrittenValue(m_Value))
			{
				m_Object->GetAddressOfProperty(m_Index);
			}
			else
			{
				m_Value = *static_cast<T*>( m_Object->GetAddressOfProperty(m_Index) );
			}
		}

		void FireSignal()
		{
		}

	protected:
		boost::intrusive_ptr<asIScriptObject> m_Object;
		unsigned int m_Index;

		DefaultStaticWriter<T> m_Writer;
	};

	//! Like ThreadSafeProperty
	class ScriptAnyTSP : public IComponentProperty
	{
	public:
		ScriptAnyTSP(boost::intrusive_ptr<asIScriptObject> obj, size_t index)
			: m_Object(obj),
			m_Index(index)
		{
			auto any = static_cast<CScriptAny*>(scalable_malloc(sizeof(CScriptAny))); FSN_ASSERT(any);
			new (any) CScriptAny(m_Object->GetAddressOfProperty(m_Index), m_Object->GetPropertyTypeId(m_Index), obj->GetEngine());
			m_Value = any;
			any->Release(); // Assigning the intrusive-ptr above increments the ref-count
			//m_Value = new CScriptAny(m_Object->GetAddressOfProperty(m_Index), m_Object->GetPropertyTypeId(m_Index), obj->GetEngine());
		}

		~ScriptAnyTSP()
		{
			//m_Value->Release();
		}

		void Synchronise()
		{
			if (m_Writer.DumpWrittenValue(m_Value))
			{
				m_Value->Retrieve(m_Object->GetAddressOfProperty(m_Index), m_Object->GetPropertyTypeId(m_Index));
			}
			else
			{
				m_Value->Store(m_Object->GetAddressOfProperty(m_Index), m_Object->GetPropertyTypeId(m_Index));
			}
		}

		void FireSignal()
		{
		}

		CScriptAny* Get() const
		{
			return m_Value.get();
		}

		void Set(void* ref, int typeId)
		{
			auto any = static_cast<CScriptAny*>(scalable_malloc(sizeof(CScriptAny))); FSN_ASSERT(any);
			m_Writer.Write(new (any) CScriptAny(ref, typeId, m_Object->GetEngine()));
			any->Release();
		}

	protected:
		boost::intrusive_ptr<asIScriptObject> m_Object;
		unsigned int m_Index;

		boost::intrusive_ptr<CScriptAny> m_Value;

		DefaultStaticWriter<boost::intrusive_ptr<CScriptAny>> m_Writer;
		//tbb::enumerable_thread_specific<CScriptAny*> m_WriteBuffers;
	};

	ASScript::ASScript()
		: m_ReloadScript(false),
		m_ModuleBuilt(false)
	{
	}

	ASScript::~ASScript()
	{
	}

	CScriptAny* ASScript::GetProperty(unsigned int index)
	{
		if (index >= m_ScriptProperties.size())
			asGetActiveContext()->SetException("Tried to access a script property that doesn't exist");

		auto scriptProperty = dynamic_cast<ScriptAnyTSP*>( m_ScriptProperties[index].get() ); FSN_ASSERT(scriptProperty);
		auto value = scriptProperty->Get();
		return value;

		//const auto& scriptObject = m_ScriptObject.GetScriptObject();
		//if (scriptObject == nullptr)
		//	asGetActiveContext()->SetException("Tried to access a script component that wasn't ready");

		//auto prop = scriptObject->GetAddressOfProperty(index);
		//int propTypeId = scriptObject->GetPropertyTypeId(index);

		//FSN_ASSERT(propTypeId != asTYPEID_BOOL); // I'm not sure what type CScriptAny stores bools as

		//if ((propTypeId - asTYPEID_INT8) <= asTYPEID_UINT64)
		//	propTypeId = asTYPEID_INT64;
		//else if (propTypeId == asTYPEID_FLOAT)
		//	propTypeId = asTYPEID_DOUBLE;

		//auto ptr = static_cast<CScriptAny*>(scalable_malloc(sizeof(CScriptAny))); FSN_ASSERT(ptr);
		//new (ptr) CScriptAny(prop, propTypeId, scriptObject->GetEngine());
		//return ptr;
	}

	bool ASScript::SetProperty(unsigned int index, void *ref, int typeId)
	{
		if (index >= m_ScriptProperties.size())
			asGetActiveContext()->SetException("Tried to access a script property that doesn't exist");

		auto scriptProperty = dynamic_cast<ScriptAnyTSP*>( m_ScriptProperties[index].get() ); FSN_ASSERT(scriptProperty);
		scriptProperty->Set(ref, typeId);
		return true;
	}

	void ASScript::SetScriptObject(asIScriptObject* obj, const std::vector<std::pair<std::string, std::string>>& properties)
	{
		m_ScriptObject = ScriptObject(obj);

		m_ScriptProperties.resize(properties.size());
		//auto objType = obj->GetObjectType();
		for (size_t i = 0, count = obj->GetPropertyCount(); i < count; ++i)
		{
			//const char* name = 0; int typeId = -1; bool isPrivate = false; int offset = 0;
			std::string nameStr(obj->GetPropertyName(i));
			auto _where = std::find_if(properties.cbegin(), properties.cend(), [nameStr](const std::pair<std::string, std::string>& v) { return v.second == nameStr; });
			if (_where != properties.cend())
			{
				auto interfaceIndex = std::distance(properties.cbegin(), _where);
				FSN_ASSERT(interfaceIndex >= 0);

				int typeId = obj->GetPropertyTypeId(i);

				IComponentProperty* comProp = new ScriptAnyTSP(obj, i);
				//switch (typeId)
				//{
				//case asTYPEID_BOOL:
				//	comProp = new ScriptTSP<bool>(obj, i); break;
				//case asTYPEID_UINT64:
				//	comProp = new ScriptTSP<uint64_t>(obj, i); break;
				//};
				m_ScriptProperties[(size_t)interfaceIndex].reset(comProp);

				AddProperty(comProp);
			}
		}
		//for (auto it = properties.begin(), end = properties.end(); it != end; ++it)
		//{
		//}
	}

	void ASScript::OnSiblingAdded(const std::shared_ptr<IComponent>& com)
	{
		//if (auto scriptCom = dynamic_cast<ASScript*>(com.get()))
		//{
		//}
	}

	void ASScript::OnSiblingRemoved(const std::shared_ptr<IComponent>& com)
	{
	}

	bool ASScript::SerialiseContinuous(RakNet::BitStream& stream)
	{
		return true;
	}

	void ASScript::DeserialiseContinuous(RakNet::BitStream& stream)
	{
	}

	bool ASScript::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		return m_DeltaSerialisationHelper.writeChanges(force_all, stream,
			GetScriptPath(), std::string());
	}

	void ASScript::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		std::bitset<DeltaSerialiser_t::NumParams> changes;
		std::string unused;
		m_DeltaSerialisationHelper.readChanges(stream, all, changes,
			m_Path, unused);

		if (changes[PropsIdx::ScriptPath])
			m_ReloadScript = true;
	}

	const std::string &ASScript::GetScriptPath() const
	{
		return m_Path;
	}

	void ASScript::SetScriptPath(const std::string& path)
	{
		m_Path = path;
		m_ReloadScript = true;

		m_DeltaSerialisationHelper.markChanged(PropsIdx::ScriptPath);
	}

}
