
#include "FusionEngineGUI_Options.h"

#include <CEGUI/CEGUI.h>
#include <CEGUI/openglrenderer.h>
#include <CEGUI/CEGUILua.h>

using namespace FusionEngine;

GUI_Options::GUI_Options()
{
	m_CurrentScheme = DefaultScheme;
	m_CurrentLayout = "OptionsMenu";
	//! \todo Make GUI schemes load dynamicaly

	CEGUI::OpenGLRenderer *renderer = new CEGUI::OpenGLRenderer(0);
	new CEGUI::System(renderer);
}

/*
GUI_Options::GUI_Options(FusionInput *inputmanager)
: m_InputManager(inputmanager)
{
	m_CurrentLayout = "OptionsMenu";

	CEGUI::OpenGLRenderer *renderer = new CEGUI::OpenGLRenderer(0);
	new CEGUI::System(renderer);
}
*/

bool GUI_Options::Initialise()
{
	// Stops the input manager from trying to gather player inputs
	//m_InputManager->Suspend();
	FusionInput::getSingleton().Suspend();

	using namespace CEGUI;

	SchemeManager::getSingleton().loadScheme(m_CurrentScheme);

	WindowManager& winMgr = WindowManager::getSingleton();
	winMgr.loadWindowLayout(m_CurrentLayout);

	// 'Save' button
	static_cast<PushButton *> (
		winMgr.getWindow("Options/Quit"))->subscribeEvent(
		PushButton::EventClicked,
		Event::Subscriber(&GUI_Options::onQuitClicked, this));

	// Call base function (to init KB/Mouse handling)
	return GUI::Initialise();
}

bool GUI_Options::Update(unsigned int split)
{
	CEGUI::System::getSingleton().injectTimePulse(float(split));
	return true;
}

void GUI_Options::Draw()
{
	CEGUI::System::getSingleton().renderGUI();
}

void GUI_Options::CleanUp()
{
	//m_InputManager->Activate();
	FusionInput::getSingleton().Activate();
}

bool GUI_Options::onQuitClicked(const CEGUI::EventArgs& e)
{
	_pushMessage(new StateMessage(StateMessage::QUIT, 0));

	return true;
}