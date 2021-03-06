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

#include "FusionEditorRectangleTool.h"

#include <ClanLib/core.h>

namespace FusionEngine
{

	EditorRectangleTool::EditorRectangleTool()
		: m_Active(false),
		m_Angle(0.0f),
		m_InitialAngle(0.0f),
		m_FeedbackAngle(0.0f),
		m_Action(Action::None)
	{}

	void EditorRectangleTool::Start(const Vector2& half_size, const Vector2& center, float angle, const RectangleToolCallback_t& done_callback)
	{
		m_HalfSize = half_size;
		m_Center = center;
		m_Angle = angle;
		m_DoneCallback = done_callback;

		m_InitialHalfSize = half_size;
		m_InitialCenter = center;
		m_InitialAngle = angle;

		m_FeedbackHalfSize = m_HalfSize;
		m_FeedbackCenter = m_Center;
		m_FeedbackAngle = m_Angle;

		CreateGui();

		m_Active = true;
	}

	void EditorRectangleTool::Finish()
	{
		m_DoneCallback(m_HalfSize, m_Center, m_Angle);
		m_DoneCallback = RectangleToolCallback_t();

		m_HalfSize = Vector2();
		m_Center = Vector2();
		m_Angle = 0.0f;

		m_InitialHalfSize = Vector2();
		m_InitialCenter = Vector2();
		m_InitialAngle = 0.0f;

		m_Action = Action::None;

		m_Active = false;
	}

	void EditorRectangleTool::Reset()
	{
		m_HalfSize = m_InitialHalfSize;
		m_Center = m_InitialCenter;
		m_Angle = m_InitialAngle;

		m_FeedbackHalfSize = m_HalfSize;
		m_FeedbackCenter = m_Center;
		m_FeedbackAngle = m_Angle;
	}

	void EditorRectangleTool::Cancel()
	{
		Reset();
		Finish();
	}

	void EditorRectangleTool::KeyChange(bool shift, bool ctrl, bool alt)
	{
		if (!m_MouseDown)
		{
			m_Action = Action::None;
			if (ctrl)
				m_Action = Action::Move;
			else if (shift)
				m_Action = Action::ResizeRelative;
			else if (alt)
				m_Action = Action::Aim;
			else
				m_Action = Action::Resize;
		}

		MouseMove(m_LastMousePos, shift, ctrl, alt);
	}

	void EditorRectangleTool::MouseMove(const Vector2& pos, bool shift, bool ctrl, bool alt)
	{
		m_LastMousePos = pos;

		m_FeedbackCenter = m_Center;

		if (m_MouseDown)
		{
			if (m_Action == Action::Resize)
			{
				m_FeedbackCenter = m_DragFrom;
				m_FeedbackHalfSize = pos - m_DragFrom;
			}
			else if (m_Action == Action::Aim)
			{
				auto aimVec = pos - m_Center;
				m_FeedbackAngle = atan2f(aimVec.y, aimVec.x);
			}
			else
			{
				const auto delta = pos - m_DragFrom;
				if (m_Action == Action::ResizeRelative)
				{
					m_FeedbackHalfSize = m_HalfSize + delta;
				}
				else if (m_Action == Action::Move)
				{
					m_FeedbackCenter = m_Center + delta;
				}
			}
		}
	}

	bool EditorRectangleTool::MousePress(const Vector2& pos, MouseInput key, bool shift, bool ctrl, bool alt)
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
			else if (alt)
				m_Action = Action::Aim;
			else
				m_Action = Action::Resize;
			m_DragFrom = pos;

			MouseMove(pos, shift, ctrl, alt);

			return true;
		}
		else
			return false;
	}

	bool EditorRectangleTool::MouseRelease(const Vector2& pos, MouseInput key, bool shift, bool ctrl, bool alt)
	{
		if (m_Action != Action::None)
		{
			const bool isScroll = key == MouseInput::ScrollDown || key == MouseInput::ScrollUp;
			if (!isScroll)
			{
				if (m_Action == Action::Resize)
				{
					m_Center = m_DragFrom;
					m_HalfSize = pos - m_Center;
				}
				else if (m_Action == Action::Aim)
				{
					auto aimVec = pos - m_Center;
					m_Angle = atan2f(aimVec.y, aimVec.x);
				}
				else
				{
					const auto delta = pos - m_DragFrom;
					if (m_Action == Action::ResizeRelative)
					{
						m_HalfSize += delta;
					}
					else if (m_Action == Action::Move)
					{
						m_Center += delta;
					}
				}

				m_MouseDown = false;

				m_Action = Action::None;
			}
			else if (m_Action == Action::ResizeRelative)
			{
				const float scrollAmount = key == MouseInput::ScrollUp ? 0.1f : -0.1f;
				m_Angle += scrollAmount;
			}

			return true;
		}
		else
			return false;
	}

	Vector2 clToFsn(const clan::Vec2f& vec)
	{
		return Vector2(vec.x, vec.y);
	}

	void EditorRectangleTool::Draw()
	{
		const clan::Colorf currentShapeColour(0.4f, 0.4f, 0.96f, 0.8f);
		const clan::Colorf modificationColour(0.6f, 0.6f, 0.98f, 0.5f);
		const clan::Colorf lineColour(0.98f, 0.98f, 0.6f, 0.6f);

		{
			const Vector2 topLeft = m_Center - m_HalfSize;
			const Vector2 bottomRight = m_Center + m_HalfSize;
			clan::Quadf quad(clan::Rectf(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y));
			quad.rotate(clan::Vec2f(m_Center.x, m_Center.y), clan::Angle(m_Angle, clan::angle_radians));

			m_DebugDraw.DrawSolidTriangle(clToFsn(quad.p), clToFsn(quad.q), clToFsn(quad.s), currentShapeColour);
			m_DebugDraw.DrawSolidTriangle(clToFsn(quad.q), clToFsn(quad.r), clToFsn(quad.s), currentShapeColour);
			m_DebugDraw.DrawSegment(m_Center, Vector2(m_Center.x + std::cosf(m_Angle) * m_HalfSize.x, m_Center.y + std::sinf(m_Angle) * m_HalfSize.x), lineColour);
		}

		if (m_MouseDown && m_Action != Action::None)
		{
			const Vector2 topLeft = m_FeedbackCenter - m_FeedbackHalfSize;
			const Vector2 bottomRight = m_FeedbackCenter + m_FeedbackHalfSize;
			clan::Quadf quad(clan::Rectf(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y));
			quad.rotate(clan::Vec2f(m_FeedbackCenter.x, m_FeedbackCenter.y), clan::Angle(m_FeedbackAngle, clan::angle_radians));

			m_DebugDraw.DrawSolidTriangle(clToFsn(quad.p), clToFsn(quad.q), clToFsn(quad.s), modificationColour);
			m_DebugDraw.DrawSolidTriangle(clToFsn(quad.q), clToFsn(quad.r), clToFsn(quad.s), modificationColour);
			m_DebugDraw.DrawSegment(m_FeedbackCenter, Vector2(m_FeedbackCenter.x + std::cosf(m_FeedbackAngle) * m_FeedbackHalfSize.x, m_FeedbackCenter.y + std::sinf(m_FeedbackAngle) * m_FeedbackHalfSize.x), lineColour);
		}
	}

	void EditorRectangleTool::CreateGui()
	{
	}

}
