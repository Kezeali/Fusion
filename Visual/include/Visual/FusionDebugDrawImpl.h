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
#ifndef H_Fusion_Visual_DebugDrawImpl
#define H_Fusion_Visual_DebugDrawImpl

#include "FusionSingleton.h"
#include "FusionVectorTypes.h"
#include <ClanLib/Core/Math/rect.h>
#include <ClanLib/Display/2D/color.h>
#include <ClanLib/Display/2D/image.h>
#include <ClanLib/Display/Font/font.h>

namespace FusionEngine
{

	class DebugDrawImpl
	{
	public:
		virtual ~DebugDrawImpl() {}

		virtual void Flip() = 0;

		virtual void SetCliprect(const clan::Rect& rect) = 0;

		virtual void ResetCliprect() = 0;
		
		virtual void DrawRectangle(const clan::Rectf& rect, const clan::Color& color) = 0;

		virtual void DrawSolidRectangle(const clan::Rectf& rect, const clan::Color& color) = 0;

		virtual void DrawTexturedRectangle(clan::Image texture, clan::Rect source, clan::Rectf dest) = 0;

		virtual void DrawTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color) = 0;

		virtual void DrawSolidTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color) = 0;

		virtual void DrawPolygon(std::vector<Vector2> vertices, const clan::Color& color) = 0;

		virtual void DrawSolidPolygon(std::vector<Vector2> vertices, const clan::Color& color) = 0;

		virtual void DrawCircle(const Vector2& center, float radius, const clan::Color& color) = 0;

		virtual void DrawSolidCircle(const Vector2& center, float radius, const Vector2& axis, const clan::Color& color) = 0;

		virtual void DrawSegment(const Vector2& p1, const Vector2& p2, const clan::Color& color) = 0;

		virtual void DrawPoint(const Vector2& p, float size, const clan::Color& color) = 0;

		virtual void RenderText(clan::Font font, clan::Pointf pos, const std::string& text, const clan::Color& color) = 0;

	};

	class DebugDrawProvider : public Singleton<DebugDrawProvider>
	{
	public:
		virtual ~DebugDrawProvider()
		{
		}

		virtual std::shared_ptr<DebugDrawImpl> Create() = 0;

		virtual std::shared_ptr<DebugDrawImpl> GetPersistentDrawingContext() const = 0;

	};

}

#endif
