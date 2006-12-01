
#include "FusionEngineGUI_Console.h"

#include <CEGUI/CEGUI.h>
#include <CEGUI/openglrenderer.h>
#include <CEGUI/CEGUILua.h>

using namespace FusionEngine;

GUI_Console::GUI_Console()
{
	m_CurrentScheme = DefaultScheme;
	m_CurrentLayout = "OptionsMenu";
	//! \todo Make GUI schemes load dynamicaly

	CEGUI::OpenGLRenderer *renderer = new CEGUI::OpenGLRenderer(0);
	new CEGUI::System(renderer);
}

bool GUI_Console::Initialise()
{
	// Stops the input manager from trying to gather player inputs
	//m_InputManager->Suspend();
	FusionInput::getSingleton().Suspend();

	using namespace CEGUI;

	SchemeManager::getSingleton().loadScheme(m_CurrentScheme);

	WindowManager& winMgr = WindowManager::getSingleton();
	winMgr.loadWindowLayout(m_CurrentLayout);

	//! \todo Make FusionGUI_Options#onSaveClicked send a message to the state man to remove the state et. al.
	// 'Save' button
	//static_cast<PushButton *> (
		//winMgr.getWindow("OptionsMenu/Save"))->subscribeEvent(
		//PushButton::EventClicked,
		//Event::Subscriber(&FusionGUI_Options::onSaveClicked, this));

	// Call base function (to init KB/Mouse handling)
	return GUI::Initialise();
}

bool GUI_Console::Update(unsigned int split)
{
	CEGUI::System::getSingleton().injectTimePulse(float(split));
	return true;
}

void GUI_Console::Draw()
{
	CEGUI::System::getSingleton().renderGUI();
}

void GUI_Console::CleanUp()
{
	//m_InputManager->Activate();
	FusionInput::getSingleton().Activate();
}
