#include "../FusionEngine/Common.h"
#include "../FusionEngine/FusionCommon.h"

#include "../FusionEngine/FusionPhysicsWorld.h"
#include "../FusionEngine/FusionPhysicsBody.h"
#include "../FusionEngine/FusionPhysicsCallback.h"

#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

#include "../FusionEngine/FusionScriptedConsoleCommand.h"

#include "../FusionEngine/FusionInputHandler.h"

#include "../FusionEngine/FusionStateManager.h"
#include "../FusionEngine/FusionGUI.h"

#include "../FusionEngine/FusionScriptingEngine.h"
#include "../FusionEngine/FusionScriptTypeRegistrationUtils.h"
#include "../FusionEngine/FusionPhysicsScriptTypes.h"

#include "../FusionEngine/FusionResource.h"
#include "../FusionEngine/FusionResourcePointer.h"
#include "../FusionEngine/FusionResourceLoader.h"
#include "../FusionEngine/FusionXMLLoader.h"
//#include "../FusionEngine/FusionTextLoader.h"
#include "../FusionEngine/FusionAudioLoader.h"
#include "../FusionEngine/FusionImageLoader.h"
#include "../FusionEngine/FusionResourceManager.h"

#include "../FusionEngine/FusionClientOptions.h"

#include "../FusionEngine/FusionXml.h"

//#include <Rocket/AngelScript/Core/ras_Core.h>
//#include <Rocket/AngelScript/Controls/ras_Controls.h>

#include "../FusionEngine/scriptstring.h"


using namespace FusionEngine;


class GUIHUDTest
{
private:
	PhysicsWorld *m_World;
	InputManager *m_Input;
	ScriptingEngine *m_ScriptManager;
	ResourceManager *m_ResMan;

	ScriptObject mso_Ship;
	ScriptUtils::Calling::Caller m_CallSimulate;
	ScriptUtils::Calling::Caller m_CallSetCommand;
	ScriptUtils::Calling::Caller m_CallDraw;
	ScriptUtils::Calling::Caller m_CallDebugPrint;
	//ScriptMethod msm_Simulate;
	//ScriptMethod msm_SetCommand;
	//ScriptMethod msm_Draw;
	//ScriptMethod msm_DebugPrint;

	SlotContainer m_InputSlots;

	bool m_quit;

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

			////////////////////
			// Scripting Manager
			m_ScriptManager = new ScriptingEngine();
			asIScriptEngine* asEngine = m_ScriptManager->GetEnginePtr();

			registerScriptTypes(m_ScriptManager);
			RegisterPhysicsTypes(asEngine);
		
			////////////////////
			// Resource Manager
			m_ResMan = new ResourceManager();
			//m_ResMan->Configure();
			//m_ResMan->AddResourceLoader(new XMLLoader());
			//m_ResMan->AddResourceLoader(new TextLoader());

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

			m_InputSlots.connect( m_Input->SignalInputChanged, this, &GUIHUDTest::OnInputEvent );

			///////////////////////
			// Register globals and some more types
			m_World = new PhysicsWorld();
			m_ScriptManager->RegisterGlobalObject("World world", m_World);

			//m_ResMan->RegisterScriptElements(m_ScriptManager);
			console->RegisterScriptElements(m_ScriptManager);

			RegisterScriptedConsoleCommand(m_ScriptManager->GetEnginePtr());

			////////////////////////////
			// GUI state & StateManager
			GUI::Register(m_ScriptManager);
			GUI *gui = new GUI(dispWindow);
			SystemsManager *stateman = new SystemsManager();
			stateman->AddSystem(GUI::getSingletonPtr());

			gui->SetModule(m_ScriptManager, "main");

			////////////////
			// Phys World
			m_World->SetMaxVelocity(20);
			m_World->SetDamping(0.8f);
			m_World->SetBodyDeactivationPeriod(10000);
			m_World->SetDeactivationVelocity(0.05f);
			m_World->SetBitmaskRes(4);
			m_World->DeactivateWrapAround();
			m_World->SetGCForDebugDraw(dispWindow.get_gc());


			//////////////////
			// Load some code
			//std::string shipScript = OpenString_PhysFS(L"ship.as");
			// Compile the code
			//m_ScriptManager->AddCode(shipScript, "main", "ship.as");
			m_ScriptManager->AddFile("ship.as", "main");
			if (!m_ScriptManager->BuildModule("main"))
			{
				delete logger;
				conWindow.display_close_message();
				return 1;
			}

			/////////////////////////
			// Get script references
			mso_Ship = m_ScriptManager->CreateObject("main", "ship");

			ScriptUtils::Calling::Caller preload = mso_Ship.GetCaller("void Preload()");
			preload();
			//ScriptMethod preload = m_ScriptManager->GetClassMethod(mso_Ship, "void Preload()");
	//#ifdef _DEBUG
	//		preload.SetTimeout(300000); // Give it some extra time to run
	//#else
	//		preload.SetTimeout(10000);
	//#endif
			//m_ScriptManager->Execute(mso_Ship, preload);

			//msm_Simulate = m_ScriptManager->GetClassMethod(mso_Ship, "void Simulate(uint)");
			//msm_SetCommand = m_ScriptManager->GetClassMethod(mso_Ship, "void SetCommand(Command)");
			//msm_Draw = m_ScriptManager->GetClassMethod(mso_Ship, "void Draw()");
			m_CallSimulate = mso_Ship.GetCaller("void Simulate(uint)");
			//m_CallSetCommand = mso_Ship.GetCaller("void SetCommand(Command)");
			m_CallDraw = mso_Ship.GetCaller("void Draw()");

			//msm_DebugPrint = m_ScriptManager->GetClassMethod(mso_Ship, "void DebugOutput()");
			m_CallDebugPrint = mso_Ship.GetCaller("void DebugOutput()");


			unsigned int inputTimer = 0;
			bool debug = true;
			
			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;

			m_quit = false;

			while (!m_quit)
			{
				dispWindow.get_gc().clear();

				split = CL_System::get_time() - lastframe;
				lastframe = CL_System::get_time();

				if (CL_DisplayMessageQueue::has_messages())
					CL_DisplayMessageQueue::process();

				if (split < 100)
				{
					stateman->Update(split);
					// Move the ships
					Update(split);
				}

				m_CallDraw();
				stateman->Draw();

				m_ScriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);

				dispWindow.flip();
			}

			m_CallDebugPrint.release();
			m_CallDraw.release();
			m_CallSimulate.release();
			{
				mso_Ship.GetCaller("void DeleteScriptElements()")();
			}
			m_ScriptManager->GetEnginePtr()->GarbageCollect();
			gui->GetContext()->GetDocument("console_doc")->Close();
			mso_Ship.Release();

			m_ScriptManager->GetEnginePtr()->GarbageCollect();

			delete stateman;
			delete m_Input;
			delete m_ScriptManager;

		}
		catch (FusionEngine::Exception& ex)
		{
			std::cout << ex.ToString();
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

	~GUIHUDTest()
	{
	}

private:
	bool Update(unsigned int split)
	{
		//Command cmd;
		//cmd.m_Thrust = m_Input->IsButtonDown("Thrust", 0);
		//cmd.m_Left = m_Input->IsButtonDown("Left", 0);
		//cmd.m_Right = m_Input->IsButtonDown("Right", 0);
		//cmd.m_PrimaryFire = m_Input->IsButtonDown("PrimaryFire", 0);

		//ScriptObject cmd = m_ScriptManager->CreateObject(0, "Command");
		//// TODO: write the script fn generator to create the function which will create
		////  command objects (in InputPL)

		//asIScriptObject *cmdStruct = cmd.GetScriptObject();
		//bool bTrue = true;
		//int propertyCount = cmdStruct->GetPropertyCount();
		//for (int i = 0; i < propertyCount; i++)
		//{
		//	std::string prop(cmdStruct->GetPropertyName(i));
		//	if (prop == "delta")
		//		continue;

		//	if (m_Input->IsButtonDown(1, prop))
		//	{
		//		bool *someCmd = (bool*)cmdStruct->GetPropertyPointer(i);
		//		someCmd = &bTrue;
		//	}
		//}

		//bool *someCmd = (bool*)cmdStruct->GetPropertyPointer(0);
		//someCmd = &bTrue;

		//m_ScriptManager->Execute(mso_Ship, msm_SetCommand, cmdStruct);

		//m_ScriptManager->Execute(mso_Ship, msm_Simulate, split);

		m_CallSimulate(split);

		m_World->RunSimulation((float)split);			

		return true;
	}

	void registerScriptTypes(ScriptingEngine* manager)
	{
		//int r;
		//asIScriptEngine* engine = manager->GetEnginePtr();

		//RegisterType<Command>("Command", engine);

		//r = engine->RegisterObjectProperty("Command", "bool thrust", offsetof(Command, m_Thrust));
		//assert(r >= 0 && "Failed to register object type");
		//engine->RegisterObjectProperty("Command", "bool left", offsetof(Command, m_Left));
		//engine->RegisterObjectProperty("Command", "bool right", offsetof(Command, m_Right));
		//engine->RegisterObjectProperty("Command", "bool primary_fire", offsetof(Command, m_PrimaryFire));
		//engine->RegisterObjectProperty("Command", "bool button_delta", offsetof(Command, m_ButtonDelta));
	}

	void OnInputEvent(const InputEvent &e)
	{
		if (e.Type == InputEvent::Binary && e.Down)
		{
			if (e.Input == "Debug")
				//m_ScriptManager->Execute(mso_Ship, msm_DebugPrint);
				m_CallDebugPrint();

			if (e.Input == "Quit")
				m_quit = true;
		}
	}
};

class EntryPoint
{
public:
	static int main(const std::vector<CL_String> &args)
	{
		GUIHUDTest app;
		return app.main(args);
	}
};

CL_ClanApplication app(&EntryPoint::main);
