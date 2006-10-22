/// Class
#include "FusionGUI_Options.h"

/// Fusion

using namespace Fusion;

FusionGUI_Options::DefaultScheme = "MainMenu";

FusionGUI_Options::FusionGUI_Options()
: m_CurrentGUI(DefaultScheme)
{
}

FusionGUI_Options::FusionGUI_Options(const std::string &scheme)
: m_CurrentGUI(scheme)
{
}

FusionGUI_Options::Initialise()
{
	WindowManager& winMgr = WindowManager::getSingleton();

	CEGUI::SchemeManager::getSingleton().loadScheme(m_CurrentGUI);

	// Mouse Events
	m_Slots.connect(CL_Mouse::sig_key_down(), this, &FusionGUI::onMouseDown);
	m_Slots.connect(CL_Mouse::sig_key_up(), this, &FusionGUI::onMouseUp);
	m_Slots.connect(CL_Mouse::sig_move(), this, &FusionGUI::onMouseMove);
	// KBD events
	m_Slots.connect(CL_Keyboard::sig_key_down(), this, &FusionGUI::onKeyDown);
	m_Slots.connect(CL_Keyboard::sig_key_up(), this, &FusionGUI::onKeyUp);

	/*
	static_cast<PushButton *> (
		winMgr.getWindow("OptionsMenu/Save"))->subscribeEvent(
		PushButton::EventClicked,
		Event::Subscriber(&FusionGUI_Options::onSaveClicked, this));
		*/
}