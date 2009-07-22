#include "../FusionEngine/Common.h"
#include "../FusionEngine/FusionCommon.h"

// Logging
#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

// Systems
#include "../FusionEngine/FusionOntologicalSystem.h"
#include "../FusionEngine/FusionGUI.h"
#include "../FusionEngine/FusionStateManager.h"

// Various
#include "../FusionEngine/FusionInputHandler.h"
#include "../FusionEngine/FusionResourceManager.h"
#include "../FusionEngine/FusionImageLoader.h"
#include "../FusionEngine/FusionPhysicsWorld.h"
#include "../FusionEngine/FusionScriptingEngine.h"
// Script Type Registration
#include "../FusionEngine/FusionScriptTypeRegistrationUtils.h"
#include "../FusionEngine/FusionPhysicsScriptTypes.h"
#include "../FusionEngine/FusionScriptedConsoleCommand.h"

#include "../FusionEngine/FusionClientOptions.h"


namespace FusionEngine
{

class EntityTest
{
private:
	InputManager *m_Input;
	ResourceManager *m_ResourceManager;
	ScriptingEngine *m_ScriptManager;

	OntologicalSystem *m_Ontology;

public:
	virtual int main(const std::vector<CL_String>& args)
	{
		CL_SetupCore core_setup;
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;
		CL_SetupSound sound_setup;
		CL_SetupVorbis voirbis_setup;

		CL_SoundOutput sound_output(44100);

		CL_ConsoleWindow conWindow("Console", 80, 10);

		CL_DisplayWindow dispWindow("Display", 800, 600);

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

			////////////////////
			// Scripting Manager
			m_ScriptManager = new ScriptingEngine();
			asIScriptEngine* asEngine = m_ScriptManager->GetEnginePtr();

			RegisterPhysicsTypes(asEngine);
			//ResourceManager::Register(m_ScriptManager);
			Console::Register(m_ScriptManager);
			RegisterScriptedConsoleCommand(m_ScriptManager->GetEnginePtr());
			GUI::Register(m_ScriptManager);

		
			////////////////////
			// Resource Manager
			m_ResourceManager = new ResourceManager();
			m_ResourceManager->AddResourceLoader("SPRITE", &LoadSpriteResource, &UnloadSpriteResource, &UnloadSpriteQuickLoadData, &gc);

			//////////////////////
			// Load client options
			ClientOptions* co = new ClientOptions(L"clientoptions.xml");

			if (co->GetOption_bool("console_logging"))
				logger->ActivateConsoleLogging();

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

			///////////
			// Systems
			SystemsManager *systemMgr = new SystemsManager();

			GUI *gui = new GUI(dispWindow);
			systemMgr->AddSystem(gui);

			m_Ontology = new OntologicalSystem(renderer, m_Input);
			systemMgr->AddSystem(m_Ontology);

			/////////////////////
			// Attach module to objects that require it
			ModulePtr module = m_ScriptManager->GetModule("main");
			console->SetModule(module);
			gui->SetModule(module);
			m_Ontology->SetModule(module);

			module->Build();

			
			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;

			while (systemMgr->KeepGoing())
			{
				dispWindow.get_gc().clear();

				split = CL_System::get_time() - lastframe;
				lastframe = CL_System::get_time();

				if (CL_DisplayMessageQueue::has_messages())
					CL_DisplayMessageQueue::process();

				if (split < 500)
				{
					m_Input->Update(split * 0.001f);
					systemMgr->Update(split);
				}

				systemMgr->Draw();

				m_ScriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);

				dispWindow.flip();
			}

			delete systemMgr;
			delete renderer;
			delete m_Input;
			delete m_ScriptManager;

		}
		catch (FusionEngine::Exception& ex)
		{
			CL_Console::write_line( CL_String(ex.ToString().c_str()) );
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
