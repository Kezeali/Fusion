/// Class
#include "FusionGUI_Options.h"

/// Fusion

/// CEGUI
#include <CEGUI/CEGUI.h>
#include <CEGUI/openglrenderer.h>

using namespace Fusion;

FusionGUI_Options::DefaultScheme = "BlueLook";
FusionGUI_MainMenu::DefaultLayout = "OptionsMenu";

FusionGUI_Options::FusionGUI_Options()
: m_CurrentGUI(DefaultScheme)
{
}

FusionGUI_Options::FusionGUI_Options(const std::string &scheme)
: m_CurrentGUI(scheme)
{
}

bool FusionGUI_Options::Initialise()
{
	using namespace CEGUI;

	SchemeManager::getSingleton().loadScheme(m_CurrentGUI);

	WindowManager& winMgr = WindowManager::getSingleton();
	winMgr.loadWindowLayout(m_CurrentLayout);

	// Mouse Events
	m_Slots.connect(CL_Mouse::sig_key_down(), this, &FusionGUI::onMouseDown);
	m_Slots.connect(CL_Mouse::sig_key_up(), this, &FusionGUI::onMouseUp);
	m_Slots.connect(CL_Mouse::sig_move(), this, &FusionGUI::onMouseMove);
	// KBD events
	m_Slots.connect(CL_Keyboard::sig_key_down(), this, &FusionGUI::onKeyDown);
	m_Slots.connect(CL_Keyboard::sig_key_up(), this, &FusionGUI::onKeyUp);

	// 'Save' button
	static_cast<PushButton *> (
		winMgr.getWindow("OptionsMenu/Save"))->subscribeEvent(
		PushButton::EventClicked,
		Event::Subscriber(&FusionGUI_Options::onSaveClicked, this));
		
}

void FusionGUI_Options::onSaveClicked()
{
}