#include "../FusionEngine/Common.h"

#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

#include "../FusionEngine/FusionInputHandler.h"

#include "../FusionEngine/FusionStateManager.h"
#include "../FusionEngine/FusionGUI.h"
#include "../FusionEngine/FusionConsoleGUI.h"

#include "../FusionEngine/FusionScriptingEngine.h"

#include "../FusionEngine/FusionResource.h"
#include "../FusionEngine/FusionResourcePointer.h"
#include "../FusionEngine/FusionResourceLoader.h"

#include <boost/shared_ptr.hpp>

using namespace FusionEngine;

class ImageLoader : public ResourceLoader
{
public:
	ImageLoader()
	{
	}

	const std::string &GetType() const
	{
		static std::string strType("IMAGE");
		return strType;
	}

	Resource* LoadResource(const std::string& tag, const std::string &path)
	{
		Resource* rsc = new Resource(GetType().c_str(), tag, path, new CL_Surface(path));
		return rsc;
	}

	void ReloadResource(Resource* resource)
	{
		if (resource->IsValid())
		{
			delete resource->GetDataPtr();
		}

		resource->SetDataPtr(new CL_Surface(resource->GetPath()));

		resource->_setValid(true);
	}

	void UnloadResource(Resource* resource)
	{
		if (resource->IsValid())
			delete resource->GetDataPtr();
		resource->SetDataPtr(NULL);

		resource->_setValid(false);
	}

};

// Function implementation with native calling convention
void PrintString(std::string &str)
{
	SendToConsole(str);
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

class GUITest : public CL_ClanApplication
{
public:
	typedef std::map<std::string, boost::shared_ptr<Resource>> ResourceList;

	// ResourceManager properties
	ResourceList m_Resources;
	ImageLoader* m_ImgLoader;

	// Entity properties
	ResourcePointer<CL_Surface> m_ImageA, m_ImageB, m_ImageC;
	int m_NextImage;

	bool m_Quit;

	// Loads a resource
	void LoadResource(std::string &path)
	{
		SendToConsole("Loading: " + path);

		Resource* res = m_ImgLoader->LoadResource(path, path);

		m_Resources[path] = ( boost::shared_ptr<Resource>(res) );

		switch (m_NextImage)
		{
		case 0:
			m_ImageA = ResourcePointer<CL_Surface>(res);
			break;
		case 1:
			m_ImageB = ResourcePointer<CL_Surface>(res);
			break;
		case 2:
			m_ImageC = ResourcePointer<CL_Surface>(res);
			break;
		};
		++m_NextImage;

		SendToConsole("Done loading " + path);
	}

	// Unloads a resource
	void UnloadResource(std::string &tag)
	{
		SendToConsole("Unloading: " + tag);

		Resource* res = m_Resources[tag].get();
		if (res == NULL)
		{
			SendToConsole("Resource '" + tag + "' doesn't exist, aborting.");
			return;
		}

		m_ImgLoader->UnloadResource(res);
		m_Resources.erase(tag);

		--m_NextImage;

		SendToConsole("Unloaded " + tag);
	}

	void Quit()
	{
		m_Quit = true;
	}

	virtual int main(int argc, char **argv)
	{
		m_Quit = false;

		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;

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

			new ScriptingEngine;
			int r;
			if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
			{
				r = ScriptingEngine::getSingleton().GetEnginePtr()->RegisterGlobalFunction("void Print(string &in)", asFUNCTION(PrintString), asCALL_CDECL); assert( r >= 0 );
				r = ScriptingEngine::getSingleton().GetEnginePtr()->RegisterGlobalFunction("void Load(string &in)", asFUNCTION(StaticLoadResource), asCALL_CDECL); assert( r >= 0 );
				r = ScriptingEngine::getSingleton().GetEnginePtr()->RegisterGlobalFunction("void Unload(string &in)", asFUNCTION(StaticUnloadResource), asCALL_CDECL); assert( r >= 0 );
				r = ScriptingEngine::getSingleton().GetEnginePtr()->RegisterGlobalFunction("void Clear()", asFUNCTION(ClearConsole), asCALL_CDECL); assert( r >= 0 );
				r = ScriptingEngine::getSingleton().GetEnginePtr()->RegisterGlobalFunction("void Quit()", asFUNCTION(StaticQuit), asCALL_CDECL); assert( r >= 0 );
				r = ScriptingEngine::getSingleton().GetEnginePtr()->RegisterGlobalFunction("void Exit()", asFUNCTION(StaticQuit), asCALL_CDECL); assert( r >= 0 );
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


			StateManager* stateman = new StateManager();

			GUI* gui = new GUI(&display);
			stateman->AddState(gui);

			ConsoleGUI* conGUI = new ConsoleGUI();
			conGUI->Initialise();
			//stateman->AddState(conGUI);

			// Load a resource
			m_NextImage = 0;
			m_ImgLoader = new ImageLoader();
			LoadResource(std::string("body.png"));


			bool p1thrusting = false;
			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;
			// Loop thing
			while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE) && !m_Quit)
			{
				display.get_gc()->clear(CL_Color(180, 220, 255));

				split = CL_System::get_time() - lastframe;
				lastframe = CL_System::get_time();

				if (input->IsButtonDown("P1Thrust") != p1thrusting)
				{
					p1thrusting = !p1thrusting;
					if (p1thrusting)
						SendToConsole("P1 is moving!");
					else
						SendToConsole("P1 has stopped moving");
				}

				// Reload the GUI
				if (input->IsButtonDown("LetterR"))
				{
					conGUI->CleanUp();
					gui->CleanUp();
					gui->Initialise();
					conGUI->Initialise();
				}
								
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

				stateman->Update(split);
				stateman->Draw();
				//GUI::getSingleton().Update(1);
				//GUI::getSingleton().Draw();

				display.flip();
				CL_System::keep_alive();

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