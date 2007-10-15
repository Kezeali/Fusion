
#include "FusionDemo.h"

// Create a global instance.
Fusion::FusionDemo app;

namespace Fusion
{

	int FusionDemo::main(int argc, char **argv)
	{
		using namespace FusionEngine;

		try
		{
			CL_SetupCore setup_core;
			CL_SetupDisplay setup_display;
			CL_SetupGL setup_gl;

			// Create displaywindow
			CL_DisplayWindow display("Fusion Gameplay Demo", 640, 480);

			FusionGame *game = new FusionGame;

			game->Run(&display);

			delete game;

		}
		catch (CL_Error& err)
		{
			// TODO: cross-platform GUI error box (wxwidgets?)
			std::cerr << "Failed to initialise ClanLib: " << err.message.c_str() << std::endl;
			return 1;
		}

		return 0;
	}

	
		//FusionGUI *gui = new FusionGUI();

		//FusionGUI_MainMenu *mainmenu = new FusionGUI_MainMenu(m_ClientOpts, m_ServerOpts);
		//mainmenu->Initialise();

		/*unsigned int lastTime = CL_System::get_time();
		unsigned int split = 0;*/

		/*bool keep_going = true;*/

		// This loop remains in 'stasis' after the game has started (FusionGame 
		//  locks the execution with its own loop), until the game quits and
		//  FusionDemo regains control.
		//while (!m_Quit && keep_going)
		//{
		//	split = CL_System::get_time() - lastTime;
		//	lastTime = CL_System::get_time();

		//	keep_going = mainmenu->Update(split);
		//	mainmenu->Draw();

		//	CL_Display::flip();
		//	CL_System::keep_alive();
		//}

		/*delete mainmenu;
		delete gui;*/

}
