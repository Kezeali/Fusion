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

	class AngelScriptSystem : public IComponentSystem
	{
	public:
		AngelScriptSystem(const std::shared_ptr<ScriptManager>& manager);
		virtual ~AngelScriptSystem()
		{}

		ISystemWorld* CreateWorld();

	private:
		SystemType GetType() const { return SystemType::Simulation; }

		std::string GetName() const { return "AngelScriptSystem"; }

		std::shared_ptr<ScriptManager> m_ScriptManager;
	};

	class AngelScriptWorld : public ISystemWorld
	{
		friend class AngelScriptTask;
	public:
		AngelScriptWorld(IComponentSystem* system, const std::shared_ptr<ScriptManager>& manager);
		~AngelScriptWorld();

		asIScriptEngine* GetScriptEngine() const { return m_Engine; }

		void BuildScripts(bool rebuild_all = false);

	private:
		std::vector<std::string> GetTypes() const;

		std::shared_ptr<IComponent> InstantiateComponent(const std::string& type);
		std::shared_ptr<IComponent> InstantiateComponent(const std::string& type, const Vector2& pos, float angle, RakNet::BitStream* continious_data, RakNet::BitStream* occasional_data);

		void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta);

		void OnPrepare(const std::shared_ptr<IComponent>& component);
		void OnActivation(const std::shared_ptr<IComponent>& component);
		void OnDeactivation(const std::shared_ptr<IComponent>& component);

		ISystemTask* GetTask();

		std::vector<std::shared_ptr<ASScript>> m_NewlyActiveScripts;
		std::vector<std::shared_ptr<ASScript>> m_ActiveScripts;

	public:
		struct ComponentScriptInfo
		{
			ComponentScriptInfo();
			ComponentScriptInfo(const ComponentScriptInfo &other);
			ComponentScriptInfo(ComponentScriptInfo &&other);

			ComponentScriptInfo& operator= (const ComponentScriptInfo &other);
			ComponentScriptInfo& operator= (ComponentScriptInfo &&other);

			std::string ClassName;

			std::vector<std::pair<std::string, std::string>> Properties;

			typedef std::pair<std::string, std::string> UsedComponent_t;
			std::set<UsedComponent_t> UsedComponents;
		};

	private:

		std::map<std::string, ComponentScriptInfo> m_ScriptInfo;

		std::shared_ptr<ScriptManager> m_ScriptManager;
		asIScriptEngine* m_Engine;
		AngelScriptTask* m_ASTask;
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

		std::map<int, std::deque<InputEvent>> m_PlayerInputEvents;
	};

}

#endif
