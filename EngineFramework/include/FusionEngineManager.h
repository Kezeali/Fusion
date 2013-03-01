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

#include <Raknet/WindowsIncludes.h>

#include "FusionPrerequisites.h"

#include "FusionTypes.h"
#include "FusionVectorTypes.h"

#include "FusionSingleton.h"
#include "FusionWorldSaver.h"

#include <ClanLib/display.h>
#include <ClanLib/sound.h>

#include <tbb/concurrent_queue.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace FusionEngine
{

	class ArchetypeFactory;
	class ArchetypeFactoryManager;
	class IComponentSystem;
	class ComponentUniverse;
	class ConsoleStdOutWriter;
	class EngineExtension;
	class EntitySynchroniser;
	class EvesdroppingManager;
	class GameMap;
	class GUI;
	class InputManager;
	class PacketDispatcher;
	class PlayerManager;
	class PlayerRegistry;
	class Profiling;
	class RegionCellArchivist;
	class SoundOutput;

	class TaskManager;
	class TaskScheduler;

	void BootUp();

	//! EngineManager
	class EngineManager : public WorldSaver, public Singleton<EngineManager>
	{
	public:
		//! CTOR
		EngineManager(const std::vector<std::string>& args);
		//! DTOR
		~EngineManager();

		void Initialise();

		const clan::DisplayWindow& GetDisplayWindow() const;
		const clan::Canvas& GetCanvas() const;
		const clan::GraphicContext& GetGC() const;
		const clan::SoundOutput& GetSoundOutput() const;
		const std::shared_ptr<ScriptManager>& GetScriptManager() const;
		CameraSynchroniser* GetCameraSynchroniser() const;

		void AddExtension(const std::shared_ptr<EngineExtension>& extension);

		void AddSystem(std::unique_ptr<IComponentSystem>&& system);

		void Run();

		//! Saves immediately
		void Save(const std::string& name, bool quick = false);
		//! Load immediately
		void Load(const std::string& name);

		void EnqueueSave(const std::string& name, bool quick = false);
		void EnqueueLoad(const std::string& name);

		unsigned int RequestPlayer();

	private:
		std::unique_ptr<Logger> m_Logger;
		std::unique_ptr<Console> m_Console;
		std::shared_ptr<Profiling> m_Profiling;
		std::shared_ptr<ConsoleStdOutWriter> m_CoutWriter;

		std::unique_ptr<EvesdroppingManager> m_EvesdroppingManager;

		LogPtr m_Log;

		bool m_EditMode;

		Vector2i m_DisplayDimensions;
		bool m_Fullscreen;

		tbb::concurrent_queue<std::pair<std::string, bool>> m_SaveQueue;
		tbb::spin_mutex m_LoadQueueMutex;
		std::string m_SaveToLoad;

		bool m_SaveProfilerData;

		std::shared_ptr<ScriptManager> m_ScriptManager;

		std::shared_ptr<ArchetypeFactoryManager> m_ArchetypeFactoryManager;

		std::shared_ptr<RegionCellArchivist> m_CellArchivist;
		std::shared_ptr<StreamingManager> m_StreamingManager;
		std::shared_ptr<CameraSynchroniser> m_CameraSynchroniser;

		clan::DisplayWindow m_DisplayWindow;

		clan::Canvas m_Canvas;

		clan::SoundOutput m_SoundOutput;

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
		std::shared_ptr<ArchetypeFactory> m_ArchetypeFactory;

		std::shared_ptr<GameMapLoader> m_MapLoader;
		std::shared_ptr<GameMap> m_Map;

		std::vector<std::shared_ptr<EngineExtension>> m_Extensions;
		std::set<std::string> m_EnabledExtensions; // Loaded from options
		std::vector<std::shared_ptr<EngineExtension>> m_ActiveExtensions;

		std::map<std::string, std::unique_ptr<IComponentSystem>> m_Systems;

		std::shared_ptr<TaskManager> m_TaskManager;
		std::shared_ptr<TaskScheduler> m_Scheduler;

		clan::Slot m_GotFocusSlot;
		clan::Slot m_GotMouseSlot;

		void ReadOptions(const ClientOptions& options);

		void RegisterScriptTypes();

		void AddResourceLoaders();

		void ProcessExtensionMessages(const std::shared_ptr<EngineExtension>& ex);

	};

}

#endif
