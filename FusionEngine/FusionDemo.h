/*
  Copyright (c) 2006 Elliot Hayward

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef Header_Fusion_Info
#define Header_Fusion_Info

#if _MSC_VER > 1000
#pragma once
#endif

/// Fusion
#include "FusionClientOptions.h"
#include "FusionServerOptions.h"

#include "FusionGame.h"

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
		FusionEngine::ClientOptions *m_ClientOpts;
		FusionEngine::ServerOptions *m_ServerOpts;

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