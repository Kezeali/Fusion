#include "../FusionEngine/Common.h"
#include "../FusionEngine/FusionCommon.h"

// Logging
#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

// Filesystem
#include "../FusionEngine/FusionPaths.h"
#include "../FusionEngine/FusionPhysFS.h"
#include "../FusionEngine/FusionVirtualFileSource_PhysFS.h"

// Network
#include "../FusionEngine/FusionRakNetwork.h"

#include "../FusionEngine/FusionPlayerRegistry.h"

// Systems
#include "../FusionEngine/FusionNetworkSystem.h"
#include "../FusionEngine/FusionOntologicalSystem.h"
#include "../FusionEngine/FusionGUI.h"
#include "../FusionEngine/FusionEditor.h"
#include "../FusionEngine/FusionStateManager.h"

// Various
#include "../FusionEngine/FusionInputHandler.h"
#include "../FusionEngine/FusionResourceManager.h"
#include "../FusionEngine/FusionImageLoader.h"
#include "../FusionEngine/FusionScriptingEngine.h"

// Script Type Registration
//#include "../FusionEngine/FusionScriptTypeRegistrationUtils.h"
#include "../FusionEngine/FusionPhysicsScriptTypes.h"
#include "../FusionEngine/FusionScriptedConsoleCommand.h"
#include "../FusionEngine/FusionEntityManager.h"
#include "../FusionEngine/FusionEntity.h"
#include "../FusionEngine/FusionScriptedEntity.h"
#include "../FusionEngine/FusionRenderer.h"
#include "../FusionEngine/FusionScriptSound.h"
#include "../FusionEngine/FusionElementUndoMenu.h"

#include "../FusionEngine/FusionClientOptions.h"

#include "../FusionEngine/FusionScriptModule.h"

//#include <ClanLib/d3d9.h>


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
		//CL_SetupD3D9 d3d_setup;
		CL_SetupSound sound_setup;
		CL_SetupVorbis voirbis_setup;

		CL_SoundOutput sound_output(44100);

		SetupPhysFS physfs( CL_System::get_exe_path().c_str() );
		FSN_ASSERT( SetupPhysFS::is_init() );

		CL_ConsoleWindow conWindow("Console", 80, 10);

		CL_DisplayWindow dispWindow("Display", 800, 600);

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
			// Clear cache
#ifdef _DEBUG
			SetupPhysFS::clear_temp();
#endif

			////////////////////
			// Scripting Manager
			ScriptingEngine *scriptingManager = new ScriptingEngine();
			asIScriptEngine* asEngine = scriptingManager->GetEnginePtr();

			//RegisterPhysicsTypes(asEngine);
			//ResourceManager::Register(m_ScriptManager);
			Console::Register(scriptingManager);
			RegisterScriptedConsoleCommand(asEngine);
			GUI::Register(scriptingManager);
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
			Editor::Register(asEngine);
			RegisterEntityUnwrap(asEngine);

			/////////////////////////////////////
			// Script SoundOutput wrapper object
			std::tr1::shared_ptr<SoundOutput> script_SoundOutput(new SoundOutput(sound_output));

			////////////////////
			// Resource Manager
			boost::scoped_ptr<ResourceManager> resourceManager(new ResourceManager());
			resourceManager->AddResourceLoader("SPRITE", &LoadSpriteResource, &UnloadSpriteResource, &UnloadSpriteQuickLoadData, NULL);

			//m_ResourceManager->PreloadResource("SPRITE", "Entities/Test/test_sprite.xml");
#ifdef _WIN32
			// Need to pause this thread until the Background-load
			//  thread has created its worker GC
			CL_Event loaderGCCreated;
			resourceManager->StartBackgroundLoadThread(gc, loaderGCCreated);
			// Wait for the event to be set
			CL_Event::wait(loaderGCCreated);
#elif defined(__APPLE__)
			CL_GraphicContext loadingGC = gc.create_worker_gc();
			resourceManager->StartBackgroundLoadThread(loadingGC);
#else
			// Might have to create a secondary pbuffer here (I read something about
			//  this giving better performance because of reduced resource locking)
			CL_GraphicContext loadingGC = gc.create_worker_gc();
			resourceManager->StartBackgroundLoadThread(loadingGC);
#endif

			CL_OpenGL::set_active(gc);

			resourceManager->SetGraphicContext(gc);

			//////////////////////
			// Load client options
			ClientOptions* co = new ClientOptions("clientoptions.xml", "clientoptions");

			if (co->GetOption_bool("console_logging"))
				logger->ActivateConsoleLogging();

			//////////////////////
			// Network Connection
			boost::scoped_ptr<Network> network(new RakNetwork());

			/////////////////
			// Input Manager
			boost::scoped_ptr<InputManager> inputMgr(new InputManager(dispWindow));

			if (!inputMgr->Test())
				FSN_EXCEPT(ExCode::IO, "main", "InputManager couldn't be started");
			inputMgr->Initialise();
			SendToConsole("Input manager started successfully");

			////////////
			// Renderer
			boost::scoped_ptr<Renderer> renderer(new Renderer(gc));

			///////////////////
			// Player Registry
			std::tr1::shared_ptr<PlayerRegistry> playerRegistry(new PlayerRegistry());

			///////////
			// Systems
			SystemsManager *systemMgr = new SystemsManager();

			std::tr1::shared_ptr<NetworkSystem> networkSystem( new NetworkSystem(network.get()) );
			systemMgr->AddSystem(networkSystem);

			std::tr1::shared_ptr<GUI> gui( new GUI(dispWindow) );
			// Init GUI
			gui->Initialise();
			gui->PushMessage(new SystemMessage(SystemMessage::HIDE));
			ElementUndoMenu::RegisterElement();

			std::tr1::shared_ptr<OntologicalSystem> ontology( new OntologicalSystem(co, renderer.get(), inputMgr.get(), networkSystem.get()) );
			
			/////////////////////
			// Attach module to objects that require it
			ModulePtr module = scriptingManager->GetModule("main");
			// Add the core extension file - this file is used to define basic script functions and classes for general use
			scriptingManager->AddFile("core/extend.as", "main");

			console->SetModule(module);
			gui->SetModule(module);
			
			script_SoundOutput->SetModule(module);

			systemMgr->AddSystem(ontology);
			//systemMgr->AddSystem(gui);

			ontology->SetModule(module);

			// Build the module (scripts are added automatically by objects which have registered a module connection)
			module->Build();
			//{
			//	BuildModuleEvent bme;
			//	bme.type = BuildModuleEvent::PostBuild;
			//	ontology->OnModuleRebuild(bme);
			//}


			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;
			float seconds = 0.f;
			int fullCleanInterval = 0;

			while (systemMgr->KeepGoing())
			{
				dispWindow.get_gc().clear();

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

				dispWindow.flip();
			}

			scriptingManager->GetEnginePtr()->GarbageCollect();

			delete systemMgr;
			scriptingManager->GetEnginePtr()->GarbageCollect();
			gui->CleanUp();
			delete scriptingManager;

			logger.reset();
			resourceManager.reset();
		}
		catch (FusionEngine::Exception &ex)
		{
			CL_Console::write_line( CL_String(ex.ToString().c_str()) );
			conWindow.display_close_message();
		}
		catch (CL_Exception &ex)
		{
			CL_Console::write_line( ex.message );
			CL_Console::write_line( "Stack Trace:" );
			std::vector<CL_String> stack = ex.get_stack_trace();
			for (std::vector<CL_String>::iterator it = stack.begin(), end = stack.end(); it != end; ++it)
			{
				CL_Console::write_line(*it);
			}
			conWindow.display_close_message();
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
