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

FusionGUI_MainMenu::FusionGUI_MainMenu()
{
    m_CurrentScheme = DefaultScheme;
	m_CurrentLayout = "MainMenu";
}

FusionGUI_MainMenu::FusionGUI_MainMenu(FusionEngine::ClientOptions *clientopts, FusionEngine::ServerOptions* serveropts)
: m_ClientOpts(clientopts),
m_ServerOpts(serveropts)
{
	m_CurrentScheme = DefaultScheme;
	m_CurrentLayout = "OptionsMenu";
}

bool FusionGUI_MainMenu::Initialise()
{
	using namespace CEGUI;

	SchemeManager::getSingleton().loadScheme(m_CurrentScheme);

	WindowManager& winMgr = WindowManager::getSingleton();
	winMgr.loadWindowLayout(m_CurrentLayout);

	ImagesetManager::getSingleton().createImagesetFromImageFile("LogoImage", "FusionLogo.png");

	// 'Logo' static image
	winMgr.getWindow("MainMenu/FusionLogo")->setProperty(
		"Image", "set:LogoImage image:full_image");

	// 'Create Game' button
	winMgr.getWindow("MainMenu/StartServer")->subscribeEvent(
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

	// 'ServerIP' editbox
	//  Nothing to do

	// 'ServerPort' editbox
	static_cast<Editbox*>(winMgr.getWindow("MainMenu/ServerPort"))->
		setValidationString("\\d{0,5}"); // Only allow numbers, up to 5 chars

	// 'ListenPort' editbox
	static_cast<Editbox*>(winMgr.getWindow("MainMenu/ListenPort"))->
		setValidationString("\\d{0,5}"); // Only allow numbers, up to 5 chars

	// Call base function (to init KB/Mouse handling)
	return FusionGUI::Initialise();
}

bool FusionGUI_MainMenu::onCreateClicked(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;
	std::string port = WindowManager::getSingleton().getWindow("MainMenu/ListenPort")->getText().c_str();

	FusionEngine::FusionGame::RunServer(port, m_ServerOpts);

	// Cegui like to know when events were handled successfully.
	return true;
}

bool FusionGUI_MainMenu::onJoinClicked(const CEGUI::EventArgs& e)
{
	using namespace CEGUI;
	std::string hostname = WindowManager::getSingleton().getWindow("MainMenu/ServerIP")->getText().c_str();
	std::string port = WindowManager::getSingleton().getWindow("MainMenu/ServerPort")->getText().c_str();

	FusionEngine::FusionGame::RunClient(hostname, port, m_ClientOpts);

	return true;
}

bool FusionGUI_MainMenu::onOptsClicked(const CEGUI::EventArgs& e)
{
	//FusionGUI_Options *opts = new FusionGUI_Options();
	return true;
}
