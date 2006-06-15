#include "FusionEngineCommon.h"

/// STL

/// Fusion

/// Class
#include "FusionGUI_Options.h"

using namespace Fusion;

FusionGUI_Options::FusionGUI_Options(CL_ResourceManager *resources, CL_Component *parent, CL_Deck *deck)
: m_ComponentManager(FusionGUI_Options::Name, resources, parent), m_Deck(deck)
{
	//! 'Create Game' button
	CL_Button *button_ctrls;
	m_ComponentManager.get_component("Button_Options_Controls", &button_ctrls);
	m_Slots.connect(button_ctrls->sig_clicked(), this, on_createClicked);

	//! 'Join Game' button
	CL_Button *button_gfx;
	m_ComponentManager.get_component("Button_Options_Graphics", &button_gfx);
	m_Slots.connect(button_gfx->sig_clicked(), this, on_joinClicked);
}

FusionGUI_MainMenu::GetComponent()
{
	return component_manager.get_component(FusionGUI_Options::Name);
}