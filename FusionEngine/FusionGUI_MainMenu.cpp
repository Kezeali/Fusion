#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionGUI_Options.h"

/// Class
#include "FusionGUI_MainMenu.h"

using namespace Fusion;

FusionGUI_MainMenu::FusionGUI_MainMenu(CL_ResourceManager *resources, CL_Component *parent, CL_Deck *deck)
: m_ComponentManager(FusionGUI_MainMenu::Name, resources, parent), m_Deck(deck)
{
	//! 'Create Game' button
	CL_Button *button_create;
	m_ComponentManager.get_component("Button_Play_Create", &button_create);
	m_Slots.connect(button_create->sig_clicked(), this, FusionGUI_MainMenu::on_createClicked);

	//! 'Join Game' button
	CL_Button *button_join;
	m_ComponentManager.get_component("Button_Play_Join", &button_join);
	m_Slots.connect(button_join->sig_clicked(), this, FusionGUI_MainMenu::on_joinClicked);

	//! 'Options' button
	CL_Button *button_opts;
	m_ComponentManager.get_component("Button_Main_Options", &button_opts);
	m_Slots.connect(button_create->sig_clicked(), this, FusionGUI_MainMenu::on_optsClicked);
}

FusionGUI_MainMenu::GetComponent()
{
	return component_manager.get_component(FusionGUI_MainMenu::Name);
}

FusionGUI_MainMenu::on_createClicked()
{
}

FusionGUI_MainMenu::on_joinClicked()
{
	
}

FusionGUI_MainMenu::on_optsClicked()
{
	m_Deck->swap(FusionGUI_Options::Name);
}