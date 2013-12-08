/*
*  Copyright (c) 2013 Fusion Project Team
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

#include "ClanLibRenderer/FusionDebugDrawImpl.h"

namespace FusionEngine { namespace ClanLibRenderer
{

	DebugDrawImpl::~DebugDrawImpl()
	{
	}

	void DebugDrawImpl::AddAction(RenderActionFunctor action)
	{
		m_IncommingRenderActions.push_back(action);
	}

	bool DebugDrawImpl::TryGetFrame(DebugDrawImpl::RenderActionFrame_t& out_frame)
	{
		return m_QueuedFrames.try_pop(out_frame);
	}

	void DebugDrawImpl::Flip()
	{
		m_QueuedFrames.push(m_IncommingRenderActions);
		m_IncommingRenderActions.clear();
	}

	void DebugDrawImpl::SetCliprect(const clan::Rect& rect)
	{
		AddAction([rect](clan::Canvas canvas, Vector2 offset)
		{
			canvas.set_cliprect(rect);
		});
	}

	void DebugDrawImpl::ResetCliprect()
	{
		AddAction([](clan::Canvas canvas, Vector2 offset)
		{
			canvas.reset_cliprect();
		});
	}

	void DebugDrawImpl::DrawRectangle(const clan::Rectf& rect, const clan::Color& color)
	{
		auto colorf = clan::Colorf(color);
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			canvas.draw_box(rect, colorf);
		});
	}

	void DebugDrawImpl::DrawSolidRectangle(const clan::Rectf& rect, const clan::Color& color)
	{
		auto colorf = clan::Colorf(color);
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			canvas.fill_rect(rect, colorf);
		});
	}

	void DebugDrawImpl::DrawTexturedRectangle(clan::Image texture, clan::Rect source, clan::Rectf dest)
	{
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			texture.draw(canvas, source, dest);
		});
	}

	template <class To, class From>
	To convertVector(const From& vec)
	{
		return clan::Vec2f(vec.x, vec.y);
	}

	void DebugDrawImpl::DrawTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color)
	{
		std::vector<clan::Vec2f> pos;
		pos.push_back(convertVector<clan::Vec2f>(vert_a));
		pos.push_back(convertVector<clan::Vec2f>(vert_b));
		pos.push_back(convertVector<clan::Vec2f>(vert_c));
		auto colorf = clan::Colorf(color);
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			canvas.draw_line_strip(pos.data(), (int)pos.size(), colorf);
		});
	}

	void DebugDrawImpl::DrawSolidTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color)
	{
		auto triangle = clan::Trianglef(convertVector<clan::Vec2f>(vert_a), convertVector<clan::Vec2f>(vert_b), convertVector<clan::Vec2f>(vert_c));
		auto colorf = clan::Colorf(color);
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			canvas.fill_triangle(triangle, colorf);
		});
	}

	void DebugDrawImpl::DrawPolygon(std::vector<Vector2> vertices, const clan::Color& color)
	{
		std::vector<clan::Vec2f> pos;
		for (auto vert : vertices)
		{
			pos.push_back(convertVector<clan::Vec2f>(vert));
		}
		auto colorf = clan::Colorf(color);
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			canvas.draw_lines(pos.data(), (int)pos.size(), colorf);
		});
	}

	void DebugDrawImpl::DrawSolidPolygon(std::vector<Vector2> vertices, const clan::Color& color)
	{
		std::vector<clan::Vec2f> pos;
		for (auto vert : vertices)
		{
			pos.push_back(convertVector<clan::Vec2f>(vert));
		}
		auto colorf = clan::Colorf(color);
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			canvas.fill_triangles(pos.data(), (int)pos.size(), colorf);
		});
	}

	void DebugDrawImpl::DrawCircle(const Vector2& center, float radius, const clan::Color& color)
	{
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			auto offCenter = center + offset;
			canvas.fill_circle(clan::Pointf(offCenter.x, offCenter.y), radius, clan::Colorf(color));
		});
	}

	void DebugDrawImpl::DrawSolidCircle(const Vector2& center, float radius, const Vector2& axis, const clan::Color& color)
	{
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			auto offCenter = center + offset;
			canvas.fill_circle(clan::Pointf(offCenter.x, offCenter.y), radius, clan::Colorf(color));
		});
	}

	void DebugDrawImpl::DrawSegment(const Vector2& p1, const Vector2& p2, const clan::Color& color)
	{
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			auto po1 = p1 + offset;
			auto po2 = p2 + offset;
			canvas.draw_line(clan::Pointf(po1.x, po1.y), clan::Pointf(po2.x, po2.y), clan::Colorf(color));
		});
	}

	void DebugDrawImpl::DrawPoint(const Vector2& p, float size, const clan::Color& color)
	{
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			auto po = p + offset;
			canvas.draw_point(clan::Pointf(po.x, po.y), clan::Colorf(color));
		});
	}

	void DebugDrawImpl::RenderText(clan::Font font, clan::Pointf pos, const std::string& text, const clan::Color& color)
	{
		AddAction([=](clan::Canvas canvas, Vector2 offset)
		{
			font.draw_text(canvas, pos + clan::Pointf(offset.x, offset.y), text, clan::Colorf(color));
		});
	}

} }
