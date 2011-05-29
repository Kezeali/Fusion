/*
*  Copyright (c) 2006-2010 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef H_FusionEngine_GUI
#define H_FusionEngine_GUI

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <Rocket/Core.h>
#include <boost/signals2.hpp>

#include "FusionState.h"
#include "FusionSingleton.h"

#include "FusionRocketInterface.h"
#include "FusionScriptModule.h"
#include "FusionSlotContainer.h"

namespace Rocket {
	namespace Controls {
		class DataFormatter;
	}
}


namespace FusionEngine
{

	//! Secondary context class
	class SecondContext
	{
	public:
			void Update(float time);
			void Draw();
			void SetDimensions(const Vector2 &size);
	};

	/*!
	 * \brief
	 * Manager for libRocket GUI whatsit (tm)
	 */
	class GUI : public System, public Singleton<GUI>
	{
	public:
		//! Basic constructor.
		GUI();
		//! Constructor.
		GUI(CL_DisplayWindow window);

		//! Destructor
		~GUI();

	public:
		//! Sets gui config
		virtual void Configure(const std::string& configfilename);
		//! Inits the gui
		virtual bool Initialise();

		virtual const std::string &GetName() const;

		void LoadFonts(const char* directory);

		//! Updates the inputs
		virtual void Update(float split);

		//! Draws the gui
		virtual void Draw();

		//! Unbinds
		virtual void CleanUp();

		Rocket::Core::Context* GetContext() const;
		//! Returns the console window document
		Rocket::Core::ElementDocument *GetConsoleWindow() const;

		void InitializeDebugger();

		void ShowDebugger();

		void HideDebugger();

		bool DebuggerIsVisible() const;

		//! Sets the period of time the mouse will be shown after it stops moving
		void SetMouseShowPeriod(unsigned int period);
		unsigned int GetMouseShowPeriod() const;

		void ShowMouse();

		//! Sends fake mouse position data to this GUI's context
		void SetMouseCursorPosition(int x, int y, int modifier = 0);
		//! Helper fn. for angelscript registration
		/*
		* \see SetMouseCursorPosition()
		*/
		void SetMouseCursorPosition_default(int x, int y);

		static void Register(ScriptManager *manager);

		void SetModule(ModulePtr module);

	protected:
		//! Holds slots
		SlotContainer m_Slots;

		//! How long after the mouse stops moving until it fades
		unsigned int m_MouseShowPeriod;
		//! When this reaches zero, the mouse will be hidden
		int m_ShowMouseTimer;

		static const float s_ClickPausePeriod;
		// Stop mouse-move events from being processed while this is > 0 - this var is set to
		//  a milisecond count after a click event is processed (to make double-clicking easier)
		float m_ClickPause;

		CL_DisplayWindow m_Display;

		bool m_Initialised;

		RocketSystem *m_RocketSystem;
		RocketRenderer *m_RocketRenderer;
		RocketFileSystem *m_RocketFileSys;

		std::list<std::tr1::shared_ptr<Rocket::Controls::DataFormatter>> m_DataFormatters;

		Rocket::Core::Context* m_Context;
		Rocket::Core::ElementDocument *m_ConsoleDocument;

		bool m_DebuggerInitialized;

		boost::signals2::connection m_ModuleConnection;

	public:
		void initScripting(ScriptManager* eng);
		//! Called when the module set with SetModule is built
		void onModuleBuild(BuildModuleEvent& event);
		//! Tells the gui system when a mouse button is pressed
		virtual void onMouseDown(const CL_InputEvent &ev, const CL_InputState &state);
		//! Tells the gui system when a mouse button is released
		virtual void onMouseUp(const CL_InputEvent &ev, const CL_InputState &state);
		//! Tells the gui system when the mouse moves
		virtual void onMouseMove(const CL_InputEvent &ev, const CL_InputState &state);

		//! Tells the gui system when a keyboard key goes down
		virtual void onKeyDown(const CL_InputEvent &ev, const CL_InputState &state);
		//! Tells the gui system when a keyboard key is released
		virtual void onKeyUp(const CL_InputEvent &ev, const CL_InputState &state);

		void onResize(int x, int y);
	};


	static Rocket::Core::Input::KeyIdentifier CLKeyToRocketKeyIdent(int key)
	{
		using namespace Rocket;

		switch (key)
		{
		case CL_KEY_BACKSPACE:    return Core::Input::KI_BACK;
		case CL_KEY_TAB:          return Core::Input::KI_TAB;
		case CL_KEY_RETURN:       return Core::Input::KI_RETURN;
		case CL_KEY_PAUSE:        return Core::Input::KI_PAUSE;
		case CL_KEY_ESCAPE:       return Core::Input::KI_ESCAPE;
		case CL_KEY_SPACE:        return Core::Input::KI_SPACE;
		case CL_KEY_0:            return Core::Input::KI_0;
		case CL_KEY_1:            return Core::Input::KI_1;
		case CL_KEY_2:            return Core::Input::KI_2;
		case CL_KEY_3:            return Core::Input::KI_3;
		case CL_KEY_4:            return Core::Input::KI_4;
		case CL_KEY_5:            return Core::Input::KI_5;
		case CL_KEY_6:            return Core::Input::KI_6;
		case CL_KEY_7:            return Core::Input::KI_7;
		case CL_KEY_8:            return Core::Input::KI_8;
		case CL_KEY_9:            return Core::Input::KI_9;
		case CL_KEY_A:            return Core::Input::KI_A;
		case CL_KEY_B:            return Core::Input::KI_B;
		case CL_KEY_C:            return Core::Input::KI_C;
		case CL_KEY_D:            return Core::Input::KI_D;
		case CL_KEY_E:            return Core::Input::KI_E;
		case CL_KEY_F:            return Core::Input::KI_F;
		case CL_KEY_G:            return Core::Input::KI_G;
		case CL_KEY_H:            return Core::Input::KI_H;
		case CL_KEY_I:            return Core::Input::KI_I;
		case CL_KEY_J:            return Core::Input::KI_J;
		case CL_KEY_K:            return Core::Input::KI_K;
		case CL_KEY_L:            return Core::Input::KI_L;
		case CL_KEY_M:            return Core::Input::KI_M;
		case CL_KEY_N:            return Core::Input::KI_N;
		case CL_KEY_O:            return Core::Input::KI_O;
		case CL_KEY_P:            return Core::Input::KI_P;
		case CL_KEY_Q:            return Core::Input::KI_Q;
		case CL_KEY_R:            return Core::Input::KI_R;
		case CL_KEY_S:            return Core::Input::KI_S;
		case CL_KEY_T:            return Core::Input::KI_T;
		case CL_KEY_U:            return Core::Input::KI_U;
		case CL_KEY_V:            return Core::Input::KI_V;
		case CL_KEY_W:            return Core::Input::KI_W;
		case CL_KEY_X:            return Core::Input::KI_X;
		case CL_KEY_Y:            return Core::Input::KI_Y;
		case CL_KEY_Z:            return Core::Input::KI_Z;
		case CL_KEY_DELETE:       return Core::Input::KI_DELETE;
		case CL_KEY_NUMPAD0:      return Core::Input::KI_NUMPAD0;
		case CL_KEY_NUMPAD1:      return Core::Input::KI_NUMPAD1;
		case CL_KEY_NUMPAD2:      return Core::Input::KI_NUMPAD2;
		case CL_KEY_NUMPAD3:      return Core::Input::KI_NUMPAD3;
		case CL_KEY_NUMPAD4:      return Core::Input::KI_NUMPAD4;
		case CL_KEY_NUMPAD5:      return Core::Input::KI_NUMPAD5;
		case CL_KEY_NUMPAD6:      return Core::Input::KI_NUMPAD6;
		case CL_KEY_NUMPAD7:      return Core::Input::KI_NUMPAD7;
		case CL_KEY_NUMPAD8:      return Core::Input::KI_NUMPAD8;
		case CL_KEY_NUMPAD9:      return Core::Input::KI_NUMPAD9;
		case CL_KEY_DECIMAL:      return Core::Input::KI_DECIMAL;
		case CL_KEY_DIVIDE:       return Core::Input::KI_DIVIDE;
		case CL_KEY_MULTIPLY:     return Core::Input::KI_MULTIPLY;
		case CL_KEY_SUBTRACT:     return Core::Input::KI_SUBTRACT;
		case CL_KEY_ADD:          return Core::Input::KI_ADD;
		case CL_KEY_UP:           return Core::Input::KI_UP;
		case CL_KEY_DOWN:         return Core::Input::KI_DOWN;
		case CL_KEY_RIGHT:        return Core::Input::KI_RIGHT;
		case CL_KEY_LEFT:         return Core::Input::KI_LEFT;
		case CL_KEY_INSERT:       return Core::Input::KI_INSERT;
		case CL_KEY_HOME:         return Core::Input::KI_HOME;
		case CL_KEY_END:          return Core::Input::KI_END;
		case CL_KEY_PRIOR:        return Core::Input::KI_PRIOR;
		case CL_KEY_NEXT:         return Core::Input::KI_NEXT;
		case CL_KEY_F1:           return Core::Input::KI_F1;
		case CL_KEY_F2:           return Core::Input::KI_F2;
		case CL_KEY_F3:           return Core::Input::KI_F3;
		case CL_KEY_F4:           return Core::Input::KI_F4;
		case CL_KEY_F5:           return Core::Input::KI_F5;
		case CL_KEY_F6:           return Core::Input::KI_F6;
		case CL_KEY_F7:           return Core::Input::KI_F7;
		case CL_KEY_F8:           return Core::Input::KI_F8;
		case CL_KEY_F9:           return Core::Input::KI_F9;
		case CL_KEY_F10:          return Core::Input::KI_F10;
		case CL_KEY_F11:          return Core::Input::KI_F11;
		case CL_KEY_F12:          return Core::Input::KI_F12;
		case CL_KEY_F13:          return Core::Input::KI_F13;
		case CL_KEY_F14:          return Core::Input::KI_F14;
		case CL_KEY_F15:          return Core::Input::KI_F15;
		case CL_KEY_F16:          return Core::Input::KI_F16;
		case CL_KEY_F17:          return Core::Input::KI_F17;
		case CL_KEY_F18:          return Core::Input::KI_F18;
		case CL_KEY_F19:          return Core::Input::KI_F19;
		case CL_KEY_NUMLOCK:      return Core::Input::KI_NUMLOCK;
		case CL_KEY_SCROLL:       return Core::Input::KI_SCROLL;
		case CL_KEY_RSHIFT:       return Core::Input::KI_RSHIFT;
		case CL_KEY_LSHIFT:       return Core::Input::KI_LSHIFT;
		case CL_KEY_RCONTROL:     return Core::Input::KI_RCONTROL;
		case CL_KEY_LCONTROL:     return Core::Input::KI_LCONTROL;
		case CL_KEY_LWIN:         return Core::Input::KI_LWIN;
		case CL_KEY_RWIN:         return Core::Input::KI_RWIN;
		case CL_KEY_MENU:         return Core::Input::KI_LMENU;

		default:                  return Core::Input::KI_UNKNOWN;
		}
		return Core::Input::KI_UNKNOWN;
	}

//	static CEGUI::uint CLKeyToCEGUIKey(int key)
//	{
//		using namespace CEGUI;
//
//		switch (key)
//		{
//		case CL_KEY_BACKSPACE:    return Key::Backspace;
//		case CL_KEY_TAB:          return Key::Tab;
//		case CL_KEY_RETURN:       return Key::Return;
//		case CL_KEY_PAUSE:        return Key::Pause;
//		case CL_KEY_ESCAPE:       return Key::Escape;
//		case CL_KEY_SPACE:        return Key::Space;
//		case CL_KEY_COMMA:        return Key::Comma;
//#ifdef CL_KEY_MINUS
//		case CL_KEY_MINUS:        return Key::Minus;
//#endif
//#ifdef CL_KEY_PERIOD
//		case CL_KEY_PERIOD:       return Key::Period;
//#endif
//		case CL_KEY_0:            return Key::Zero;
//		case CL_KEY_1:            return Key::One;
//		case CL_KEY_2:            return Key::Two;
//		case CL_KEY_3:            return Key::Three;
//		case CL_KEY_4:            return Key::Four;
//		case CL_KEY_5:            return Key::Five;
//		case CL_KEY_6:            return Key::Six;
//		case CL_KEY_7:            return Key::Seven;
//		case CL_KEY_8:            return Key::Eight;
//		case CL_KEY_9:            return Key::Nine;
//		case CL_KEY_A:            return Key::A;
//		case CL_KEY_B:            return Key::B;
//		case CL_KEY_C:            return Key::C;
//		case CL_KEY_D:            return Key::D;
//		case CL_KEY_E:            return Key::E;
//		case CL_KEY_F:            return Key::F;
//		case CL_KEY_G:            return Key::G;
//		case CL_KEY_H:            return Key::H;
//		case CL_KEY_I:            return Key::I;
//		case CL_KEY_J:            return Key::J;
//		case CL_KEY_K:            return Key::K;
//		case CL_KEY_L:            return Key::L;
//		case CL_KEY_M:            return Key::M;
//		case CL_KEY_N:            return Key::N;
//		case CL_KEY_O:            return Key::O;
//		case CL_KEY_P:            return Key::P;
//		case CL_KEY_Q:            return Key::Q;
//		case CL_KEY_R:            return Key::R;
//		case CL_KEY_S:            return Key::S;
//		case CL_KEY_T:            return Key::T;
//		case CL_KEY_U:            return Key::U;
//		case CL_KEY_V:            return Key::V;
//		case CL_KEY_W:            return Key::W;
//		case CL_KEY_X:            return Key::X;
//		case CL_KEY_Y:            return Key::Y;
//		case CL_KEY_Z:            return Key::Z;
//		case CL_KEY_DELETE:       return Key::Delete;
//		case CL_KEY_NUMPAD0:      return Key::Numpad0;
//		case CL_KEY_NUMPAD1:      return Key::Numpad1;
//		case CL_KEY_NUMPAD2:      return Key::Numpad2;
//		case CL_KEY_NUMPAD3:      return Key::Numpad3;
//		case CL_KEY_NUMPAD4:      return Key::Numpad4;
//		case CL_KEY_NUMPAD5:      return Key::Numpad5;
//		case CL_KEY_NUMPAD6:      return Key::Numpad6;
//		case CL_KEY_NUMPAD7:      return Key::Numpad7;
//		case CL_KEY_NUMPAD8:      return Key::Numpad8;
//		case CL_KEY_NUMPAD9:      return Key::Numpad9;
//#ifdef CL_KEY_DECIMAL
//		case CL_KEY_DECIMAL:      return Key::Decimal;
//#endif
//		case CL_KEY_DIVIDE:       return Key::Divide; //or Key::Slash
//#ifdef CL_KEY_MULTIPLY
//		case CL_KEY_MULTIPLY:      return Key::Multiply;
//#endif
//		case CL_KEY_SUBTRACT:     return Key::Subtract; // or Key::Minus;
//#ifdef CL_KEY_ADD
//		case CL_KEY_ADD:          return Key::Add;
//#endif
//		case CL_KEY_NUMPAD_ENTER:        return Key::NumpadEnter;
//			// My numpad has no equals key...
//		//case '=':                 return Key::NumpadEquals;
//		case CL_KEY_UP:           return Key::ArrowUp;
//		case CL_KEY_DOWN:         return Key::ArrowDown;
//		case CL_KEY_RIGHT:        return Key::ArrowRight;
//		case CL_KEY_LEFT:         return Key::ArrowLeft;
//		case CL_KEY_INSERT:       return Key::Insert;
//		case CL_KEY_HOME:         return Key::Home;
//		case CL_KEY_END:          return Key::End;
//		case CL_KEY_PRIOR:        return Key::PageUp;
//		case CL_KEY_NEXT:         return Key::PageDown;
//		case CL_KEY_F1:           return Key::F1;
//		case CL_KEY_F2:           return Key::F2;
//		case CL_KEY_F3:           return Key::F3;
//		case CL_KEY_F4:           return Key::F4;
//		case CL_KEY_F5:           return Key::F5;
//		case CL_KEY_F6:           return Key::F6;
//		case CL_KEY_F7:           return Key::F7;
//		case CL_KEY_F8:           return Key::F8;
//		case CL_KEY_F9:           return Key::F9;
//		case CL_KEY_F10:          return Key::F10;
//		case CL_KEY_F11:          return Key::F11;
//		case CL_KEY_F12:          return Key::F12;
//		case CL_KEY_F13:          return Key::F13;
//		case CL_KEY_F14:          return Key::F14;
//		case CL_KEY_F15:          return Key::F15;
//		case CL_KEY_NUMLOCK:      return Key::NumLock;
//		case CL_KEY_SCROLL:       return Key::ScrollLock;
//		case CL_KEY_RSHIFT:       return Key::RightShift;
//		case CL_KEY_LSHIFT:       return Key::LeftShift;
//		case CL_KEY_RCONTROL:     return Key::RightControl;
//		case CL_KEY_LCONTROL:     return Key::LeftControl;
//#ifdef CL_KEY_RALT
//		case CL_KEY_RALT:         return Key::RightAlt;
//#endif
//#ifdef CL_KEY_LALT
//		case CL_KEY_LALT:         return Key::LeftAlt;
//#endif
//		case CL_KEY_LWIN:         return Key::LeftWindows;
//		case CL_KEY_RWIN:         return Key::RightWindows;
//		//case CL_KEY_SYSREQ:       return Key::SysRq;
//		case CL_KEY_MENU:         return Key::AppMenu;
//		//case CL_KEY_POWER:        return Key::Power;
//
//			// --Keys I'm not sure about--
//		case ':':                 return Key::Colon;
//		case ';':                 return Key::Semicolon;
//		case '=':                 return Key::Equals;
//		//case '(':                 return Key::LeftBracket;
//		case ')':                 return Key::RightBracket;
//		//case '\\':                return Key::Backslash;
//
//		default:                return 0;
//		}
//		return 0;
//	}

}

#endif
