
#include "FusionCommon.h"

#include "FusionGUI.h"

#include <CEGUI/CEGUIDefaultResourceProvider.h>
#include "FusionConsole.h"

namespace FusionEngine
{

	GUI::GUI()
		: FusionState(false), // GUI is non-blockin by default
		m_Modifiers(NOMOD),
		m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000)
	{
	}

	GUI::~GUI()
	{
		CleanUp();
	}

	bool GUI::Initialise()
	{
		CL_Display::get_current_window()->hide_cursor();

		try
		{
			using namespace CEGUI;
			// -- Create a new system and renderer --
			CEGUI::OpenGLRenderer *renderer = new CEGUI::OpenGLRenderer(0, 640, 480);
			new CEGUI::System(renderer);

			// -- Setup the resource paths and groups --
			DefaultResourceProvider* rp = static_cast<DefaultResourceProvider*>
				(System::getSingleton().getResourceProvider());

			rp->setResourceGroupDirectory("schemes", "../datafiles/schemes/");
			rp->setResourceGroupDirectory("imagesets", "../datafiles/imagesets/");
			rp->setResourceGroupDirectory("fonts", "../datafiles/fonts/");
			rp->setResourceGroupDirectory("layouts", "../datafiles/layouts/");
			rp->setResourceGroupDirectory("looknfeels", "../datafiles/looknfeel/");
			rp->setResourceGroupDirectory("lua_scripts", "../datafiles/lua_scripts/");

		  Imageset::setDefaultResourceGroup("imagesets");
      Font::setDefaultResourceGroup("fonts");
      Scheme::setDefaultResourceGroup("schemes");
      WidgetLookManager::setDefaultResourceGroup("looknfeels");
      WindowManager::setDefaultResourceGroup("layouts");
      ScriptModule::setDefaultResourceGroup("lua_scripts");

			// -- Load the schemes --
			SchemeManager::getSingleton().loadScheme("SharpHour.scheme");
			SchemeManager::getSingleton().loadScheme("WindowsLook.scheme");
			SchemeManager::getSingleton().loadScheme("VanillaSkin.scheme");

			// Cursor
			System::getSingleton().setDefaultMouseCursor("SharpHour-Images", "MouseArrow");

			// -- Create the root window --
			WindowManager& winMgr = WindowManager::getSingleton();
			// load an image to use as a background
			//CEGUI::ImagesetManager::getSingleton().createImagesetFromImageFile("BackgroundImage", "FusionLogo.tga");

			//// here we will use a StaticImage as the root, then we can use it to place a background image
			//CEGUI::Window* background = winMgr.createWindow("WindowsLook/StaticImage");
			//// set area rectangle
			//background->setArea(URect(cegui_reldim(0), cegui_reldim(0), cegui_reldim(1), cegui_reldim(1)));
			//// disable frame and standard background
			//background->setProperty("FrameEnabled", "false");
			//background->setProperty("BackgroundEnabled", "false");
			//// set the background image
			//background->setProperty("Image", "set:BackgroundImage image:full_image");
			CEGUI::Window* background = winMgr.createWindow("DefaultWindow");
			// install this as the root GUI sheet
			System::getSingleton().setGUISheet(background);

			// -- Load a font --
			FontManager::getSingleton().createFont("defaultfont.font");
		}
		catch (CEGUI::Exception& e)
		{
			SendToConsole(e.getMessage().c_str(), Console::MTERROR);
			return false;
		}

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
		CEGUI::System::getSingleton().injectTimePulse((float)(split*0.001));

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
		CEGUI::System* guiSys = CEGUI::System::getSingletonPtr();
		if (guiSys != NULL)
		{
			CEGUI::Renderer* renderer = guiSys->getRenderer();
			delete guiSys;
			delete renderer;
		}
		m_Slots.disconnect_all();

		CL_Display::get_current_window()->show_cursor();
	}

	bool GUI::AddWindow(const std::string& window)
	{
		using namespace CEGUI;

		//if (WindowManager::getSingleton().isWindowPresent(window))
		//	return false;

		System::getSingleton().getGUISheet()->addChildWindow(window);
		
		return true;
	}

	bool GUI::AddWindow(CEGUI::Window *window)
	{
		using namespace CEGUI;

		//if (WindowManager::getSingleton().isWindowPresent(window->getName()))
		//	return false;

		System::getSingleton().getGUISheet()->addChildWindow(window);
		
		return true;
	}

	bool GUI::RemoveWindow(const std::string& window)
	{
		using namespace CEGUI;

		System::getSingleton().getGUISheet()->removeChildWindow(window);

		return true;
	}

	bool GUI::RemoveWindow(CEGUI::Window *window)
	{
		using namespace CEGUI;

		System::getSingleton().getGUISheet()->removeChildWindow(window);

		return true;
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
			CEGUI::System::getSingleton().injectMouseWheelChange(0.1);
			break;
		case CL_MOUSE_WHEEL_DOWN:
			CEGUI::System::getSingleton().injectMouseWheelChange(-0.1);
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
		if (key.id == CL_KEY_SHIFT)
			m_Modifiers |= SHIFT;

		// Grab characters
		if (!key.str.empty())
		{
			const char* c_str = key.str.c_str();
			// Inject all the characters given
			for (int c = 0; c < key.str.length(); c++)
				CEGUI::System::getSingleton().injectChar( CEGUI::utf32(c_str[c]) );
		}

		CEGUI::System::getSingleton().injectKeyDown( CLKeyToCEGUIKey(key.id) );
	}

	void GUI::onKeyUp(const CL_InputEvent &key)
	{
		if (key.id == CL_KEY_SHIFT)
			m_Modifiers ^= SHIFT;

		CEGUI::System::getSingleton().injectKeyUp( CLKeyToCEGUIKey(key.id) );
	}

}
