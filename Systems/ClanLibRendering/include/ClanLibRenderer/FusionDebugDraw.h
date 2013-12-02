/*
  Copyright (c) 2013 Fusion Project Team

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

#ifndef H_FusionEngine_ClanLibRenderer_DebugDraw
#define H_FusionEngine_ClanLibRenderer_DebugDraw
#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionDebugDrawImpl.h"

#include "FusionRenderAction.h"

#include <ClanLib/display.h>
#include <tbb/concurrent_queue.h>

namespace FusionEngine
{
	namespace ClanLibRenderer
	{

		class DebugDraw : public DebugDrawImpl
		{
		public:
			virtual ~DebugDraw();

			virtual void SetCliprect(const clan::Rect& rect);

			virtual void ResetCliprect();

			virtual void DrawTexturedRectangle(clan::Image texture, clan::Rect source, clan::Rectf dest);

			virtual void DrawPolygon(std::vector<Vector2> vertices, const clan::Color& color);

			virtual void DrawSolidPolygon(std::vector<Vector2> vertices, const clan::Color& color);

			virtual void DrawCircle(const Vector2& center, float radius, const clan::Color& color);

			virtual void DrawSolidCircle(const Vector2& center, float radius, const Vector2& axis, const clan::Color& color);

			virtual void DrawSegment(const Vector2& p1, const Vector2& p2, const clan::Color& color);

			virtual void DrawPoint(const Vector2& p, float size, const clan::Color& color);

			virtual void RenderText(clan::Font font, clan::Pointf pos, const std::string& text, const clan::Color& color);

		private:
			tbb::concurrent_queue<RenderActionFunctor> m_Actions;
		};

		inline void DebugDraw::SetCliprect(const clan::Rect& rect)
		{
			//RenderAction renderAction;
			//renderAction.func = [rect](clan::Canvas canvas, Vector2 offset)
			//{
			//	canvas.set_cliprect(rect);
			//};
			m_Actions.push([rect](clan::Canvas canvas, Vector2 offset)
			{
				canvas.set_cliprect(rect);
			});
		}

		inline void DebugDraw::ResetCliprect()
		{
			m_Actions.push([](clan::Canvas canvas, Vector2 offset)
			{
				canvas.reset_cliprect();
			});
		}

		inline void DebugDraw::DrawTexturedRectangle(clan::Image texture, clan::Rect source, clan::Rectf dest)
		{
			m_Actions.push([=](clan::Canvas canvas, Vector2 offset)
			{
				texture.draw(canvas, source, dest);
			});
		}

		inline void DebugDraw::DrawPolygon(std::vector<Vector2> vertices, const clan::Color& color)
		{
			std::vector<clan::Vec2f> pos;
			for (auto vert : vertices)
			{
				pos.push_back(clan::Vec2f(vert.x, vert.y));
			}
			m_Actions.push([=](clan::Canvas canvas, Vector2 offset)
			{
				canvas.draw_lines(pos.data(), (int)pos.size(), clan::Colorf(color));
			});
		}

		inline void DebugDraw::DrawSolidPolygon(std::vector<Vector2> vertices, const clan::Color& color)
		{
			std::vector<clan::Vec2f> pos;
			for (auto vert : vertices)
			{
				pos.push_back(clan::Vec2f(vert.x, vert.y));
			}
			m_Actions.push([=](clan::Canvas canvas, Vector2 offset)
			{
				canvas.fill_triangles(pos.data(), (int)pos.size(), clan::Colorf(color));
			});
		}

		inline void DebugDraw::DrawCircle(const Vector2& center, float radius, const clan::Color& color)
		{
			m_Actions.push([=](clan::Canvas canvas, Vector2 offset)
			{
				auto offCenter = center + offset;
				canvas.fill_circle(clan::Pointf(offCenter.x, offCenter.y), radius, clan::Colorf(color));
			});
		}

		inline void DebugDraw::DrawSolidCircle(const Vector2& center, float radius, const Vector2& axis, const clan::Color& color)
		{
			m_Actions.push([=](clan::Canvas canvas, Vector2 offset)
			{
				auto offCenter = center + offset;
				canvas.fill_circle(clan::Pointf(offCenter.x, offCenter.y), radius, clan::Colorf(color));
			});
		}

		inline void DebugDraw::DrawSegment(const Vector2& p1, const Vector2& p2, const clan::Color& color)
		{
			m_Actions.push([=](clan::Canvas canvas, Vector2 offset)
			{
				auto po1 = p1 + offset;
				auto po2 = p2 + offset;
				canvas.draw_line(clan::Pointf(po1.x, po1.y), clan::Pointf(po2.x, po2.y), clan::Colorf(color));
			});
		}

		inline void DebugDraw::DrawPoint(const Vector2& p, float size, const clan::Color& color)
		{
			m_Actions.push([=](clan::Canvas canvas, Vector2 offset)
			{
				auto po = p + offset;
				canvas.draw_point(clan::Pointf(po.x, po.y), clan::Colorf(color));
			});
		}

		inline void DebugDraw::RenderText(clan::Font font, clan::Pointf pos, const std::string& text, const clan::Color& color)
		{
			m_Actions.push([=](clan::Canvas canvas, Vector2 offset)
			{
				font.draw_text(canvas, pos + clan::Pointf(offset.x, offset.y), text, clan::Colorf(color));
			});
		}

	}
}

#endif
