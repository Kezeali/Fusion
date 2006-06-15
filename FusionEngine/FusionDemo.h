#ifndef Header_Fusion_Info
#define Header_Fusion_Info

#if _MSC_VER > 1000
#pragma once
#endif

/// Fusion
#include "FusionClientOptions.h"
#include "FusionServerOptions.h"

namespace Fusion
{

	/*!
	 * \brief
	 * A simplified form of the Fusion front-end.
	 *
	 * This is the gui menu system that will be used untill the gameplay system
	 * (FusionEngine) is completed. When the FusionEngine is completed, it will
	 * (read - might) be a shared library (for ease of updating), so a seperate
	 * frontend will need to be created.
	 */
	class FusionDemo : public CL_Application
	{
	public:
		FusionEngine::ClientOptions *ClientOpts;
		FusionEngine::ServerOptions *ServerOpts;

	private:
		CL_GUIManager *m_GuiManager;
		CL_Deck *m_GuiDeck;
		FusionGUI_MainMenu *m_GuiMenu;
		FusionGUI_Options *m_GuiOptions;

		bool m_Quit;

	public:
		virtual int main(int argc, char **argv);

		//! Quits the demo by setting m_Quit to true.
		void Quit() { m_Quit = true; }

		void RunClient();
		void RunServer();

	private:
		void setupGui();
		void on_guiPaint();
	};

}

#endif