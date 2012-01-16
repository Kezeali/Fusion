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

#ifndef H_FusionEngineManager
#define H_FusionEngineManager

#include "FusionPrerequisites.h"

#include "FusionTypes.h"
#include "FusionVectorTypes.h"

#include "FusionWorldSaver.h"

#include <ClanLib/core.h> // For CL_String (bleh)
#include <ClanLib/display.h>
#include <ClanLib/sound.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace FusionEngine
{

	class IComponentSystem;
	class ComponentUniverse;
	class ConsoleStdOutWriter;
	class EngineExtension;
	class EntitySynchroniser;
	class GameMap;
	class GUI;
	class InputManager;
	class PacketDispatcher;
	class PlayerManager;
	class PlayerRegistry;
	class Profiling;
	class RegionMapLoader;
	class SoundOutput;

	class TaskManager;
	class TaskScheduler;

	void BootUp();

	class EngineManager : public WorldSaver
	{
	public:
		EngineManager(const std::vector<CL_String>& args);
		~EngineManager();

		void Initialise();

		const CL_DisplayWindow& GetDisplayWindow() const;
		const CL_GraphicContext& GetGC() const;
		const CL_SoundOutput& GetSoundOutput() const;
		const std::shared_ptr<ScriptManager>& GetScriptManager() const;
		CameraSynchroniser* GetCameraSynchroniser() const;

		void AddExtension(const std::shared_ptr<EngineExtension>& extension);

		void AddSystem(std::unique_ptr<IComponentSystem>&& system);

		void Run();

		void Save(const std::string& name, bool quick = false);

		void Load(const std::string& name);

	private:
		std::unique_ptr<Logger> m_Logger;
		std::unique_ptr<Console> m_Console;
		std::shared_ptr<Profiling> m_Profiling;
		std::shared_ptr<ConsoleStdOutWriter> m_CoutWriter;

		LogPtr m_Log;

		bool m_EditMode;

		Vector2i m_DisplayDimensions;
		bool m_Fullscreen;

		bool m_Save;
		bool m_Load;

		std::shared_ptr<ScriptManager> m_ScriptManager;

		std::shared_ptr<RegionMapLoader> m_CellArchivist;
		std::shared_ptr<StreamingManager> m_StreamingManager;
		std::shared_ptr<CameraSynchroniser> m_CameraSynchroniser;

		CL_DisplayWindow m_DisplayWindow;
		CL_SoundOutput m_SoundOutput;

		std::shared_ptr<InputManager> m_InputManager;

		std::shared_ptr<ResourceManager> m_ResourceManager;

		std::shared_ptr<GUI> m_GUI;

		std::shared_ptr<SoundOutput> m_ScriptSoundOutput;

		std::shared_ptr<PlayerRegistry> m_PlayerRegistry;

		std::shared_ptr<RakNetwork> m_Network;
		std::shared_ptr<PacketDispatcher> m_PacketDispatcher;
		std::shared_ptr<NetworkManager> m_NetworkManager;

		std::shared_ptr<PlayerManager> m_PlayerManager;

		std::shared_ptr<ComponentUniverse> m_ComponentUniverse;
		std::shared_ptr<EntitySynchroniser> m_EntitySynchroniser;
		std::shared_ptr<EntityManager> m_EntityManager;
		std::shared_ptr<P2PEntityInstantiator> m_EntityInstantiator;

		std::shared_ptr<GameMapLoader> m_MapLoader;
		std::shared_ptr<GameMap> m_Map;

		std::vector<std::shared_ptr<EngineExtension>> m_Extensions;
		std::set<std::string> m_EnabledExtensions; // Loaded from options
		std::vector<std::shared_ptr<EngineExtension>> m_ActiveExtensions;

		std::map<std::string, std::unique_ptr<IComponentSystem>> m_Systems;

		std::shared_ptr<TaskManager> m_TaskManager;
		std::shared_ptr<TaskScheduler> m_Scheduler;

		void ReadOptions(ClientOptions* options);

		void RegisterScriptTypes();

		void AddResourceLoaders();

		void ProcessExtensionMessages(const std::shared_ptr<EngineExtension>& ex);

	};

}

#endif
