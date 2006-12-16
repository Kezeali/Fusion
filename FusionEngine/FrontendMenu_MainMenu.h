/*
  Copyright (c) 2006 FusionTeam

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

#ifndef Header_Fusion_FusionGUI_MainMenu
#define Header_Fusion_FusionGUI_MainMenu

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Fusion
#include "FusionGUI.h"

#include "FusionClientOptions.h"
#include "FusionServerOptions.h"

namespace Fusion
{

	/*!
	 * \brief
	 * GUI for fusion mainmenu
	 */
	class FusionGUI_MainMenu
	{
	public:
		//! Basic constructor
		FusionGUI_MainMenu();
		/*!
		* \brief
		* Constructor.
		*/
		FusionGUI_MainMenu(FusionEngine::ClientOptions *clientopts, FusionEngine::ServerOptions *serveropts);

	public:
		//! Init gui
		bool Initialise();

		//! Called when the Create button is clicked
		bool onCreateClicked(const CEGUI::EventArgs& e);
		//! Called when the Join button is clicked
		bool onJoinClicked(const CEGUI::EventArgs& e);
		//! Called when the Options button is clicked
		bool onOptsClicked(const CEGUI::EventArgs& e);

	protected:
		//! Options to send to the game
		FusionEngine::ClientOptions *m_ClientOpts;
		//! Options to send to the game
		FusionEngine::ServerOptions *m_ServerOpts;

	};

}

#endif
