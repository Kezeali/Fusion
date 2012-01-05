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

#ifndef H_FusionAngelScriptSystem
#define H_FusionAngelScriptSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentSystem.h"
#include "FusionEntityComponent.h"

#include <angelscript.h>

#define FSN_PARALLEL_SCRIPT_EXECUTION

namespace FusionEngine
{

	class InputEvent;

	class ASScript;

	class AngelScriptWorld;
	class AngelScriptTask;
	class AngelScriptTaskB;

	class AngelScriptSystem : public IComponentSystem
	{
	public:
		AngelScriptSystem(const std::shared_ptr<ScriptManager>& manager);
		virtual ~AngelScriptSystem()
		{}

		std::shared_ptr<ISystemWorld> CreateWorld();

	private:
		SystemType GetType() const { return SystemType::Simulation; }

		std::string GetName() const { return "AngelScriptSystem"; }

		std::shared_ptr<ScriptManager> m_ScriptManager;
	};

	class AngelScriptWorld : public ISystemWorld, public std::enable_shared_from_this<AngelScriptWorld>
	{
		friend class AngelScriptTask;
		friend class AngelScriptTaskB;
	public:
		AngelScriptWorld(IComponentSystem* system, const std::shared_ptr<ScriptManager>& manager);
		~AngelScriptWorld();

		asIScriptEngine* GetScriptEngine() const { return m_Engine; }

		void BuildScripts(bool rebuild_all = false);

	private:
		std::vector<std::string> GetTypes() const;

		ComponentPtr InstantiateComponent(const std::string& type);
		ComponentPtr InstantiateComponent(const std::string& type, const Vector2& pos, float angle, RakNet::BitStream* continious_data, RakNet::BitStream* occasional_data);

		void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta);

		void Prepare(const ComponentPtr& component);
		void CancelPreparation(const ComponentPtr& component);
		void OnActivation(const ComponentPtr& component);
		void OnDeactivation(const ComponentPtr& component);

		ISystemTask* GetTask();
		std::vector<ISystemTask*> GetTasks();

		std::vector<boost::intrusive_ptr<ASScript>> m_NewlyActiveScripts;
		std::vector<boost::intrusive_ptr<ASScript>> m_ActiveScripts;

		bool m_Updating;

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
		};

	private:

		std::map<std::string, ComponentScriptInfo> m_ScriptInfo;

		std::shared_ptr<ScriptManager> m_ScriptManager;
		asIScriptEngine* m_Engine;
		AngelScriptTask* m_ASTask;
		AngelScriptTaskB* m_ASTaskB;
	};

	class AngelScriptTask : public ISystemTask
	{
	public:
		AngelScriptTask(AngelScriptWorld* sysworld, std::shared_ptr<ScriptManager> script_manager);
		~AngelScriptTask();

		void Update(const float delta);

		SystemType GetTaskType() const { return SystemType::Simulation; }

		PerformanceHint GetPerformanceHint() const { return LongParallel; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		AngelScriptWorld* m_AngelScriptWorld;
		std::shared_ptr<ScriptManager> m_ScriptManager;

		std::map<int, std::vector<InputEvent>> m_PlayerInputEvents;
	};

	// Remember the friend decls in AngelScriptWorld and AngelScriptComponent!
	class AngelScriptTaskB : public ISystemTask
	{
	public:
		AngelScriptTaskB(AngelScriptWorld* sysworld, std::shared_ptr<ScriptManager> script_manager);
		~AngelScriptTaskB();

		void Update(const float delta);

		SystemType GetTaskType() const { return SystemType::Rendering; }

		PerformanceHint GetPerformanceHint() const { return Short; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		AngelScriptWorld* m_AngelScriptWorld;
		std::shared_ptr<ScriptManager> m_ScriptManager;
	};

}

#endif
