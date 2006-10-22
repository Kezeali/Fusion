/// Class
#include "FusionGUI_MainMenu.h"

/// STL

/// Fusion
#include "FusionGUI_Options.h"
#include "FusionGame.h"

using namespace Fusion;

FusionGUI_MainMenu::DefaultScheme = "MainMenu";

FusionGUI_MainMenu::FusionGUI_MainMenu()
: m_CurrentGUI(DefaultScheme)
{
}

FusionGUI_MainMenu::FusionGUI_MainMenu(const std::string &scheme)
: m_CurrentGUI(scheme)
{
}

bool FusionGUI_MainMenu::Initialise()
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

	// 'Create Game' button
	static_cast<PushButton *> (
		winMgr.getWindow("MainMenu/StartServer"))->subscribeEvent(
		PushButton::EventClicked,
		Event::Subscriber(&FusionGUI_MainMenu::onCreateClicked, this));

	// 'Join Game' button
	static_cast<PushButton *> (
		winMgr.getWindow("MainMenu/JoinServer"))->subscribeEvent(
		PushButton::EventClicked,
		Event::Subscriber(&FusionGUI_MainMenu::onCreateClicked, this));

	// 'Options' button
	static_cast<PushButton *> (
		winMgr.getWindow("MainMenu/Options"))->subscribeEvent(
		PushButton::EventClicked,
		Event::Subscriber(&FusionGUI_MainMenu::onCreateClicked, this));

	return true;
}

void FusionGUI_MainMenu::onCreateClicked()
{
	FusionEngine::FusionGame *game = new FusionEngine::FusionGame;
}

void FusionGUI_MainMenu::onJoinClicked()
{
}

void FusionGUI_MainMenu::onOptsClicked()
{
}