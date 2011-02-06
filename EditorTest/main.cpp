#include "../FusionEngine/FusionStableHeaders.h"
#include "../FusionEngine/FusionPrerequisites.h"

// Logging
#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

// Filesystem
#include "../FusionEngine/FusionPaths.h"
#include "../FusionEngine/FusionPhysFS.h"
#include "../FusionEngine/FusionVirtualFileSource_PhysFS.h"

// Resource Loading
#include "../FusionEngine/FusionResourceManager.h"
#include "../FusionEngine/FusionAudioLoader.h"
#include "../FusionEngine/FusionImageLoader.h"

// Network
#include "../FusionEngine/FusionRakNetwork.h"
#include "../FusionEngine/FusionPlayerRegistry.h"

// Systems
#include "../FusionEngine/FusionEditor.h"
#include "../FusionEngine/FusionGUI.h"
#include "../FusionEngine/FusionFirstCause.h"
#include "../FusionEngine/FusionNetworkSystem.h"
#include "../FusionEngine/FusionOntologicalSystem.h"
#include "../FusionEngine/FusionStateManager.h"

// Various
#include "../FusionEngine/FusionInputHandler.h"
#include "../FusionEngine/FusionScriptManager.h"

#include "../FusionEngine/FusionClientOptions.h"
#include "../FusionEngine/FusionElementUndoMenu.h"
#include "../FusionEngine/FusionEntity.h"
#include "../FusionEngine/FusionEntityManager.h"
#include "../FusionEngine/FusionExceptionFactory.h"
#include "../FusionEngine/FusionPhysicsScriptTypes.h"
#include "../FusionEngine/FusionRenderer.h"
#include "../FusionEngine/FusionScriptedConsoleCommand.h"
#include "../FusionEngine/FusionScriptedEntity.h"
#include "../FusionEngine/FusionScriptModule.h"
#include "../FusionEngine/FusionScriptSound.h"

#include <ClanLib/application.h>
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>
#include <ClanLib/vorbis.h>

namespace FusionEngine
{

class EditorTest
{
public:
	virtual int main(const std::vector<CL_String>& args)
	{
		CL_SetupCore core_setup;
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;
		CL_SetupSound sound_setup;
		CL_SetupVorbis voirbis_setup;

		CL_SoundOutput sound_output(44100);

		SetupPhysFS physfs( CL_System::get_exe_path().c_str() );
		FSN_ASSERT( SetupPhysFS::is_init() );

		CL_ConsoleWindow conWindow("Console", 80, 10);

		CL_DisplayWindow dispWindow("Display", 1024, 768);

		try
		{
			boost::scoped_ptr<Logger> logger;
			boost::scoped_ptr<ConsoleStdOutWriter> coutWriter;
			boost::scoped_ptr<Console> console;

			console.reset(new Console());
			coutWriter.reset(new ConsoleStdOutWriter());
			coutWriter->Enable();
			logger.reset(new Logger());

			CL_GraphicContext &gc = dispWindow.get_gc();
			CL_OpenGL::set_active(NULL);

			////////////////////
			// Configure PhysFS
			SetupPhysFS::configure("lastflare", "Fusion", "7z");
			if (!SetupPhysFS::mount(s_PackagesPath, "", "7z", false))
				SendToConsole("Default resource path could not be located");
			SetupPhysFS::mount_archives(s_PackagesPath, "", "zip", false);
			// Clear cache
#ifdef _DEBUG
			SetupPhysFS::clear_temp();
#endif

			////////////////////
			// Script Manager
			ScriptManager *scriptingManager = new ScriptManager();
			asIScriptEngine* asEngine = scriptingManager->GetEnginePtr();

			//RegisterPhysicsTypes(asEngine);
			//ResourceManager::Register(m_ScriptManager);
			Console::Register(scriptingManager);
			RegisterScriptedConsoleCommand(asEngine);
			GUI::Register(scriptingManager);
			ContextMenu::Register(asEngine);
			Renderable::Register(asEngine);
			SoundOutput::Register(asEngine);
			SoundSession::Register(asEngine);
			SoundSample::Register(asEngine);
			Entity::Register(asEngine);
			ScriptedEntity::Register(asEngine); // TODO: perhaps this should be PhysicalEntity::Register? (i.e. redifine the method within that scope)
			EntityManager::Register(asEngine);
			Camera::Register(asEngine);
			Viewport::Register(asEngine);
			StreamingManager::Register(asEngine);
			OntologicalSystem::Register(asEngine);
			ElementUndoMenu::Register(asEngine);
			GameMapLoader::Register(asEngine);
			Editor::Register(asEngine);
			RegisterEntityUnwrap(asEngine);

			/////////////////////////////////////
			// Script SoundOutput wrapper object
			std::tr1::shared_ptr<SoundOutput> script_SoundOutput(new SoundOutput(sound_output));

			////////////////////
			// Resource Manager
			boost::scoped_ptr<ResourceManager> resourceManager(new ResourceManager(gc));
			resourceManager->AddResourceLoader("IMAGE", &LoadImageResource, &UnloadImageResouce, NULL);
			resourceManager->AddResourceLoader("AUDIO", &LoadAudio, &UnloadAudio, NULL);
			resourceManager->AddResourceLoader("AUDIO:STREAM", &LoadAudioStream, &UnloadAudio, NULL); // Note that this intentionally uses the same unload method
			resourceManager->AddResourceLoader("SPRITE", &LoadSpriteResource, &UnloadSpriteResource, &UnloadSpriteQuickLoadData, NULL);
			
#ifdef _WIN32
			resourceManager->StartLoaderThread(gc);
#elif defined(__APPLE__)
			CL_GraphicContext loadingGC = gc.create_worker_gc();
			resourceManager->StartLoaderThread(loadingGC);
#else
			// Might have to create a secondary pbuffer here (I read something about
			//  this giving better performance because of reduced resource locking)
			CL_GraphicContext loadingGC = gc.create_worker_gc();
			resourceManager->StartLoaderThread(loadingGC);
#endif
			// Make sure the GC is properly configured
			CL_OpenGL::set_active(gc);

			/////////////////////////
			// Load optional settings
			ClientOptions* co = new ClientOptions("settings.xml", "settings");

			if (co->GetOption_bool("console_logging"))
				logger->ActivateConsoleLogging();

			/////////////////
			// Input Manager
			boost::scoped_ptr<InputManager> inputMgr(new InputManager(dispWindow));

			if (!inputMgr->Test())
				FSN_EXCEPT(ExCode::IO, "main", "InputManager couldn't find a keyboard device.");
			inputMgr->Initialise();
			SendToConsole("Input manager started successfully");

			////////////
			// Renderer
			boost::scoped_ptr<Renderer> renderer(new Renderer(gc));

			///////////////////
			// Player Registry
			std::shared_ptr<PlayerRegistry> playerRegistry(new PlayerRegistry());

			///////////
			// Systems
			SystemsManager *systemMgr = new SystemsManager();

			std::shared_ptr<NetworkSystem> networkSystem( new NetworkSystem() );
			systemMgr->AddSystem(networkSystem);

			std::shared_ptr<GUI> gui( new GUI(dispWindow) );
			// Init GUI
			gui->Initialise();
			gui->PushMessage(SystemMessage::HIDE);
			ElementUndoMenu::RegisterElement();
			
			/////////////////////
			// Attach module to objects that require it
			ModulePtr module = scriptingManager->GetModule("main");
			// Add the core extension file - this file is used to define basic script functions and classes for general use
			scriptingManager->AddFile("core/extend.as", "main");

			// Create some non-specific intelligent force that certainly has no particular theological bias:
			boost::scoped_ptr<FirstCause> god( new FirstCause(co, renderer.get(), inputMgr.get()) );
			god->Initialise(module);

			console->SetModule(module);
			gui->SetModule(module);
			
			script_SoundOutput->SetModule(module);

			// Build the module (scripts will be added automatically by objects which have registered a module connection)
			int r = module->Build();
			if( r < 0 )
			{
				AddLogEntry(g_LogGeneral, "Failed to build scripts.", LOG_CRITICAL);
				logger.reset(); // Destroy the logger to destroy the console logfile before the console (TODO: automate this in console destructor with a callback)
				FSN_EXCEPT(FusionEngine::Exception, "startup", "Failed to build scripts.");
			}

			// Start ontology / editor
			god->BeginExistence(systemMgr);

			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;
			float seconds = 0.f;
			int fullCleanInterval = 0;

			CL_Font statsFont(gc, "Arial", 18);
#ifdef _DEBUG
			asUINT currentSize, totalDestroyed, totalDetected;
#endif

			while (systemMgr->KeepGoing())
			{
				split = CL_System::get_time() - lastframe;
				lastframe = CL_System::get_time();

				CL_KeepAlive::process();

				resourceManager->UnloadUnreferencedResources();
				resourceManager->DeliverLoadedResources();

				if (split < 1000)
				{
					seconds = split * 0.001f;
					inputMgr->Update(seconds);
					gui->Update(seconds);
					systemMgr->Update(seconds);
				}

				systemMgr->Draw();

				scriptingManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);

#ifdef _DEBUG
				{
					scriptingManager->GetEnginePtr()->GetGCStatistics(&currentSize, &totalDestroyed, &totalDetected);
					std::stringstream stats;
					stats << "Tracked object count: " << currentSize << "\n";
					stats << "     GC has destroyed " << totalDestroyed << " objects total\n";
					stats << "     GC has detected  " << totalDetected << " garbage objects in total";
					statsFont.draw_text(gc, CL_Pointf(20, 25), stats.str());
				}
#endif

				dispWindow.flip();
#ifdef _DEBUG
				dispWindow.get_gc().clear();
#endif
			}

			scriptingManager->GetEnginePtr()->GarbageCollect();

			god.reset();
			delete systemMgr;
			scriptingManager->GetEnginePtr()->GarbageCollect();
			gui->CleanUp();
			delete scriptingManager;

			logger.reset();
			resourceManager.reset();
		}
		catch (FusionEngine::Exception &ex)
		{
#ifdef _DEBUG
			CL_Console::write_line( CL_String(ex.ToString().c_str()) );
			conWindow.display_close_message();
#endif
			//TODO: Show a OS native GUI messagebox in Release builds
		}
		catch (CL_Exception &ex)
		{
#ifdef _DEBUG
			CL_Console::write_line( ex.message );
			CL_Console::write_line( "Stack Trace:" );
			std::vector<CL_String> stack = ex.get_stack_trace();
			for (std::vector<CL_String>::iterator it = stack.begin(), end = stack.end(); it != end; ++it)
			{
				CL_Console::write_line(*it);
			}
			conWindow.display_close_message();
#endif
			//TODO: Show a OS native GUI messagebox in Release builds
		}

		return 0;
	}

	~EditorTest()
	{
	}

};

} // end of FusionEngine namespace

class EntryPoint
{
public:
	static int main(const std::vector<CL_String> &args)
	{
		FusionEngine::EditorTest app;
		return app.main(args);
	}
};

CL_ClanApplication app(&EntryPoint::main);
