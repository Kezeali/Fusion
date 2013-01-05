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

#include "FusionGUI.h"
#include <ClanLib/core.h>

namespace FusionEngine
{

	EditorPolygonTool::EditorPolygonTool()
		: m_Mode(Freeform),
		m_Active(false),
		m_FeedbackType(Add),
		m_DrawFeedbackTri(false),
		m_Moving(false)
	{}

	void EditorPolygonTool::Start(const std::vector<Vector2>& verts, const PolygonToolCallback_t& done_callback, EditorPolygonTool::Mode mode)
	{
		m_Verts = verts;
		m_DoneCallback = done_callback;
		m_Mode = mode;

		m_InitialVerts = verts;

		CreateGui();

		m_Active = true;
	}

	void EditorPolygonTool::Finish()
	{
		m_DoneCallback(m_Verts);
		m_DoneCallback = PolygonToolCallback_t();
		m_Verts.clear();

		m_InitialVerts.clear();

		m_GrabbedVerts.clear();

		m_DrawFeedbackTri = false;

		m_GuiDoc->Close();
		m_GuiDoc.reset();

		m_Active = false;
	}

	void EditorPolygonTool::Reset()
	{
		m_Verts = m_InitialVerts;
	}

	void EditorPolygonTool::Cancel()
	{
		Reset();
		Finish();
	}

	void EditorPolygonTool::KeyChange(bool shift, bool ctrl, bool alt)
	{
		MouseMove(m_LastMousePos, shift, ctrl, alt);
	}

	void EditorPolygonTool::MouseMove(const Vector2& pos, bool shift, bool ctrl, bool alt)
	{
		m_LastMousePos = pos;

		m_DrawFeedbackTri = false;
		if (m_Moving)
		{
			m_FeedbackPoint = pos;
			m_FeedbackType = Move;
		}
		else if (ctrl || (shift && !m_Moving))
		{
			const auto v = GetNearestVert(pos, 10.f);
			if (v < m_Verts.size())
			{
				m_FeedbackPoint = m_Verts[v];
				m_FeedbackType = ctrl ? Remove : Move;
			}
			else
				m_FeedbackPoint = pos;
		}
		else
		{
			switch (m_Mode)
			{
			case Freeform:
				UpdateFeedbackPoint(pos, !alt);
				break;
			case Line:
				UpdateFeedbackPoint(pos, alt);
				break;
			case Convex:
				break;
			};
			m_FeedbackType = Add;
			if (!m_Verts.empty())
				m_DrawFeedbackTri = true;
		}
	}

	bool EditorPolygonTool::MousePress(const Vector2& pos, MouseInput key, bool shift, bool ctrl, bool alt)
	{
		if (key == LeftButton)
		{
			if (shift)
			{
				if (alt)
					GrabNearestVert(pos);
				else if (ctrl)
				{
					UngrabNearestVert(pos);
				}
				else
				{
					GrabNearestVert(pos, false);
					m_Moving = true;
					m_MoveFrom = pos;
				}
			}
			else if (ctrl)
			{
				RemoveNearestVert(pos);
			}
			else
			{
				switch (m_Mode)
				{
				case Freeform:
					AddFreeformPoint(pos, !alt);
					break;
				case Line:
					AddFreeformPoint(pos, alt);
					break;
				case Convex:
					break;
				};
			}
			return true;
		}
		else
			return false;
	}

	bool EditorPolygonTool::MouseRelease(const Vector2& pos, MouseInput key, bool shift, bool ctrl, bool alt)
	{
		if (key == LeftButton)
		{
			if (m_Moving && !m_GrabbedVerts.empty())
			{
				MoveGrabbedVerts(pos);
				m_GrabbedVerts.erase(m_TempGrabbedVert);
			}
			m_TempGrabbedVert = std::numeric_limits<size_t>::max();
			m_Moving = false;
			m_MoveFrom = Vector2();

			return true;
		}
		else
			return false;
	}

	void EditorPolygonTool::Draw(clan::Canvas& canvas)
	{
		clan::Colorf lineColour(0.4f, 0.4f, 0.96f, 0.8f);
		clan::Colorf pointColour(1.0f, 1.0f, 0.98f, 1.0f);
		if (m_Verts.size() > 1)
		{
			auto it2 = m_Verts.begin() + 1u;
			for (auto it = m_Verts.begin(); it2 != m_Verts.end(); ++it, ++it2)
				canvas.line(it->x, it->y, it2->x, it2->y, lineColour);
			if (m_Mode != Mode::Line && m_Verts.size() > 2)
			{
				const auto& last = m_Verts.back();
				const auto& first = m_Verts.front();
				canvas.line(last.x, last.y, first.x, first.y, lineColour);
			}
		}
		for (auto it = m_Verts.begin(); it != m_Verts.end(); ++it)
			canvas.circle(it->x, it->y, 1.0f, pointColour);

		clan::Colorf feedbackColour;
		if (m_FeedbackType == Add)
			feedbackColour = clan::Colorf(0.6f, 0.98f, 0.6f);
		else if (m_FeedbackType == Remove)
			feedbackColour = clan::Colorf(0.98f, 0.6f, 0.6f);
		else
			feedbackColour = clan::Colorf(0.6f, 0.6f, 0.98f);
		canvas.circle(m_FeedbackPoint.x, m_FeedbackPoint.y, 1.0f, feedbackColour);

		if (m_DrawFeedbackTri)
		{
			clan::Colorf feedbackLineColour(0.96f, 0.96f, 0.96f, 0.8f);

			canvas.circle(m_FeedbackTri[0].x, m_FeedbackTri[0].y, 1.0f, feedbackColour);
			auto it2 = m_FeedbackTri.begin() + 1u;
			for (auto it = m_FeedbackTri.begin(); it2 != m_FeedbackTri.end(); ++it, ++it2)
			{
				canvas.circle(it2->x, it2->y, 1.0f, feedbackColour);
				canvas.line(it->x, it->y, it2->x, it2->y, feedbackLineColour);
			}
		}
		
		clan::Colorf grabbedColour(0.40f, 0.40f, 1.f);
		for (auto it = m_GrabbedVerts.begin(); it != m_GrabbedVerts.end(); ++it)
		{
			const Vector2& v = m_Verts[*it];
			canvas.circle(v.x, v.y, 0.5f, grabbedColour);
		}
	}

	void EditorPolygonTool::UpdateFeedbackPoint(const Vector2& pos, bool to_nearest_edge)
	{
		m_FeedbackTri[1] = pos;

		if (!to_nearest_edge || m_Verts.size() < 2)
		{
			m_FeedbackPoint = pos;

			if (!m_Verts.empty())
			{
				m_FeedbackTri[0] = m_Verts.back();
				m_FeedbackTri[2] = !to_nearest_edge ? m_Verts.back() : m_Verts.front();
			}
		}
		else
		{
			clan::Vec2f p(pos.x, pos.y);
			float dist = std::numeric_limits<float>::max();
			auto lineIt1 = m_Verts.end();
			auto it2 = m_Verts.begin() + 1u;
			for (auto it = m_Verts.begin(); it2 != m_Verts.end(); ++it, ++it2)
			{
				auto nearestPoint = clan::LineMath::closest_point(p, clan::Vec2f(it->x, it->y), clan::Vec2f(it2->x, it2->y));
				const float curDist = nearestPoint.distance(p);
				if (curDist < dist)
				{
					m_FeedbackPoint.x = nearestPoint.x;
					m_FeedbackPoint.y = nearestPoint.y;

					m_FeedbackTri[0] = *it;
					m_FeedbackTri[2] = *it2;
					dist = curDist;
				}
			}

			if (m_Verts.size() > 2)
			{
				const auto& last = m_Verts.back();
				const auto& first = m_Verts.front();
				auto nearestPoint = clan::LineMath::closest_point(p, clan::Vec2f(last.x, last.y), clan::Vec2f(first.x, first.y));
				const float curDist = nearestPoint.distance(p);
				if (curDist < dist)
				{
					m_FeedbackPoint.x = nearestPoint.x;
					m_FeedbackPoint.y = nearestPoint.y;

					m_FeedbackTri[0] = last;
					m_FeedbackTri[2] = first;
				}
			}
		}
	}

	void EditorPolygonTool::AddFreeformPoint(const Vector2& pos, bool to_nearest_edge)
	{
		m_GrabbedVerts.clear();

		if (!to_nearest_edge || m_Verts.size() < 2)
		{
			m_Verts.push_back(pos);
		}
		else
		{
			clan::Vec2f p(pos.x, pos.y);
			float dist = std::numeric_limits<float>::max();
			auto lineIt1 = m_Verts.end();

			auto it2 = m_Verts.begin() + 1u;
			for (auto it = m_Verts.begin(); it2 != m_Verts.end(); ++it, ++it2)
			{
				clan::LineSegment2f line(clan::Vec2f(it->x, it->y), clan::Vec2f(it2->x, it2->y));
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
				clan::LineSegment2f line(clan::Vec2f(last.x, last.y), clan::Vec2f(first.x, first.y));
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

	void EditorPolygonTool::RemoveNearestVert(const Vector2& pos)
	{
		m_GrabbedVerts.clear();

		const float max_distance = 10.f;

		float dist = std::numeric_limits<float>::max();
		auto nearestIt = m_Verts.end();
		for (auto it = m_Verts.begin(); it != m_Verts.end(); ++it)
		{
			const float curDist = Vector2::distance(pos, *it);
			if (curDist < dist && curDist < max_distance)
			{
				nearestIt = it;
				dist = curDist;
			}
		}
		if (nearestIt != m_Verts.end())
			m_Verts.erase(nearestIt);
	}

	void EditorPolygonTool::GrabNearestVert(const Vector2& pos, bool hold)
	{
		const auto v = GetNearestVert(pos, 10.f);
		if (v < m_Verts.size())
		{
			if (hold)
				m_GrabbedVerts.insert(v);
			else if (m_GrabbedVerts.insert(v).second)
				m_TempGrabbedVert = v;
		}
	}
	
	void EditorPolygonTool::UngrabNearestVert(const Vector2& pos)
	{
		const auto v = GetNearestVert(pos, 10.f);
		if (v < m_Verts.size())
			m_GrabbedVerts.erase(v);
	}

	size_t EditorPolygonTool::GetNearestVert(const Vector2& pos, const float max_distance)
	{
		float dist = std::numeric_limits<float>::max();
		auto nearestVert = m_Verts.size();
		for (size_t i = 0; i < m_Verts.size(); ++i)
		{
			const float curDist = Vector2::distance(pos, m_Verts[i]);
			if (curDist < dist && curDist < max_distance)
			{
				nearestVert = i;
				dist = curDist;
			}
		}
		return nearestVert;
	}

	void EditorPolygonTool::MoveGrabbedVerts(const Vector2& to)
	{
		auto delta = to - m_MoveFrom;
		for (auto it = m_GrabbedVerts.begin(); it != m_GrabbedVerts.end(); ++it)
		{
			m_Verts[*it] += delta;
		}
	}

	void EditorPolygonTool::CreateGui()
	{
		if (!m_GuiDoc)
		{
			m_GuiDoc = GUI::getSingleton().GetContext("editor")->LoadDocument("/Data/core/gui/editor_shapetool_toolbar.rml");
			m_GuiDoc->RemoveReference();

			//if (auto body = m_GuiDoc->GetElementById("content"))
			//	Rocket::Core::Factory::InstanceElementText(body, "Polygon Tool");

			if (auto title = m_GuiDoc->GetElementById("title"))
				Rocket::Core::Factory::InstanceElementText(title, "Polygon Tool");
		}
		if (m_GuiDoc)
		{
			m_GuiDoc->Show();
		}
	}

}
