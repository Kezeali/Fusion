/// Class
#include "FusionGUI_MainMenu.h"

/// STL

/// Fusion
#include "FusionGUI_Options.h"
#include "FusionGame.h"

/// CEGUI
#include <CEGUI/CEGUI.h>
#include <CEGUI/openglrenderer.h>

using namespace Fusion;

FusionGUI_MainMenu::DefaultScheme = "BlueLook";
FusionGUI_MainMenu::DefaultLayout = "MainMenu";

FusionGUI_MainMenu::FusionGUI_MainMenu()
: m_CurrentScheme(DefaultScheme),
m_CurrentLayout(DefaultLayout)
{
}

FusionGUI_MainMenu::FusionGUI_MainMenu(const std::string &scheme)
: m_CurrentGUI(scheme)
{
}

bool FusionGUI_MainMenu::Initialise()
{
	using namespace CEGUI;

	SchemeManager::getSingleton().loadScheme(m_CurrentGUI);

	WindowManager& winMgr = WindowManager::getSingleton();
	winMgr.loadWindowLayout(m_CurrentLayout);

	ImagesetManager::getSingleton().createImagesetFromImageFile("LogoImage", "FusionLogo.png");

	// Mouse Events
	m_Slots.connect(CL_Mouse::sig_key_down(), this, &FusionGUI::onMouseDown);
	m_Slots.connect(CL_Mouse::sig_key_up(), this, &FusionGUI::onMouseUp);
	m_Slots.connect(CL_Mouse::sig_move(), this, &FusionGUI::onMouseMove);
	// KBD events
	m_Slots.connect(CL_Keyboard::sig_key_down(), this, &FusionGUI::onKeyDown);
	m_Slots.connect(CL_Keyboard::sig_key_up(), this, &FusionGUI::onKeyUp);

	// 'Logo' static image
	winMgr.getWindow("MainMenu/FusionLogo")->setProperty(
		"Image", "set:LogoImage image:full_image");

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