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

#include "FusionAngelScriptSystem.h"

#include "FusionAngelScriptComponent.h"
#include "FusionEntity.h"

#include <tbb/parallel_for.h>
#include <tbb/spin_mutex.h>

namespace FusionEngine
{

	AngelScriptSystem::AngelScriptSystem(const std::shared_ptr<ScriptManager>& manager)
		: m_ScriptManager(manager)
	{
	}

	ISystemWorld* AngelScriptSystem::CreateWorld()
	{
		return new AngelScriptWorld(this, m_ScriptManager);
	}

	AngelScriptWorld::AngelScriptWorld(IComponentSystem* system, const std::shared_ptr<ScriptManager>& manager)
		: ISystemWorld(system),
		m_ScriptManager(manager)
	{
		m_Engine = m_ScriptManager->GetEnginePtr();

		m_ASTask = new AngelScriptTask(this, m_ScriptManager);
	}

	AngelScriptWorld::~AngelScriptWorld()
	{
		delete m_ASTask;
	}

	std::vector<std::string> AngelScriptWorld::GetTypes() const
	{
		static const std::string types[] = { "ASScript" };
		return std::vector<std::string>(types, types + sizeof(types));
	}

	std::shared_ptr<IComponent> AngelScriptWorld::InstantiateComponent(const std::string& type)
	{
		return InstantiateComponent(type, Vector2::zero(), 0.f, nullptr, nullptr);
	}

	std::shared_ptr<IComponent> AngelScriptWorld::InstantiateComponent(const std::string& type, const Vector2&, float, RakNet::BitStream* continious_data, RakNet::BitStream* occasional_data)
	{
		if (type == "ASScript")
		{
			auto com = std::make_shared<ASScript>();
			return com;
		}
		return std::shared_ptr<IComponent>();
	}

	void AngelScriptWorld::OnActivation(const std::shared_ptr<IComponent>& component)
	{
		auto scriptComponent = std::dynamic_pointer_cast<ASScript>(component);
		if (scriptComponent)
		{
			m_ActiveScripts.push_back(scriptComponent);
		}
	}

	void AngelScriptWorld::OnDeactivation(const std::shared_ptr<IComponent>& component)
	{
		auto scriptComponent = std::dynamic_pointer_cast<ASScript>(component);
		if (scriptComponent)
		{
			// Find and remove the deactivated body (from the Active Bodies list)
			auto _where = std::find(m_ActiveScripts.begin(), m_ActiveScripts.end(), scriptComponent);
			if (_where != m_ActiveScripts.end())
			{
				_where->swap(m_ActiveScripts.back());
				m_ActiveScripts.pop_back();
			}
		}
	}

	ISystemTask* AngelScriptWorld::GetTask()
	{
		return m_ASTask;
	}

	void AngelScriptWorld::MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta)
	{
		if (type == "ASScript")
		{
			ASScript::DeltaSerialiser_t::copyChanges(result, current_data, delta);
		}
	}

	AngelScriptTask::AngelScriptTask(AngelScriptWorld* sysworld, std::shared_ptr<ScriptManager> script_manager)
		: ISystemTask(sysworld),
		m_AngelScriptWorld(sysworld),
		m_ScriptManager(script_manager)
	{
	}

	AngelScriptTask::~AngelScriptTask()
	{
	}

	void AngelScriptTask::Update(const float delta)
	{
		auto& scripts = m_AngelScriptWorld->m_ActiveScripts;

		std::map<ModulePtr, EntityPtr> modulesToBuild;

		tbb::spin_mutex mutex;

		auto execute_scripts = [&](const tbb::blocked_range<size_t>& r)
		{
			for (size_t i = r.begin(), end = r.end(); i != end; ++i)
			{
				auto& script = scripts[i];

				if (script->m_ModuleBuilt)
				{
					if (script->m_Module->IsBuilt())
					{
						tbb::spin_mutex::scoped_lock lock(mutex);
						auto objectType = script->m_Module->GetASModule()->GetObjectTypeByIndex(0);
						script->m_ScriptObject = script->m_Module->CreateObject(objectType->GetTypeId());
					}

					script->m_ModuleBuilt = false;
				}

				if (script->m_ReloadScript)
				{
					script->m_ScriptObject.Release();

					const auto& moduleName = script->GetParent()->GetName();

					tbb::spin_mutex::scoped_lock lock(mutex);
					auto module = m_ScriptManager->GetModule(moduleName.c_str(), asGM_ALWAYS_CREATE);
					script->m_Module = module;
					if (m_ScriptManager->AddFile(script->m_Path, moduleName.c_str()))
					{
						//modulesToBuild.insert(module);
						modulesToBuild.insert( std::make_pair(module, script->GetParent()->shared_from_this()) );
					}

					script->m_ModuleBuilt = true;

					script->m_ReloadScript = false;
				}

				if (script->m_ScriptObject.IsValid())
				{
					ScriptUtils::Calling::Caller caller;
					auto _where = script->m_ScriptMethods.find("void update(float)");
					if (_where != script->m_ScriptMethods.end())
					{
						caller = ScriptUtils::Calling::Caller::CallerForMethodFuncId(script->m_ScriptObject.GetScriptObject(), _where->second);
						m_ScriptManager->ConnectToCaller(caller);
					}
					else
					{
						caller = script->m_ScriptObject.GetCaller("void update(float)");
						script->m_ScriptMethods["void update(float)"] = caller.get_funcid();
					}
					if (caller)
						caller(delta);
				}
			}
		};

#ifdef FSN_PARALLEL_SCRIPT_EXECUTION
		tbb::parallel_for(tbb::blocked_range<size_t>(0, scripts.size()), execute_scripts);
#else
		execute_scripts(tbb::blocked_range<size_t>(0, scripts.size()));
#endif

		m_ScriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);

		for (auto it = modulesToBuild.begin(), end = modulesToBuild.end(); it != end; ++it)
		{
			//auto& module = (*it);
			auto& module = it->first;
			auto& entity = it->second;

			std::map<std::string, std::shared_ptr<IComponent>> convenientComponents;

			std::string componentConvenienceScript;
			//auto& componentNames = entity->GetComponentNames();
			auto& interfaces = entity->GetInterfaces();
			for (auto it = interfaces.cbegin(), end = interfaces.cend(); it != end; ++it)
			{
				const auto& interfaceName = it->first;
				for (auto cit = it->second.begin(), cend = it->second.end(); cit != cend; ++cit)
				{
					const auto convenientIdentifier = fe_newlower(cit->first);
					componentConvenienceScript += interfaceName + "@ " + convenientIdentifier + ";\n";

					convenientComponents[convenientIdentifier] = cit->second;
				}
			}

			module->AddCode(entity->GetName(), componentConvenienceScript);

			if (module->Build() < 0)
				continue; // TODO: Report error

			for (auto it = convenientComponents.begin(), end = convenientComponents.end(); it != end; ++it)
			{
				int index = module->GetASModule()->GetGlobalVarIndexByName(it->first.c_str());
				if (index >= 0)
				{
					IComponent** component = static_cast<IComponent**>(module->GetASModule()->GetAddressOfGlobalVar(index));
					it->second->addRef();
					*component = (it->second.get());
				}
			}
		}
	}

}