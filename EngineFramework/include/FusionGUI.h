/*
*  Copyright (c) 2006-2012 Fusion Project Team
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

#ifndef H_FusionGUI
#define H_FusionGUI

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <Rocket/Core.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/signals2.hpp>

#include "FusionSingleton.h"

#include "FusionRocketInterface.h"
#include "FusionRocketReferenceCountable.h"
#include "FusionScriptModule.h"
#include "FusionSlotContainer.h"

namespace Rocket {
	namespace Controls {
		class DataFormatter;
		class DataSource;
	}
}

namespace FusionEngine
{

	class MessageBoxMaker;

	//! Secondary context class
	class GUIContext
	{
	public:
		GUIContext();
		GUIContext(const std::string& name, clan::InputContext ic, const Vector2i& size, bool enable_mouse = true);
		GUIContext(Rocket::Core::Context* context, clan::InputContext ic, bool enable_mouse = true);
		~GUIContext();

		void Update(float dt);
		void Draw();
		void SetDimensions(const Vector2i& size);

		//! Sets the period of time the mouse will be shown after it stops moving
		void SetMouseShowPeriod(float period);
		float GetMouseShowPeriod() const;

		void ShowMouse();

		//! Sends fake mouse position data to this GUI's context
		void SetMouseCursorPosition(int x, int y, int modifier = 0);

		Rocket::Core::Context* m_Context;

	private:
		//! How long after the mouse stops moving until it fades
		float m_MouseShowPeriod;
		//! When this reaches zero, the mouse will be hidden
		float m_ShowMouseTimer;

		static const float s_ClickPausePeriod;
		// Stop mouse-move events from being processed while this is > 0 - this var is set to
		//  a milisecond count after a click event is processed (to make double-clicking easier)
		float m_ClickPause;

		SlotContainer m_Slots;

		//! Tells the gui system when a mouse button is pressed
		void onMouseDown(const clan::InputEvent &ev);
		//! Tells the gui system when a mouse button is released
		void onMouseUp(const clan::InputEvent &ev);
		//! Tells the gui system when the mouse moves
		void onMouseMove(const clan::InputEvent &ev);

		//! Tells the gui system when a keyboard key goes down
		void onKeyDown(const clan::InputEvent &ev);
		//! Tells the gui system when a keyboard key is released
		void onKeyUp(const clan::InputEvent &ev);

		GUIContext(const GUIContext& other)
			: m_Context(other.m_Context),
			m_MouseShowPeriod(other.m_MouseShowPeriod),
			m_ShowMouseTimer(other.m_ShowMouseTimer),
			m_ClickPause(other.m_ClickPause),
			m_Slots(other.m_Slots)
		{
			m_Context->AddReference();
		}

		GUIContext(GUIContext&& other)
			: m_Context(std::move(other.m_Context)),
			m_MouseShowPeriod(other.m_MouseShowPeriod),
			m_ShowMouseTimer(other.m_ShowMouseTimer),
			m_ClickPause(other.m_ClickPause),
			m_Slots(std::move(other.m_Slots))
		{
			other.m_Context = 0;
		}

		GUIContext& operator= (const GUIContext& other)
		{
			m_Context = other.m_Context;
			m_Context->AddReference();
			m_MouseShowPeriod = other.m_MouseShowPeriod;
			m_ShowMouseTimer = other.m_ShowMouseTimer;
			m_ClickPause = other.m_ClickPause;
			m_Slots = other.m_Slots;
			return *this;
		}

		GUIContext& operator= (GUIContext&& other)
		{
			m_Context = std::move(other.m_Context);
			other.m_Context = 0;
			m_MouseShowPeriod = other.m_MouseShowPeriod;
			m_ShowMouseTimer = other.m_ShowMouseTimer;
			m_ClickPause = other.m_ClickPause;
			m_Slots = std::move(other.m_Slots);
			return *this;
		}
	};

	/*!
	 * \brief
	 * Manager for libRocket GUI whatsit (tm)
	 */
	class GUI : public Singleton<GUI>
	{
	public:
		//! Basic constructor.
		GUI();
		//! Constructor.
		GUI(clan::Canvas canvas);

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

		const std::shared_ptr<GUIContext>& CreateContext(const std::string& name, Vector2i size = Vector2i(0, 0));

		Rocket::Core::Context* GetContext(const std::string& name = "screen") const;

		//! Returns the console window document
		Rocket::Core::ElementDocument *GetConsoleWindow() const;

		void InitialiseConsole(ScriptManager* script_manager);

		void InitializeDebugger(const std::string& context);

		void ShowDebugger();

		void HideDebugger();

		bool DebuggerIsVisible() const;

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
		//  a millisecond count after a click event is processed (to make double-clicking easier)
		float m_ClickPause;

		clan::Canvas m_Canvas;

		bool m_Initialised;

		boost::intrusive_ptr<RocketSystem> m_RocketSystem;
		boost::intrusive_ptr<RocketRenderer> m_RocketRenderer;
		boost::intrusive_ptr<RocketFileSystem> m_RocketFileSys;

		std::list<std::shared_ptr<Rocket::Controls::DataSource>> m_DataSources;
		std::list<std::shared_ptr<Rocket::Controls::DataFormatter>> m_DataFormatters;

		std::unique_ptr<MessageBoxMaker> m_MessageBoxMaker;

		std::map<std::string, std::shared_ptr<GUIContext>> m_Contexts;

		//Rocket::Core::Context* m_Context;
		Rocket::Core::ElementDocument *m_ConsoleDocument;

		bool m_DebuggerInitialized;

		boost::signals2::connection m_ModuleConnection;

	public:
		void initScripting(ScriptManager* eng);
		//! Called when the module set with SetModule is built
		void onModuleBuild(BuildModuleEvent& event);

		void onResize(int x, int y);
	};

	namespace
	{
		Rocket::Core::Input::KeyIdentifier CLKeyToRocketKeyIdent(int key)
		{
			using namespace Rocket;

			switch (key)
			{
			case keycode_backspace:    return Core::Input::KI_BACK;
			case keycode_tab:          return Core::Input::KI_TAB;
			case keycode_return:       return Core::Input::KI_RETURN;
			case keycode_pause:        return Core::Input::KI_PAUSE;
			case keycode_escape:       return Core::Input::KI_ESCAPE;
			case keycode_space:        return Core::Input::KI_SPACE;
			case keycode_0:            return Core::Input::KI_0;
			case keycode_1:            return Core::Input::KI_1;
			case keycode_2:            return Core::Input::KI_2;
			case keycode_3:            return Core::Input::KI_3;
			case keycode_4:            return Core::Input::KI_4;
			case keycode_5:            return Core::Input::KI_5;
			case keycode_6:            return Core::Input::KI_6;
			case keycode_7:            return Core::Input::KI_7;
			case keycode_8:            return Core::Input::KI_8;
			case keycode_9:            return Core::Input::KI_9;
			case keycode_a:            return Core::Input::KI_A;
			case keycode_b:            return Core::Input::KI_B;
			case keycode_c:            return Core::Input::KI_C;
			case keycode_d:            return Core::Input::KI_D;
			case keycode_e:            return Core::Input::KI_E;
			case keycode_f:            return Core::Input::KI_F;
			case keycode_g:            return Core::Input::KI_G;
			case keycode_h:            return Core::Input::KI_H;
			case keycode_i:            return Core::Input::KI_I;
			case keycode_j:            return Core::Input::KI_J;
			case keycode_k:            return Core::Input::KI_K;
			case keycode_l:            return Core::Input::KI_L;
			case keycode_m:            return Core::Input::KI_M;
			case keycode_n:            return Core::Input::KI_N;
			case keycode_o:            return Core::Input::KI_O;
			case keycode_p:            return Core::Input::KI_P;
			case keycode_q:            return Core::Input::KI_Q;
			case keycode_r:            return Core::Input::KI_R;
			case keycode_s:            return Core::Input::KI_S;
			case keycode_t:            return Core::Input::KI_T;
			case keycode_u:            return Core::Input::KI_U;
			case keycode_v:            return Core::Input::KI_V;
			case keycode_w:            return Core::Input::KI_W;
			case keycode_x:            return Core::Input::KI_X;
			case keycode_y:            return Core::Input::KI_Y;
			case keycode_z:            return Core::Input::KI_Z;
			case keycode_delete:       return Core::Input::KI_DELETE;
			case keycode_numpad0:      return Core::Input::KI_NUMPAD0;
			case keycode_numpad1:      return Core::Input::KI_NUMPAD1;
			case keycode_numpad2:      return Core::Input::KI_NUMPAD2;
			case keycode_numpad3:      return Core::Input::KI_NUMPAD3;
			case keycode_numpad4:      return Core::Input::KI_NUMPAD4;
			case keycode_numpad5:      return Core::Input::KI_NUMPAD5;
			case keycode_numpad6:      return Core::Input::KI_NUMPAD6;
			case keycode_numpad7:      return Core::Input::KI_NUMPAD7;
			case keycode_numpad8:      return Core::Input::KI_NUMPAD8;
			case keycode_numpad9:      return Core::Input::KI_NUMPAD9;
			case keycode_decimal:      return Core::Input::KI_DECIMAL;
			case keycode_divide:       return Core::Input::KI_DIVIDE;
			case keycode_multiply:     return Core::Input::KI_MULTIPLY;
			case keycode_subtract:     return Core::Input::KI_SUBTRACT;
			case keycode_add:          return Core::Input::KI_ADD;
			case keycode_up:           return Core::Input::KI_UP;
			case keycode_down:         return Core::Input::KI_DOWN;
			case keycode_right:        return Core::Input::KI_RIGHT;
			case keycode_left:         return Core::Input::KI_LEFT;
			case keycode_insert:       return Core::Input::KI_INSERT;
			case keycode_home:         return Core::Input::KI_HOME;
			case keycode_end:          return Core::Input::KI_END;
			case keycode_prior:        return Core::Input::KI_PRIOR;
			case keycode_next:         return Core::Input::KI_NEXT;
			case keycode_f1:           return Core::Input::KI_F1;
			case keycode_f2:           return Core::Input::KI_F2;
			case keycode_f3:           return Core::Input::KI_F3;
			case keycode_f4:           return Core::Input::KI_F4;
			case keycode_f5:           return Core::Input::KI_F5;
			case keycode_f6:           return Core::Input::KI_F6;
			case keycode_f7:           return Core::Input::KI_F7;
			case keycode_f8:           return Core::Input::KI_F8;
			case keycode_f9:           return Core::Input::KI_F9;
			case keycode_f10:          return Core::Input::KI_F10;
			case keycode_f11:          return Core::Input::KI_F11;
			case keycode_f12:          return Core::Input::KI_F12;
			case keycode_f13:          return Core::Input::KI_F13;
			case keycode_f14:          return Core::Input::KI_F14;
			case keycode_f15:          return Core::Input::KI_F15;
			case keycode_f16:          return Core::Input::KI_F16;
			case keycode_f17:          return Core::Input::KI_F17;
			case keycode_f18:          return Core::Input::KI_F18;
			case keycode_f19:          return Core::Input::KI_F19;
			case keycode_numlock:      return Core::Input::KI_NUMLOCK;
			case keycode_scroll:       return Core::Input::KI_SCROLL;
			case keycode_rshift:       return Core::Input::KI_RSHIFT;
			case keycode_lshift:       return Core::Input::KI_LSHIFT;
			case keycode_rcontrol:     return Core::Input::KI_RCONTROL;
			case keycode_lcontrol:     return Core::Input::KI_LCONTROL;
			case keycode_lwin:         return Core::Input::KI_LWIN;
			case keycode_rwin:         return Core::Input::KI_RWIN;
			case keycode_menu:         return Core::Input::KI_LMENU;

			default:                  return Core::Input::KI_UNKNOWN;
			}
			return Core::Input::KI_UNKNOWN;
		}
	}

}

#endif
