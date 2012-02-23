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

		m_Active = false;
	}

	void EditorCircleTool::Reset()
	{
		m_Center = m_InitialCenter;
		m_Radius = m_InitialRadius;
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

		MouseMove(m_LastMousePos, 0, shift, ctrl, alt);
	}

	void EditorCircleTool::MouseMove(const Vector2& pos, int key, bool shift, bool ctrl, bool alt)
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

	void EditorCircleTool::MousePress(const Vector2& pos, int key, bool shift, bool ctrl, bool alt)
	{
		m_LastMousePos = pos;

		m_MouseDown = true;

		m_Action = Action::None;
		if (ctrl)
			m_Action = Action::Move;
		else if (shift)
			m_Action = Action::ResizeRelative;
		else
			m_Action = Action::Resize;
		m_DragFrom = pos;

		MouseMove(pos, 0, shift, ctrl, alt);
	}

	void EditorCircleTool::MouseRelease(const Vector2& pos, int key, bool shift, bool ctrl, bool alt)
	{
		const bool isScroll = key == CL_MOUSE_WHEEL_DOWN || key == CL_MOUSE_WHEEL_UP;
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
			const int scrollAmount = key == CL_MOUSE_WHEEL_DOWN ? 10 : -10;
			m_Radius += scrollAmount;
		}

		m_Action = Action::None;
	}

	void EditorCircleTool::Draw(CL_GraphicContext& gc)
	{
		CL_Colorf currentShapeColour(0.4f, 0.4f, 0.96f, 0.8f);
		CL_Colorf modificationColour(0.6f, 0.6f, 0.98f, 0.5f);

		if (m_Radius > 0.f)
			CL_Draw::circle(gc, m_Center.x, m_Center.y, m_Radius, currentShapeColour);

		if (m_MouseDown && m_Action != Action::None)
			CL_Draw::circle(gc, m_FeedbackCenter.x, m_FeedbackCenter.y, m_FeedbackRadius, modificationColour);
	}

}
