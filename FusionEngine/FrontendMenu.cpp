
#include "FusionGUI.h"

using namespace Fusion;

bool FusionGUI::Initialise()
{
	// Mouse Events
	m_Slots.connect(CL_Mouse::sig_key_down(), this, &FusionGUI::onMouseDown);
	m_Slots.connect(CL_Mouse::sig_key_up(), this, &FusionGUI::onMouseUp);
	m_Slots.connect(CL_Mouse::sig_move(), this, &FusionGUI::onMouseMove);
	// KBD events
	m_Slots.connect(CL_Keyboard::sig_key_down(), this, &FusionGUI::onKeyDown);
	m_Slots.connect(CL_Keyboard::sig_key_up(), this, &FusionGUI::onKeyUp);

	return true;
}

bool FusionGUI::Update(unsigned int split)
{
	CEGUI::System::getSingleton().injectTimePulse(float(split));
	return true;
}

void FusionGUI::Draw()
{
	CEGUI::System::getSingleton().renderGUI();
}

void FusionGUI::onMouseDown(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_MOUSE_LEFT:
		CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::LeftButton);
		break;
	case CL_MOUSE_MIDDLE:
		CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MiddleButton);
		break;
	case CL_MOUSE_RIGHT:
		CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::RightButton);
		break;
	case CL_MOUSE_XBUTTON1:
		CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::X1Button);
		break;
	case CL_MOUSE_XBUTTON2:
		CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::X2Button);
		break;
	case CL_MOUSE_WHEEL_UP:
		// CEGUI doesn't seem to handle this
		break;
	case CL_MOUSE_WHEEL_DOWN:
		// CEGUI doesn't seem to handle this
		break;
	}

}

void FusionGUI::onMouseUp(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_MOUSE_LEFT:
		CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::LeftButton);
		break;
	case CL_MOUSE_MIDDLE:
		CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::MiddleButton);
		break;
	case CL_MOUSE_RIGHT:
		CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::RightButton);
		break;
	case CL_MOUSE_XBUTTON1:
		CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::X1Button);
		break;
	case CL_MOUSE_XBUTTON2:
		CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::X2Button);
		break;
	case CL_MOUSE_WHEEL_UP:
		// CEGUI doesn't seem to handle this
		break;
	case CL_MOUSE_WHEEL_DOWN:
		// CEGUI doesn't seem to handle this
		break;
	}

}

void FusionGUI::onMouseMove(const CL_InputEvent &e)
{
	CEGUI::System::getSingleton().injectMousePosition(float(e.mouse_pos.x), float(e.mouse_pos.y));
}

void FusionGUI::onKeyDown(const CL_InputEvent &key)
{
	// This should filter-out non-alphanumeric keys...
	if (isalnum(key.id))
		CEGUI::System::getSingleton().injectChar(CEGUI::utf32(key.id));
	else
		// Dunno if this will work; ClanLib probably uses different keycodes :P
		CEGUI::System::getSingleton().injectKeyDown(key.id);
}

void FusionGUI::onKeyUp(const CL_InputEvent &key)
{
	// This should filter-out alphanumeric keys...
	if (isalnum(key.id) == 0)
		// Dunno if this will work; ClanLib probably uses different keycodes :P
		CEGUI::System::getSingleton().injectKeyUp(key.id);
}