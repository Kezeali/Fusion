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

#include "FusionConsoleGUI.h"

#include "FusionGUI.h"
#include "FusionConsole.h"
#include <CEGUI/CEGUI.h>

namespace FusionEngine
{

	ConsoleGUI::ConsoleGUI()
	{
	}

	/////////////////
	/// Public

	bool ConsoleGUI::Initialise()
	{
		using namespace CEGUI;

		WindowManager& winMgr = WindowManager::getSingleton();
		Window *console_sheet = winMgr.loadWindowLayout("Console.layout");


		m_Wind = static_cast<CEGUI::Window*> (
			winMgr.getWindow("Console/Wind")
			);
		m_EditBox = static_cast<CEGUI::Editbox*> (
			winMgr.getWindow("Console/Wind/Editbox")
			);
		m_HistoryBox = static_cast<CEGUI::MultiLineEditbox*> (
			winMgr.getWindow("Console/Wind/History")
			);

		// Subscribe to the KeyUp event to capture enter presses
		m_EditBox->subscribeEvent(
			Editbox::EventTextAccepted,
			Event::Subscriber(&ConsoleGUI::onEditBoxAccepted, this));

		// Connect to the newline signal from the console
		m_Slots.connect(Console::getSingleton().OnNewLine, this, &ConsoleGUI::onConsoleNewLine);


		// Fill the console with the past history
		Console::ConsoleLines lines = Console::getSingleton().GetHistory();

		Console::ConsoleLines::iterator it = lines.begin();
		for (; it != lines.end(); ++it)
		{
			enterText( (*it) );
		}

		GUI::getSingleton().AddWindow(console_sheet);
	}

	bool ConsoleGUI::Update(unsigned int split)
	{
	}

	void ConsoleGUI::Draw()
	{
	}

	void ConsoleGUI::CleanUp()
	{
		FusionInput::getSingleton().Activate();
	}

	bool ConsoleGUI::onMouseEnter(const CEGUI::EventArgs& e)
	{
		// Stops the input manager from trying to gather player inputs
		FusionInput::getSingleton().Suspend();
	}

	bool ConsoleGUI::onMouseLeave(const CEGUI::EventArgs& e)
	{
		FusionInput::getSingleton().Activate();
	}

	bool ConsoleGUI::onEditBoxAccepted(const CEGUI::EventArgs& e)
	{
		using namespace CEGUI;

		// Get text out of the command entry box
		String edit_text(m_EditBox->getText());
		if (!edit_text.empty())
		{
			// Add this entry to the command history buffer
			m_History.push_back(edit_text);
			// Reset history position
			m_HistoryPos = m_History.size();

			// Put the text in the display
			enterText(edit_text);


			// Store the text in the console
			Console::getSingleton().Add(edit_text);

			// Erase text in text entry box.
			m_EditBox->setText("");

			// Something has been entered into the history, so even if the
			//  user has been scrolling up with un-submitted text in the editbox,
			//  keep the entire histroy.
			m_RecentInHistory = false;
		}

		// re-activate the text entry box
		m_EditBox->activate();

		return true;
	}

	bool ConsoleGUI::onEditBoxKeyUp(const CEGUI::EventArgs &e)
	{
		using namespace CEGUI;

		switch (static_cast<const KeyEventArgs&>(e).scancode)
		{
		case Key::ArrowUp:
			// Save the current value to the history if it isn't already part of it
			if (m_HistoryPos == static_cast<int>(m_History.size()))
			{
				String edit_text(m_EditBox->getText());
				if (!edit_text.empty())
				{
					// Remember to erase this item from the history if the user
					//  scrolls back down to it:
					m_RecentInHistory = true;
					// Push the item
					m_History.push_back(edit_text);
					// Reset history position
					m_HistoryPos = m_History.size();
				}
			}

			m_HistoryPos = ceguimax(m_HistoryPos - 1, -1);
			// Make sure there's still history left to scroll through
			if (m_HistoryPos >= 0)
			{
				m_EditBox->setText(m_History[m_HistoryPos]);
				m_EditBox->setCaratIndex(static_cast<size_t>(-1));
			}
			/*else
			{
				editbox->setText("");
			}*/

			m_EditBox->activate();
			break;

		case Key::ArrowDown:
			m_HistoryPos = ceguimin(m_HistoryPos + 1, static_cast<int>(m_History.size()));
			// Within the history buffer:
			if (m_HistoryPos < static_cast<int>(m_History.size()))
			{
				m_EditBox->setText(m_History[m_HistoryPos]);
				m_EditBox->setCaratIndex(static_cast<size_t>(-1));
			}
			// Below the bottom of the history buffer:
			else
			{
				m_EditBox->setText("");
			}

			// If this is the bottom of the list...
			if (m_HistoryPos+1 == static_cast<int>(m_History.size()))
			{
				//... and if the current item was never submitted, erase it from the history.
				if (m_RecentInHistory)
				{
					m_RecentInHistory = false;
					m_HistoryPos = ceguimax(m_HistoryPos - 1, -1); 
				}
			}

			m_EditBox->activate();
			break;

		default:
			return false;
		}

		return true;
	}

	void ConsoleGUI::onConsoleNewLine(const std::string &data)
	{
		enterText(data);
	}

	/////////////////
	/// Private

	void ConsoleGUI::enterText(const CEGUI::String &text)
	{
		if (!text.empty())
		{
			// Append newline to this entry
			text += '\n';
			// Append new text to history output
			m_HistoryBox->setText(m_HistoryBox->getText() + edit_text);
			// Scroll to bottom of history output
			m_HistoryBox->setCaratIndex(static_cast<size_t>(-1));
		}
	}

}
