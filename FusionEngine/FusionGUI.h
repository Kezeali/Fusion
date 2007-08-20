/*
  Copyright (c) 2006-2007 Fusion Project Team

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

#ifndef Header_Fusion_GUI
#define Header_Fusion_GUI

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionState.h"
#include "FusionSingleton.h"

#include <CEGUI/CEGUI.h>
#include <CEGUI/openglrenderer.h>

namespace FusionEngine
{

	/*!
	 * \brief
	 * Wrapper for CEGUI - for the gameplay portion of fusion.
	 * This is the only GUI class used ingame - Windows such as Console and
	 * Options, and the HUD, are loaded into this object.
	 *
	 * \todo Intergrate GUI resources into ResourceManager
	 *
	 * \remarks
	 * State is non-blocking
	 */
	class GUI : public FusionState, public Singleton<GUI>
	{
	public:
		enum Modifiers
		{
			NOMOD = 0,
			SHIFT = 1,
			CTRL = 2,
			ALT = 4
		};

	public:
		//! Basic constructor.
		GUI();

		//! Destructor
		~GUI();

	public:
		//! Inits the gui
		virtual bool Initialise();

		//! Updates the inputs
		virtual bool Update(unsigned int split);

		//! Draws the gui
		virtual void Draw();

		//! Unbinds
		virtual void CleanUp();

		//! Adds the given window layout (to the root window)
		virtual bool AddWindow(const std::string &window);

		//!Removes the given window
		virtual bool RemoveWindow(const std::string &window);

		//! Adds the given window layout (to the root window)
		virtual bool AddWindow(CEGUI::Window *window);

		//!Removes the given window
		virtual bool RemoveWindow(CEGUI::Window *window);

		//! Sets the period of time the mouse will be shown after it stops moving
		void SetMouseShowPeriod(unsigned int period);

	protected:
		//! Name of the config file for the skin
		std::string m_CurrentScheme;
		//! Name of the config file for the current gui page
		std::string m_CurrentLayout;

		//! Lists added windows
		std::vector<std::string> m_CurrentWindows;

		//! Holds slots
		SlotContainer m_Slots;

		//! How long after the mouse stops moving until it fades
		unsigned int m_MouseShowPeriod;
		//! When this reaches zero, the mouse will be hidden
		int m_ShowMouseTimer;

		short m_Modifiers;

	public:
		//! Tells CEGUI when a mouse button is pressed
		virtual void onMouseDown(const CL_InputEvent &key);
		//! Tells CEGUI when a mouse button is released
		virtual void onMouseUp(const CL_InputEvent &key);
		//! Tells CEGUI when the mouse moves
		virtual void onMouseMove(const CL_InputEvent &e);

		//! Tells CEGUI when a keyboard key goes down
		virtual void onKeyDown(const CL_InputEvent &key);
		//! Tells CEGUI when a keyboard key is released
		virtual void onKeyUp(const CL_InputEvent &key);
	};


	static CEGUI::uint CLKeyToCEGUIKey(int key)
	{
		using namespace CEGUI;

		switch (key)
		{
		case CL_KEY_BACKSPACE:    return Key::Backspace;
		case CL_KEY_TAB:          return Key::Tab;
		case CL_KEY_RETURN:       return Key::Return;
		case CL_KEY_PAUSE:        return Key::Pause;
		case CL_KEY_ESCAPE:       return Key::Escape;
		case CL_KEY_SPACE:        return Key::Space;
		case CL_KEY_COMMA:        return Key::Comma;
#ifdef CL_KEY_MINUS
		case CL_KEY_MINUS:        return Key::Minus;
#endif
#ifdef CL_KEY_PERIOD
		case CL_KEY_PERIOD:       return Key::Period;
#endif
		case CL_KEY_0:            return Key::Zero;
		case CL_KEY_1:            return Key::One;
		case CL_KEY_2:            return Key::Two;
		case CL_KEY_3:            return Key::Three;
		case CL_KEY_4:            return Key::Four;
		case CL_KEY_5:            return Key::Five;
		case CL_KEY_6:            return Key::Six;
		case CL_KEY_7:            return Key::Seven;
		case CL_KEY_8:            return Key::Eight;
		case CL_KEY_9:            return Key::Nine;
		case CL_KEY_A:            return Key::A;
		case CL_KEY_B:            return Key::B;
		case CL_KEY_C:            return Key::C;
		case CL_KEY_D:            return Key::D;
		case CL_KEY_E:            return Key::E;
		case CL_KEY_F:            return Key::F;
		case CL_KEY_G:            return Key::G;
		case CL_KEY_H:            return Key::H;
		case CL_KEY_I:            return Key::I;
		case CL_KEY_J:            return Key::J;
		case CL_KEY_K:            return Key::K;
		case CL_KEY_L:            return Key::L;
		case CL_KEY_M:            return Key::M;
		case CL_KEY_N:            return Key::N;
		case CL_KEY_O:            return Key::O;
		case CL_KEY_P:            return Key::P;
		case CL_KEY_Q:            return Key::Q;
		case CL_KEY_R:            return Key::R;
		case CL_KEY_S:            return Key::S;
		case CL_KEY_T:            return Key::T;
		case CL_KEY_U:            return Key::U;
		case CL_KEY_V:            return Key::V;
		case CL_KEY_W:            return Key::W;
		case CL_KEY_X:            return Key::X;
		case CL_KEY_Y:            return Key::Y;
		case CL_KEY_Z:            return Key::Z;
		case CL_KEY_DELETE:       return Key::Delete;
		case CL_KEY_NUMPAD0:      return Key::Numpad0;
		case CL_KEY_NUMPAD1:      return Key::Numpad1;
		case CL_KEY_NUMPAD2:      return Key::Numpad2;
		case CL_KEY_NUMPAD3:      return Key::Numpad3;
		case CL_KEY_NUMPAD4:      return Key::Numpad4;
		case CL_KEY_NUMPAD5:      return Key::Numpad5;
		case CL_KEY_NUMPAD6:      return Key::Numpad6;
		case CL_KEY_NUMPAD7:      return Key::Numpad7;
		case CL_KEY_NUMPAD8:      return Key::Numpad8;
		case CL_KEY_NUMPAD9:      return Key::Numpad9;
#ifdef CL_KEY_DECIMAL
		case CL_KEY_DECIMAL:      return Key::Decimal;
#endif
		case CL_KEY_DIVIDE:       return Key::Divide; //or Key::Slash
#ifdef CL_KEY_MULTIPLY
		case CL_KEY_MULTIPLY:      return Key::Multiply;
#endif
		case CL_KEY_SUBTRACT:     return Key::Subtract; // or Key::Minus;
#ifdef CL_KEY_ADD
		case CL_KEY_ADD:          return Key::Add;
#endif
		case CL_KEY_NUMPAD_ENTER:        return Key::NumpadEnter;
			// My numpad has no equals key...
		//case '=':                 return Key::NumpadEquals;
		case CL_KEY_UP:           return Key::ArrowUp;
		case CL_KEY_DOWN:         return Key::ArrowDown;
		case CL_KEY_RIGHT:        return Key::ArrowRight;
		case CL_KEY_LEFT:         return Key::ArrowLeft;
		case CL_KEY_INSERT:       return Key::Insert;
		case CL_KEY_HOME:         return Key::Home;
		case CL_KEY_END:          return Key::End;
		case CL_KEY_PRIOR:        return Key::PageUp;
		case CL_KEY_NEXT:         return Key::PageDown;
		case CL_KEY_F1:           return Key::F1;
		case CL_KEY_F2:           return Key::F2;
		case CL_KEY_F3:           return Key::F3;
		case CL_KEY_F4:           return Key::F4;
		case CL_KEY_F5:           return Key::F5;
		case CL_KEY_F6:           return Key::F6;
		case CL_KEY_F7:           return Key::F7;
		case CL_KEY_F8:           return Key::F8;
		case CL_KEY_F9:           return Key::F9;
		case CL_KEY_F10:          return Key::F10;
		case CL_KEY_F11:          return Key::F11;
		case CL_KEY_F12:          return Key::F12;
		case CL_KEY_F13:          return Key::F13;
		case CL_KEY_F14:          return Key::F14;
		case CL_KEY_F15:          return Key::F15;
		case CL_KEY_NUMLOCK:      return Key::NumLock;
		case CL_KEY_SCROLL:       return Key::ScrollLock;
		case CL_KEY_RSHIFT:       return Key::RightShift;
		case CL_KEY_LSHIFT:       return Key::LeftShift;
		case CL_KEY_RCONTROL:     return Key::RightControl;
		case CL_KEY_LCONTROL:     return Key::LeftControl;
#ifdef CL_KEY_RALT
		case CL_KEY_RALT:         return Key::RightAlt;
#endif
#ifdef CL_KEY_LALT
		case CL_KEY_LALT:         return Key::LeftAlt;
#endif
		case CL_KEY_LWIN:         return Key::LeftWindows;
		case CL_KEY_RWIN:         return Key::RightWindows;
		//case CL_KEY_SYSREQ:       return Key::SysRq;
		case CL_KEY_MENU:         return Key::AppMenu;
		//case CL_KEY_POWER:        return Key::Power;

			// --Keys I'm not sure about--
		case ':':                 return Key::Colon;
		case ';':                 return Key::Semicolon;
		case '=':                 return Key::Equals;
		//case '(':                 return Key::LeftBracket;
		case ')':                 return Key::RightBracket;
		//case '\\':                return Key::Backslash;

		default:                return 0;
		}
		return 0;
	}

}

#endif
