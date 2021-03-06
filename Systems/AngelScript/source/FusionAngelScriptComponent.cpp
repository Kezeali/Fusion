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

#include "PrecompiledHeaders.h"

#include "FusionAngelScriptComponent.h"

#include "FusionCommonAppTypes.h"
#include "FusionEntity.h"
#include "FusionEntityRepo.h"
#include "FusionException.h"
#include "FusionLogger.h"
#include "FusionProfiling.h"
#include "FusionSerialisationError.h"
#include "FusionScriptAnyProperty.h"

#include "FusionB2ContactListenerASScript.h"

#include "scriptany.h"

#include <ScriptUtils/Calling/Caller.h>

#include <RakNet/StringCompressor.h>

#include <boost/mpl/for_each.hpp>

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

		if (m_File == NULL)
			FSN_EXCEPT(FileSystemException, std::string("Couldn't open file: ") + PHYSFS_getLastError());
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

	void LoadScriptResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		auto engine = ScriptManager::getSingleton().GetEnginePtr();

		FSN_ASSERT(!resource->IsLoaded());

		if (resource->GetPath().empty())
		{
			resource->SetDataPtr(nullptr);
			resource->setLoaded(false);
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded");
		}

		asIScriptModule* module = engine->GetModule(resource->GetPath().c_str(), asGM_CREATE_IF_NOT_EXISTS);

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
			resource->setLoaded(false);
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded: " + ex.what());
		}

		if (r >= 0)
		{
			resource->SetDataPtr(module);
			resource->setLoaded(true);
		}
		else
		{
			resource->SetDataPtr(nullptr);
			resource->setLoaded(false);
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded");
		}
	}

	void UnloadScriptResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);

			auto engine = ScriptManager::getSingleton().GetEnginePtr();

			engine->DiscardModule(
				static_cast<asIScriptModule*>(resource->GetDataPtr())->GetName());
		}

		resource->SetDataPtr(nullptr);
	}

	template <typename T>
	class PropertyFollowerCallbackForMethod : public IPropertyFollowerCallbackForMethod
	{
	public:
		PropertyFollowerCallbackForMethod(asIScriptFunction* fn)
			: function(fn)
		{
			FSN_ASSERT(function);
			hasNew = false;
		}
		void operator() (const T& value)
		{
			currentValue = value;
			hasNew = true;
		}
		void ExecuteScriptMethod(asIScriptContext* ctx)
		{
			if (hasNew)
			{
				hasNew = false;
				FSN_ASSERT(ctx->GetState() == asEXECUTION_PREPARED);
				//ctx->Prepare(function);
				int r = ctx->SetArgObject(0, &currentValue); FSN_ASSERT(r >= 0);
				if (r)
					ctx->Execute();
			}
		}
	private:
		T currentValue;
		tbb::atomic<bool> hasNew;
		asIScriptFunction* function;
	};

	template <>
	class PropertyFollowerCallbackForMethod<boost::intrusive_ptr<CScriptAny>> : public IPropertyFollowerCallbackForMethod
	{
	public:
		PropertyFollowerCallbackForMethod(asIScriptFunction* fn)
			: function(fn)
		{
			hasNew = false;
		}
		void operator() (const boost::intrusive_ptr<CScriptAny>& value)
		{
			currentValue = value;
			hasNew = true;
		}
		void ExecuteScriptMethod(asIScriptContext* ctx)
		{
			if (hasNew)
			{
				hasNew = false;
				FSN_ASSERT(ctx->GetState() == asEXECUTION_PREPARED);
				//ctx->Prepare(function);
				int r = ctx->SetArgObject(0, currentValue.get()); FSN_ASSERT(r >= 0);
				if (r)
					ctx->Execute();
			}
		}
	private:
		boost::intrusive_ptr<CScriptAny> currentValue;
		tbb::atomic<bool> hasNew;
		asIScriptFunction* function;
	};

	class PropertyFollowerFactoryForMethods
	{
	public:
		PropertyFollowerFactoryForMethods(ASScript::ScriptMethodData* script_method_data)
			: method_data(*script_method_data),
			param0_type_id(-1)
		{
			FSN_ASSERT(method_data.function);
			if (method_data.function->GetParamCount() > 0)
				param0_type_id = method_data.function->GetParamTypeId(0);
			else
				FSN_EXCEPT(InvalidArgumentException, std::string() + "Method: " + method_data.function->GetDeclaration() + " has no parameters, so it can't be a property change handler!");
		}
		template <typename T>
		void operator() (T)
		{
			if (Scripting::RegisteredAppType<T>::type_id == param0_type_id)
			{
				Generate<T>();
			}
		}
		template <typename T>
		void Generate()
		{
			auto cb = std::make_shared<PropertyFollowerCallbackForMethod<T>>(method_data.function);
			method_data.persistentFollower = std::make_shared<PersistentConnectionAgent<T>>([cb](const T& value) { (*cb)(value); } );
			method_data.followedPropertyCallback = cb;
		}
		ASScript::ScriptMethodData& method_data;
		int param0_type_id;
	};

	ASScript::ScriptInterface::ScriptInterface(ASScript* owner, asIScriptObject* script_object)
		: object(script_object),
		component(owner)
	{
		FSN_ASSERT(owner);
	}

	void ASScript::ScriptInterface::Yield()
	{
		component->Yield();
	}

	void ASScript::ScriptInterface::YieldUntil(std::function<bool (void)> condition, float timeout)
	{
		component->YieldUntil(condition, timeout);
	}

	void ASScript::ScriptInterface::CreateCoroutine(asIScriptFunction *fn)
	{
		component->CreateCoroutine(fn);
	}

	void ASScript::ScriptInterface::CreateCoroutine(const std::string& functionName, float delay)
	{
		component->CreateCoroutine(functionName, delay);
	}

	CScriptAny* ASScript::ScriptInterface::GetPropertyRaw(unsigned int index)
	{
		return component->GetPropertyRaw(index);
	}

	bool ASScript::ScriptInterface::SetPropertyRaw(unsigned int index, void *ref, int typeId)
	{
		return component->SetPropertyRaw(index, ref, typeId);
	}

	ComponentProperty* ASScript::ScriptInterface::GetProperty(unsigned int index)
	{
		return component->GetProperty(index);
	}

	void ASScript::ScriptInterface::EnumReferences(asIScriptEngine *engine)
	{
		engine->GCEnumCallback(object.get());
	}

	void ASScript::ScriptInterface::ReleaseAllReferences(asIScriptEngine *engine)
	{
		object.reset();
	}

	EntityPtr ASScript_ScriptInterface_GetParent(ASScript::ScriptInterface* interface_obj)
	{
		FSN_ASSERT(interface_obj->component);
		return interface_obj->component->GetParent()->shared_from_this();
	}

	// TODO: implement a asScriptObject* MakeEntityWrapper(activeScript, entityReferenced)
	//! Init an entity wrapper in an arbitory object
	static std::uint32_t InitEntityPtr(const EntityPtr& referenceHolder, const EntityPtr& entityReferenced)
	{
		if (entityReferenced)
		{
			referenceHolder->HoldReference(entityReferenced);

			if (referenceHolder->IsSyncedEntity() && entityReferenced->IsSyncedEntity())
			{
				if (referenceHolder->GetID() == entityReferenced->GetID())
				{
					return std::numeric_limits<uint32_t>::max();
				}
				else
				{
					uint32_t token = referenceHolder->GetManager()->StoreReference(referenceHolder->GetID(), entityReferenced->GetID());
					if (token == 0)
						asGetActiveContext()->SetException("Failed to create a synced entity pointer: no more pointer-IDs are available");
					return token;
				}
			}
		}
		return 0;
	}

	//! Init an entity wrapper in the current script
	static std::uint32_t ScriptInitEntityPtr(std::weak_ptr<Entity>& owner, const EntityPtr& entityReferenced)
	{
		if (entityReferenced)
		{
			auto activeScript = ASScript::GetActiveScript(); FSN_ASSERT(activeScript); FSN_ASSERT(activeScript->GetParent());

			auto referenceHolder = activeScript->GetParent()->shared_from_this();
			referenceHolder->HoldReference(entityReferenced);
			owner = referenceHolder;

			return InitEntityPtr(referenceHolder, entityReferenced);
		}
		return 0;
	}

	static void DeinitEntityPtr(const std::weak_ptr<Entity>& referenceHolder, uint32_t token, const EntityPtr& entityReferenced)
	{
		if (token != std::numeric_limits<uint32_t>::max())
		if (auto locked = referenceHolder.lock())
		{
			locked->DropReference(entityReferenced);

			locked->GetManager()->DropReference(token);
		}
	}

	static ASScript::ScriptInterface* ScriptInterface_FromComponent(EntityComponent* component)
	{
		auto scriptComponent = convert_ref<EntityComponent, ASScript>(component);
		if (scriptComponent)
		{
			auto scriptInterface = scriptComponent->GetScriptInterface();
			scriptInterface->addRef();
			scriptComponent->release();
			return scriptInterface.get();
		}

		return nullptr;
	}

	static void ASScript_ScriptInterface_bindMethodToProperty(const std::string& decl, ComponentProperty* prop, ASScript::ScriptInterface* obj)
	{
		obj->component->BindMethod(EvesdroppingManager::getSingleton().GetSignalingSystem(), decl, prop->GetID());
	}

	void ASScript::ScriptInterface::Register(asIScriptEngine* engine)
	{
		{
			int r = engine->RegisterFuncdef("void coroutine_t()"); FSN_ASSERT(r >= 0);
		}

		{
			int r;
			ASScript::ScriptInterface::RegisterGCType(engine, "ASScript");

			s_ASScriptTypeId = engine->GetTypeIdByDecl("ASScript");
			FSN_ASSERT(s_ASScriptTypeId >= 0);

			r = engine->RegisterObjectBehaviour("EntityComponent", asBEHAVE_REF_CAST, "ASScript@ f()", asFUNCTION(ScriptInterface_FromComponent), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod("ASScript", "void bindMethod(const string &in, PropertyAny@)", asFUNCTION(ASScript_ScriptInterface_bindMethodToProperty), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod("ASScript", "void yield()", asMETHOD(ASScript::ScriptInterface, Yield), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void createCoroutine(coroutine_t @)", asMETHODPR(ASScript::ScriptInterface, CreateCoroutine, (asIScriptFunction*), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void createCoroutine(const string &in, float delay = 0.0f)", asMETHODPR(ASScript::ScriptInterface, CreateCoroutine, (const std::string&, float), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "any@ getPropertyRaw(uint) const", asMETHOD(ASScript::ScriptInterface, GetPropertyRaw), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void setPropertyRaw(uint, ?&in)", asMETHODPR(ASScript::ScriptInterface, SetPropertyRaw, (unsigned int, void*,int), bool), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

			r = engine->RegisterObjectMethod("ASScript", "PropertyAny@ getProperty(uint) const", asMETHOD(ASScript::ScriptInterface, GetProperty), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			
			r = engine->RegisterObjectMethod("ASScript", "Entity getParent()", asFUNCTION(ASScript_ScriptInterface_GetParent), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterGlobalFunction("uint32 initEntityPointer(EntityW &out, const Entity &in)", asFUNCTION(ScriptInitEntityPtr), asCALL_CDECL); FSN_ASSERT(r >= 0);
			r = engine->RegisterGlobalFunction("void deinitEntityPointer(const EntityW &in, uint32, const Entity &in)", asFUNCTION(DeinitEntityPtr), asCALL_CDECL); FSN_ASSERT(r >= 0);
		}
	}

	std::uint32_t ASScript::CreateEntityWrapperId(const EntityPtr& entityReferenced)
	{
		return InitEntityPtr(GetParent()->shared_from_this(), entityReferenced);
	}

	asIScriptObject* ASScript::CreateEntityWrapper(const EntityPtr& entityReferenced)
	{
		if (!m_ScriptObject)
			return nullptr;
		if (!m_ScriptObject->object)
			return nullptr;

		int entityTypeId = m_ScriptObject->object->GetEngine()->GetTypeIdByDecl("EntityW");

		FSN_ASSERT(GetParent());
		auto parentEntity = GetParent()->shared_from_this();

		// Alloc a wrapper
		const int entityWrapperObjectTypeId = m_EntityWrapperTypeId & ~asTYPEID_OBJHANDLE;
		auto scriptType = ScriptManager::getSingleton().GetEnginePtr()->GetObjectTypeById(entityWrapperObjectTypeId);
		asIScriptObject* entityWrapper = static_cast<asIScriptObject*>(ScriptManager::getSingleton().GetEnginePtr()->CreateScriptObject(scriptType));

		// Init the wrapper
		auto token = InitEntityPtr(parentEntity, entityReferenced);

		{
			FSN_ASSERT(entityWrapper->GetPropertyTypeId(0) == entityTypeId && entityWrapper->GetPropertyTypeId(1) == entityTypeId &&
				entityWrapper->GetPropertyTypeId(2) == asTYPEID_UINT32);

			auto app_obj = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(0));
			*app_obj = entityReferenced;

			auto owner = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(1));
			*owner = parentEntity;

			auto pointerId = static_cast<std::uint32_t*>(entityWrapper->GetAddressOfProperty(2));
			*pointerId = token;
		}

		return entityWrapper;
	}

	ASScript::ASScript()
		: m_ReloadScript(false),
		m_ModuleReloaded(false),
		m_EntityWrapperTypeId(-1)
	{
	}

	ASScript::~ASScript()
	{
		m_ModuleResourceConnection.disconnect();
	}

	std::shared_ptr<Box2DContactListener> ASScript::GetContactListener()
	{
		if (m_ContactListener)
			return m_ContactListener;
		else
			return m_ContactListener = std::make_shared<ASScriptB2ContactListener>(this);
	}

	bool ASScript::PopCollisionEnterEvent(boost::intrusive_ptr<ScriptCollisionEvent>& ev)
	{
		if (m_ContactListener)
			return m_ContactListener->m_CollisionEnterEvents.try_pop(ev);
		else
			return false;
	}

	bool ASScript::PopCollisionExitEvent(boost::intrusive_ptr<ScriptCollisionEvent>& ev)
	{
		if (m_ContactListener)
			return m_ContactListener->m_CollisionExitEvents.try_pop(ev);
		else
			return false;
	}

	void ASScript::AddEventHandlerMethodDecl(size_t id, const std::string& decl)
	{
		if (id >= m_EventHandlerMethodIndex.size())
			m_EventHandlerMethodIndex.resize(id + 1, std::numeric_limits<size_t>::max());
		if (auto ptr = GetMethodData(decl))
		{
			auto index = std::distance(&m_ScriptMethods[0], ptr);
			if (index >= 0)
				m_EventHandlerMethodIndex[id] = (size_t)index;
		}
	}

	bool ASScript::PrepareMethod(const boost::intrusive_ptr<asIScriptContext>& ctx, const std::string& decl)
	{
		if (!m_ScriptObject)
			FSN_EXCEPT(InvalidArgumentException, "Script isn't instanciated");

		auto _where = m_MethodDeclIndex.find(decl);
		if (_where != m_MethodDeclIndex.end())
		{
			FSN_ASSERT(m_ScriptMethods.size() > _where->second);
			return PrepareMethod(ctx, m_ScriptMethods[_where->second].function);
		}
		else
		{
			auto func = GetScriptInterface()->object->GetObjectType()->GetMethodByDecl(decl.c_str());

			if (func)
			{
				size_t index = 0;
				for (auto it = m_ScriptMethods.begin(); it != m_ScriptMethods.end(); ++it, ++index)
				{
					if (it->function == func)
					{
						m_MethodDeclIndex[decl] = index;
						return PrepareMethod(ctx, func);
					}
				}
			}
			return false;
		}
	}

	bool ASScript::PrepareMethod(const boost::intrusive_ptr<asIScriptContext>& ctx, size_t event_id)
	{
		if (m_EventHandlerMethodIndex.size() > event_id)
		{
			const auto& index = m_EventHandlerMethodIndex[event_id];
			if (index != std::numeric_limits<size_t>::max())
				return PrepareMethod(ctx, m_ScriptMethods[index].function);
			else
				return false;
		}
		else
		{
			m_EventHandlerMethodIndex.resize(event_id + 1, std::numeric_limits<size_t>::max());
			return false;
		}
	}

	bool ASScript::PrepareMethod(const boost::intrusive_ptr<asIScriptContext>& ctx, asIScriptFunction* func)
	{
		FSN_ASSERT(ctx);
		FSN_ASSERT(ctx->GetState() == asEXECUTION_UNINITIALIZED);
		int r = ctx->Prepare(func);
		if (r >= 0)
		{
			ctx->SetObject(m_ScriptObject->object.get());
		}
		else
		{
			ctx->SetObject(NULL);
			return false;
		}

		return true;
	}

	ASScript::ScriptMethodData* ASScript::GetMethodData(const std::string& decl)
	{
		if (!m_ScriptObject)
			FSN_EXCEPT(InvalidArgumentException, "Script isn't instanciated");

		ScriptMethodData* methodData = nullptr;

		auto _where = m_MethodDeclIndex.find(decl);
		if (_where != m_MethodDeclIndex.end())
		{
			FSN_ASSERT(m_ScriptMethods.size() > _where->second);
			methodData = &m_ScriptMethods[_where->second];
		}
		else
		{
			auto func = GetScriptInterface()->object->GetObjectType()->GetMethodByDecl(decl.c_str());

			if (func)
			{
				size_t index = 0;
				for (auto it = m_ScriptMethods.begin(); it != m_ScriptMethods.end(); ++it, ++index)
				{
					if (it->function == func)
					{
						m_MethodDeclIndex[decl] = index;
						methodData = &(*it);
					}
				}
			}
		}

		return methodData;
	}
	
	ASScript::ScriptMethodData* ASScript::GetMethodData(size_t event_id)
	{
		if (!m_ScriptObject)
			FSN_EXCEPT(InvalidArgumentException, "Script isn't instanciated");

		if (m_EventHandlerMethodIndex.size() > event_id)
		{
			const auto& index = m_EventHandlerMethodIndex[event_id];
			if (index != std::numeric_limits<size_t>::max())
				return &m_ScriptMethods[index];
			else
				return nullptr;
		}
		else
		{
			m_EventHandlerMethodIndex.resize(event_id + 1, std::numeric_limits<size_t>::max());
			return nullptr;
		}
	}

	bool ASScript::HasMethod(const std::string& decl)
	{
		return GetMethodData(decl) != nullptr;
	}

	bool ASScript::HasMethod(size_t event_id)
	{
		return GetMethodData(event_id) != nullptr;
	}

	void ASScript::BindMethod(PropertySignalingSystem_t& system, const std::string& decl, PropertyID id)
	{
		if (auto method = GetMethodData(decl))
		{
			FSN_ASSERT(method->persistentFollower);
			method->persistentFollower->Subscribe(system, id);
		}
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

			if (scriptCom)
			{
				if (scriptCom->GetPropertyTypeId(0) == s_ASScriptTypeId)
				{
					// Note: The script object stores a pointer to the intermediate ScriptInterface (which is garbage collected)
					const auto scriptInterface = *static_cast<ASScript::ScriptInterface**>( scriptCom->GetAddressOfProperty(0) );
					return scriptInterface->component;
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
						return (*static_cast<ASScript::ScriptInterface**>(propAddress))->component;
					}
					else
						return nullptr;
				}
			}
		}
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

			if (method && objectType != m_ScriptObject->object->GetObjectType())
			{
				const std::string thisTypeName = m_ScriptObject->object->GetObjectType()->GetName();
				ctx->SetException(("Tried to create a coroutine for a method from another class. This class: " + thisTypeName + ", Method: " + fn->GetDeclaration()).c_str());
				return;
			}

			auto coCtx = engine->CreateContext();
			coCtx->Prepare(fn);
			if (method)
				coCtx->SetObject(m_ScriptObject->object.get());

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

			asIScriptFunction* func = nullptr;
			bool method = false;

			std::string decl = "void " + functionName + "()";

			
			auto _where = m_MethodDeclIndex.find(decl);
			if (_where != m_MethodDeclIndex.end())
			{
				FSN_ASSERT(_where->second < m_ScriptMethods.size());
				func = m_ScriptMethods[_where->second].function;
				method = true;
			}
			else
			{
				if (func = GetScriptInterface()->object->GetObjectType()->GetMethodByDecl(decl.c_str()))
					method = true;
				else
				{
					func = m_Module->GetFunctionByDecl(decl.c_str());
					method = false;
				}
			}

			if (func)
			{
				auto coCtx = engine->CreateContext();
				coCtx->Prepare(func);
				if (method)
					coCtx->SetObject(m_ScriptObject->object.get());

				ConditionalCoroutine cco;
				cco.new_ctx = coCtx;
				if (!fe_fzero(delay))
					cco.SetTimeout(delay);
				m_ActiveCoroutinesWithConditions[coCtx] = std::move(cco);
				coCtx->Release();
			}
			else
			{
				// No function matching the decl
				ctx->SetException(("Function '" + decl + "' doesn't exist").c_str());
			}
		}
	}

	void getWrappedEntity(std::weak_ptr<Entity>*& out, asIScriptObject* entityWrapper)
	{
		FSN_ASSERT(std::string(entityWrapper->GetPropertyName(0)) == "app_obj");
		out = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(0));
	}

	CScriptAny* ASScript::GetPropertyRaw(unsigned int index)
	{
		if (index >= m_ScriptProperties.size())
		{
			if (asGetActiveContext())
			{
				asGetActiveContext()->SetException("Tried to access a script property that doesn't exist");
				return nullptr;
			}
			else
			{
				FSN_EXCEPT(InvalidArgumentException, "Tried to access a script property that doesn't exist");
			}
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

				std::weak_ptr<Entity>* wrappedEntity = nullptr;
				getWrappedEntity(wrappedEntity, entityWrapper);

				// TODO: static int Entity::GetTypeId() & GetWptrTypeId;
				int entityTypeId = engine->GetTypeIdByDecl("EntityW");
				auto unwrappedAny = new CScriptAny(wrappedEntity, entityTypeId, engine);
				return unwrappedAny;
			}
			else return new CScriptAny(engine);
		}
	}

	bool ASScript::SetPropertyRaw(unsigned int index, void *ref, int typeId)
	{
		if (index >= m_ScriptProperties.size())
		{
			if (asGetActiveContext())
			{
				asGetActiveContext()->SetException( "Tried to access a script property that doesn't exist");
				return false;
			}
			else
			{
				FSN_EXCEPT(InvalidArgumentException, "Tried to access a script property that doesn't exist");
			}
		}

		auto scriptProperty = static_cast<ScriptAnyTSP*>( m_ScriptProperties[index].get() ); FSN_ASSERT(scriptProperty);

		if (scriptProperty->GetTypeId() != m_EntityWrapperTypeId)
		{
			try
			{
				scriptProperty->SetAny(ref, typeId);
			}
			catch (InvalidArgumentException& e)
			{
				if (asGetActiveContext())
				{
					asGetActiveContext()->SetException(e.what());
					return false;
				}
				else
					throw e;
			}
		}
		else
		{
			auto engine = m_Module->GetEngine();
			int entityTypeId = engine->GetTypeIdByDecl("EntityW");

			if (typeId != entityTypeId)
				return false;

			auto passedObj = static_cast<std::weak_ptr<Entity>*>(ref);

			if (!passedObj) // script passed null
				return false;

			if (auto lockedTarget = passedObj->lock()) // Check that the pointer isn't expired
			{
				//auto wrapperCtor = ScriptUtils::Calling::Caller::FactoryCaller(engine->GetObjectTypeById(m_EntityWrapperTypeId & ~asTYPEID_OBJHANDLE), "Entity &in");
				//FSN_ASSERT(wrapperCtor);
				//void* entityWrapper = wrapperCtor(passedObj);

				FSN_ASSERT(GetParent());
				auto parentEntity = GetParent()->shared_from_this();

				// Alloc a wrapper
				const int entityWrapperObjectTypeId = m_EntityWrapperTypeId & ~asTYPEID_OBJHANDLE;
				auto scriptType = ScriptManager::getSingleton().GetEnginePtr()->GetObjectTypeById(entityWrapperObjectTypeId);
				asIScriptObject* entityWrapper = static_cast<asIScriptObject*>(ScriptManager::getSingleton().GetEnginePtr()->CreateScriptObject(scriptType));

				// Init the wrapper
				auto token = InitEntityPtr(parentEntity, lockedTarget);

				{
					FSN_ASSERT(entityWrapper->GetPropertyTypeId(0) == entityTypeId && entityWrapper->GetPropertyTypeId(1) == entityTypeId &&
						entityWrapper->GetPropertyTypeId(2) == asTYPEID_UINT32);

					auto app_obj = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(0));
					*app_obj = *passedObj;

					auto owner = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(1));
					*owner = parentEntity;

					auto pointerId = static_cast<std::uint32_t*>(entityWrapper->GetAddressOfProperty(2));
					*pointerId = token;
				}

				// Pass the wrapper to the script property so it will get injected into the script component at the next prop-sync
				auto any = boost::intrusive_ptr<CScriptAny>(new CScriptAny(&entityWrapper, m_EntityWrapperTypeId, engine), false);
				try
				{
					scriptProperty->Set(any);
				}
				catch (InvalidArgumentException& e)
				{
					if (asGetActiveContext())
					{
						asGetActiveContext()->SetException(e.what());
						return false;
					}
					else
						throw e;
				}
			}
			else // Failed to lock ref
				return false;
		}
		return true;
	}

	ComponentProperty* ASScript::GetProperty(unsigned int index)
	{
		if (index >= m_ScriptPropertyInterfaces.size())
		{
			if (asGetActiveContext())
			{
				asGetActiveContext()->SetException("Tried to access a script property that doesn't exist");
				return nullptr;
			}
			else
			{
				FSN_EXCEPT(InvalidArgumentException, "Tried to access a script property that doesn't exist");
			}
		}

		return m_ScriptPropertyInterfaces[index];
	}

	void ASScript::SetScriptObject(asIScriptObject* obj, const std::vector<std::pair<std::string, std::string>>& properties)
	{
		if (m_ScriptObject)
		{
			m_ScriptPropertyInterfaces.clear();
			auto newEnd = std::remove_if(this->m_Properties.begin(), this->m_Properties.end(), [](const std::pair<std::string, PropertyPtr>& prop)->bool
			{
				return dynamic_cast<ScriptAnyTSP*>(prop.second->GetImpl()) != nullptr;
			});
			this->m_Properties.erase(newEnd, this->m_Properties.end());
			m_ScriptProperties.clear();
			m_ScriptPropertyInfo.clear();
			m_ScriptObject.reset();
		}
		if (obj)
		{
			// Required to construct script property objects
			FSN_ASSERT(m_EntityWrapperTypeId >= 0);

			m_ScriptObject = new ScriptInterface(this, obj);
			m_ScriptObject->release();

			m_FirstInit = true;

			m_ScriptProperties.resize(properties.size());
			m_ScriptPropertyInterfaces.resize(properties.size());
			m_ScriptPropertyInfo.resize(properties.size());
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

					auto comProp = new ScriptAnyTSP(obj, i, m_EntityWrapperTypeId);

					FSN_ASSERT((comProp->GetTypeId() & asTYPEID_OBJHANDLE) == 0 || comProp->GetTypeId() == m_EntityWrapperTypeId);

					auto& propInfo = m_ScriptPropertyInfo[interfaceIndex];
					propInfo.name = nameStr;
					propInfo.type_id = comProp->GetTypeId();

					// Copy in any values that were deserialised before the script was reloaded
					try
					{
						if (m_CachedProperties.size() > interfaceIndex)
						{
							const auto& cachedProp = m_CachedProperties[interfaceIndex];
							if (cachedProp->GetTypeId() != ASScriptSerialisaiton::s_EntityTypeID)
							{
								comProp->Set(cachedProp);
								comProp->Synchronise();
							}
						}
						// Editable data (stored by name, rather than index)
						//else if (m_LastDeserMode == Editable)
						//{
						//	auto _where = m_EditableCachedProperties.find(comProp->GetName());
						//	if (_where != m_EditableCachedProperties.end())
						//	{
						//		const auto& cachedProp = _where->second;
						//		if (cachedProp->GetTypeId() != ASScriptSerialisaiton::s_EntityTypeID)
						//		{
						//			comProp->Set(cachedProp);
						//			comProp->Synchronise();
						//		}
						//		else
						//		{
						//			const auto& name = _where->first;
						//			auto namedWrapperEntry = m_EditableUninitialisedEntityWrappers.find(name);
						//			if (namedWrapperEntry != m_EditableUninitialisedEntityWrappers.end())
						//				m_UninitialisedEntityWrappers.push_back(std::make_pair(interfaceIndex, namedWrapperEntry->second));
						//		}
						//	}
						//}
					}
					catch (InvalidArgumentException&)
					{
						AddLogEntry("Failed to re-load script properties - data type has changed for " + std::string(obj->GetObjectType()->GetName()) + "." +  comProp->GetName());
					}

					// Store the new property thing
					m_ScriptProperties[interfaceIndex].reset(comProp);
					// Add the property and store the generated interface object
					m_ScriptPropertyInterfaces[interfaceIndex] = AddProperty(comProp->GetName(), comProp).get();
				}
			}

			m_CachedProperties.clear();
			m_EditableCachedProperties.clear();

			// Init method data
			auto objType = obj->GetObjectType();
			m_ScriptMethods.resize(objType->GetMethodCount());
			for (size_t i = 0, count = objType->GetMethodCount(); i < count; ++i)
			{
				auto method = objType->GetMethodByIndex(i);
				auto& methodInfo = m_ScriptMethods[i];
				
				methodInfo.function = method;
				methodInfo.id = method->GetId();

				m_MethodDeclIndex[method->GetDeclaration(false)] = i;

				try
				{
					if (method->GetParamCount() == 1)
					{
						PropertyFollowerFactoryForMethods factory(&methodInfo);
						boost::mpl::for_each<Scripting::CommonAppTypes>(factory);

						if (!methodInfo.persistentFollower && method->GetParamTypeId(0) == objType->GetEngine()->GetTypeIdByDecl("any"))
						{
							factory.Generate<boost::intrusive_ptr<CScriptAny>>();
						}

						FSN_ASSERT(!methodInfo.persistentFollower || methodInfo.followedPropertyCallback);
					}
				}
				catch (InvalidArgumentException&)
				{
				}
			}

			// Pass the script-interface to the script object
			{
				auto setAppObj = ScriptUtils::Calling::Caller::Create(obj, "void _setAppObj(ASScript @)");
				if (setAppObj)
				{
					m_ScriptObject->addRef();
					try
					{
						setAppObj.SetThrowOnException(true);
						setAppObj(m_ScriptObject.get());
					}
					catch (ScriptUtils::Exception& e)
					{
						m_ScriptObject->release();
						throw e;
					}
				}
			}
		}
	}

	void ASScript::CheckChangedPropertiesIn()
	{
		for (auto it = m_ScriptProperties.begin(), end = m_ScriptProperties.end(); it != end; ++it)
		{
			auto scriptProperty = static_cast<ScriptAnyTSP*>( it->get() ); FSN_ASSERT(scriptProperty);
			// Mark the property for serialisation and post it to be synched if it has changed since last synch
			scriptProperty->MarkForSerialisationIfDirty();
		}
	}

	void ASScript::ModuleLoaded(ResourceDataPtr resource)
	{
		m_Module.SetTarget(resource);
		m_ModuleReloaded = true;

		if (m_Module.IsLoaded())
		{
			m_EntityWrapperTypeId = m_Module->GetTypeIdByDecl("EntityWrapper@"); FSN_ASSERT(m_EntityWrapperTypeId >= 0);
		}
	}

	bool ASScript::HotReloadEvent(ResourceDataPtr resource, ResourceContainer::HotReloadEvent ev)
	{
		if (ev == ResourceContainer::HotReloadEvent::Validate)
		{
			m_Module.Release();
		}
		else if (ev == ResourceContainer::HotReloadEvent::PreReload)
		{
			// TODO: get the script world to perform the build here, so that when the resource
			//  manager "reloads" the script resources the rebuilt modules are ready
		}
		else if (ev == ResourceContainer::HotReloadEvent::PostReload)
		{
			ModuleLoaded(resource);
		}
		return true; // Allow reloading
	}

	bool ASScript::InitialiseEntityWrappers()
	{
		FSN_ASSERT(m_Module.IsLoaded() && m_ScriptObject);

		bool done = true;

		auto engine = m_Module->GetEngine();
		auto entityManager = GetParent()->GetManager();

		for (auto it = m_UninitialisedEntityWrappers.begin(), end = m_UninitialisedEntityWrappers.end(); it != end;)
		{
			unsigned int propertyIndex = std::get<0>(*it);
			//const std::string& propertyName = std::get<1>(*it);
			uint32_t pointerId = std::get<1>(*it);

			if (pointerId != 0)
			{
				EntityPtr refedEntity;
				bool validId = false;
				if (pointerId < std::numeric_limits<uint32_t>::max())
				{
					ObjectID id = entityManager->RetrieveReference(pointerId);
					if (id != 0)
					{
						refedEntity = entityManager->GetEntity(id, m_FirstInit);
						validId = true;
					}
				}
				else
				{
					refedEntity = GetParent()->shared_from_this();
					validId = true;
				}

				if (validId && refedEntity)
				{
					asIScriptObject* entityWrapper = nullptr;

					auto scriptprop = static_cast<ScriptAnyTSP*>(m_ScriptProperties[propertyIndex].get());

					const int entityWrapperObjectTypeId = m_EntityWrapperTypeId & ~asTYPEID_OBJHANDLE;
					auto scriptType = ScriptManager::getSingleton().GetEnginePtr()->GetObjectTypeById(entityWrapperObjectTypeId);
					entityWrapper = static_cast<asIScriptObject*>(ScriptManager::getSingleton().GetEnginePtr()->CreateScriptObject(scriptType));
					FSN_ASSERT(entityWrapper);

					auto prop = boost::intrusive_ptr<CScriptAny>(new CScriptAny(&entityWrapper, m_EntityWrapperTypeId, engine), false);
					scriptprop->Set(prop);

					auto app_obj = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(0));
					*app_obj = refedEntity;

					auto owner = static_cast<std::weak_ptr<Entity>*>(entityWrapper->GetAddressOfProperty(1));
					*owner = GetParent()->shared_from_this();

					auto pointer_id = static_cast<uint32_t*>(entityWrapper->GetAddressOfProperty(2));
					*pointer_id = pointerId;

					GetParent()->HoldReference(refedEntity);
				}
				else
				{
					done = false;
					++it;
					continue;
				}
			}
			it = m_UninitialisedEntityWrappers.erase(it);
			end = m_UninitialisedEntityWrappers.end();
		}
		//m_UninitialisedEntityWrappers.clear();
		m_FirstInit = false;

		return done;
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

	void ASScript::SerialiseContinuous(RakNet::BitStream& stream)
	{
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
			val.typeId = ASScriptSerialisaiton::s_EntityTypeID;
			stream.Write(val.typeId);
			val.typeId = temp;

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
		else if (val.typeId == ScriptManager::getSingleton().GetStringTypeId())
		{
			stream.Write(ASScriptSerialisaiton::s_CompressedStringTypeID);

			std::string strVal;
			any->Retrieve(&strVal, val.typeId);
			if (strVal.length() > ASScriptSerialisaiton::s_MaxStringLength)
			{
				FSN_EXCEPT(SerialisationError, "String value too long");
			}
			RakNet::StringCompressor::Instance()->EncodeString(strVal.c_str(), strVal.length() + 1, &stream);
		}
		else if ((val.typeId & asTYPEID_SCRIPTOBJECT) != 0)
		{
			FSN_ASSERT_FAIL("Not implemented");
			// TODO: check if it implements ISerialisable and serialise it, if not, just copy each prop
		}
		else if ((val.typeId & asTYPEID_APPOBJECT) != 0)
		{
			FSN_EXCEPT(SerialisationError, "Can't serialise app objects");
		}
		else
		{
			FSN_ASSERT((val.typeId & asTYPEID_MASK_OBJECT) == 0);

			stream.Write(val.typeId);
			stream.Write(val.valueInt);
		}

		return true;
	}

	bool ASScript::DeserialiseProp(RakNet::BitStream& stream, CScriptAny* prop, size_t index, const std::string& name)
	{
		FSN_ASSERT(prop);

		stream.Read(prop->value.typeId);

		if (prop->value.typeId == ASScriptSerialisaiton::s_EntityTypeID)
		{
			prop->value.valueObj = 0;

			//ObjectID id;
			//stream.Read(id);

			uint32_t pointer_id;
			stream.Read(pointer_id);

			if (name.empty())
				m_UninitialisedEntityWrappers.push_back(std::make_pair(index, pointer_id));
			else
				m_EditableUninitialisedEntityWrappers[name] = pointer_id;
		}
		else if (prop->value.typeId == ASScriptSerialisaiton::s_CompressedStringTypeID)
		{
			RakNet::RakString rakStringVal;
			if (RakNet::StringCompressor::Instance()->DecodeString(&rakStringVal, ASScriptSerialisaiton::s_MaxStringLength, &stream))
			{
				auto actualTypeId = ScriptManager::getSingleton().GetStringTypeId();
				std::string strVal(rakStringVal.C_String());

				prop->Store(&strVal, actualTypeId);
			}
			else
			{
				FSN_EXCEPT(SerialisationError, "Compressed string value seems to be too long");
			}
		}
		else
		{
			auto v = prop->value.valueInt;
			stream.Read(v);
			prop->Store(&v, prop->value.typeId);
		}

		return true;
	}

	//void ASScript::DeserNonStandardProp(RakNet::BitStream& stream, CScriptAny* prop, size_t index, const std::string& name)
	//{
	//	if (prop->value.typeId == ASScriptSerialisaiton::s_EntityTypeID)
	//	{
	//		ObjectID id;
	//		stream.Read(id);

	//		m_UninitialisedEntityWrappers.push_back(std::make_pair(index, id));
	//	}
	//}

	void ASScript::SerialiseOccasional(RakNet::BitStream& stream)
	{
		//const SerialiseMode mode = SerialiseMode::All;

		SerialisationUtils::write(stream, GetScriptPath());

		if (m_ScriptObject)
		{
			stream.Write(m_ScriptProperties.size());
			for (auto it = m_ScriptProperties.begin(), end = m_ScriptProperties.end(); it != end; ++it)
			{
				auto scriptprop = static_cast<ScriptAnyTSP*>((*it).get());

				//if (mode == Editable)
				//{
				//	std::string name = scriptprop->GetName();
				//	stream.Write(name.length());
				//	stream.Write(name.data(), name.length());
				//}
				{
					SerialiseProp(stream, scriptprop->Get());
					// Changes have been written
					scriptprop->Unmark();
				}
			}
		}
		else 
		{
			/*if (mode == Editable)
			{
				if (m_EditableCachedProperties.empty())
					FSN_EXCEPT(SerialisationError, "Data wasn't deserialised in editable mode, and the script object isn't available to reference against");
				for (auto it = m_EditableCachedProperties.begin(), end = m_EditableCachedProperties.end(); it != end; ++it)
				{
					std::string name = it->first;
					stream.Write(name.length());
					stream.Write(name.data(), name.length());
					SerialiseProp(stream, it->second.get());
				}
			}
			else */if (/*mode == All && */!m_CachedProperties.empty()) // Script may have just been deserialised and not activated
			{
				stream.Write(m_CachedProperties.size());
				for (auto it = m_CachedProperties.begin(), end = m_CachedProperties.end(); it != end; ++it)
				{
					auto& prop = *it;
					SerialiseProp(stream, prop.get());
				}
			}
		}
	}

	void ASScript::DeserialiseOccasional(RakNet::BitStream& stream)
	{
		std::string newPath;
		SerialisationUtils::read(stream, newPath);

		if (newPath != m_Path)
		{
			m_Path = newPath;
			m_ReloadScript = true;
		}

		auto engine = ScriptManager::getSingleton().GetEnginePtr();

		//if (mode != Changes)
		{
			m_EditableCachedProperties.clear();
			m_CachedProperties.clear();
		}

		auto numProperties = m_ScriptProperties.size();
		stream.Read(numProperties);
		if (m_ScriptObject)
		{
			if (/*mode != Editable && */m_ScriptProperties.size() != numProperties)
			{
				FSN_EXCEPT(SerialisationError, "Script data has incorrect number of properties");
			}
			FSN_ASSERT(m_ScriptObject->object);

			//if (mode == Editable)
			//{
			//	std::map<std::string, boost::intrusive_ptr<CScriptAny>> namedProperties;
			//	for (size_t i = 0; i < numProperties; ++i)
			//	{
			//		size_t length; std::string name;
			//		stream.Read(length);
			//		name.resize(length);
			//		stream.Read(&name[0], length);

			//		auto prop = new CScriptAny(engine);
			//		DeserialiseProp(stream, prop, i, name);
			//		namedProperties[name] = prop;
			//		prop->Release();
			//	}
			//	for (auto it = m_ScriptProperties.begin(), end = m_ScriptProperties.end(); it != end; ++it)
			//	{
			//		auto scriptprop = static_cast<ScriptAnyTSP*>((*it).get());
			//		auto _where = namedProperties.find(scriptprop->GetName());
			//		if (_where != namedProperties.end())
			//		{
			//			try
			//			{
			//				scriptprop->Set(_where->second);
			//			}
			//			catch (InvalidArgumentException&)
			//			{
			//				AddLogEntry("Failed to deserialise script properties - data type has changed for " +
			//					std::string(m_ScriptObject->object->GetObjectType()->GetName()) + "." +  scriptprop->GetName());
			//			}
			//		}
			//	}
			//}
			//else
			{
				unsigned int index = 0;
				for (auto it = m_ScriptProperties.begin(), end = m_ScriptProperties.end(); it != end; ++it)
				{
					//if (mode == All || stream.ReadBit())
					{
						auto scriptprop = static_cast<ScriptAnyTSP*>((*it).get());
						//auto prop = scriptprop->Get();
						auto prop = boost::intrusive_ptr<CScriptAny>(new CScriptAny(engine), false);
						DeserialiseProp(stream, prop.get(), index++);
						if (prop->GetTypeId() != -1)
						{
							try
							{
								scriptprop->Set(prop);
							}
							catch (InvalidArgumentException& e)
							{
								AddLogEntry("Failed to deserialise script properties - data type has changed for " +
									std::string(m_ScriptObject->object->GetObjectType()->GetName()) + "." +  scriptprop->GetName());
								throw e;
							}
						}
					}
				}
			}
		}
		else
		{
			m_CachedProperties.resize(numProperties);

			std::string name;
			for (size_t i = 0; i < numProperties; ++i)
			{
				//if (mode == Editable)
				//{
				//	size_t length;
				//	stream.Read(length);
				//	name.resize(length);
				//	stream.Read(&name[0], length);
				//}

				//if (mode == All || stream.ReadBit())
				{
					auto prop = new CScriptAny(engine);

					DeserialiseProp(stream, prop, i, name);

					m_CachedProperties[i] = prop;
					//if (mode == Editable)
					//	m_EditableCachedProperties[name] = prop;

					prop->Release();
				}
			}
		}
	}

	void ASScript::OnPostDeserialisation()
	{
	}

	const std::string &ASScript::GetScriptPath() const
	{
		return m_Path;
	}

	void ASScript::SetScriptPath(const std::string& path)
	{
		if (path != m_Path)
			m_ReloadScript = true;
		m_Path = path;

		m_DeltaSerialisationHelper.markChanged(PropsIdx::ScriptPath);
	}

}
