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

#include "PrecompiledHeaders.h"

#include "FusionEngineManager.h"

#include "FusionArchetypeFactory.h"
#include "FusionCameraSynchroniser.h"
#include "FusionClientOptions.h"
#include "FusionComponentScriptTypeRegistration.h"
#include "FusionComponentSystem.h"
#include "FusionComponentUniverse.h"
#include "FusionContextMenu.h"
#include "FusionConsole.h"
#include "FusionConsoleStdOutWriter.h"
#include "FusionEngineExtension.h"
#include "FusionEntityManager.h"
#include "FusionEntitySynchroniser.h"
#include "FusionGUI.h"
#include "FusionLog.h"
#include "FusionLogger.h"
#include "FusionNetworkManager.h"
#include "FusionP2PEntityInstantiator.h"
#include "FusionPacketDispatcher.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionPlayerManager.h"
#include "FusionPlayerRegistry.h"
#include "FusionProfiling.h"
#include "FusionRakNetwork.h"
#include "FusionRegionMapLoader.h"
#include "FusionResourceManager.h"
#include "FusionScriptedConsoleCommand.h"
#include "FusionScriptInputEvent.h"
#include "FusionScriptManager.h"
#include "FusionScriptSound.h"
#include "FusionStreamingManager.h"
#include "FusionSpriteDefinition.h"
#include "FusionTaskManager.h"
#include "FusionTaskScheduler.h"

// Resource Loaders
#include "FusionAudioLoader.h"
#include "FusionImageLoader.h"
#include "FusionPolygonLoader.h"

#include <angelscript.h>
#include <boost/lexical_cast.hpp>
#include <ClanLib/display.h>

namespace FusionEngine
{

	EngineManager::EngineManager(const std::vector<CL_String>& args)
		: m_EditMode(false),
		m_DisplayDimensions(800, 600),
		m_Fullscreen(false)
	{
		// Configure PhysFS
		SetupPhysFS::configure("lastflare", "Fusion", "7z");
		if (!SetupPhysFS::mount(s_PackagesPath, "", "7z", false))
			SendToConsole("Default resource path could not be located");
		SetupPhysFS::mount_archives(s_PackagesPath, "", "zip", false);

		// Clear cache
		//SetupPhysFS::clear_temp();

		// Init Logger
		m_Logger.reset(new Logger);

		// Init profiling
		m_Profiling.reset(new Profiling);
		
		// Init Console
		m_Console.reset(new Console);
	}

	EngineManager::~EngineManager()
	{
		m_Console->UnbindCommand("cam_range");
	}

	void EngineManager::Initialise()
	{
		try
		{
			ClientOptions* options = new ClientOptions("settings.xml", "settings");

			ReadOptions(options);
			m_EditMode = options->GetOption_bool("edit");
			options->GetOption("screen_width", &m_DisplayDimensions.x);
			options->GetOption("screen_height", &m_DisplayDimensions.y);
			m_Fullscreen = options->GetOption_bool("fullscreen");

			// Enable native console
			//if (m_NativeConsole)
			{
				//m_ConsoleWindow = CL_ConsoleWindow("Console", 80, 10);
				m_CoutWriter.reset(new ConsoleStdOutWriter());
				m_CoutWriter->Enable();
			}

			// Get the general log file
			m_Log = Logger::getSingleton().OpenLog(g_LogGeneral);

			// Init the display window
			m_DisplayWindow = CL_DisplayWindow("Fusion", m_DisplayDimensions.x, m_DisplayDimensions.y, m_Fullscreen, !m_Fullscreen);

			// Init sound
			m_SoundOutput = CL_SoundOutput(44100);

			// Init ScriptManager
			m_ScriptManager = std::make_shared<ScriptManager>();
			
			RegisterScriptTypes();

			// Register the Console singleton in the script env
			m_ScriptManager->RegisterGlobalObject("Console console", Console::getSingletonPtr());

			// Init InputManager
			m_InputManager.reset(new InputManager(m_DisplayWindow));

			m_InputManager->Test();
			m_InputManager->Initialise();

			// Init resource manager
			m_ResourceManager.reset(new ResourceManager(m_DisplayWindow.get_gc()));

			// Init GUI
			m_GUI.reset(new GUI(m_DisplayWindow));
			m_GUI->Initialise();

			m_ScriptSoundOutput = std::make_shared<SoundOutput>(m_SoundOutput);

			m_PlayerRegistry.reset(new PlayerRegistry);

			// Init network
			m_Network.reset(new RakNetwork());
			m_PacketDispatcher.reset(new PacketDispatcher());
			m_NetworkManager.reset(new NetworkManager(m_Network.get(), m_PacketDispatcher.get()));

			m_PlayerManager.reset(new PlayerManager());

			// Init Streaming
			m_CellArchivist.reset(new RegionMapLoader(m_EditMode));
			m_StreamingManager = std::make_shared<StreamingManager>(m_CellArchivist.get());

			m_ScriptManager->RegisterGlobalObject("StreamingManager streaming", m_StreamingManager.get());

			// Init Camera
			m_CameraSynchroniser = std::make_shared<CameraSynchroniser>(m_StreamingManager.get());

			// Init component / entity management
			m_EvesdroppingManager.reset(new EvesdroppingManager());
			m_ComponentUniverse.reset(new ComponentUniverse());
			m_EntitySynchroniser.reset(new EntitySynchroniser(m_InputManager.get(), m_CameraSynchroniser.get(), m_StreamingManager.get()));

			m_EntityManager.reset(new EntityManager(m_InputManager.get(), m_EntitySynchroniser.get(), m_StreamingManager.get(), m_ComponentUniverse.get(), m_CellArchivist.get()));
			m_EntityInstantiator.reset(new P2PEntityInstantiator(m_ComponentUniverse.get(), m_EntityManager.get()));

			m_ArchetypeFactory.reset(new ArchetypeFactory(m_EntityInstantiator.get()));

			m_MapLoader.reset(new GameMapLoader(options));

			m_CellArchivist->SetInstantiator(m_EntityInstantiator.get(), m_ComponentUniverse.get(), m_EntityManager.get(), m_ArchetypeFactory.get());

			m_TaskManager.reset(new TaskManager());
			m_Scheduler.reset(new TaskScheduler(m_TaskManager.get(), m_EntityManager.get(), m_CellArchivist.get()));


			m_ScriptManager->RegisterGlobalObject("Game game", this);

#ifdef PROFILE_BUILD
			m_Scheduler->SetFramerateLimiter(false);
			m_Scheduler->SetUnlimited(true);
#else
			m_Scheduler->SetFramerateLimiter(false);
			int maxFrameskip = 0;
			if (options->GetOption("max_frameskip", &maxFrameskip) && maxFrameskip >= 0)
				m_Scheduler->SetMaxFrameskip((unsigned int)maxFrameskip);
#endif
			auto streamingManager = m_StreamingManager.get(); // Note that this is intentionally not a smart-ptr, because the console command shouldn't keep this object alive!
			m_Console->BindCommand("cam_range", [streamingManager](const std::vector<std::string>& cmdargs)->std::string
			{
				if (cmdargs.size() >= 2)
				{
					auto range = boost::lexical_cast<float>(cmdargs[1]);
					streamingManager->SetRange(range);
				}
				return "Streaming range is " + boost::lexical_cast<std::string>(streamingManager->GetRange());
			});

			m_Console->BindCommand("exec", [](const std::vector<std::string>& cmdargs)->std::string
			{
				if (cmdargs.size() >= 2)
				{
					std::string userCode;
					{
						auto it = cmdargs.begin(), end = cmdargs.end();
						for (++it; it != end; ++it)
							userCode += *it;
					}
					const std::string code = "void _fsnConsoleExecuteString() {\n" + userCode +"\n;}";

					auto module = ScriptManager::getSingleton().GetModule("core_gui_console");
					asIScriptFunction* fn = 0;
					auto r = module->GetASModule()->CompileFunction("execute_string", code.c_str(), -1, 0, &fn);
					if (r < 0)
						return "Failed to compile: " + userCode;

					auto caller = ScriptUtils::Calling::Caller::CallerForGlobalFuncId(ScriptManager::getSingleton().GetEnginePtr(), fn->GetId());
					if (caller)
					{
						ScriptManager::getSingleton().ConnectToCaller(caller);
						caller();
					}

					fn->Release();
				}
				return "";
			});
		}
		catch (std::exception& ex)
		{
#ifdef _WIN32
			if (!m_DisplayWindow.is_null())
				MessageBoxA(m_DisplayWindow.get_hwnd(), (std::string("Failed to initialise engine: ") + ex.what()).c_str(), "Error", MB_OK);
#endif
		}
	}

	void EngineManager::EnqueueSave(const std::string& name, bool quick)
	{
		m_SaveQueue.push(std::make_pair(name, quick));
	}

	void EngineManager::EnqueueLoad(const std::string& name)
	{
		tbb::spin_mutex::scoped_lock lock(m_LoadQueueMutex);
		m_SaveToLoad = name;
	}

	void EngineManager::Save(const std::string& name, bool quick)
	{
		m_StreamingManager->StoreAllCells();
		if (quick)
			m_CellArchivist->Stop();
		try
		{
			if (auto file = m_CellArchivist->CreateDataFile("active_entities"))
				m_EntityManager->SaveActiveEntities(*file);
			if (auto file = m_CellArchivist->CreateDataFile("non_streaming_entities"))
				m_EntityManager->SaveNonStreamingEntities(*file);

			m_EntityManager->SaveCurrentReferenceData();

			if (auto file = m_CellArchivist->CreateDataFile("unused_ids"))
				m_EntityInstantiator->SaveState(*file);

			if (quick)
				m_CellArchivist->EnqueueQuickSave(name);
			else
				m_CellArchivist->Save(name);
		}
		catch (std::ios::failure& e)
		{
			AddLogEntry(e.what());
			throw e;
		}
		if(quick)
			m_CellArchivist->Start();
	}

	void EngineManager::Load(const std::string& name)
	{
		try
		{
			SendToConsole("Loading: Stopping archivist...");
			m_CellArchivist->Stop();
			m_ResourceManager->CancelAllDeliveries();
			SendToConsole("Loading: Deactivating entities...");
			m_EntityManager->DeactivateAllEntities(false);
			m_CameraSynchroniser->Clear();
			SendToConsole("Loading: Clearing entities...");
			//entityManager.reset(new EntityManager(inputMgr.get(), entitySynchroniser.get(), streamingMgr.get(), componentUniverse.get(), cellArchivist.get()));
			//instantiationSynchroniser.reset(new P2PEntityInstantiator(componentUniverse.get(), entityManager.get()));
			//cellArchivist->SetSynchroniser(instantiationSynchroniser.get());
			//propChangedQueue = entityManager->m_PropChangedQueue;
			m_EntitySynchroniser->Clear();
			m_EntityManager->Clear();
			m_EntityInstantiator->Reset();
			SendToConsole("Loading: Clearing cells...");
			m_StreamingManager->Reset();
			SendToConsole("Loading: Garbage-collecting...");
			int r = m_ScriptManager->GetEnginePtr()->GarbageCollect(asGC_FULL_CYCLE); FSN_ASSERT(r == 0);

			// Hack: Remove local players to re-generate join events
			//std::vector<PlayerInfo> localPlayers(PlayerRegistry::LocalPlayersBegin(), PlayerRegistry::LocalPlayersEnd());
			//for (unsigned int i = 0; i < PlayerRegistry::GetLocalPlayerCount(); ++i)
			//	PlayerRegistry::RemoveLocalPlayer(i);

			if (m_Map)
			{
				SendToConsole("Loading: Non-streaming entities (from map)...");
				m_Map->LoadNonStreamingEntities(false, m_EntityManager.get(), m_ComponentUniverse.get(), m_ArchetypeFactory.get(), m_EntityInstantiator.get());
			}

			m_CellArchivist->Load(name);

			SendToConsole("Loading: Loading unused IDs...");
			if (auto file = m_CellArchivist->LoadDataFile("unused_ids"))
				m_EntityInstantiator->LoadState(*file);

			SendToConsole("Loading: Non-streaming entities (from data-file)...");
			if (auto file = m_CellArchivist->LoadDataFile("non_streaming_entities"))
				m_EntityManager->LoadNonStreamingEntities(*file, m_ArchetypeFactory.get(), m_EntityInstantiator.get());

			m_CellArchivist->Start();

			SendToConsole("Loading: Activating entities...");
			if (auto file = m_CellArchivist->LoadDataFile("active_entities"))
				m_EntityManager->LoadActiveEntities(*file);
			// TODO: allow the entity manager to pause the simulation until all these entities are active

			// Hack, contd.: Re-add the local players
			//for (auto it = localPlayers.begin(); it != localPlayers.end(); ++it)
			//	PlayerRegistry::AddLocalPlayer(it->NetID, it->LocalIndex);
		}
		catch (std::exception& e)
		{
			AddLogEntry(e.what());
			throw e;
		}
	}

	void EngineManager::ReadOptions(ClientOptions* options)
	{
		auto loggingLevel = options->GetOption_str("logging_level");
		if (loggingLevel.empty() || loggingLevel == "normal")
			m_Logger->SetDefaultThreshold(LOG_NORMAL);
		else if (loggingLevel == "info")
			m_Logger->SetDefaultThreshold(LOG_INFO);
		else if (loggingLevel == "trivial")
			m_Logger->SetDefaultThreshold(LOG_TRIVIAL);
		else if (loggingLevel == "critical")
			m_Logger->SetDefaultThreshold(LOG_CRITICAL);

		if (options->GetOption_bool("console_logging"))
			m_Logger->ActivateConsoleLogging();

		auto activeExtensionsStr = options->GetOption_str("active_extensions");
		auto activeExtensions = fe_splitstring(activeExtensionsStr, ",");
		m_EnabledExtensions.insert(activeExtensions.begin(), activeExtensions.end());
	}

	unsigned int EngineManager::RequestPlayer()
	{
		return m_PlayerManager->RequestNewPlayer();
	}

	void EngineManager::RegisterScriptTypes()
	{
		auto engine = m_ScriptManager->GetEnginePtr();

		Console::Register(m_ScriptManager.get());
		RegisterScriptedConsoleCommand(engine);
		GUI::Register(m_ScriptManager.get());
		ContextMenu::Register(engine);

		engine->RegisterTypedef("PlayerID", "uint8");

		RegisterSingletonType<EngineManager>("Game", engine);
		engine->RegisterObjectMethod("Game", "uint requestPlayer()", asMETHODPR(EngineManager, RequestPlayer, (void), unsigned int), asCALL_THISCALL);
		//engine->RegisterObjectMethod("Game", "void requestPlayer(uint)", asMETHODPR(EngineManager, RequestPlayer, (unsigned int), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("Game", "void save(const string &in, bool quick = false)", asMETHOD(EngineManager, EnqueueSave), asCALL_THISCALL);
		engine->RegisterObjectMethod("Game", "void load(const string &in)", asMETHOD(EngineManager, EnqueueLoad), asCALL_THISCALL);

		RegisterValueType<CL_Rectf>("Rect", engine, asOBJ_APP_CLASS_CK);
		struct ScriptRect
		{
			static void CTOR1(float left, float top, float right, float bottom, void* ptr)
			{
				new (ptr) CL_Rectf(left, top, right, bottom);
			}
		};
		engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", asFUNCTION(ScriptRect::CTOR1), asCALL_CDECL_OBJLAST);

		EvesdroppingManager::RegisterScriptInterface(engine);

		// IProperty and Property<T>
		ComponentProperty::Register(engine);

		// Component types
		IComponent::RegisterType<IComponent>(engine, "IComponent");
		engine->RegisterObjectMethod("IComponent", "string getType()", asMETHOD(IComponent, GetType), asCALL_THISCALL);

		ITransform_RegisterScriptInterface(engine);
		IRigidBody_RegisterScriptInterface(engine);
		IFixture_RegisterScriptInterface(engine);
		ICircleShape_RegisterScriptInterface(engine);
		IRenderCom_RegisterScriptInterface(engine);
		ISprite_RegisterScriptInterface(engine);
		IScript_RegisterScriptInterface(engine);
		ICamera_RegisterScriptInterface(engine);

		Entity::Register(engine);
		P2PEntityInstantiator::Register(engine);
		
		Camera::Register(engine);
		StreamingManager::Register(engine);
	}

	void EngineManager::AddResourceLoaders()
	{
		m_ResourceManager->AddResourceLoader("IMAGE", &LoadImageResource, &UnloadImageResource, NULL);
		m_ResourceManager->AddResourceLoader(ResourceLoader("TEXTURE", &LoadTextureResource, &UnloadTextureResource, &LoadTextureResourceIntoGC));

		m_ResourceManager->AddResourceLoader(ResourceLoader("ANIMATION", &LoadAnimationResource, &UnloadAnimationResource));

		m_ResourceManager->AddResourceLoader("AUDIO", &LoadAudio, &UnloadAudio, NULL);
		m_ResourceManager->AddResourceLoader("AUDIO:STREAM", &LoadAudioStream, &UnloadAudio, NULL); // Note that this intentionally uses the same unload method

		m_ResourceManager->AddResourceLoader("SPRITE", &LoadSpriteResource, &UnloadSpriteResource, NULL);

		m_ResourceManager->AddResourceLoader(ResourceLoader("POLYGON", &LoadPolygonResource, &UnloadPolygonResource));
	}

	const CL_DisplayWindow& EngineManager::GetDisplayWindow() const
	{
		return m_DisplayWindow;
	}

	const CL_GraphicContext& EngineManager::GetGC() const
	{
		return m_DisplayWindow.get_gc();
	}

	const CL_SoundOutput& EngineManager::GetSoundOutput() const
	{
		return m_SoundOutput;
	}

	const std::shared_ptr<ScriptManager>& EngineManager::GetScriptManager() const
	{
		return m_ScriptManager;
	}

	CameraSynchroniser* EngineManager::GetCameraSynchroniser() const
	{
		return m_CameraSynchroniser.get();
	}

	void EngineManager::AddExtension(const std::shared_ptr<EngineExtension>& extension)
	{
		m_Extensions.push_back(extension);
	}

	void EngineManager::AddSystem(std::unique_ptr<IComponentSystem>&& system)
	{
		const std::string name = system->GetName();
		m_Systems.insert(std::make_pair(name, std::move(system)));
	}

	void EngineManager::Run()
	{
		try
		{
			// TODO: add resource loaders from systems
			AddResourceLoaders();

			for (auto it = m_Extensions.begin(), end = m_Extensions.end(); it != end; ++it)
				(*it)->RegisterScriptType(m_ScriptManager->GetEnginePtr());

			// Register system script interfaces
			for (auto it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
				it->second->RegisterScriptInterface(m_ScriptManager->GetEnginePtr());

			auto guiModule = m_ScriptManager->GetModule("core_gui_console");
			m_GUI->SetModule(guiModule);
			guiModule->Build();

			m_ActiveExtensions.clear();
			// Initialise extensions
			for (auto exit = m_Extensions.begin(), exend = m_Extensions.end(); exit != exend; ++exit)
			{
				(*exit)->SetDisplay(m_DisplayWindow);
				(*exit)->SetComponentFactory(m_ComponentUniverse);
				(*exit)->SetEntityInstantiator(m_EntityInstantiator);
				(*exit)->SetEntityManager(m_EntityManager);
				(*exit)->SetArchetypeFactory(m_ArchetypeFactory);
				(*exit)->SetMapLoader(m_CellArchivist);
				(*exit)->SetStreamingManager(m_StreamingManager);
				(*exit)->SetWorldSaver(this);
				(*exit)->SetDataArchiver(m_CellArchivist);

				if (m_EnabledExtensions.find((*exit)->GetName()) != m_EnabledExtensions.end())
				{
					m_ActiveExtensions.push_back(*exit);
				}
			}
			
			m_ResourceManager->StartLoaderThread();

			// Create worlds
			std::vector<std::shared_ptr<ISystemWorld>> worlds;
			for (auto it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
			{
				auto world = it->second->CreateWorld();
				// Add the world to the universe
				m_ComponentUniverse->AddWorld(world);
				// Notify the extensions
				for (auto exit = m_Extensions.begin(), exend = m_Extensions.end(); exit != exend; ++exit)
					(*exit)->OnWorldCreated(world);
				// Add the world's tasks to the scheduler
				worlds.push_back(world);
			}
			m_Scheduler->SetUniverse(worlds);

			// Check for any messages posted during world creation
			m_ComponentUniverse->CheckMessages();

			// Activate extensions
			for (auto it = m_ActiveExtensions.begin(), end = m_ActiveExtensions.end(); it != end; ++it)
			{
				(*it)->Activate();
			}

			unsigned short listenPort = 11122;
			m_Network->Startup(listenPort);

			// Load the map
			if (!m_EditMode/* && varMap.count("connect") == 0*/)
			{
				m_Map = m_MapLoader->LoadMap("Maps/default.gad", m_EntityInstantiator.get());
				m_CellArchivist->SetMap(m_Map);

				m_StreamingManager->Initialise(m_Map->GetCellSize());

				m_Map->LoadNonStreamingEntities(true, m_EntityManager.get(), m_ComponentUniverse.get(), m_ArchetypeFactory.get(), m_EntityInstantiator.get());
			}
			// Start the asynchronous cell loader
			m_CellArchivist->Start();

			bool connecting = false;

			auto gc = m_DisplayWindow.get_gc();

			boost::signals2::scoped_connection rawInputConnection = m_InputManager->SignalRawInput.connect([this](const RawInput& raw)
			{
				if (raw.InputType == RawInput::EventType::Button && !raw.ButtonPressed && raw.Code == VK_OEM_3)
					this->m_GUI->GetConsoleWindow()->Show();
			});

			float time = CL_System::get_time() * 0.001f;
			float dt;
			bool quit = false;
			auto closeSig = m_DisplayWindow.sig_window_close().connect_functor([&quit]() { quit = true; });
			while (!quit)
			{
				CL_KeepAlive::process();

				const float now = CL_System::get_time() * 0.001f;
				dt = now - time;
				time = now;

				// Update extensions
				for (auto it = m_ActiveExtensions.begin(), end = m_ActiveExtensions.end(); it != end; ++it)
				{
					(*it)->Update(time, dt);
					quit |= (*it)->HasRequestedQuit();
					ProcessExtensionMessages(*it);
				}
				// Update GUI
				m_GUI->Update(dt);

				m_InputManager->Update(dt);
				m_NetworkManager->DispatchPackets();

				// Run serial resource manager operations
				m_ResourceManager->UnloadUnreferencedResources();
				m_ResourceManager->DeliverLoadedResources();

				const auto executed = m_Scheduler->Execute((m_EditMode || connecting) ? SystemType::Rendering : (SystemType::Rendering | SystemType::Simulation));

				if (executed & SystemType::Rendering)
				{
#ifdef PROFILE_BUILD
					m_DisplayWindow.flip(0);
#else
					m_DisplayWindow.flip();
#endif
					gc.clear();
				}

				if ((executed & SystemType::Simulation) || m_EditMode)
				{
					m_CellArchivist->BeginTransaction();
					// Actually activate / deactivate components
					m_EntityManager->ProcessActivationQueues();
					m_EntitySynchroniser->ProcessQueue(m_EntityManager.get());
					m_CellArchivist->EndTransaction();

					m_ComponentUniverse->CheckMessages();
				}

				// Propagate property changes
				// TODO: throw if properties are changed during Rendering step?
				//PropChangedQueue::value_type changed;
				//while (propChangedQueue.try_pop(changed))
				//{
				//	auto com = changed.first.lock();
				//	if (com)
				//	{
				//		changed.second->Synchronise();
				//		changed.second->FireSignal();
				//	}
				//}
				m_EvesdroppingManager->GetSignalingSystem().Fire();

				if (executed & SystemType::Simulation)
				{
					{
						std::pair<std::string, bool> enqueued;
						while (m_SaveQueue.try_pop(enqueued))
							Save(enqueued.first, enqueued.second);
					}
					{
						tbb::spin_mutex::scoped_lock lock(m_LoadQueueMutex);
						if (!m_SaveToLoad.empty())
						{
							Load(m_SaveToLoad);
							m_SaveToLoad.clear();
						}
					}
				}

#ifdef FSN_PROFILING_ENABLED
				Profiling::getSingleton().AddTime("~Buffer size", 0.0);
				Profiling::getSingleton().AddTime("~Incomming Packets", 0.0);
				Profiling::getSingleton().AddTime("~Packets Processed", 0.0);

				m_Profiling->AddTime("ActualDT", dt);

				// Record profiling data
				m_Profiling->StoreTick();
#endif
			} // while !quit

			// Shutdown
			for (auto it = m_Extensions.begin(), end = m_Extensions.end(); it != end; ++it)
				(*it)->CleanUp();
			//propChangedQueue.clear();
			m_CameraSynchroniser.reset();
			m_EntityManager->Clear();
			m_StreamingManager.reset();
			m_CellArchivist->Stop();
			m_EntityManager.reset();
				
			m_EntitySynchroniser.reset();
			m_ScriptManager->GetEnginePtr()->GarbageCollect();
			m_ComponentUniverse.reset();
			m_ScriptManager->GetEnginePtr()->GarbageCollect();
		}
		catch (Exception& ex)
		{
			m_Log->AddEntry("Unhandled exception: " + ex.ToString(), LOG_CRITICAL);
#ifdef _WIN32
			if (!m_DisplayWindow.is_null())
				MessageBoxA(m_DisplayWindow.get_hwnd(), ("Unhandled exception: " + ex.ToString()).c_str(), "Error", MB_OK);
#endif
		}
		catch (std::exception& ex)
		{
			m_Log->AddEntry(std::string("Unhandled exception: ") + ex.what(), LOG_CRITICAL);
#ifdef _WIN32
			if (!m_DisplayWindow.is_null())
				MessageBoxA(m_DisplayWindow.get_hwnd(), (std::string("Unhandled exception: ") + ex.what()).c_str(), "Error", MB_OK);
#endif
		}
	}

	void EngineManager::ProcessExtensionMessages(const std::shared_ptr<EngineExtension>& ex)
	{
		auto message = ex->PopMessage();
		while (message.first)
		{
			switch (message.first)
			{
			case EngineExtension::MessageType::Save:
				Save(message.second);
				break;
			case EngineExtension::MessageType::Load:
				Load(message.second);
				break;
			case EngineExtension::MessageType::SwitchToEditMode:
				// TODO: restart engine-manager in edit-mode (perhaps set the edit-mode option and make Run exit with a 'restart' flag?)
				break;
			};
			message = ex->PopMessage();
		}
	}

}
