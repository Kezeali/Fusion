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
#include <tbb/tick_count.h>

namespace FusionEngine
{

	int ASScript::s_ASScriptTypeId = -1;

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
			m_Index(index),
			m_Owner(nullptr)
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

		int GetTypeId() const { return m_TypeId; }

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
			if (typeId == m_TypeId)
			{
				//auto any = static_cast<CScriptAny*>(scalable_malloc(sizeof(CScriptAny))); FSN_ASSERT(any);
				//m_Writer.Write(new (any) CScriptAny(ref, typeId, m_Object->GetEngine()));
				auto any = new CScriptAny(ref, typeId, m_Object->GetEngine());
				m_Writer.Write(any);
				any->Release();

				if (m_Owner)
					m_Owner->OnPropertyChanged(this);
			}
			else
				FSN_EXCEPT(InvalidArgumentException, "Tried to assign a value of incorrect type to a script property");
		}

		void Set(CScriptAny* any)
		{
			if (any->GetTypeId() == m_TypeId)
			{
				m_Writer.Write(any);
				any->Release();

				if (m_Owner)
					m_Owner->OnPropertyChanged(this);
			}
			else
				FSN_EXCEPT(InvalidArgumentException, "Tried to assign a value of incorrect type to a script property");
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

	static std::weak_ptr<Entity> InitEntityPtr(EntityPtr& entityReferenced)
	{
		auto activeScript = ASScript::GetActiveScript(); FSN_ASSERT(activeScript); FSN_ASSERT(activeScript->GetParent());
		auto referenceHolder = activeScript->GetParent()->shared_from_this();
		referenceHolder->HoldReference(entityReferenced);
		return referenceHolder;
	}

	static void DeinitEntityPtr(std::weak_ptr<Entity>& referenceHolder, EntityPtr& entityReferenced)
	{
		if (auto locked = referenceHolder.lock())
			locked->DropReference(entityReferenced);
	}

	void ASScript::Register(asIScriptEngine* engine)
	{
		{
			int r = engine->RegisterFuncdef("void coroutine_t()"); FSN_ASSERT(r >= 0);
		}

		{
			int r;
			ASScript::RegisterType<ASScript>(engine, "ASScript");

			s_ASScriptTypeId = engine->GetTypeIdByDecl("ASScript");
			FSN_ASSERT(s_ASScriptTypeId >= 0);

			r = engine->RegisterObjectBehaviour("IComponent", asBEHAVE_REF_CAST, "ASScript@ f()", asFUNCTION((convert_ref<IComponent, ASScript>)), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod("ASScript", "void yield()", asMETHOD(ASScript, Yield), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void createCoroutine(coroutine_t @)", asMETHODPR(ASScript, CreateCoroutine, (asIScriptFunction*), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void createCoroutine(const string &in, float delay = 0.0f)", asMETHODPR(ASScript, CreateCoroutine, (const std::string&, float), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "any@ getProperty(uint) const", asMETHOD(ASScript, GetProperty), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void setProperty(uint, ?&in)", asMETHODPR(ASScript, SetProperty, (unsigned int, void*,int), bool), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
			
			r = engine->RegisterObjectMethod("ASScript", "Entity getParent()", asFUNCTION(ASScript_GetParent), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterGlobalFunction("EntityW initEntityPointer(Entity &in)", asFUNCTION(InitEntityPtr), asCALL_CDECL); FSN_ASSERT(r >= 0);
			r = engine->RegisterGlobalFunction("void deinitEntityPointer(EntityW &in, Entity &in)", asFUNCTION(DeinitEntityPtr), asCALL_CDECL); FSN_ASSERT(r >= 0);
		}
	}

	ASScript::ASScript()
		: m_ReloadScript(false),
		m_ModuleReloaded(false),
		m_EntityWrapperTypeId(-1)
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

	void ASScript::YieldUntil(std::function<bool (void)> condition, float timeout)
	{
		auto ctx = asGetActiveContext();
		if (ctx)
		{
			ctx->Suspend();

			ConditionalCoroutine co;
			co.condition = condition;
			co.SetTimeout(timeout);
			m_ActiveCoroutinesWithConditions[ctx] = std::move(co);
		}
	}

	ASScript* ASScript::GetActiveScript()
	{
		auto ctx = asGetActiveContext();
		if (ctx)
		{
			asIScriptObject* scriptCom = static_cast<asIScriptObject*>( asGetActiveContext()->GetThisPointer(asGetActiveContext()->GetCallstackSize()-1) );

			if (scriptCom->GetPropertyTypeId(0) == s_ASScriptTypeId)
			{
				return *static_cast<ASScript**>( scriptCom->GetAddressOfProperty(0) );
			}
			else // Safe (but slow) lookup
			{
				if (std::string(scriptCom->GetObjectType()->GetBaseType()->GetName()) != "ScriptComponent")
					return nullptr;

				int appObjOffset = -1;

				auto baseObjType = scriptCom->GetObjectType()->GetBaseType();
				for (asUINT i = 0, count = baseObjType->GetPropertyCount(); i < count; ++i)
				{
					const char* name; int typeId; bool isPrivate;
					if (baseObjType->GetProperty(i, &name, &typeId, &isPrivate, &appObjOffset) >= 0)
					{
						if (isPrivate && std::string(name) == "app_obj")
						{
							break;
						}
					}
				}

				if (appObjOffset != -1)
				{
					auto propAddress = reinterpret_cast<void*>(reinterpret_cast<asBYTE*>(scriptCom) + appObjOffset);
					return *static_cast<ASScript**>(propAddress);
				}
				else
					return nullptr;
			}
		}
		else
			return nullptr;
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

			ConditionalCoroutine cco;
			cco.new_ctx = coCtx;
			m_ActiveCoroutinesWithConditions[coCtx] = std::move(cco);
			coCtx->Release();
		}
	}

	void ASScript::CreateCoroutine(const std::string& functionName, float delay)
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

			ConditionalCoroutine cco;
			cco.new_ctx = coCtx;
			if (!fe_fzero(delay))
				cco.SetTimeout(delay);
			m_ActiveCoroutinesWithConditions[coCtx] = std::move(cco);
			coCtx->Release();
		}
	}

	void getWrappedEntity(EntityPtr*& out, asIScriptObject* entityWrapper)
	{
		FSN_ASSERT(std::string(entityWrapper->GetPropertyName(0)) == "app_obj");
		out = static_cast<EntityPtr*>(entityWrapper->GetAddressOfProperty(0));
	}

	CScriptAny* ASScript::GetProperty(unsigned int index)
	{
		if (index >= m_ScriptProperties.size())
		{
			asGetActiveContext()->SetException("Tried to access a script property that doesn't exist");
			return nullptr;
		}

		auto scriptProperty = static_cast<ScriptAnyTSP*>( m_ScriptProperties[index].get() ); FSN_ASSERT(scriptProperty);
		auto any = scriptProperty->Get();
		if (any->GetTypeId() != m_EntityWrapperTypeId)
		{
			any->AddRef();
			return any;
		}
		else
		{
			auto engine = m_Module->GetEngine();

			if (any->value.valueObj != 0)
			{
				auto entityWrapper = static_cast<asIScriptObject*>(any->value.valueObj);

				EntityPtr* wrappedEntity = nullptr;
				getWrappedEntity(wrappedEntity, entityWrapper);

				// TODO: static int Entity::GetTypeId();
				int entityTypeId = engine->GetTypeIdByDecl("Entity");
				auto unwrappedAny = new CScriptAny(wrappedEntity, entityTypeId, engine);
				return unwrappedAny;
			}
			else return new CScriptAny(engine);
		}
	}

	bool ASScript::SetProperty(unsigned int index, void *ref, int typeId)
	{
		if (index >= m_ScriptProperties.size())
		{
			asGetActiveContext()->SetException("Tried to access a script property that doesn't exist");
			return false;
		}

		auto scriptProperty = static_cast<ScriptAnyTSP*>( m_ScriptProperties[index].get() ); FSN_ASSERT(scriptProperty);

		if (scriptProperty->GetTypeId() != m_EntityWrapperTypeId)
		{
			scriptProperty->Set(ref, typeId);
		}
		else
		{
			auto engine = m_Module->GetEngine();
			int entityTypeId = engine->GetTypeIdByDecl("Entity");

			if (typeId != entityTypeId)
				return false;

			auto passedObj = static_cast<EntityPtr*>(ref);

			if (!passedObj)
				return false;
			
			//auto wrapperCtor = ScriptUtils::Calling::Caller::FactoryCaller(engine->GetObjectTypeById(m_EntityWrapperTypeId & ~asTYPEID_OBJHANDLE), "Entity &in");
			//FSN_ASSERT(wrapperCtor);
			//void* entityWrapper = wrapperCtor(passedObj);

			const int entityWrapperObjectTypeId = m_EntityWrapperTypeId & ~asTYPEID_OBJHANDLE;
			asIScriptObject* entityWrapper = static_cast<asIScriptObject*>(ScriptManager::getSingleton().GetEnginePtr()->CreateScriptObject(entityWrapperObjectTypeId));

			FSN_ASSERT(GetParent());
			GetParent()->HoldReference(*passedObj);

			FSN_ASSERT(entityWrapper->GetPropertyTypeId(0) == entityTypeId && entityWrapper->GetPropertyTypeId(1) == engine->GetTypeIdByDecl("EntityW"));

			auto app_obj = static_cast<EntityPtr*>(entityWrapper->GetAddressOfProperty(0));
			*app_obj = *passedObj;

			auto owner = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(1));
			*owner = GetParent()->shared_from_this();

			auto any = new CScriptAny(&entityWrapper, m_EntityWrapperTypeId, engine);
			scriptProperty->Set(any);
		}
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
					FSN_ASSERT(std::distance(properties.cbegin(), _where) >= 0);
					const auto interfaceIndex = (size_t)std::distance(properties.cbegin(), _where);

					auto comProp = new ScriptAnyTSP(obj, i);

					FSN_ASSERT((comProp->GetTypeId() & asTYPEID_OBJHANDLE) == 0 || comProp->GetTypeId() == m_EntityWrapperTypeId);

					// Copy in any values that were deserialised before the script was reloaded
					if (m_CacheProperties.size() > interfaceIndex)
					{
						const auto& cachedProp = m_CacheProperties[interfaceIndex];
						if (cachedProp->GetTypeId() != -1)
						{
							cachedProp->AddRef();
							comProp->Set(cachedProp.get());
							comProp->Synchronise();
						}
					}

					m_ScriptProperties[interfaceIndex].reset(comProp);

					AddProperty(comProp);
					comProp->SetOwner(this);
				}
			}

			m_CacheProperties.clear();

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
			{
				OnPropertyChanged(scriptProperty);
				// TODO: mark this property to be serialised
				//  (also do so in SetProperty(...))
			}
		}
	}

	void ASScript::OnModuleLoaded(ResourceDataPtr resource)
	{
		m_Module.SetTarget(resource);
		m_ModuleReloaded = true;

		if (m_Module.IsLoaded())
		{
			m_EntityWrapperTypeId = m_Module->GetTypeIdByDecl("EntityWrapper@"); FSN_ASSERT(m_EntityWrapperTypeId >= 0);
		}
	}

	void ASScript::InitialiseEntityWrappers()
	{
		FSN_ASSERT(m_Module.IsLoaded());

		auto engine = m_Module->GetEngine();

		for (auto it = m_UninitialisedEntityWrappers.begin(), end = m_UninitialisedEntityWrappers.end(); it != end; ++it)
		{
			ObjectID id = it->second;

			auto _where = std::find_if(GetParent()->m_ReferencedEntities.begin(), GetParent()->m_ReferencedEntities.end(), [id](const std::pair<EntityPtr, size_t>& entity)
			{
				return entity.first->GetID() == id;
			});

			unsigned int propertyIndex = it->first;

			if (_where != GetParent()->m_ReferencedEntities.end())
			{
				asIScriptObject* entityWrapper = nullptr;

				auto scriptprop = static_cast<ScriptAnyTSP*>(m_ScriptProperties[propertyIndex].get());

				const int entityWrapperObjectTypeId = m_EntityWrapperTypeId & ~asTYPEID_OBJHANDLE;
				entityWrapper = static_cast<asIScriptObject*>(ScriptManager::getSingleton().GetEnginePtr()->CreateScriptObject(entityWrapperObjectTypeId));
				FSN_ASSERT(entityWrapper);

				auto prop = new CScriptAny(&entityWrapper, m_EntityWrapperTypeId, engine);
				scriptprop->Set(prop);

				auto app_obj = static_cast<EntityPtr*>(entityWrapper->GetAddressOfProperty(0));
				*app_obj = _where->first;

				auto owner = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(1));
				*owner = GetParent()->shared_from_this();

				GetParent()->HoldReference(_where->first);
			}
			else
			{
				if (id != 0)
					FSN_ASSERT_FAIL("Failed to intitialise entity wrapper");
			}
		}
		m_UninitialisedEntityWrappers.clear();
	}

	void ASScript::OnSiblingAdded(const ComponentPtr& com)
	{
		//if (auto scriptCom = dynamic_cast<ASScript*>(com.get()))
		//{
		//}
	}

	void ASScript::OnSiblingRemoved(const ComponentPtr& com)
	{
	}

	bool ASScript::SerialiseContinuous(RakNet::BitStream& stream)
	{
		return false;
	}

	void ASScript::DeserialiseContinuous(RakNet::BitStream& stream)
	{
	}

	bool ASScript::SerialiseProp(RakNet::BitStream& stream, CScriptAny* any)
	{
		FSN_ASSERT(any);

		auto& val = any->value;
		FSN_ASSERT((val.typeId & asTYPEID_OBJHANDLE) == 0 || val.typeId == m_EntityWrapperTypeId);
		
		if (val.typeId == m_EntityWrapperTypeId)
		{
			auto temp = val.typeId;
			val.typeId = -1;
			stream.Write(val);
			val.typeId = temp;

			if (val.valueObj)
			{
				auto propAsScriptObj = static_cast<asIScriptObject*>(val.valueObj);
				FSN_ASSERT(propAsScriptObj && std::string(propAsScriptObj->GetPropertyName(0)) == "app_obj");
				auto app_obj = *static_cast<EntityPtr*>(propAsScriptObj->GetAddressOfProperty(0));
				if (app_obj)
					stream.Write(app_obj->GetID());
				else
					stream.Write(ObjectID(0));
			}
			else
				stream.Write(ObjectID(0));
		}
		else if ((val.typeId & asTYPEID_SCRIPTOBJECT) != 0)
		{
			FSN_ASSERT_FAIL("Not implemented");
			// TODO: check if it implements ISerialisable and serialise it, if not, just copy each prop
		}
		else if ((val.typeId & asTYPEID_APPOBJECT) != 0)
		{
			FSN_EXCEPT(InvalidArgumentException, "Can't serialise app objects");
		}
		else
			stream.Write(val);

		return true;
	}

	bool ASScript::DeserialiseProp(RakNet::BitStream& stream, CScriptAny* prop, unsigned int index)
	{
		FSN_ASSERT(prop);

		stream.Read(prop->value);

		if (prop->value.typeId == -1)
		{
			ObjectID id;
			stream.Read(id);

			//ScriptUtils::Calling::Caller::FactoryCaller();

			auto _where = std::find_if(GetParent()->m_ReferencedEntities.begin(), GetParent()->m_ReferencedEntities.end(), [id](const std::pair<EntityPtr, size_t>& entity)
			{
				return entity.first->GetID() == id;
			});

			asIScriptObject* entityWrapper = nullptr;
			if (m_Module.IsLoaded())
			{
				const int entityWrapperObjectTypeId = m_EntityWrapperTypeId & ~asTYPEID_OBJHANDLE;
				entityWrapper = static_cast<asIScriptObject*>(ScriptManager::getSingleton().GetEnginePtr()->CreateScriptObject(entityWrapperObjectTypeId));
				
				prop->Store(&entityWrapper, m_EntityWrapperTypeId);
			}

			if (entityWrapper && _where != GetParent()->m_ReferencedEntities.end())
			{
				auto app_obj = static_cast<EntityPtr*>(entityWrapper->GetAddressOfProperty(0));
				*app_obj = _where->first;

				auto owner = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(1));
				*owner = GetParent()->shared_from_this();

				GetParent()->HoldReference(_where->first);
			}
			else
			{
				m_UninitialisedEntityWrappers.push_back(std::make_pair(index, id));
			}
			
			
		}

		return true;
	}

	bool ASScript::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		bool changeWritten = m_DeltaSerialisationHelper.writeChanges(force_all, stream,
			GetScriptPath(), std::string());

		if (m_ScriptObject)
		{
			stream.Write(m_ScriptProperties.size());
			for (auto it = m_ScriptProperties.begin(), end = m_ScriptProperties.end(); it != end; ++it)
			{
				auto scriptprop = static_cast<ScriptAnyTSP*>((*it).get());
				SerialiseProp(stream, scriptprop->Get());
			}
		}
		else if (!m_CacheProperties.empty()) // Script may have just been deserialised
		{
			stream.Write(m_CacheProperties.size());
			for (auto it = m_CacheProperties.begin(), end = m_CacheProperties.end(); it != end; ++it)
			{
				auto& prop = *it;
				SerialiseProp(stream, prop.get());
			}
		}

		return changeWritten;
	}

	void ASScript::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		std::bitset<DeltaSerialiser_t::NumParams> changes;
		std::string unused;
		m_DeltaSerialisationHelper.readChanges(stream, all, changes,
			m_Path, unused);

		if (changes[PropsIdx::ScriptPath])
			m_ReloadScript = true;

		auto engine = ScriptManager::getSingleton().GetEnginePtr();

		auto numProperties = m_ScriptProperties.size();
		stream.Read(numProperties);
		if (m_ScriptObject)
		{
			FSN_ASSERT(m_ScriptProperties.size() == numProperties);
			unsigned int index = 0;
			for (auto it = m_ScriptProperties.begin(), end = m_ScriptProperties.end(); it != end; ++it)
			{
				auto scriptprop = static_cast<ScriptAnyTSP*>((*it).get());
				//auto prop = scriptprop->Get();
				auto prop = new CScriptAny(engine);
				DeserialiseProp(stream, prop, index++);
				scriptprop->Set(prop);
				prop->Release();
			}
		}
		else
		{
			for (size_t i = 0; i < numProperties; ++i)
			{
				auto prop = new CScriptAny(engine);
				DeserialiseProp(stream, prop, i);
				m_CacheProperties.push_back(prop);
				prop->Release();
			}
		}
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
