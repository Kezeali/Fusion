
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
using namespace Fusion;

// Create a global instance.
Fusion::FusionDemo app;

int Fusion::FusionDemo::main(int argc, char **argv)
{
	try
	{
		CL_SetupCore setup_core;
		CL_SetupDisplay setup_display;
		CL_SetupGL setup_gl;
		CL_SetupGUI setup_gui;

		// Create displaywindow
		CL_DisplayWindow display("Fusion Gameplay Demo", 640, 480);

		// Setup managers, load resources, connect slots, etc.
		setupGui(&com_manager);


		// Initialise options
		m_ClientOpts = new ClientOptions();
		m_ServerOpts = new ServerOptions();

		while (!m_Quit)
		{
			CL_Display::flip();
			CL_System::keep_alive();
		}

		delete m_ClientOpts;
		delete m_ServerOpts;
	}
	catch (CL_Error err)
	{
		CL_MessageBox::info("Exception caught", err.message.c_str(), m_GuiManager, 0);
		std::cout << "Exception caught: " << err.message.c_str() << std::endl;
	}

	return 0;
}

void Fusion::FusionDemo::setupGui()
{
	CL_ResourceManager style_res("../FusionGUI/style.xml");
	CL_ResourceManager layout_res("../FusionGUI/FusionGUI.xml");

	// Don't know what this was ment to do
	//resources.add_resources(extra_resources);

	CL_StyleManager_Silver style(&style_res);
	CL_GUIManager gui_manager(&style);
	// Store a pointer to the manager for use in signal methods
	//! \remarks MCS: Since this is a pointer to a stack variable, wont
	//! it be automatically deleted when the function ends..? This could
	//! cause problems.
	m_GuiManager = &gui_manager;

	m_GuiDeck = new CL_Deck();

	m_GuiMenu = new FusionGUI_MainMenu(&layout_res, gui_manager, m_GuiDeck);
	m_GuiOptions = new FusionGUI_Options(&layout_res, gui_manager, m_GuiDeck);

	m_GuiDeck->add(FusionGUI_MainMenu::Name, m_GuiMenu->get_component());
	m_GuiDeck->add(FusionGUI_Options::Name, m_GuiOptions->get_component());

	CL_Slot slot_paint = gui.sig_paint().connect(this, &Fusion::FusionDemo::on_guiPaint);
}

void Fusion::FusionDemo::on_guiPaint()
{
	CL_Display::clear(CL_Color(150, 150, 220));
}

void Fusion::FusionDemo::RunClient()
{
		FusionGame *game = new FusionGame();
		game->RunClient("localhost", "4444", m_ClientOpts);
		delete game;
}

void Fusion::FusionDemo::RunServer()
{
		FusionGame *game = new FusionGame();
		game->RunServer("4444", m_ServerOpts);
		delete game;
}