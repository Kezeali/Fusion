/*
  Copyright (c) 2006-2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
		
		
	File Author(s):

		Elliot Hayward

*/

#include "Common.h"
#include "FusionCommon.h"

#include "FusionGUI.h"

#include "FusionConsole.h"
#include "FusionLogger.h"

namespace FusionEngine
{

	GUI::GUI()
		: FusionState(false), // GUI is non-blockin by default
		m_Modifiers(NOMOD),
		m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000)
	{
	}

	GUI::GUI(CL_DisplayWindow window)
		: FusionState(false), /* GUI is non-blockin by default */
		m_Modifiers(NOMOD),
		m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000),
		m_Display(window)
	{
		// Just in case, I guess? (re-initialized in GUI::Initialise() after the CEGUI renderer has been set up)
		//m_GLState = CL_OpenGLState(window->get_gc());
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
		//CL_Display::get_current_window()->hide_cursor();
		using namespace Rocket;

		m_RocketFileSys = new RocketFileSystem();
		m_RocketRenderer = new RocketRenderer(m_Display.get_gc());
		m_RocketSystem = new RocketSystem();
		
		Rocket::Core::SetFileInterface(m_RocketFileSys);
		Rocket::Core::SetRenderInterface(m_RocketRenderer);
		Rocket::Core::SetSystemInterface(m_RocketSystem);
		Rocket::Core::Initialise();

		CL_GraphicContext gc = m_Display.get_gc();

		m_Context = Rocket::Core::CreateContext("default", EMP::Core::Vector2i(gc.get_width(), gc.get_width()));

		LoadFonts("gui/");
		
		m_Document = m_Context->LoadDocument("gui/demo.rml");
		if (m_Document != NULL)
			m_Document->Show();


		CL_InputContext ic = m_Display.get_ic();

		// Mouse Events
		m_Slots.connect(ic.get_mouse().sig_key_down(), this, &GUI::onMouseDown);
		m_Slots.connect(ic.get_mouse().sig_key_up(), this, &GUI::onMouseUp);
		m_Slots.connect(ic.get_mouse().sig_pointer_move(), this, &GUI::onMouseMove);
		// KBD events
		m_Slots.connect(ic.get_keyboard().sig_key_down(), this, &GUI::onKeyDown);
		m_Slots.connect(ic.get_keyboard().sig_key_up(), this, &GUI::onKeyUp);

		return true;
	}

	void GUI::LoadFonts(const char* directory)
	{
		EMP::Core::String font_names[4];
		font_names[0] = "Delicious-Roman.otf";
		font_names[1] = "Delicious-Italic.otf";
		font_names[2] = "Delicious-Bold.otf";
		font_names[3] = "Delicious-BoldItalic.otf";

		for (int i = 0; i < sizeof(font_names) / sizeof(EMP::Core::String); i++)
		{
			Rocket::Core::FontDatabase::LoadFontFace(EMP::Core::String(directory) + font_names[i]);
		}
	}

	bool GUI::Update(unsigned int split)
	{
		m_Context->Update();

		// Hide the cursor if the timeout has been reached
		if ( m_ShowMouseTimer <= 0 )
		{
			m_Context->ShowMouseCursor(false);
		}
		else
		{
			m_ShowMouseTimer -= split;
		}

		return true;
	}

	void GUI::Draw()
	{
		m_Context->Render();
	}

	void GUI::CleanUp()
	{
		m_RocketFileSys->RemoveReference();
		m_RocketSystem->RemoveReference();
		m_RocketRenderer->RemoveReference();
		m_Context->RemoveReference();
		Rocket::Core::Shutdown();

		m_RocketFileSys = NULL;
		m_RocketSystem = NULL;
		m_RocketRenderer = NULL;
		m_Context = NULL;

		m_Display.show_cursor();
	}

	bool GUI::AddWindow(const std::string& window)
	{
		using namespace Rocket;

		//if (WindowManager::getSingleton().isWindowPresent(window))
		//	return false;

		//System::getSingleton().getGUISheet()->addChildWindow(window);
		
		return true;
	}

	bool GUI::RemoveWindow(const std::string& window)
	{
		using namespace Rocket;

		//System::getSingleton().getGUISheet()->removeChildWindow(window);

		return true;
	}

	void GUI::SetMouseShowPeriod(unsigned int period)
	{
		m_MouseShowPeriod = period;
	}


	void GUI::onMouseDown(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		switch(ev.id)
		{
		case CL_MOUSE_LEFT:
			m_Context->ProcessMouseButtonDown(0, modifier);
			break;
		case CL_MOUSE_RIGHT:
			m_Context->ProcessMouseButtonDown(1, modifier);
			break;
		case CL_MOUSE_MIDDLE:
			m_Context->ProcessMouseButtonDown(2, modifier);
			break;
		case CL_MOUSE_XBUTTON1:
			m_Context->ProcessMouseButtonDown(3, modifier);
			break;
		case CL_MOUSE_XBUTTON2:
			m_Context->ProcessMouseButtonDown(4, modifier);
			break;
		}

	}

	void GUI::onMouseUp(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		switch(ev.id)
		{
		case CL_MOUSE_LEFT:
			m_Context->ProcessMouseButtonUp(0, modifier);
			break;
		case CL_MOUSE_RIGHT:
			m_Context->ProcessMouseButtonUp(1, modifier);
			break;
		case CL_MOUSE_MIDDLE:
			m_Context->ProcessMouseButtonUp(2, modifier);
			break;
		case CL_MOUSE_XBUTTON1:
			m_Context->ProcessMouseButtonUp(3, modifier);
			break;
		case CL_MOUSE_XBUTTON2:
			m_Context->ProcessMouseButtonUp(4, modifier);
			break;
		case CL_MOUSE_WHEEL_UP:
			m_Context->ProcessMouseWheel(1, modifier);
			break;
		case CL_MOUSE_WHEEL_DOWN:
			m_Context->ProcessMouseWheel(-1, modifier);
			break;
		}

	}

	void GUI::onMouseMove(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, modifier);

		if (m_ShowMouseTimer <= 0)
		{
			m_ShowMouseTimer = m_MouseShowPeriod;
			m_Context->ShowMouseCursor(true);
		}
	}

	void GUI::onKeyDown(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		// Grab characters
		if (!ev.str.empty())
		{
			std::string str(ev.str.begin(), ev.str.end());
			m_Context->ProcessTextInput( Rocket::Core::String(str.c_str()) );
			//const wchar_t* c_str = ev.str.c_str();
			// Inject all the characters given
			//for (int c = 0; c < ev.str.length(); c++)
			//	m_Context->ProcessTextInput( EMP::Core::word(c_str[c]) );
		}

		m_Context->ProcessKeyDown(CLKeyToRocketKeyIdent(ev.id), modifier);
	}

	void GUI::onKeyUp(const CL_InputEvent &ev, const CL_InputState &state)
	{
		//if (key.id == CL_KEY_SHIFT)
		//	m_Modifiers ^= SHIFT;
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		m_Context->ProcessKeyUp(CLKeyToRocketKeyIdent(ev.id), modifier);
	}

}
