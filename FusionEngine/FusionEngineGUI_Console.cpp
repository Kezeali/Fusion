
#include "FusionEngineGUI_Console.h"

/// Fusion
#include "FusionConsole.h"


namespace FusionEngine
{

	GUI_Console::GUI_Console()
	{
		m_CurrentScheme = DefaultScheme;
		m_CurrentLayout = "Console";
		//! \todo Make GUI schemes load dynamicaly

		m_Wind = static_cast<CEGUI::Window*> (
			CEGUI::WindowManager::getSingleton().getWindow("Console/Wind")
			);
		m_EditBox = static_cast<CEGUI::Editbox*> (
			CEGUI::WindowManager::getSingleton().getWindow("Console/Wind/Editbox")
			);
		m_HistoryBox = static_cast<CEGUI::MultiLineEditbox*> (
			CEGUI::WindowManager::getSingleton().getWindow("Console/Wind/History")
			);
	}

	/////////////////
	/// Public

	bool GUI_Console::Initialise()
	{
		// Stops the input manager from trying to gather player inputs
		FusionInput::getSingleton().Suspend();

		using namespace CEGUI;

		SchemeManager::getSingleton().loadScheme(m_CurrentScheme);

		WindowManager& winMgr = WindowManager::getSingleton();
		winMgr.loadWindowLayout(m_CurrentLayout);

		// Subscribe to the KeyUp event to capture enter presses
		static_cast<Editbox *> (
			winMgr.getWindow("Console/Wind/Editbox"))->subscribeEvent(
			Editbox::EventTextAccepted,
			Event::Subscriber(&GUI_Console::onEditBoxAccepted, this));

		// Connect to the newline signal from the console
		m_Slots.connect(Console::getSingleton().OnNewLine, this, &GUI_Console::onConsoleNewLine);

		// Call base function (to init KB/Mouse handling)
		return GUI::Initialise();
	}

	bool GUI_Console::Update(unsigned int split)
	{
		return GUI::Update(split);
	}

	void GUI_Console::Draw()
	{
		CEGUI::System::getSingleton().renderGUI();
	}

	void GUI_Console::CleanUp()
	{
		FusionInput::getSingleton().Activate();
	}

	bool GUI_Console::onEditBoxAccepted(const CEGUI::EventArgs& e)
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

			Console::getSingleton().Add(

			// Something has been entered into the history, so even if the
			//  user has been scrolling up with un-submitted text in the editbox,
			//  keep the entire histroy.
			m_RecentInHistory = false;
		}

		// re-activate the text entry box
		m_EditBox->activate();

		return true;
	}

	bool GUI_Console::onEditBoxKeyUp(const CEGUI::EventArgs &e)
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

	void GUI_Console::onConsoleNewLine(const std::string &data)
	{
	}

	/////////////////
	/// Private

	void GUI_Console::enterText(const CEGUI::String &text)
	{
		if (!text.empty())
		{
			// Append newline to this entry
			text += '\n';
			// Append new text to history output
			m_HistoryBox->setText(m_HistoryBox->getText() + edit_text);
			// Scroll to bottom of history output
			m_HistoryBox->setCaratIndex(static_cast<size_t>(-1));
			// Erase text in text entry box.
			m_EditBox->setText("");
		}
	}

}
