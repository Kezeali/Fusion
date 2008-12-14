#include "../FusionEngine/Common.h"
#include "../FusionEngine/FusionCommon.h"

#include "../FusionEngine/FusionShapeMesh.h"
#include "../FusionEngine/FusionPhysicsWorld.h"
#include "../FusionEngine/FusionPhysicsBody.h"
#include "../FusionEngine/FusionPhysicsTypes.h"
#include "../FusionEngine/FusionPhysicsCallback.h"

#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

#include "../FusionEngine/FusionInputHandler.h"

#include "../FusionEngine/FusionStateManager.h"
#include "../FusionEngine/FusionGUI.h"
#include "../FusionEngine/FusionConsoleGUI.h"

#include "../FusionEngine/FusionScriptingEngine.h"
#include "../FusionEngine/FusionScriptTypeRegistrationUtils.h"
#include "../FusionEngine/FusionPhysicsScriptTypes.h"

#include "../FusionEngine/FusionResource.h"
#include "../FusionEngine/FusionResourcePointer.h"
#include "../FusionEngine/FusionResourceLoader.h"
#include "../FusionEngine/FusionXMLLoader.h"
#include "../FusionEngine/FusionTextLoader.h"
#include "../FusionEngine/FusionAudioLoader.h"
#include "../FusionEngine/FusionImageLoader.h"
#include "../FusionEngine/FusionResourceManager.h"

#include "../FusionEngine/FusionInputPluginLoader.h"
#include "../FusionEngine/FusionClientOptions.h"

//#include "../FusionEngine/FusionCommand.h"

#include <boost/smart_ptr.hpp>

using namespace FusionEngine;

//class OutputUserData : public ICollisionHandler
//{
//private:
//	PhysicsBody* m_MyBody;
//
//public:
//	OutputUserData(PhysicsBody* body)
//		: m_MyBody(body)
//	{
//	}
//
//	bool CanCollideWith(const PhysicsBody *other)
//	{
//		return (other != m_MyBody);
//	}
//
//	void CollisionWith(const PhysicsBody *other, const std::vector<Contact> &contacts)
//	{
//		if (other->GetUserData() != NULL)
//		{
//			char *data = (char *)(other->GetUserData());
//			std::cout << "Egads! " << data << " got me!" << std::endl;
//		}
//	}
//
//};
//
//class Projectile
//{
//	PhysicsBody* m_Body;
//
//public:
//	Projectile(PhysicsBody* body)
//		: m_Body(body),
//		m_Detonated(false)
//	{}
//
//	bool m_Detonated;
//
//	const Vector2& GetPosition() const
//	{
//		return m_Body->GetPosition();
//	}
//
//	bool IsDetonated() const
//	{
//		return m_Detonated;
//	}
//
//	PhysicsBody* GetBody() const
//	{
//		return m_Body;
//	}
//
//};

//typedef CL_SharedPtr<Projectile> ProjectilePtr;
//class GUIOverlayTest;
//
//class Explosive : public ICollisionHandler
//{
//	GUIOverlayTest* m_Env;
//	ProjectilePtr m_MyBody;
//	bool m_HasExploded;
//	float m_Payload;
//
//public:
//	Explosive(GUIOverlayTest* env, ProjectilePtr body, float payload)
//		: m_Env(env),
//		m_MyBody(body),
//		m_Payload(payload),
//		m_HasExploded(false)
//	{
//	}
//
//	bool CanCollideWith(const PhysicsBody *other)
//	{
//		return true;
//	}
//
//	void CollisionWith(const PhysicsBody *other, const std::vector<Contact> &contacts);
//};


class GUIHUDTest : public CL_ClanApplication
{
public:
	//GUIHUDTest() : CL_ClanApplication() {}
private:
	PhysicsWorld *m_World;
	InputManager *m_Input;
	ScriptingEngine *m_ScriptManager;
	ResourceManager *m_ResMan;

	ScriptObject mso_Ship;
	ScriptMethod msm_Simulate;
	ScriptMethod msm_SetCommand;
	ScriptMethod msm_Draw;
	ScriptMethod msm_DebugPrint;

	bool Update(unsigned int split)
	{
		//Command cmd;
		//cmd.m_Thrust = m_Input->IsButtonDown("Thrust", 0);
		//cmd.m_Left = m_Input->IsButtonDown("Left", 0);
		//cmd.m_Right = m_Input->IsButtonDown("Right", 0);
		//cmd.m_PrimaryFire = m_Input->IsButtonDown("PrimaryFire", 0);

		ScriptObject cmd = m_ScriptManager->CreateObject(0, "Command");
		// TODO: write the script fn generator to create the function which will create
		//  command objects (in InputPL)

		asIScriptStruct *cmdStruct = cmd.GetScriptStruct();
		bool bTrue = true;
		int propertyCount = cmdStruct->GetPropertyCount();
		for (int i = 0; i < propertyCount; i++)
		{
			std::string prop(cmdStruct->GetPropertyName(i));
			if (prop == "delta")
				continue;

			if (m_Input->IsButtonDown(prop, 1))
			{
				bool *someCmd = (bool*)cmdStruct->GetPropertyPointer(i);
				someCmd = &bTrue;
			}
		}

		bool *someCmd = (bool*)cmdStruct->GetPropertyPointer(0);
		someCmd = &bTrue;

		m_ScriptManager->Execute(mso_Ship, msm_SetCommand, cmdStruct);

		m_ScriptManager->Execute(mso_Ship, msm_Simulate, split);

		m_World->RunSimulation(split);

		if (m_Input->IsButtonDown("Debug", 0))
			m_ScriptManager->Execute(mso_Ship, msm_DebugPrint);

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

public:
	virtual int main(int argc, char **argv)
	{
		CL_SetupCore core_setup;
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;
		CL_SetupSound sound_setup;
		CL_SetupVorbis voirbis_setup;

		CL_SoundOutput sound_output(44100);

		CL_ConsoleWindow conWindow("Console", 80, 10);
		conWindow.redirect_stdio();

		CL_DisplayWindow dispWindow("Display", 800, 600);

		Logger* logger = 0;
		ConsoleStdOutWriter* cout = 0;
		Console* console = 0;

		try
		{
			console = new Console;
			cout = new ConsoleStdOutWriter();
			cout->Activate();
			logger = new Logger(false);

			////////////////////
			// Scripting Manager
			m_ScriptManager = new ScriptingEngine;
			asIScriptEngine* asEngine = m_ScriptManager->GetEnginePtr();

			registerScriptTypes(m_ScriptManager);
			RegisterPhysicsTypes(asEngine);
		
			////////////////////
			// Resource Manager
			m_ResMan = new ResourceManager(argv[0]);
			m_ResMan->Configure();
			m_ResMan->AddResourceLoader(new XMLLoader());
			m_ResMan->AddResourceLoader(new TextLoader());

			//////////////////////
			// Load client options
			ClientOptions* co = new ClientOptions("clientoptions.xml");

			if (co->GetOption_bool("console_logging"))
				logger->ActivateConsoleLogging();

			/////////////////
			// Input Manager
			m_Input = new InputManager(&dispWindow);

			if (!m_Input->Test())
				throw CL_Error("Input manager could not be started.");
			m_Input->Initialise(m_ResMan, co);
			SendToConsole("Input manager started successfully");

			///////////////////////
			// Register globals and some more types
			m_World = new PhysicsWorld();
			m_ScriptManager->RegisterGlobalObject("World world", (void*) m_World);

			m_ResMan->RegisterScriptElements(m_ScriptManager);
			console->RegisterScriptElements(m_ScriptManager);

			////////////////////////////
			// GUI state & StateManager
			CL_OpenGLState state(dispWindow.get_gc());
			state.set_active(); // Makes sure GC is set correctly

			new GUI(&dispWindow);
			StateManager *stateman = new StateManager();
			stateman->AddState(GUI::getSingletonPtr());

			boost::shared_ptr<ConsoleGUI> conGUI = boost::shared_ptr<ConsoleGUI>(new ConsoleGUI());
			//stateman->AddState(conGUI);
			conGUI->Initialise();

			////////////////
			// Phys World
			m_World->Initialise(800, 600);
			m_World->SetMaxVelocity(20);
			m_World->SetDamping(0.8f);
			m_World->SetBodyDeactivationPeriod(10000);
			m_World->SetDeactivationVelocity(0.05f);
			m_World->SetBitmaskRes(4);
			m_World->DeactivateWrapAround();

			/////////////////////////
			// Load inputs (controls)
			//m_Input->MapControl('W', "Thrust", 0);
			//m_Input->MapControl('A', "Left", 0);
			//m_Input->MapControl('D', "Right", 0);
			//m_Input->MapControl(CL_KEY_SPACE, "PrimaryFire", 0);

			//m_Input->MapControl('B', "Debug", 0);

			//InputPluginLoader ipl;
			//{
			//	ResourcePointer<TiXmlDocument> inputDoc = m_ResMan->GetResource<TiXmlDocument>("input/coreinputs.xml");
			//	ipl.LoadInputs(inputDoc.GetDataPtr());
			//}
			//ipl.CreateCommandClass(m_ScriptManager);

			//////////////////
			// Load some code
			ResourcePointer<std::string> shipScript = m_ResMan->GetResource<std::string>("ship.as");
			if (!shipScript.IsValid())
				throw CL_Error("Oh snap! Couldn't load ship.as!");
			// Compile the code
			m_ScriptManager->AddCode(shipScript->c_str(), 0);
			if (!m_ScriptManager->BuildModule(0))
			{
				conWindow.display_close_message();
				return 1;
			}

			/////////////////////////
			// Get script references
			mso_Ship = m_ScriptManager->CreateObject(0, "ship");

			ScriptMethod preload = m_ScriptManager->GetClassMethod(mso_Ship, "void Preload()");
	#ifdef _DEBUG
			preload.SetTimeout(300000); // Give it some extra time to run
	#else
			preload.SetTimeout(10000);
	#endif
			m_ScriptManager->Execute(mso_Ship, preload);

			msm_Simulate = m_ScriptManager->GetClassMethod(mso_Ship, "void Simulate(uint)");
			msm_SetCommand = m_ScriptManager->GetClassMethod(mso_Ship, "void SetCommand(Command)");
			msm_Draw = m_ScriptManager->GetClassMethod(mso_Ship, "void Draw()");

			msm_DebugPrint = m_ScriptManager->GetClassMethod(mso_Ship, "void DebugOutput()");


			unsigned int inputTimer = 0;
			bool debug = true;
			
			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;

			while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE))
			{
				dispWindow.get_gc()->clear(CL_Color(0, 0, 0));

				split = CL_System::get_time() - lastframe;
				lastframe = CL_System::get_time();

				CL_System::keep_alive();

				stateman->Update(split);
				// Move the ships
				Update(split);


				// Current time
				unsigned int time = CL_System::get_time();


				// Draw the terrain
				//m_TerrainGraphical->draw(
				//	m_TerrainPhysical->GetPosition().x, m_TerrainPhysical->GetPosition().y
				//	);

				m_ScriptManager->Execute(mso_Ship, msm_Draw);

				
				stateman->Draw();

				dispWindow.flip();
			}

		}
		catch (FusionEngine::Exception& ex)
		{
			std::cout << ex.ToString();
			conWindow.display_close_message();
		}
		catch (CL_Error& ex)
		{
			std::cout << ex.message;
			conWindow.display_close_message();
		}

		if (logger != 0)
			delete logger;
		if (cout != 0)
			delete cout;
		if (console != 0)
			delete console;


		return 0;
	}

} app;
