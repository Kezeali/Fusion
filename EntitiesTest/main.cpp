#include "../FusionEngine/Common.h"
#include "../FusionEngine/FusionCommon.h"

// Logging
#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

// Network
#include "../FusionEngine/FusionRakNetwork.h"

#include "../FusionEngine/FusionPlayerRegistry.h"

// Systems
#include "../FusionEngine/FusionNetworkSystem.h"
#include "../FusionEngine/FusionOntologicalSystem.h"
#include "../FusionEngine/FusionGUI.h"
#include "../FusionEngine/FusionStateManager.h"

// Various
#include "../FusionEngine/FusionInputHandler.h"
#include "../FusionEngine/FusionResourceManager.h"
#include "../FusionEngine/FusionImageLoader.h"
#include "../FusionEngine/FusionPhysicsWorld.h"
#include "../FusionEngine/FusionScriptingEngine.h"
#include "../FusionEngine/FusionVirtualFileSource_PhysFS.h"
// Script Type Registration
#include "../FusionEngine/FusionScriptTypeRegistrationUtils.h"
#include "../FusionEngine/FusionPhysicsScriptTypes.h"
#include "../FusionEngine/FusionScriptedConsoleCommand.h"
#include "../FusionEngine/FusionEntityManager.h"
#include "../FusionEngine/FusionEntity.h"
#include "../FusionEngine/FusionScriptedEntity.h"
#include "../FusionEngine/FusionScriptSound.h"

#include "../FusionEngine/FusionClientOptions.h"

//#include <ClanLib/d3d9.h>


namespace FusionEngine
{

class EntityTest
{
private:
	Network *m_Network;
	InputManager *m_Input;
	ResourceManager *m_ResourceManager;
	ScriptingEngine *m_ScriptManager;

	std::tr1::shared_ptr<PlayerRegistry> m_PlayerRegistry;

	OntologicalSystem *m_Ontology;

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

		CL_ConsoleWindow conWindow("Console", 80, 10);

		CL_DisplayWindow dispWindow("Display", 800, 600);

		// TODO: store these in shared_ptrs
		Logger* logger = 0;
		ConsoleStdOutWriter* cout = 0;
		Console* console = 0;

		try
		{
			console = new Console();
			cout = new ConsoleStdOutWriter();
			cout->Enable();
			logger = new Logger();

			CL_GraphicContext &gc = dispWindow.get_gc();
			CL_OpenGL::set_active(NULL);

			////////////////////
			// Scripting Manager
			m_ScriptManager = new ScriptingEngine();
			asIScriptEngine* asEngine = m_ScriptManager->GetEnginePtr();

			RegisterPhysicsTypes(asEngine);
			//ResourceManager::Register(m_ScriptManager);
			Console::Register(m_ScriptManager);
			RegisterScriptedConsoleCommand(asEngine);
			GUI::Register(m_ScriptManager);
			//Renderable::Register(asEngine);
			RefCounted::RegisterType<Renderable>(asEngine, "Renderable");
			SoundOutput::Register(asEngine);
			SoundSession::Register(asEngine);
			SoundSample::Register(asEngine);
			Entity::Register(asEngine);
			EntityManager::Register(asEngine);
			RegisterEntityUnwrap(asEngine);

			/////////////////////////////////////
			// Script SoundOutput wrapper object
			std::tr1::shared_ptr<SoundOutput> script_SoundOutput(new SoundOutput(sound_output));

		
			////////////////////
			// Resource Manager
			m_ResourceManager = new ResourceManager(CL_GraphicContext()/*gc*/);
			m_ResourceManager->AddResourceLoader("SPRITE", &LoadSpriteResource, &UnloadSpriteResource, &UnloadSpriteQuickLoadData, NULL);

			//m_ResourceManager->PreloadResource("SPRITE", L"Entities/Test/test_sprite.xml");
#ifdef _WIN32
			// Need to pause this thread until the Background-load
			//  thread has created its worker GC
			CL_Event loaderGCCreated;
			m_ResourceManager->StartBackgroundLoadThread(gc, loaderGCCreated);
			// Wait for the event to be set
			CL_Event::wait(loaderGCCreated);
#elif defined(__APPLE__)
			CL_GraphicContext loadingGC = gc.create_worker_gc();
			m_ResourceManager->StartBackgroundLoadThread(loadingGC);
#else
			// Might have to create a secondary pbuffer here (I read something about
			//  this giving better performance because of reduced resource locking)
			CL_GraphicContext loadingGC = gc.create_worker_gc();
			m_ResourceManager->StartBackgroundLoadThread(loadingGC);
#endif

			CL_OpenGL::set_active(gc);

			//////////////////////
			// Load client options
			ClientOptions* co = new ClientOptions(L"clientoptions.xml");

			if (co->GetOption_bool("console_logging"))
				logger->ActivateConsoleLogging();

			//////////////////////
			// Network Connection
			m_Network = new RakNetwork();

			/////////////////
			// Input Manager
			m_Input = new InputManager(dispWindow);

			if (!m_Input->Test())
				FSN_EXCEPT(ExCode::IO, "main", "InputManager couldn't be started");
			m_Input->Initialise();
			SendToConsole("Input manager started successfully");

			////////////
			// Renderer
			Renderer *renderer = new Renderer(gc);

			///////////////////
			// Player Registry
			m_PlayerRegistry.reset(new PlayerRegistry());

			///////////
			// Systems
			SystemsManager *systemMgr = new SystemsManager();

			NetworkSystem *networkSystem = new NetworkSystem(m_Network);
			systemMgr->AddSystem(networkSystem);

			GUI *gui = new GUI(dispWindow);
			m_Ontology = new OntologicalSystem(renderer, m_Input, m_Network);

			systemMgr->AddSystem(m_Ontology);
			systemMgr->AddSystem(gui);

			/////////////////////
			// Attach module to objects that require it
			ModulePtr module = m_ScriptManager->GetModule("main");
			console->SetModule(module);
			gui->SetModule(module);
			m_Ontology->SetModule(module);

			script_SoundOutput->SetModule(module);

			// Build the module (scripts are added automatically by objects which have registered a module connection)
			module->Build();


			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;
			float seconds = 0.f;

			while (systemMgr->KeepGoing())
			{
				dispWindow.get_gc().clear();

				split = CL_System::get_time() - lastframe;
				lastframe = CL_System::get_time();

				if (CL_DisplayMessageQueue::has_messages())
					CL_DisplayMessageQueue::process();

				m_ResourceManager->DisposeUnusedResources();
				m_ResourceManager->DeliverLoadedResources();

				if (split < 500)
				{
					seconds = split * 0.001f;
					m_Input->Update(seconds);
					systemMgr->Update(seconds);
				}

				systemMgr->Draw();

				m_ScriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);

				dispWindow.flip();
			}

			delete systemMgr;
			delete renderer;
			delete m_Input;
			delete m_ScriptManager;
			delete m_Network;
		}
		catch (FusionEngine::Exception &ex)
		{
			CL_Console::write_line( CL_String(ex.ToString().c_str()) );
			conWindow.display_close_message();
		}
		catch (CL_Exception &ex)
		{
			CL_Console::write_line( ex.message );
			CL_Console::write_line( L"Stack Trace:" );
			std::vector<CL_String> stack = ex.get_stack_trace();
			for (std::vector<CL_String>::iterator it = stack.begin(), end = stack.end(); it != end; ++it)
			{
				CL_Console::write_line(*it);
			}
			conWindow.display_close_message();
		}

		//delete GUI::getSingletonPtr();

		if (logger != 0)
			delete logger;
		if (cout != 0)
			delete cout;
		if (console != 0)
			delete console;


		return 0;
	}

	~EntityTest()
	{
	}

};

} // end of FusionEngine namespace

class EntryPoint
{
public:
	static int main(const std::vector<CL_String> &args)
	{
		FusionEngine::EntityTest app;
		return app.main(args);
	}
};

CL_ClanApplication app(&EntryPoint::main);
