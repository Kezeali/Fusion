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

#include "FusionEntity.h"

#include "scriptany.h"

namespace FusionEngine
{

	CLBinaryStream::CLBinaryStream(const std::string& filename, CLBinaryStream::OpenMode open_mode)
		: m_File(nullptr)
	{
		Open(filename, open_mode);
	}

	CLBinaryStream::~CLBinaryStream()
	{
		if (m_File)
			PHYSFS_close(m_File);
	}

	void CLBinaryStream::Open(const std::string& filename, CLBinaryStream::OpenMode open_mode)
	{
		if (open_mode == OpenMode::open_write)
			m_File = PHYSFS_openWrite(filename.c_str());
		else
			m_File = PHYSFS_openRead(filename.c_str());
	}

	void CLBinaryStream::Read(void *data, asUINT size)
	{
		auto r = PHYSFS_read(m_File, data, 1u, size);
		if (r < 0 || (r != size && !PHYSFS_eof(m_File)))
			FSN_EXCEPT(FileSystemException, std::string("Couldn't read from file: ") + PHYSFS_getLastError());
	}

	void CLBinaryStream::Write(const void *data, asUINT size)
	{
		auto r = PHYSFS_write(m_File, data, 1u, size);
		if (r != size)
			FSN_EXCEPT(FileSystemException, std::string("Couldn't write to file: ") + PHYSFS_getLastError());
	}

	void LoadScriptResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		auto engine = ScriptManager::getSingleton().GetEnginePtr();

		asIScriptModule* module = engine->GetModule(resource->GetPath().c_str(), asGM_ALWAYS_CREATE);

		std::string moduleFileName = resource->GetPath().substr(resource->GetPath().rfind('/'));
		moduleFileName.erase(moduleFileName.size() - 3);
		moduleFileName = "ScriptCache" + moduleFileName + ".bytecode";

		int r = 0;

		try
		{
			CLBinaryStream bcStream(moduleFileName, CLBinaryStream::open_read);
			r = module->LoadByteCode(&bcStream);
		}
		catch (FusionEngine::FileSystemException &ex)
		{
			resource->SetDataPtr(nullptr);
			resource->_setValid(false);
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded: " + ex.what());
		}

		if (r >= 0)
		{
			resource->SetDataPtr(module);
			resource->_setValid(true);
		}
		else
		{
			resource->SetDataPtr(nullptr);
			resource->_setValid(false);
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded");
		}
	}

	void UnloadScriptResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		if (resource->IsLoaded())
		{
			resource->_setValid(false);

			auto engine = ScriptManager::getSingleton().GetEnginePtr();

			engine->DiscardModule(
				static_cast<asIScriptModule*>(resource->GetDataPtr())->GetName());
		}

		resource->SetDataPtr(nullptr);
	}

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
			m_TypeId = m_Object->GetPropertyTypeId(m_Index);

			//auto any = static_cast<CScriptAny*>(scalable_malloc(sizeof(CScriptAny))); FSN_ASSERT(any);
			//new (any) CScriptAny(m_Object->GetAddressOfProperty(m_Index), m_TypeId, obj->GetEngine());
			auto any = new CScriptAny(m_Object->GetAddressOfProperty(m_Index), m_TypeId, obj->GetEngine());
			m_Value = any;
			any->Release(); // Assigning the intrusive-ptr above increments the ref-count
			//m_Value = new CScriptAny(m_Object->GetAddressOfProperty(m_Index), m_Object->GetPropertyTypeId(m_Index), obj->GetEngine());
		}

		void SetOwner(IComponent* com)
		{
			m_Owner = com;
		}

		~ScriptAnyTSP()
		{
			//m_Value->Release();
		}

		bool IsDirty()
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

		void Synchronise()
		{
			if (m_Writer.DumpWrittenValue(m_Value))
			{
				m_Value->Retrieve(m_Object->GetAddressOfProperty(m_Index), m_TypeId);
			}
			else
			{
				m_Value->Store(m_Object->GetAddressOfProperty(m_Index), m_TypeId);
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
			//auto any = static_cast<CScriptAny*>(scalable_malloc(sizeof(CScriptAny))); FSN_ASSERT(any);
			//m_Writer.Write(new (any) CScriptAny(ref, typeId, m_Object->GetEngine()));
			auto any = new CScriptAny(ref, typeId, m_Object->GetEngine());
			m_Writer.Write(any);
			any->Release();

			m_Owner->OnPropertyChanged(this);
		}

	protected:
		boost::intrusive_ptr<asIScriptObject> m_Object;
		unsigned int m_Index;

		int m_TypeId;

		IComponent* m_Owner;

		boost::intrusive_ptr<CScriptAny> m_Value;
		DefaultStaticWriter<boost::intrusive_ptr<CScriptAny>> m_Writer;
	};

	EntityPtr ASScript_GetParent(ASScript* obj)
	{
		return obj->GetParent()->shared_from_this();
	}

	void ASScript::Register(asIScriptEngine* engine)
	{
		{
			int r = engine->RegisterFuncdef("void coroutine_t()"); FSN_ASSERT(r >= 0);
		}

		{
			int r;
			ASScript::RegisterType<ASScript>(engine, "ASScript");

			r = engine->RegisterObjectBehaviour("IComponent", asBEHAVE_REF_CAST, "ASScript@ f()", asFUNCTION((convert_ref<IComponent, ASScript>)), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod("ASScript", "void yield()", asMETHOD(ASScript, Yield), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void createCoroutine(coroutine_t @)", asMETHODPR(ASScript, CreateCoroutine, (asIScriptFunction*), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void createCoroutine(const string &in)", asMETHODPR(ASScript, CreateCoroutine, (const std::string&), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "any &getProperty(uint) const", asMETHOD(ASScript, GetProperty), asCALL_THISCALL);
			r = engine->RegisterObjectMethod("ASScript", "void setProperty(uint, ?&in)", asMETHODPR(ASScript, SetProperty, (unsigned int, void*,int), bool), asCALL_THISCALL); assert( r >= 0 );
			
			r = engine->RegisterObjectMethod("ASScript", "Entity getParent()", asFUNCTION(ASScript_GetParent), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		}
	}

	ASScript::ASScript()
		: m_ReloadScript(false),
		m_ModuleReloaded(false)
	{
	}

	ASScript::~ASScript()
	{
	}

	boost::intrusive_ptr<asIScriptContext> ASScript::PrepareMethod(ScriptManager* script_manager, const std::string& decl)
	{
		boost::intrusive_ptr<asIScriptContext> ctx;
		auto _where = m_ScriptMethods.find(decl);
		if (_where != m_ScriptMethods.end())
		{
			ctx = script_manager->CreateContext();
			int r = ctx->Prepare(_where->second);
			if (r >= 0)
			{
				ctx->SetObject(m_ScriptObject.get());
			}
			else
			{
				ctx->SetObject(NULL);
				ctx.reset();
			}
			//caller = ScriptUtils::Calling::Caller::CallerForMethodFuncId(script->m_ScriptObject.GetScriptObject(), _where->second);
			//m_ScriptManager->ConnectToCaller(caller);
		}
		else
		{
			//caller = script->m_ScriptObject.GetCaller("void update(float)");
			//script->m_ScriptMethods["void update(float)"] = caller.get_funcid();

			int funcId = m_ScriptObject->GetObjectType()->GetMethodIdByDecl(decl.c_str());

			ctx = script_manager->CreateContext();
			int r = ctx->Prepare(funcId);
			if (r >= 0)
			{
				ctx->SetObject(m_ScriptObject.get());
				m_ScriptMethods[decl] = funcId;
			}
			else
			{
				ctx->SetObject(NULL);
				ctx.reset();
			}
		}
		return ctx;
	}

	void ASScript::Yield()
	{
		auto ctx = asGetActiveContext();
		if (ctx)
		{
			ctx->Suspend();
		}
	}

	void ASScript::CreateCoroutine(asIScriptFunction *fn)
	{
		auto ctx = asGetActiveContext();
		if (ctx)
		{
			auto engine = ctx->GetEngine();

			if (fn == nullptr)
			{
				ctx->SetException("Tried to create a coroutine for a null function-pointer");
				return;
			}

			const auto objectType = fn->GetObjectType();
			const bool method = objectType != nullptr;

			if (method && objectType != m_ScriptObject->GetObjectType())
			{
				const std::string thisTypeName = m_ScriptObject->GetObjectType()->GetName();
				ctx->SetException(("Tried to create a coroutine for a method from another class. This class: " + thisTypeName + ", Method: " + fn->GetDeclaration()).c_str());
				return;
			}

			auto coCtx = engine->CreateContext();
			coCtx->Prepare(fn->GetId());
			if (method)
				coCtx->SetObject(m_ScriptObject.get());

			m_ActiveCoroutines.push_back(coCtx);
			coCtx->Release();
		}
	}

	void ASScript::CreateCoroutine(const std::string& functionName)
	{
		auto ctx = asGetActiveContext();
		if (ctx)
		{
			auto engine = ctx->GetEngine();

			int funcId = -1;
			bool method = false;

			std::string decl = "void " + functionName + "()";

			auto _where = m_ScriptMethods.find(decl);
			if (_where != m_ScriptMethods.end())
			{
				funcId = _where->second;
				method = true;
			}
			else
			{
				funcId = m_ScriptObject->GetObjectType()->GetMethodIdByDecl(decl.c_str());
				if (funcId >= 0)
					method = true;
				else
				{
					funcId = m_Module->GetFunctionIdByDecl(decl.c_str());
					method = false;
				}
			}
			if (funcId < 0)
			{
				// No function matching the decl
				ctx->SetException(("Function '" + decl + "' doesn't exist").c_str());
				return;
			}

			auto coCtx = engine->CreateContext();
			coCtx->Prepare(funcId);
			if (method)
				coCtx->SetObject(m_ScriptObject.get());

			m_ActiveCoroutines.push_back(coCtx);
			coCtx->Release();
		}
	}

	CScriptAny* ASScript::GetProperty(unsigned int index)
	{
		if (index >= m_ScriptProperties.size())
		{
			asGetActiveContext()->SetException("Tried to access a script property that doesn't exist");
			return nullptr;
		}

		auto scriptProperty = static_cast<ScriptAnyTSP*>( m_ScriptProperties[index].get() ); FSN_ASSERT(scriptProperty);
		auto value = scriptProperty->Get();
		return value;
	}

	bool ASScript::SetProperty(unsigned int index, void *ref, int typeId)
	{
		if (index >= m_ScriptProperties.size())
		{
			asGetActiveContext()->SetException("Tried to access a script property that doesn't exist");
			return false;
		}

		auto scriptProperty = static_cast<ScriptAnyTSP*>( m_ScriptProperties[index].get() ); FSN_ASSERT(scriptProperty);
		scriptProperty->Set(ref, typeId);
		return true;
	}

	void ASScript::SetScriptObject(asIScriptObject* obj, const std::vector<std::pair<std::string, std::string>>& properties)
	{
		m_ScriptObject = obj;

		if (obj)
		{

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

					auto comProp = new ScriptAnyTSP(obj, i);

					m_ScriptProperties[(size_t)interfaceIndex].reset(comProp);

					AddProperty(comProp);
					comProp->SetOwner(this);
				}
			}

			auto objType = obj->GetObjectType();
			for (size_t i = 0, count = objType->GetMethodCount(); i < count; ++i)
			{
				auto method = objType->GetMethodDescriptorByIndex(i);
				m_ScriptMethods[method->GetDeclaration(false)] = method->GetId();
			}

		}
	}

	void ASScript::CheckChangedPropertiesIn()
	{
		for (auto it = m_ScriptProperties.begin(), end = m_ScriptProperties.end(); it != end; ++it)
		{
			auto scriptProperty = static_cast<ScriptAnyTSP*>( it->get() ); FSN_ASSERT(scriptProperty);
			if (scriptProperty->IsDirty())
				OnPropertyChanged(scriptProperty);
		}
	}

	void ASScript::OnModuleLoaded(ResourceDataPtr resource)
	{
		m_Module.SetTarget(resource);
		m_ModuleReloaded = true;
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
