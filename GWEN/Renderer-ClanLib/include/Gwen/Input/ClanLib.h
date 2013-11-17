#pragma once
#ifndef GWEN_INPUT_SFML_H
#define GWEN_INPUT_SFML_H

#include "Gwen/InputHandler.h"
#include "Gwen/Gwen.h"
#include "Gwen/Controls/Canvas.h"

#include <ClanLib/display.h>

namespace Gwen
{
	namespace Input
	{
		class ClanLib
		{
		public:

			ClanLib()
			{
				m_Canvas = NULL;
				m_MouseX = 0;
				m_MouseY = 0;
				m_LeftMouseDown = false;
				m_RightMouseDown = false;
				m_MiddleMouseDown = false;
				m_XButton1MouseDown = false;
				m_XButton2MouseDown = false;
			}

			void Initialize( Gwen::Controls::Canvas* c )
			{
				m_Canvas = c;
			}

			unsigned char TranslateKeyCode(int key)
			{
				switch (key)
				{
				case clan::keycode_backspace:    return Gwen::Key::Backspace;
				case clan::keycode_tab:          return Gwen::Key::Tab;
				case clan::keycode_return:       return Gwen::Key::Return;
				case clan::keycode_escape:       return Gwen::Key::Escape;
				case clan::keycode_space:        return Gwen::Key::Space;
				case clan::keycode_delete:       return Gwen::Key::Delete;
				case clan::keycode_up:           return Gwen::Key::Up;
				case clan::keycode_down:         return Gwen::Key::Down;
				case clan::keycode_right:        return Gwen::Key::Right;
				case clan::keycode_left:         return Gwen::Key::Left;
				case clan::keycode_home:         return Gwen::Key::Home;
				case clan::keycode_end:          return Gwen::Key::End;
				case clan::keycode_shift:        return Gwen::Key::Shift;
				case clan::keycode_rshift:       return Gwen::Key::Shift;
				case clan::keycode_lshift:       return Gwen::Key::Shift;
				case clan::keycode_control:      return Gwen::Key::Control;
				case clan::keycode_rcontrol:     return Gwen::Key::Control;
				case clan::keycode_lcontrol:     return Gwen::Key::Control;
				case clan::keycode_menu:         return Gwen::Key::Alt;
				default:                         return Gwen::Key::Invalid;
				}
				return Gwen::Key::Invalid;
			}

			void onMouseDown(const clan::InputEvent &ev)
			{
				m_ClickPause = 0.01f;//s_ClickPausePeriod;

				switch(ev.id)
				{
				case clan::mouse_left:
					m_Canvas->InputMouseButton(0, true);
					break;
				case clan::mouse_right:
					m_Canvas->InputMouseButton(1, true);
					break;
				case clan::mouse_middle:
					m_Canvas->InputMouseButton(2, true);
					break;
				case clan::mouse_xbutton1:
					m_Canvas->InputMouseButton(3, true);
					break;
				case clan::mouse_xbutton2:
					m_Canvas->InputMouseButton(4, true);
					break;
				case clan::mouse_wheel_up:
					m_Canvas->InputMouseWheel(-1);
					break;
				case clan::mouse_wheel_down:
					m_Canvas->InputMouseWheel(1);
					break;
				}
			}

			void onMouseUp(const clan::InputEvent &ev)
			{
				m_ClickPause = 0.01f;//s_ClickPausePeriod;

				switch(ev.id)
				{
				case clan::mouse_left:
					m_Canvas->InputMouseButton(0, false);
					break;
				case clan::mouse_right:
					m_Canvas->InputMouseButton(1, false);
					break;
				case clan::mouse_middle:
					m_Canvas->InputMouseButton(2, false);
					break;
				case clan::mouse_xbutton1:
					m_Canvas->InputMouseButton(3, false);
					break;
				case clan::mouse_xbutton2:
					m_Canvas->InputMouseButton(4, false);
					break;
				case clan::mouse_wheel_up:
					m_Canvas->InputMouseWheel(0);
					break;
				case clan::mouse_wheel_down:
					m_Canvas->InputMouseWheel(0);
					break;
				}
			}

			void onMouseMove(const clan::InputEvent &ev)
			{
				if (m_ClickPause <= 0)
				{
					int dx = ev.mouse_pos.x - m_MouseX;
					int dy = ev.mouse_pos.y - m_MouseY;
					m_MouseX = ev.mouse_pos.x;
					m_MouseY = ev.mouse_pos.y;

					m_Canvas->InputMouseMoved( m_MouseX, m_MouseY, dx, dy );
				}

				//if (m_ShowMouseTimer <= 0 && m_MouseShowPeriod > 0)
				//{
				//	m_ShowMouseTimer = m_MouseShowPeriod;
				//	m_Context->ShowMouseCursor(true);
				//}
			}

			inline bool notEscapeOrDelete(int id)
			{
				return id != clan::keycode_backspace && id != clan::keycode_escape && id != clan::keycode_delete;
			}

			void onKeyDown(const clan::InputEvent &ev)
			{
				// Grab characters
				if (!ev.alt && !ev.ctrl && !ev.str.empty() && notEscapeOrDelete(ev.id))
				{
					m_Canvas->InputCharacter(Gwen::Utility::StringToUnicode(ev.str)[0]);
				}

				m_Canvas->InputKey(TranslateKeyCode(ev.id), true);

				//SendToConsole("Key Down");
			}

			void onKeyUp(const clan::InputEvent &ev)
			{
				m_Canvas->InputKey(TranslateKeyCode(ev.id), false);

				//SendToConsole("Key Up");
			}

		protected:

			Gwen::Controls::Canvas*	m_Canvas;
			int m_MouseX;
			int m_MouseY;
			bool m_LeftMouseDown;
			bool m_RightMouseDown;
			bool m_MiddleMouseDown;
			bool m_XButton1MouseDown;
			bool m_XButton2MouseDown;
			float m_ClickPause;

		};
	}
}
#endif
