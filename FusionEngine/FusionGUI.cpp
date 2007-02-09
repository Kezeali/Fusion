
#include "FusionGUI.h"

namespace FusionEngine
{

	GUI::GUI()
		: m_CurrentScheme(DefaultScheme),
		m_CurrentLayout(DefaultLayout)
	{
		// Create a new system
		CEGUI::OpenGLRenderer *renderer = new CEGUI::OpenGLRenderer(0);
		new CEGUI::System(renderer);
	}

	GUI::~GUI()
	{
		delete CEGUI::System::getSingletonPtr();
	}

	bool GUI::Initialise()
	{
		// Mouse Events
		m_Slots.connect(CL_Mouse::sig_key_down(), this, &GUI::onMouseDown);
		m_Slots.connect(CL_Mouse::sig_key_up(), this, &GUI::onMouseUp);
		m_Slots.connect(CL_Mouse::sig_move(), this, &GUI::onMouseMove);
		// KBD events
		m_Slots.connect(CL_Keyboard::sig_key_down(), this, &GUI::onKeyDown);
		m_Slots.connect(CL_Keyboard::sig_key_up(), this, &GUI::onKeyUp);

		return true;
	}

	bool GUI::Update(unsigned int split)
	{
		CEGUI::System::getSingleton().injectTimePulse((float)split);

		// Hide the cursor if the timeout has been reached
		if ( m_ShowMouseTimer <= 0 )
		{
			CEGUI::MouseCursor::getSingleton().hide();
		}
		else
		{
			m_ShowMouseTimer -= split;
		}

		return true;
	}

	void GUI::Draw()
	{
		CEGUI::System::getSingleton().renderGUI();
	}

	void GUI::CleanUp()
	{
	}

	bool GUI::AddWindow(Window *window)
	{
		using namespace CEGUI;

		if (!WindowManager::getSingleton().isWindowPresent())
			return false;

		System::getSingleton().getGUISheet()->addChildWindow(window);
		
		return true;
	}

	bool GUI::RemoveWindow(Window *window)
	{
		using namespace CEGUI;

		System::getSingleton().getGUISheet()->removeChildWindow(window);
	}

	void GUI::SetMouseShowPeriod(unsigned int period)
	{
		m_MouseShowPeriod = period;
	}


	void GUI::onMouseDown(const CL_InputEvent &key)
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

	void GUI::onMouseUp(const CL_InputEvent &key)
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

	void GUI::onMouseMove(const CL_InputEvent &e)
	{
		CEGUI::System::getSingleton().injectMousePosition(float(e.mouse_pos.x), float(e.mouse_pos.y));

		if (m_ShowMouseTimer <= 0)
		{
			m_ShowMouseTimer = m_MouseShowPeriod;
			CEGUI::MouseCursor::getSingleton().show();
		}
	}

	void GUI::onKeyDown(const CL_InputEvent &key)
	{
		// This should filter-out non-alphanumeric keys...
		//if (isalnum(key.id))
		//	CEGUI::System::getSingleton().injectChar(CEGUI::utf32(key.id));
		//else

		CEGUI::System::getSingleton().injectKeyDown(
			CLKeyToCEGUIKey(key.id)
			);
	}

	void GUI::onKeyUp(const CL_InputEvent &key)
	{
		// This should filter-out alphanumeric keys...
		//if (isalnum(key.id) == 0)

		CEGUI::System::getSingleton().injectKeyUp(
			CLKeyToCEGUIKey(key.id)
			);
	}

}
