/*
  Copyright (c) 2006 Fusion Project Team

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

#ifndef Header_Fusion_ConsoleGUI
#define Header_Fusion_ConsoleGUI

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionBoostSignals2.h"


/// Inherited
#include "FusionState.h"

namespace FusionEngine
{

	//! Max length of the input history list
	static const size_t g_ConGUIDefaultMaxHistory = 255;

	//! Event Connections
	typedef std::list<CEGUI::Event::Connection> EventConnectionList;

	/*!
	 * \brief
	 * Connects the CEGUI console window with the Console class.
	 */
	class ConsoleGUI : public FusionState
	{
	public:
		//! Basic constructor.
		ConsoleGUI();

		//! Constructor +max_history
		ConsoleGUI(size_t max_history);

		//! Destructor
		~ConsoleGUI();

	public:
		//! Inits the gui
		virtual bool Initialise();

		//! Updates the inputs
		virtual bool Update(unsigned int split);

		//! Draws the gui
		virtual void Draw();

		//! Unbinds
		virtual void CleanUp();

		//! Sets the max number of items to keep in the entry history
		void SetMaxHistory(unsigned int max);
		//! Returns the max number of items that will be kept in the entry history.
		unsigned int GetMaxHistory() const;

		bool onMouseEnter(const CEGUI::EventArgs& e);
		bool onMouseLeave(const CEGUI::EventArgs& e);
		bool onEditBoxAccepted(const CEGUI::EventArgs& e);
		bool onEditBoxKeyUp(const CEGUI::EventArgs &e);

		void onConsoleNewData(const std::string &data);
		void onConsoleClear();

	protected:
		CEGUI::Window *m_Wind;
		CEGUI::Editbox *m_EditBox;
		CEGUI::MultiLineEditbox *m_HistoryBox;

		std::deque<CEGUI::String> m_History;
		int m_HistoryPos;
		bool m_RecentInHistory;

		size_t m_MaxHistory;

		//CL_SlotContainer m_Slots;
		boost::signals2::connection m_Connection_NewData;
		boost::signals2::connection m_Connection_Clear;

		EventConnectionList m_EventConnections;

		//! Enters text
		void enterText(CEGUI::String text);

	};

}

#endif
