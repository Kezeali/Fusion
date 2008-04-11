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

#include "FusionInputHandler.h"

#include "FusionGUI.h"
#include "FusionConsole.h"

#include "FusionScriptingEngine.h"

#include <CEGUI/CEGUI.h>
#include <CEGUI/falagard/CEGUIFalWidgetLookManager.h>

namespace FusionEngine
{

	ConsoleGUI::ConsoleGUI()
		: m_MaxHistory(g_ConGUIDefaultMaxHistory)
	{
	}

	ConsoleGUI::ConsoleGUI(size_t max_history)
	{
		m_MaxHistory = max_history;
	}

	ConsoleGUI::~ConsoleGUI()
	{
		//FusionInput::getSingleton().Activate();
	}

	/////////////////
	/// Public

	bool ConsoleGUI::Initialise()
	{
		using namespace CEGUI;

		Window *console_sheet = 0;
		try
		{
			WindowManager& winMgr = WindowManager::getSingleton();

			console_sheet = winMgr.loadWindowLayout("Console.layout");

			m_Wind = static_cast<CEGUI::Window*> (
				winMgr.getWindow("Console/Wind")
				);
			m_EditBox = static_cast<CEGUI::Editbox*> (
				winMgr.getWindow("Console/Wind/Editbox")
				);
			m_HistoryBox = static_cast<CEGUI::MultiLineEditbox*> (
				winMgr.getWindow("Console/Wind/History")
				);

			// Capture editbox accepted (e.g. enter pressed) events
			m_EventConnections.push_back(
				m_EditBox->subscribeEvent(
				Editbox::EventTextAccepted,
				Event::Subscriber(&ConsoleGUI::onEditBoxAccepted, this)));
			// Submit button
			m_EventConnections.push_back(
				m_Wind->getChild(1)->subscribeEvent(
				PushButton::EventClicked,
				Event::Subscriber(&ConsoleGUI::onEditBoxAccepted, this)));
			// Key events to scroll history
			m_EventConnections.push_back(
				m_Wind->subscribeEvent(
				Window::EventKeyDown,
				Event::Subscriber(&ConsoleGUI::onEditBoxKeyUp, this)));

			// Input activation/suspension
			m_EventConnections.push_back(
				m_EditBox->subscribeEvent(
				Window::EventActivated,
				Event::Subscriber(&ConsoleGUI::onMouseEnter, this)));

			m_EventConnections.push_back(
				m_EditBox->subscribeEvent(
				Window::EventDeactivated,
				Event::Subscriber(&ConsoleGUI::onMouseLeave, this)));

			// Connect to the newline signal from the console
			//m_Slots.connect(Console::getSingleton().OnNewLine, this, &ConsoleGUI::onConsoleNewLine);
			m_ConsoleOnNewLineSlot = 
				Console::getSingleton().OnNewLine.connect(this, &ConsoleGUI::onConsoleNewLine);

			m_ConsoleOnClearSlot =
				Console::getSingleton().OnClear.connect(this, &ConsoleGUI::onConsoleClear);

			// Fill the console with the past history
			Console::ConsoleLines lines = Console::getSingleton().GetHistory();


			// Finally, add the window to the GUI, and hope
			bool success = GUI::getSingleton().AddWindow(console_sheet);

			Console::ConsoleLines::iterator it = lines.begin();
			for (; it != lines.end(); ++it)
			{
				enterText( (*it) );
			}

			return success;

		}
		catch (CEGUI::Exception& e)
		{
			SendToConsole(e.getMessage().c_str(), Console::MTERROR);
			return false;
		}
	}

	bool ConsoleGUI::Update(unsigned int split)
	{
		return true;
	}

	void ConsoleGUI::Draw()
	{
	}

	void ConsoleGUI::CleanUp()
	{
		CEGUI::WindowManager::getSingleton().destroyWindow("Console/Wind");
		FusionInput::getSingleton().Activate();

		Console::getSingleton().OnNewLine.disconnect(m_ConsoleOnNewLineSlot);
		Console::getSingleton().OnClear.disconnect(m_ConsoleOnClearSlot);

		EventConnectionList::iterator it = m_EventConnections.begin();
		for (; it != m_EventConnections.end(); ++it)
		{
			(*it)->disconnect();
		}
	}

	void ConsoleGUI::SetMaxHistory(unsigned int max)
	{
		m_MaxHistory = max;
		m_History.resize(max);
		m_HistoryPos = (int)m_History.size();
	}

	unsigned int ConsoleGUI::GetMaxHistory() const
	{
		return m_MaxHistory;
	}

	bool ConsoleGUI::onMouseEnter(const CEGUI::EventArgs& e)
	{
		// Stops the input manager from trying to gather player inputs
		FusionInput::getSingleton().Suspend();

		SendToConsole("Game input suspended");

		return true;
	}

	bool ConsoleGUI::onMouseLeave(const CEGUI::EventArgs& e)
	{
		FusionInput::getSingleton().Activate();

		SendToConsole("Game input activated");

		return true;
	}

	bool ConsoleGUI::onEditBoxAccepted(const CEGUI::EventArgs& e)
	{
		using namespace CEGUI;

		// Get text out of the command entry box
		String editText(m_EditBox->getText());
		if (!editText.empty())
		{
			// Add this entry to the command history buffer
			m_History.push_back(editText);
			// Limit History size
			if (m_History.size() >= m_MaxHistory)
				m_History.pop_front();
			// Reset history position
			m_HistoryPos = (int)m_History.size();

			// Put the text in the display
			// - Removed because this is done via a signal from the console after SendToConsole(..) is called
			//enterText(edit_text);

			// Store the text in the Console
			SendToConsole(editText.c_str());

			// Erase text in text entry box.
			m_EditBox->setText("");

			// Something has been entered into the history, so even if the
			//  user has been scrolling up with un-submitted text in the editbox,
			//  keep the entire histroy.
			m_RecentInHistory = false;
		}

		// re-activate the text entry box
		m_EditBox->activate();

		ScriptingEngine::getSingleton().ExecuteString(editText.c_str(), 0);

		return true;
	}

	bool ConsoleGUI::onEditBoxKeyUp(const CEGUI::EventArgs &e)
	{
		using namespace CEGUI;

		switch (static_cast<const KeyEventArgs&>(e).scancode)
		{
		case Key::ArrowUp:
			// Save the current value to the history if it isn't already part of it
			if (m_HistoryPos == (int)m_History.size())
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
					m_HistoryPos = (int)m_History.size();
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
			if (m_HistoryPos < (int)m_History.size())
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
			if (m_HistoryPos+1 == (int)m_History.size())
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

	void ConsoleGUI::onConsoleClear()
	{
		m_HistoryBox->setText("");
	}

	/////////////////
	/// Private

	void ConsoleGUI::enterText(CEGUI::String text)
	{
		if (!text.empty())
		{
			// Append newline to this entry
			text += "\n";
			CEGUI::String currentText = m_HistoryBox->getText();

			//! \todo Fix historyBox size-cap hack
			if (currentText.size() > 5000)
			{
				CEGUI::String::size_type offset = 0;
				int cutoff = currentText.size() * 0.50;

				offset = currentText.find('\n', cutoff);
				// We may encounter a really long line which doesn't allow the offset to
				//  be between 'end' and 'currentText.size()': in this case we find the closest line in the backward direction
				if (offset >= currentText.size())
				{
					offset = currentText.rfind('\n', cutoff);
				}

				if (offset == 0)
					m_HistoryBox->setText(""); // The entire buffer was one line, remove the whole thing
				else if (offset < currentText.size())
					m_HistoryBox->setText(currentText.c_str() + offset);
			}

			//std::cout << m_HistoryBox->getName() << std::endl;
			//std::cout << CEGUI::System::getSingleton().getDefaultXMLParserName() << ", " <<
			//	CEGUI::WindowManager::getSingleton().isWindowPresent("Console/Wind") << std::endl;
			//std::cout << currentText << ", " << text << std::endl;
			// Append new text to history output
			CEGUI::String ccText = currentText + text;
			m_HistoryBox->setText(ccText);

			// Scroll to bottom of history output
			m_HistoryBox->setCaratIndex(static_cast<size_t>(-1));
		}
	}

}
