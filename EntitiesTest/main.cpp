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
#include "../FusionEngine/FusionVirtualFileSource_PhysFS.h"
// Script Type Registration
#include "../FusionEngine/FusionScriptTypeRegistrationUtils.h"
#include "../FusionEngine/FusionPhysicsScriptTypes.h"
#include "../FusionEngine/FusionScriptedConsoleCommand.h"
#include "../FusionEngine/FusionEntity.h"

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
			RefCounted::RegisterType<Renderable>(m_ScriptManager->GetEnginePtr(), "Renderable");

		
			////////////////////
			// Resource Manager
			m_ResourceManager = new ResourceManager(gc);
			m_ResourceManager->AddResourceLoader("SPRITE", &LoadSpriteResource, &UnloadSpriteResource, &UnloadSpriteQuickLoadData, NULL);

			m_ResourceManager->PreloadResource("SPRITE", L"Entities/Test/test_sprite.xml");
			//m_ResourceManager->StartBackgroundPreloadThread();

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

			ResourcePointer<CL_Sprite> resource = m_ResourceManager->GetResource<CL_Sprite>("Entities/Test/test_sprite.xml", "SPRITE");
			resource->set_color(CL_Color::white);
			resource->set_alpha(1.f);

			CL_VirtualDirectory dir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
			CL_ResourceManager clanResources("Entities/Test/resources.xml", dir);
			CL_Sprite sprite(gc, "Explosion", &clanResources);

			
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

				if (split < 500)
				{
					seconds = split * 0.001f;
					m_Input->Update(seconds);
					systemMgr->Update(seconds);
				}

				systemMgr->Draw();
				//if (resource.Lock())
				//{
				//	//resource->update((float)split*0.001f);
				//	resource->draw(gc, 150.f, 50.f);
				//	resource.Unlock();
				//}

				//sprite.update((float)split*0.001f);
				//sprite.draw(gc, 25.f, 50.f);

				m_ScriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);

				dispWindow.flip();
			}

			delete systemMgr;
			delete renderer;
			delete m_Input;
			delete m_ScriptManager;
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
