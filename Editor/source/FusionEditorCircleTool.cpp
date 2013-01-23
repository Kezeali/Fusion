/*
*  Copyright (c) 2012 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionEditorCircleTool.h"

#include "FusionGUI.h"
#include <ClanLib/core.h>

namespace FusionEngine
{

	EditorCircleTool::EditorCircleTool()
		: m_Active(false),
		m_Radius(0.0f),
		m_InitialRadius(0.0f),
		m_FeedbackRadius(0.0f),
		m_Action(Action::None)
	{}

	void EditorCircleTool::Start(const Vector2& center, float radius, const CircleToolCallback_t& done_callback)
	{
		m_Center = center;
		m_Radius = radius;
		m_DoneCallback = done_callback;

		m_InitialCenter = center;
		m_InitialRadius = radius;

		m_FeedbackCenter = m_Center;
		m_FeedbackRadius = m_Radius;

		CreateGui();

		m_Active = true;
	}

	void EditorCircleTool::Finish()
	{
		m_DoneCallback(m_Center, m_Radius);
		m_DoneCallback = CircleToolCallback_t();

		m_Center = Vector2();
		m_Radius = 0.0f;

		m_InitialCenter = Vector2();
		m_InitialRadius = 0.0f;

		m_Action = Action::None;

		if (m_GuiDoc)
			m_GuiDoc->Close();
		m_GuiDoc.reset();

		m_Active = false;
	}

	void EditorCircleTool::Reset()
	{
		m_Center = m_InitialCenter;
		m_Radius = m_InitialRadius;

		m_FeedbackCenter = m_Center;
		m_FeedbackRadius = m_Radius;
	}

	void EditorCircleTool::Cancel()
	{
		Reset();
		Finish();
	}

	void EditorCircleTool::KeyChange(bool shift, bool ctrl, bool alt)
	{
		if (!m_MouseDown)
		{
			m_Action = Action::None;
			if (ctrl)
				m_Action = Action::Move;
			else if (shift)
				m_Action = Action::ResizeRelative;
			else
				m_Action = Action::Resize;
		}

		MouseMove(m_LastMousePos, shift, ctrl, alt);
		
		// Block input if this key-press was used to set an action
		//return m_Action != Action::None;
	}

	void EditorCircleTool::MouseMove(const Vector2& pos, bool shift, bool ctrl, bool alt)
	{
		m_LastMousePos = pos;

		m_FeedbackCenter = m_Center;

		if (m_MouseDown)
		{
			if (m_Action == Action::Resize)
			{
				m_FeedbackRadius = (pos - m_Center).length();
			}
			else
			{
				const auto delta = pos - m_DragFrom;
				if (m_Action == Action::ResizeRelative)
				{
					m_FeedbackRadius = m_Radius + delta.y;
				}
				else if (m_Action == Action::Move)
				{
					m_FeedbackCenter = m_Center + delta;
				}
			}
		}
	}

	bool EditorCircleTool::MousePress(const Vector2& pos, MouseInput key, bool shift, bool ctrl, bool alt)
	{
		m_LastMousePos = pos;

		if (key == MouseInput::LeftButton)
		{
			m_MouseDown = true;

			m_Action = Action::None;
			if (ctrl)
				m_Action = Action::Move;
			else if (shift)
				m_Action = Action::ResizeRelative;
			else
				m_Action = Action::Resize;
			m_DragFrom = pos;

			MouseMove(pos, shift, ctrl, alt);

			return true;
		}
		else
			return false;
	}

	bool EditorCircleTool::MouseRelease(const Vector2& pos, MouseInput key, bool shift, bool ctrl, bool alt)
	{
		if (m_Action != Action::None)
		{
			const bool isScroll = key == MouseInput::ScrollDown || key == MouseInput::ScrollUp;
			if (!isScroll)
			{
				if (m_Action == Action::Resize)
				{
					m_Radius = (pos - m_Center).length();
				}
				else
				{
					const auto delta = pos - m_DragFrom;
					if (m_Action == Action::ResizeRelative)
					{
						m_Radius += delta.y;
					}
					else if (m_Action == Action::Move)
					{
						m_Center += delta;
					}
				}
			}
			else if (m_Action == Action::ResizeRelative)
			{
				const int scrollAmount = key == MouseInput::ScrollDown ? 10 : -10;
				m_Radius += scrollAmount;
			}

			m_Action = Action::None;

			return true;
		}
		else
			return false;
	}

	void EditorCircleTool::Draw(clan::Canvas& canvas)
	{
		const clan::Colorf currentShapeColour(0.4f, 0.4f, 0.96f, 0.8f);
		const clan::Colorf modificationColour(0.6f, 0.6f, 0.98f, 0.5f);

		canvas.draw_line(m_Center.x - 4.f, m_Center.y, m_Center.x + 4.f, m_Center.y, clan::Colorf(1.f, 0.f, 0.f));
		canvas.draw_line(m_Center.x, m_Center.y - 4.f, m_Center.x, m_Center.y + 4.f, clan::Colorf(0.f, 1.f, 0.f));
		if (m_Radius > 0.f)
			canvas.draw_circle(m_Center.x, m_Center.y, m_Radius, currentShapeColour);

		if (m_MouseDown && m_Action != Action::None)
			canvas.draw_circle(m_FeedbackCenter.x, m_FeedbackCenter.y, m_FeedbackRadius, modificationColour);
	}

	void EditorCircleTool::CreateGui()
	{
		if (!m_GuiDoc)
		{
			m_GuiDoc = GUI::getSingleton().GetContext("editor")->LoadDocument("/Data/core/gui/editor_shapetool_toolbar.rml");
			m_GuiDoc->RemoveReference();

			//if (auto body = m_GuiDoc->GetElementById("content"))
			//	Rocket::Core::Factory::InstanceElementText(body, "Circle Tool");

			if (auto title = m_GuiDoc->GetElementById("title"))
				Rocket::Core::Factory::InstanceElementText(title, "Circle Tool");
		}
		if (m_GuiDoc)
		{
			m_GuiDoc->Show();
		}
	}

}
