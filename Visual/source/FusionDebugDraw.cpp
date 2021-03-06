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

#include "Visual/FusionDebugDraw.h"

#include "Visual/FusionDebugDrawImpl.h"

namespace FusionEngine
{

	DebugDraw::DebugDraw()
		: m_Impl(DebugDrawProvider::getSingleton().Create())
	{
	}

	void DebugDraw::Flip() const
	{
		m_Impl->Flip();
	}

	void DebugDraw::SetCliprect(const clan::Rect& rect)
	{
		m_Impl->SetCliprect(rect);
	}

	void DebugDraw::ResetCliprect()
	{
		m_Impl->ResetCliprect();
	}

	void DebugDraw::DrawRectangle(const clan::Rectf& rect, const clan::Color& color) const
	{
		m_Impl->DrawRectangle(rect, color);
	}

	void DebugDraw::DrawSolidRectangle(const clan::Rectf& rect, const clan::Color& color) const
	{
		m_Impl->DrawSolidRectangle(rect, color);
	}

	void DebugDraw::DrawTexturedRectangle(clan::Image texture, clan::Rect source, clan::Rectf dest) const
	{
		m_Impl->DrawTexturedRectangle(texture, source, dest);
	}

	void DebugDraw::DrawTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color) const
	{
		m_Impl->DrawTriangle(vert_a, vert_b, vert_c, color);
	}

	void DebugDraw::DrawSolidTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color) const
	{
		m_Impl->DrawSolidTriangle(vert_a, vert_b, vert_c, color);
	}

	void DebugDraw::DrawPolygon(std::vector<Vector2> vertices, const clan::Color& color) const
	{
		m_Impl->DrawPolygon(std::move(vertices), color);
	}

	void DebugDraw::DrawSolidPolygon(std::vector<Vector2> vertices, const clan::Color& color) const
	{
		m_Impl->DrawSolidPolygon(std::move(vertices), color);
	}

	void DebugDraw::DrawCircle(const Vector2& center, float radius, const clan::Color& color) const
	{
		m_Impl->DrawCircle(center, radius, color);
	}

	void DebugDraw::DrawSolidCircle(const Vector2& center, float radius, const Vector2& axis, const clan::Color& color) const
	{
		m_Impl->DrawSolidCircle(center, radius, axis, color);
	}

	void DebugDraw::DrawSegment(const Vector2& p1, const Vector2& p2, const clan::Color& color) const
	{
		m_Impl->DrawSegment(p1, p2, color);
	}

	void DebugDraw::DrawPoint(const Vector2& p, float size, const clan::Color& color) const
	{
		m_Impl->DrawPoint(p, size, color);
	}

	void DebugDraw::RenderText(clan::Font font, clan::Pointf pos, const std::string& text, const clan::Color& color) const
	{
		m_Impl->RenderText(font, pos, text, color);
	}

}
