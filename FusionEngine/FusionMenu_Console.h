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

#ifndef Header_FusionEngine_GUI_Console
#define Header_FusionEngine_GUI_Console

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionEngineGUI.h"

/// Fusion
#include "FusionInputHandler.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Displays and runs a console
	 */
	class GUI_Console : public GUI
	{
	public:
		//! Basic constructor.
		GUI_Console();

	public:
		//! Inits the gui
		bool Initialise();

		//! Updates the inputs
		bool Update(unsigned int split);

		//! Draws the gui
		void Draw();

		//! Unbinds
		void CleanUp();

		//! Runs on enter key presses to execute commands
		bool onEditBoxAccepted(const CEGUI::EventArgs& e);

		//! Runs on key up to scroll the history
		bool onEditBoxKeyUp(const CEGUI::EventArgs &e);

		//! Gathers data added to the console from other sources
		void onConsoleNewLine(const std::string &data);

	private:
		void enterText(const CEGUI::String &text);

	private:
		//! The current index in the history buffer.
		int m_HistoryPos;
		//! The raw history (for using up to enter previous commands)
		std::vector<CEGUI::String> m_History;

		//! True if recent is in history.
		/*!
		 * This gets set to true if the last edited text in the command box has been
		 * added to the history - i.e. the user scrolled up while they were typing it.
		 */
		bool m_RecentInHistory;

		//! Pointer to the root window for convenience.
		CEGUI::Window *m_Wind;
		//! Pointer to the command editbox for convenience.
		CEGUI::Editbox *m_EditBox;
		//! Pointer to the history multi-line editbox for convenience.
		CEGUI::MultiLineEditbox *m_HistoryBox;

		CL_Slot m_OnNewLineSlot;

	};

}

#endif
