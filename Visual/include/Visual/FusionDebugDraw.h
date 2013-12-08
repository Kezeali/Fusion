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

#pragma once
#ifndef H_Fusion_Visual_DebugDraw
#define H_Fusion_Visual_DebugDraw

#include <memory>

namespace FusionEngine
{

	class DebugDrawImpl;
	class DebugDrawProvider;

	//! A context for drawing debug overlays
	class DebugDraw
	{
	public:
		DebugDraw();

		DebugDraw(DebugDraw&& other)
			: m_Impl(std::move(other.m_Impl))
		{
		}

		DebugDraw(const DebugDraw& other)
			: m_Impl(other.m_Impl)
		{
		}

		void Flip() const;

		void SetCliprect(const clan::Rect& rect);

		void ResetCliprect();

		void DrawRectangle(const clan::Rectf& rect, const clan::Color& color) const;

		void DrawSolidRectangle(const clan::Rectf& rect, const clan::Color& color) const;

		void DrawTexturedRectangle(clan::Image texture, clan::Rect source, clan::Rectf dest) const;

		void DrawTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color) const;

		void DrawSolidTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color) const;

		void DrawPolygon(std::vector<Vector2> vertices, const clan::Color& color) const;

		void DrawSolidPolygon(std::vector<Vector2> vertices, const clan::Color& color) const;

		void DrawCircle(const Vector2& center, float radius, const clan::Color& color) const;

		void DrawSolidCircle(const Vector2& center, float radius, const Vector2& axis, const clan::Color& color) const;

		void DrawSegment(const Vector2& p1, const Vector2& p2, const clan::Color& color) const;

		void DrawPoint(const Vector2& p, float size, const clan::Color& color) const;

		void RenderText(clan::Font font, clan::Pointf pos, const std::string& text, const clan::Color& color) const;

	private:
		std::shared_ptr<DebugDrawImpl> m_Impl;
	};

}

#endif
