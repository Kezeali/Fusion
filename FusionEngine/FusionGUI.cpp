
#include "FusionCommon.h"

#include "FusionGUI.h"

#include "FusionCEGUIPhysFSResourceProvider.h"
#include "FusionConsole.h"

#include "FusionResourceManager.h"

#include <CEGUI/CEGUITinyXMLParser.h>
#include <CEGUI/CEGUIDefaultResourceProvider.h>

namespace FusionEngine
{

	GUI::GUI()
		: FusionState(false), // GUI is non-blockin by default
		m_Modifiers(NOMOD),
		m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000)
	{
	}

	GUI::GUI(CL_DisplayWindow* window)
		: FusionState(false), /* GUI is non-blockin by default */
		m_Modifiers(NOMOD),
		m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000),
		m_Display(window->get_gc())
	{
		// Just in case, I guess? (re-initialized in GUI::Initialise() after the CEGUI renderer has been set up)
		m_GLState = CL_OpenGLState(window->get_gc());
	}

	GUI::~GUI()
	{
		CleanUp();
	}

	void GUI::Configure(const std::string& fname)
	{
		//ResourcePointer<TiXmlDocument> cfgResource = ResourceManager::getSingleton().GetResource("CEGUIConfig.xml", "XML");

		//if (cfgResource.IsValid())
		//{
		//	TiXmlDocument* cfgDoc = cfgResource.GetDataPtr();
		//	
		//	TinyXPath::S_xpath_string(cfgDoc->RootElement(), "/ceguiconfig/paths/datafiles");
		//}
	}

	bool GUI::Initialise()
	{
		CL_Display::get_current_window()->hide_cursor();

		try
		{
			using namespace CEGUI;
			// -- Create a new system, renderer and resource provider --
			CEGUI::OpenGLRenderer *renderer = new CEGUI::OpenGLRenderer(0, m_Display->get_width(), m_Display->get_height());
			PhysFSResourceProvider* rp = new PhysFSResourceProvider(); // custom PhysFS resource provider

			new CEGUI::System(renderer, rp);
			CEGUI::Logger::getSingleton().setLoggingLevel(Insane);

			//CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>
   //     (CEGUI::System::getSingleton().getResourceProvider());

			m_GLState = CL_OpenGLState(m_Display);
			m_GLState.set_active();

			// -- Setup the resource paths and groups --

			rp->setResourceGroupDirectory("schemes", "datafiles/schemes/");
			rp->setResourceGroupDirectory("imagesets", "datafiles/imagesets/");
			rp->setResourceGroupDirectory("fonts", "datafiles/fonts/");
			rp->setResourceGroupDirectory("layouts", "datafiles/layouts/");
			rp->setResourceGroupDirectory("looknfeels", "datafiles/looknfeel/");
			rp->setResourceGroupDirectory("lua_scripts", "datafiles/lua_scripts/");

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
			CEGUI::Window* background = winMgr.createWindow("SharpHour/StaticImage");
			// install this as the root GUI sheet
			System::getSingleton().setGUISheet(background);

			// -- Load a font --
			FontManager::getSingleton().createFont("defaultfont.font");

			//CEGUI::Logger::getSingleton().setLoggingLevel(Standard);
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
		try
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
		}
		catch (CEGUI::Exception& e)
		{
			SendToConsole(e.getMessage().c_str(), Console::MTERROR);
		}

		return true;
	}

	void GUI::Draw()
	{
		try
		{
			m_GLState.set_active();
			CEGUI::System::getSingleton().renderGUI();
		}
		catch (CEGUI::Exception& e)
		{
			SendToConsole(e.getMessage().c_str(), Console::MTERROR);
		}
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
			CEGUI::System::getSingleton().injectMouseWheelChange(0.5);
			break;
		case CL_MOUSE_WHEEL_DOWN:
			CEGUI::System::getSingleton().injectMouseWheelChange(-0.5);
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
