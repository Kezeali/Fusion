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

#ifndef H_FusionAngelScriptSystem
#define H_FusionAngelScriptSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentSystem.h"
#include "FusionSystemWorld.h"
#include "FusionSystemTask.h"
#include "FusionEntityComponent.h"

#include "FusionResource.h"

#include <array>
#include <memory>
#include <set>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <angelscript.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

#include <tbb/spin_mutex.h>

#define FSN_PARALLEL_SCRIPT_EXECUTION

namespace FusionEngine
{

	class InputEvent;

	class Box2DWorld;

	class ASScript;

	class AngelScriptWorld;
	class AngelScriptTask;
	class AngelScriptInstantiationTask;

	class AngelScriptSystem : public System::ISystem
	{
	public:
		AngelScriptSystem(const std::shared_ptr<ScriptManager>& manager);
		virtual ~AngelScriptSystem()
		{}

		std::shared_ptr<System::WorldBase> CreateWorld();

	private:
		System::SystemType GetType() const { return System::Simulation; }

		std::string GetName() const { return "AngelScriptSystem"; }

		void RegisterScriptInterface(asIScriptEngine* engine);

		std::shared_ptr<ScriptManager> m_ScriptManager;
	};

	class AngelScriptWorld : public System::WorldBase
	{
		friend class AngelScriptTask;
		friend class AngelScriptInstantiationTask;
	public:
		AngelScriptWorld(System::ISystem* system, const std::shared_ptr<ScriptManager>& manager);
		~AngelScriptWorld();

		void CreateScriptMethodMap();

		asIScriptEngine* GetScriptEngine() const { return m_Engine; }

		void BuildScripts(bool rebuild_all = false);

		bool ScriptHasChanged(const std::string& path);

		//! Preprocesses the given script and generates code for an EntityWrapper type to be used in it
		std::string GenerateBaseCodeForScript(std::string& filename);

	private:
		std::vector<std::string> GetTypes() const;

		ComponentPtr InstantiateComponent(const std::string& type);
		ComponentPtr InstantiateComponent(const std::string& type, const Vector2& pos, float angle, RakNet::BitStream* continious_data, RakNet::BitStream* occasional_data);

		void Prepare(const ComponentPtr& component);
		void CancelPreparation(const ComponentPtr& component);
		void OnActivation(const ComponentPtr& component);
		void OnDeactivation(const ComponentPtr& component);

		void OnWorldAdded(const std::string& other_world);
		void OnWorldRemoved(const std::string& other_world);

		void ProcessMessage(Messaging::Message message) override;

		System::TaskList_t MakeTasksList() const override;

		struct DependencyNode
		{
			std::string Name;
			std::string Filename;
			std::set<std::shared_ptr<DependencyNode>> IncludedBy;
			std::set<std::shared_ptr<DependencyNode>> UsedBy;
		};

		bool updateChecksum(const std::string& filename, const std::string& filedata);
		std::shared_ptr<DependencyNode> getDependencyNode(const std::string &name);

		std::vector<boost::intrusive_ptr<ASScript>> m_NewlyActiveScripts;
		std::vector<boost::intrusive_ptr<ASScript>> m_ActiveScripts;

		bool m_Updating;

		std::map<std::string, std::uint32_t> m_ScriptChecksums;
		std::map<std::string, std::shared_ptr<DependencyNode>> m_DependencyData;

		enum EventHandlerMethodTypeIds : size_t
		{
			PlayerAdded = 0u,
			PlayerRemoved,
			SensorEnter,
			SensorExit,
			CollisionEnter,
			CollisionExit,
			Input,
			Update,
			NumHandlerTypes
		};

	public:
		struct ComponentScriptInfo
		{
			ComponentScriptInfo();
			ComponentScriptInfo(const ComponentScriptInfo &other);
			ComponentScriptInfo(ComponentScriptInfo &&other);

			ComponentScriptInfo& operator= (const ComponentScriptInfo &other);
			ComponentScriptInfo& operator= (ComponentScriptInfo &&other);

			std::string ClassName;

			std::string Module;

			std::vector<std::pair<std::string, std::string>> Properties;

			typedef std::pair<std::string, std::string> UsedComponent_t;
			std::set<UsedComponent_t> UsedComponents;

			std::set<std::string> IncludedScripts;

			bool AutoYield;
		};

		typedef boost::multi_index_container<
			ComponentScriptInfo,
			boost::multi_index::indexed_by<
			boost::multi_index::hashed_unique<boost::multi_index::member<ComponentScriptInfo, std::string, &ComponentScriptInfo::ClassName>>,
			boost::multi_index::hashed_unique<boost::multi_index::member<ComponentScriptInfo, std::string, &ComponentScriptInfo::Module>>
			>> ScriptInfoMap_t;

		typedef std::map<std::string, ComponentScriptInfo> ScriptInfoClassMap_t;

	private:
		ScriptInfoClassMap_t m_ScriptInfo;
		ScriptInfoMap_t m_ScriptInfof;

		std::shared_ptr<ScriptManager> m_ScriptManager;
		asIScriptEngine* m_Engine;
		AngelScriptTask* m_ASTask;
		AngelScriptInstantiationTask* m_InstantiateTask;

		std::unordered_map<std::string, std::function<void (ASScript*, int)>> m_ScriptMethodExecutors;
		std::array<std::string, EventHandlerMethodTypeIds::NumHandlerTypes> m_EventHandlerMethodDeclarations;

		boost::signals2::connection m_CheckForChangesConnection;

		void insertScriptToBuild(std::map<std::string, std::pair<std::string, AngelScriptWorld::ComponentScriptInfo>>& scriptsToBuild, const std::string& filename, std::string& script, bool check_dependencies);

		//! Instantiates the object that implements the script-type 'ScriptComponent' in the module loaded by the given component
		/*
		* Yes, this method is badly named, but it's concise
		*/
		bool instantiateScript(const boost::intrusive_ptr<ASScript>& script);
	};

	class AngelScriptTask : public System::SystemTaskBase
	{
	public:
		AngelScriptTask(AngelScriptWorld* sysworld, std::shared_ptr<ScriptManager> script_manager);
		~AngelScriptTask();

		void Update() override;

		System::SystemType GetTaskType() const { return System::Simulation; }

		PerformanceHint GetPerformanceHint() const { return LongParallel; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

		std::vector<std::string> GetDependencies() const override
		{
			std::vector<std::string> deps;
			deps.push_back("CLRenderTask");
			return deps;
		}

	protected:
		AngelScriptWorld* m_AngelScriptWorld;
		std::shared_ptr<ScriptManager> m_ScriptManager;

		std::map<int, std::vector<InputEvent>> m_PlayerInputEvents;

		tbb::spin_mutex m_PlayerAddedMutex;

		std::vector<std::pair<unsigned int, PlayerID>> m_PlayerAddedEvents;
		std::vector<std::pair<unsigned int, PlayerID>> m_PlayerRemovedEvents;

		boost::signals2::connection m_PlayerInputConnection;
		boost::signals2::connection m_PlayerAddedConnection;
		boost::signals2::connection m_PlayerRemovedConnection;

		boost::intrusive_ptr<asIScriptContext> ExecuteContext(const boost::intrusive_ptr<ASScript>& script, const boost::intrusive_ptr<asIScriptContext>& ctx);
	};

	// Remember the friend decls in AngelScriptWorld and AngelScriptComponent!
	class AngelScriptInstantiationTask : public System::SystemTaskBase
	{
	public:
		AngelScriptInstantiationTask(AngelScriptWorld* sysworld, std::shared_ptr<ScriptManager> script_manager);
		~AngelScriptInstantiationTask();

		void Update() override;

		System::SystemType GetTaskType() const { return System::Rendering; }

		PerformanceHint GetPerformanceHint() const { return Short; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		AngelScriptWorld* m_AngelScriptWorld;
		std::shared_ptr<ScriptManager> m_ScriptManager;
	};

	std::string GenerateBaseCode(const AngelScriptWorld::ComponentScriptInfo& scriptInfo, const AngelScriptWorld::ScriptInfoClassMap_t& scriptComponents);

}

#endif
