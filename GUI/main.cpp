#include "..\FusionEngine\Common.h"

#include "..\FusionEngine\FusionConsole.h"
#include "..\FusionEngine\FusionConsoleStdOutWriter.h"
#include "..\FusionEngine\FusionLogger.h"

#include "..\FusionEngine\FusionStateManager.h"
#include "..\FusionEngine\FusionGUI.h"
#include "..\FusionEngine\FusionConsoleGUI.h"

using namespace FusionEngine;

class GUITest : public CL_ClanApplication
{
	virtual int main(int argc, char **argv)
	{
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;

		CL_ConsoleWindow console("GUI Test");
		console.redirect_stdio();

		CL_DisplayWindow* display = new CL_DisplayWindow("GUI Test: Display", 640, 480);

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

			CL_OpenGLState gl_state(display->get_gc());
			gl_state.set_active();

			StateManager* stateman = new StateManager();

			GUI* gui = new GUI();
			stateman->AddState(gui);

			ConsoleGUI* conGUI = new ConsoleGUI();
			conGUI->Initialise();
			//stateman->AddState(conGUI);

			//glEnable(GL_CULL_FACE);
			//glDisable(GL_FOG);
			//glClearColor(0.0f,0.0f,0.0f,1.0f);
			//glViewport(0,0, 640,480);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0f,(GLfloat)640/(GLfloat)480,0.1f,100.0f);


			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;
			// Loop thing
			while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE))
			{
				//display->get_gc()->clear(CL_Color(180, 220, 255));
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// This Will Clear The Background Color To Black
				glClearDepth(1.0);						// Enables Clearing Of The Depth Buffer
				glDepthFunc(GL_LESS);					// The Type Of Depth Test To Do
				glEnable(GL_DEPTH_TEST);				// Enables Depth Testing
				glShadeModel(GL_SMOOTH);				// Enables Smooth Color Shading

				glMatrixMode(GL_MODELVIEW);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
				glLoadIdentity();						// Reset The View


				split = CL_System::get_time() - lastframe;
				lastframe = CL_System::get_time();

				try
				{
					stateman->Update(split);
					stateman->Draw();
					//GUI::getSingleton().Update(1);
					//GUI::getSingleton().Draw();
				}
				catch (CEGUI::Exception& e)
				{
					SendToConsole(e.getMessage().c_str(), Console::MTERROR);
				}


				display->flip();
				CL_System::keep_alive(4);

				gl_state.set_active();
			}

			delete stateman;
			delete conGUI;
		}
		catch (CL_Error e)
		{
			// Something bad must have happened
			std::cout << e.message << std::endl;
			console.wait_for_key();
		}

		if (logger != 0)
			delete logger;
		if (cout != 0)
			delete cout;
		delete Console::getSingletonPtr();

		delete display;

		// Zero!
		return 0;
	}
} app;
