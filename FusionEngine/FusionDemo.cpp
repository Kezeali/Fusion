
#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionClientOptions.h"
#include "FusionServerOptions.h"
#include "FusionGUI_MainMenu.h"
#include "FusionGUI_Options.h"

/// Class
#include "FusionDemo.h"

using namespace FusionEngine;

namespace Fusion
{
// Create a global instance.
Fusion::FusionDemo app;

int FusionDemo::main(int argc, char **argv)
{
	try
	{
		CL_SetupCore setup_core;
		CL_SetupDisplay setup_display;
		CL_SetupGL setup_gl;
		CL_SetupGUI setup_gui;

		// Create displaywindow
		CL_DisplayWindow display("Fusion Gameplay Demo", 640, 480);

		// Initialise options
		m_ClientOpts = new ClientOptions();
		m_ClientOpts->LoadFromFile("clientcfg.xml");
		m_ServerOpts = new ServerOptions();
		m_ServerOpts->LoadFromFile("servercfg.xml");

		FusionGUI_MainMenu *mainmenu = new FusionGUI_MainMenu(m_ClientOpts);
		mainmenu->Initialise();

		unsigned int lastTime = CL_System::get_time();
        unsigned int split = 0;

		while (!m_Quit)
		{
		    split = CL_System::get_time() - lastTime;
            lastTime = CL_System::get_time();

		    mainmenu->Update(split);
		    mainmenu->Draw();

			CL_Display::flip();
			CL_System::keep_alive();
		}

		delete m_ClientOpts;
		delete m_ServerOpts;
	}
	catch (CL_Error err)
	{
		std::cout << "Exception caught: " << err.message.c_str() << std::endl;
	}

	return 0;
}

void FusionDemo::RunClient()
{
		FusionGame *game = new FusionGame();
		game->RunClient("localhost", "4444", m_ClientOpts);
		delete game;
}

void FusionDemo::RunServer()
{
		FusionGame *game = new FusionGame();
		game->RunServer("4444", m_ServerOpts);
		delete game;
}

}
