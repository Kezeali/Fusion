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

#include "FusionEditorPolygonTool.h"

#include <ClanLib/core.h>

namespace FusionEngine
{

	EditorPolygonTool::EditorPolygonTool()
		: m_Mode(Freeform),
		m_Active(false)
	{}

	void EditorPolygonTool::Start(const std::vector<Vector2>& verts, const PolygonToolCallback_t& done_callback, EditorPolygonTool::Mode mode)
	{
		m_Verts = verts;
		m_DoneCallback = done_callback;
		m_Mode = mode;

		m_Active = true;
	}

	void EditorPolygonTool::Finish()
	{
		m_DoneCallback(m_Verts);
		m_DoneCallback = PolygonToolCallback_t();
		m_Verts.clear();

		m_Active = false;
	}

	void EditorPolygonTool::MouseMove(const Vector2& pos, int key, bool shift, bool ctrl, bool alt)
	{
		switch (m_Mode)
		{
		case Freeform:
			UpdateFeedbackPoint(pos, alt);
			break;
		case Convex:
			break;
		case Line:
			break;
		};
	}

	void EditorPolygonTool::MousePress(const Vector2& pos, int key, bool shift, bool ctrl, bool alt)
	{
		if (key == CL_MOUSE_LEFT)
		{
			switch (m_Mode)
			{
			case Freeform:
				AddFreeformPoint(pos, alt);
				break;
			case Convex:
				break;
			case Line:
				break;
			};
		}
		else if (key == CL_MOUSE_MIDDLE)
		{
			RemoveNearestPoint(pos);
		}
	}

	void EditorPolygonTool::Draw(CL_GraphicContext& gc)
	{
		CL_Colorf lineColour(0.4f, 0.4f, 0.96f, 0.8f);
		CL_Colorf pointColour(1.0f, 1.0f, 0.98f, 1.0f);
		if (m_Verts.size() > 1)
		{
			auto it2 = m_Verts.begin() + 1u;
			for (auto it = m_Verts.begin(); it2 != m_Verts.end(); ++it, ++it2)
				CL_Draw::line(gc, it->x, it->y, it2->x, it2->y, lineColour);
			if (m_Mode != Mode::Line && m_Verts.size() > 2)
			{
				const auto& last = m_Verts.back();
				const auto& first = m_Verts.front();
				CL_Draw::line(gc, last.x, last.y, first.x, first.y, lineColour);
			}
		}
		for (auto it = m_Verts.begin(); it != m_Verts.end(); ++it)
			CL_Draw::circle(gc, it->x, it->y, 1.0f, pointColour);

		CL_Draw::circle(gc, m_FeedbackPoint.x, m_FeedbackPoint.y, 1.0f, CL_Colorf(0.6f, 0.98f, 0.6f));
	}

	void EditorPolygonTool::AddFreeformPoint(const Vector2& pos, bool to_nearest_edge)
	{
		if (!to_nearest_edge || m_Verts.size() < 2)
		{
			m_Verts.push_back(pos);
		}
		else
		{
			CL_Vec2f p(pos.x, pos.y);
			float dist = std::numeric_limits<float>::max();
			auto lineIt1 = m_Verts.end();

			auto it2 = m_Verts.begin() + 1u;
			for (auto it = m_Verts.begin(); it2 != m_Verts.end(); ++it, ++it2)
			{
				CL_LineSegment2f line(CL_Vec2f(it->x, it->y), CL_Vec2f(it2->x, it2->y));
				const float curDist = line.point_distance(p);
				if (curDist < dist)
				{
					lineIt1 = it2;
					dist = curDist;
				}
			}
			if (m_Verts.size() > 2)
			{
				const auto& last = m_Verts.back();
				const auto& first = m_Verts.front();
				CL_LineSegment2f line(CL_Vec2f(last.x, last.y), CL_Vec2f(first.x, first.y));
				const float curDist = line.point_distance(p);
				if (curDist < dist)
				{
					m_Verts.insert(m_Verts.begin(), pos);
					return;
				}
			}
			m_Verts.insert(lineIt1, pos);
		}
	}

	void EditorPolygonTool::RemoveNearestPoint(const Vector2& pos)
	{
		float dist = std::numeric_limits<float>::max();
		auto nearestIt = m_Verts.end();
		for (auto it = m_Verts.begin(); it != m_Verts.end(); ++it)
		{
			const float curDist = Vector2::distance(pos, *it);
			if (curDist < dist)
			{
				nearestIt = it;
				dist = curDist;
			}
		}
		if (nearestIt != m_Verts.end())
			m_Verts.erase(nearestIt);
	}

	void EditorPolygonTool::GrabNearestPoint(const Vector2& pos)
	{
		float dist = std::numeric_limits<float>::max();
		auto nearestIt = m_Verts.end();
		for (size_t i = 0; i < m_Verts.size(); ++i)
		{
			const float curDist = Vector2::distance(pos, m_Verts[i]);
			if (curDist < dist)
			{
				m_GrabbedPoint = i;
				dist = curDist;
			}
		}
	}

	void EditorPolygonTool::UpdateFeedbackPoint(const Vector2& pos, bool to_nearest_edge)
	{
		if (!to_nearest_edge || m_Verts.size() < 2)
		{
			m_FeedbackPoint = pos;
		}
		else
		{
			CL_Vec2f p(pos.x, pos.y);
			float dist = std::numeric_limits<float>::max();
			auto lineIt1 = m_Verts.end();
			auto it2 = m_Verts.begin() + 1u;
			for (auto it = m_Verts.begin(); it2 != m_Verts.end(); ++it, ++it2)
			{
				auto nearestPoint = CL_LineMath::closest_point(p, CL_Vec2f(it->x, it->y), CL_Vec2f(it2->x, it2->y));
				const float curDist = nearestPoint.distance(p);
				if (curDist < dist)
				{
					m_FeedbackPoint.x = nearestPoint.x;
					m_FeedbackPoint.y = nearestPoint.y;
					dist = curDist;
				}
			}

			if (m_Verts.size() > 2)
			{
				const auto& last = m_Verts.back();
				const auto& first = m_Verts.front();
				auto nearestPoint = CL_LineMath::closest_point(p, CL_Vec2f(last.x, last.y), CL_Vec2f(first.x, first.y));
				const float curDist = nearestPoint.distance(p);
				if (curDist < dist)
				{
					m_FeedbackPoint.x = nearestPoint.x;
					m_FeedbackPoint.y = nearestPoint.y;
				}
			}
		}
	}

}
