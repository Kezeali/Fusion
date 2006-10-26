/// Class
#include "FusionGUI_Options.h"

/// Fusion

/// CEGUI
#include <CEGUI/CEGUI.h>
#include <CEGUI/openglrenderer.h>

using namespace Fusion;


FusionGUI_Options::FusionGUI_Options()
{
	m_CurrentScheme = DefaultScheme;
	m_CurrentLayout = "OptionsMenu";
}

FusionGUI_Options::FusionGUI_Options(FusionEngine::ClientOptions *clientopts)
: m_ClientOpts(clientopts)
{
	m_CurrentScheme = DefaultScheme;
	m_CurrentLayout = "OptionsMenu";
}

bool FusionGUI_Options::Initialise()
{
	using namespace CEGUI;

	SchemeManager::getSingleton().loadScheme(m_CurrentScheme);

	WindowManager& winMgr = WindowManager::getSingleton();
	winMgr.loadWindowLayout(m_CurrentLayout);

	// 'Save' button
	static_cast<PushButton *> (
		winMgr.getWindow("OptionsMenu/Save"))->subscribeEvent(
		PushButton::EventClicked,
		Event::Subscriber(&FusionGUI_Options::onSaveClicked, this));

	// Call base function (to init KB/Mouse handling)
	return FusionGUI::Initialise();
}

bool FusionGUI_Options::onSaveClicked(const CEGUI::EventArgs& e)
{
	m_ClientOpts->Save();

	return true;
}