#include "../FusionEngine/Common.h"

#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

#include "../FusionEngine/FusionInputHandler.h"

#include "../FusionEngine/FusionStateManager.h"
#include "../FusionEngine/FusionGUI.h"
#include "../FusionEngine/FusionConsoleGUI.h"

#include "../FusionEngine/FusionScriptingEngine.h"
#include "../FusionEngine/FusionScriptTypeRegistrationUtils.h"

#include "../FusionEngine/FusionResource.h"
#include "../FusionEngine/FusionResourcePointer.h"
#include "../FusionEngine/FusionResourceLoader.h"
#include "../FusionEngine/FusionXMLLoader.h"
#include "../FusionEngine/FusionTextLoader.h"
#include "../FusionEngine/FusionAudioLoader.h"
#include "../FusionEngine/FusionImageLoader.h"
#include "../FusionEngine/FusionResourceManager.h"

#include <boost/smart_ptr.hpp>

using namespace FusionEngine;

// Function implementation with native calling convention
void PrintString(std::string &str)
{
	SendToConsole("Script says: " + str);
}

// Function implementation with generic script interface
void PrintString_Generic(asIScriptGeneric *gen)
{
	std::string *str = (std::string*)gen->GetArgAddress(0);
	SendToConsole(*str);
}

void ClearConsole()
{
	Console* con = Console::getSingletonPtr();
	if (con != NULL)
		con->Clear();
}

void StaticLoadResource(std::string& path);
void StaticUnloadResource(std::string& tag);
void StaticQuit();

static const char *script1 =
"class ship                                    \n"
"{                                             \n"
"    Image body;                               \n"
"    Sound engineSound;                        \n"
"    void Preload()                            \n"
"    {                                         \n"
"        body = GetImage(\"body.png\");        \n"
"        engineSound =GetSound(\"engine.wav\");\n"
"        Print(\"Preloaded\");                 \n"
"    }                                         \n"
"    void Draw(float x, float y)               \n"
"    {                                         \n"
"        body.draw(x, y);                      \n"
"    }                                         \n"
"    void Update(uint dt)                      \n"
"    {                                         \n"
"                                              \n"
"    }                                         \n"
"};                                            \n"
"void Test()                                   \n"
"{                                             \n"
"   GetImage(\"body.png\").get().draw(10, 10); \n"
"}                                             \n"
"                                              \n";

	class Command
	{
	public:
		Command() {}

	public:
		bool m_Thrust;
		bool m_Left;
		bool m_Right;
		bool m_PrimaryFire;
		bool m_SecondaryFire;
		bool m_SpecialFire;
		
	};

class GUITest : public CL_ClanApplication
{
public:
	//typedef std::vector<boost::shared_ptr<ResourcePointer>> DataList;

	//// ResourceManager properties
	//ResourceList m_Resources;
	//ImageLoader* m_ImgLoader;

	//// Entity properties
	ResourcePointer<CL_Surface> m_ImageA, m_ImageB, m_ImageC;
	ResourceManager* m_ResMan ;
	int m_NextImage;

	bool m_Quit;

	// Loads a resource
	void LoadResource(std::string &path)
	{
		SendToConsole("Loading: " + path);

		try
		{
			switch (m_NextImage)
			{
			case 0:
				m_ImageA = m_ResMan->GetResource<CL_Surface>(path);
				break;
			case 1:
				m_ImageB = m_ResMan->GetResource<CL_Surface>(path);
				break;
			case 2:
				m_ImageC = m_ResMan->GetResource<CL_Surface>(path);
				break;
			};
			++m_NextImage;

			SendToConsole("Done loading " + path);

		}
		catch (FileSystemException& e)
		{
			SendToConsole("Failed to load: " + e.GetDescription());
		}

	}

	// Unloads a resource
	void UnloadResource(std::string &tag)
	{
		SendToConsole("Unloading: " + tag);

		if (m_ImageA.GetTag() == tag)
			m_ImageA.Release();

		else if (m_ImageB.GetTag() == tag)
			m_ImageB.Release();

		else if (m_ImageC.GetTag() == tag)
			m_ImageC.Release();

		m_ResMan->DisposeUnusedResources();

		SendToConsole("Unloaded " + tag);
	}

	ResourcePointer<CL_Surface> Get(std::string& path)
	{
		return m_ResMan->GetResource<CL_Surface>(path);
	}

	void Quit()
	{
		m_Quit = true;
	}

	virtual int main(int argc, char **argv)
	{
		m_Quit = false;

		CL_SetupCore core_setup;
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;
		CL_SetupSound sound_setup;
		CL_SetupVorbis voirbis_setup;

		CL_SoundOutput sound_output(44100);

		CL_ConsoleWindow console("GUI Test");
		console.redirect_stdio();

		CL_DisplayWindow display("GUI Test: Display", 800, 600);

		Logger* logger = 0;
		ConsoleStdOutWriter* cout = 0;

		try
		{
			new Console;
			cout = new ConsoleStdOutWriter();
			cout->Activate();
			logger = new Logger(true);
			// An unrelated test
			//logger->TagLink("console", "con");
			//logger->Add("Testing", "con");

			//CL_OpenGLState gl_state(display.get_gc());
			//gl_state.set_active();

			ScriptingEngine* scEngW = new ScriptingEngine;
			asIScriptEngine* scrEngine = ScriptingEngine::getSingleton().GetEnginePtr();
			int r;
			if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
			{
				r = scrEngine->RegisterGlobalFunction("void Print(string &in)", asFUNCTION(PrintString), asCALL_CDECL); assert( r >= 0 );
				r = scrEngine->RegisterGlobalFunction("void Clear()", asFUNCTION(ClearConsole), asCALL_CDECL); assert( r >= 0 );

				r = scrEngine->RegisterGlobalFunction("void Quit()", asFUNCTION(StaticQuit), asCALL_CDECL); assert( r >= 0 );
				r = scrEngine->RegisterGlobalFunction("void exit()", asFUNCTION(StaticQuit), asCALL_CDECL); assert( r >= 0 );


				RegisterType<Command>("Command", scrEngine);

				r = scrEngine->RegisterObjectProperty("Command", "bool thrust", offsetof(Command, m_Thrust));
				assert(r >= 0 && "Failed to register object type");
				scrEngine->RegisterObjectProperty("Command", "bool left", offsetof(Command, m_Left));
				scrEngine->RegisterObjectProperty("Command", "bool right", offsetof(Command, m_Right));
				scrEngine->RegisterObjectProperty("Command", "bool primary_fire", offsetof(Command, m_PrimaryFire));
			}
			else
			{
				r = ScriptingEngine::getSingleton().GetEnginePtr()->RegisterGlobalFunction("void Print(string &in)", asFUNCTION(PrintString_Generic), asCALL_GENERIC); assert( r >= 0 );
			}


			FusionInput* input = new FusionInput();
			if (!input->Test())
				throw CL_Error("Input handler could not be started.");
			input->Initialise();
			SendToConsole("InputHandler started successfully");

			input->MapControl(CL_KEY_UP, "P1Thrust");
			input->MapControl('R', "LetterR");
			input->MapControl(CL_KEY_CONTROL, "KeyCtrl");
			input->MapControl(CL_KEY_BACKSPACE, "DeleteResource");

			m_ResMan = new ResourceManager(argv);
			m_ResMan->AddResourceLoader(new XMLLoader());
			m_ResMan->AddResourceLoader(new TextLoader());

			m_ResMan->RegisterScriptElements(scEngW);

			//scEngW->AddCode(script1, 0);
			//ResourcePointer<std::string> shipScript = m_ResMan->GetResource<std::string>("ship.as");
			//if (!shipScript.IsValid())
			//	throw CL_Error("Oh snap! Couldn't load ship.as!");
			//scEngW->AddCode(shipScript->c_str(), 0);
			//scEngW->BuildModule(0);

			StateManager* stateman = new StateManager();

			// Make sure GC is set correctly
			CL_OpenGLState state(display.get_gc());
			state.set_active();
			GUI* gui = new GUI(&display);
			stateman->AddState(gui);

			ConsoleGUI* conGUI = new ConsoleGUI();
			conGUI->Initialise();
			//stateman->AddState(conGUI);

			// Load a resource
			m_ResMan->PretagResource("IMAGE", "body.png", "body.png");


			//ScriptClass shipClass = scEngW->GetClass(0, "ship");
			//ScriptObject ship = shipClass.Instantiate();

			//ScriptMethod preload = scEngW->GetClassMethod(ship, "void Preload()");//shipClass.GetMethod("void Preload()");
			//preload.SetTimeout(10000);
			//scEngW->Execute(ship, preload);

			//ScriptMethod draw = shipClass.GetMethod("void Draw()");
			//ScriptMethod simulate = shipClass.GetMethod("void Simulate(uint)");
			//ScriptMethod setCommand = shipClass.GetMethod("void SetCommand(Command)");
			//int shipTypeId = scrEngine->GetTypeIdByDecl(0, "ship");
			//asIScriptStruct* shipObject = (asIScriptStruct*)scrEngine->CreateScriptObject(shipTypeId);

			//int preloadId = scrEngine->GetMethodIDByDecl(shipTypeId, "void Preload()");
			//asIScriptContext* context = scrEngine->CreateContext();
			//context->Prepare(preloadId);
			//context->SetObject(shipObject);
			//context->Execute();

			//int drawId = scrEngine->GetMethodIDByDecl(shipTypeId, "void Draw()");
			//int updateID = scrEngine->GetMethodIDByDecl(shipTypeId, "void Update(uint dt)");


			bool p1thrusting = false;
			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;
			// Loop thing
			while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE) && !m_Quit)
			{
				// Catch failures to load resources
				try {
				CL_System::keep_alive();
				} catch (FusionEngine::FileSystemException& ex) {
					SendToConsole(ex);
				}

				split = CL_System::get_time() - lastframe;
				lastframe = CL_System::get_time();

				// Reload the GUI
				//if (input->IsButtonDown("LetterR"))
				//{
				//	conGUI->CleanUp();
				//	gui->CleanUp();
				//	gui->Initialise();
				//	conGUI->Initialise();
				//}
								
				// Clear the display
				display.get_gc()->clear(CL_Color(180, 220, 255));
				//for (int i = 0; i < 10; i++)
				//{
				//	ResourcePointer<CL_Surface> &rscPtr = m_Images[i];
				//	if (rscPtr.IsValid())
				//		rscPtr->draw(50, 50);
				//}
				if (m_ImageA.IsValid())
					m_ImageA->draw(50, 50);
				if (m_ImageB.IsValid())
					m_ImageB->draw(50, 50);
				if (m_ImageC.IsValid())
					m_ImageC->draw(50, 50);
				

				//Command cmd;
				//cmd.m_Thrust = input->IsButtonDown("P1Thrust");
				//
				//scEngW->Execute(ship, setCommand, &cmd);

				//scEngW->Execute(ship, simulate, split);

				//if (!CL_Keyboard::get_keycode(CL_KEY_SPACE))
				//{
				//	scEngW->Execute(ship, draw);
				//}

				
				stateman->Update(split);
				stateman->Draw();

				display.flip(0);
				//gl_state.set_active();
			}

			delete stateman;
			delete conGUI;
		}
		catch (CL_Error& e)
		{
			// Something bad must have happened
			std::cout << e.message << std::endl;
			console.display_close_message();
		}

		if (logger != 0)
			delete logger;
		if (cout != 0)
			delete cout;
		delete Console::getSingletonPtr();

		// Zero!
		return 0;
	}
} app;

void StaticLoadResource(std::string &path)
{
	app.LoadResource(path);
}

void StaticUnloadResource(std::string &tag)
{
	app.UnloadResource(tag);
}

void StaticQuit()
{
	app.Quit();
}