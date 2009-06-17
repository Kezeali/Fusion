/*
  Copyright (c) 2006 Fusion Project Team

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

#include "Common.h"

#include "FusionPhysicsDebugDraw.h"

#include <Box2D.h>

namespace FusionEngine
{

	DebugDraw::DebugDraw(CL_GraphicContext gc)
		: b2DebugDraw(),
		m_gc(gc)
	{
	}

	void DebugDraw::SetGraphicContext(CL_GraphicContext gc)
	{
		m_gc = gc;
	}

	void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		CL_Colorf clcolor(color.r, color.g, color.b, 1.0f);

		CL_Vec2f *positions = new CL_Vec2f[vertexCount];
		for (int32 i = 0; i < vertexCount; i++)
		{
			positions[i].x = vertices[i].x;
			positions[i].y = vertices[i].y;
		}

		CL_PrimitivesArray vertex_data(m_gc);
		vertex_data.set_attributes(0, positions);
		vertex_data.set_attribute(1, clcolor);
		m_gc.draw_primitives(cl_line_loop, vertexCount, vertex_data);

		delete[] positions;
	}

	void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		CL_Colorf clcolor(color.r, color.g, color.b, 1.0f);

		CL_Vec2f *positions = new CL_Vec2f[vertexCount];
		for (int32 i = 0; i < vertexCount; i++)
		{
			positions[i].x = vertices[i].x;
			positions[i].y = vertices[i].y;
		}

		CL_PrimitivesArray vertex_data(m_gc);
		vertex_data.set_attributes(0, positions);
		vertex_data.set_attribute(1, clcolor);
		m_gc.draw_primitives(cl_triangle_fan, vertexCount, vertex_data); // draw fill
		m_gc.draw_primitives(cl_line_loop, vertexCount, vertex_data); // draw outline

		delete[] positions;
	}

	void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
	{
		CL_Colorf clcolor(color.r, color.g, color.b, 1.0f);

		const int segs = 8;
		const float32 coef = 2.0*s_pi / (float32)segs;

		CL_Vec2f *positions = new CL_Vec2f[segs + 1];
		for(int n = 0; n <= segs; n++)
		{
			float32 rads = n*coef;
			positions[n].x = radius * cos(rads) + center.x;
			positions[n].y = radius * sin(rads) + center.y;
		}

		CL_PrimitivesArray vertex_data(m_gc);
		vertex_data.set_attributes(0, positions);
		vertex_data.set_attribute(1, clcolor);
		m_gc.draw_primitives(cl_line_strip, segs, vertex_data);

		delete[] positions;
	}

	void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
	{
		CL_Colorf clcolor(color.r, color.g, color.b, 1.0f);

		const int segs = 8;
		const float32 coef = 2.0*s_pi / (float32)segs;

		const float32 angle = atanf(axis.y / axis.x);

		CL_Vec2f *positions = new CL_Vec2f[segs + 1];
		for(int n = 0; n <= segs; n++)
		{
			float32 rads = n*coef;
			positions[n].x = radius * cos(rads) + center.x;
			positions[n].y = radius * sin(rads) + center.y;
		}

		CL_PrimitivesArray vertex_data(m_gc);
		vertex_data.set_attributes(0, positions);
		vertex_data.set_attribute(1, clcolor);
		m_gc.draw_primitives(cl_triangle_fan, segs, vertex_data);

		for(int n = 0; n <= segs; n++)
		{
			float32 rads = n*coef;
			positions[n].x = radius * cos(rads + angle) + center.x;
			positions[n].y = radius * sin(rads + angle) + center.y;
		}
		vertex_data.set_attributes(0, positions);
		m_gc.draw_primitives(cl_line_strip, segs, vertex_data);

		delete[] positions;
	}

	void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
		CL_Colorf clcolor(color.r, color.g, color.b, 1.0f);

		CL_Vec2i positions[] =
		{
			CL_Vec2i(p1.x, p1.y),
			CL_Vec2i(p2.x, p2.y)
		};

		CL_PrimitivesArray vertex_data(m_gc);
		vertex_data.set_attributes(0, positions);
		vertex_data.set_attribute(1, clcolor);
		m_gc.draw_primitives(cl_polygon, 2, vertex_data);
	}

	void DebugDraw::DrawXForm(const b2XForm& /*xf*/)
	{
		// TODO
	}

	void DebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
	{
		CL_Colorf clcolor(color.r, color.g, color.b, 1.0f);

		if (size <= 1)
			CL_Draw::point(m_gc, p.x, p.y, clcolor);
		else
			DrawSolidCircle(p, size, b2Vec2_zero, color);
	}

	void DebugDraw::DrawAABB(b2AABB *aabb, const b2Color &color)
	{
		CL_Colorf clcolor(color.r, color.g, color.b, 1.0f);

		CL_Draw::box(m_gc, aabb->lowerBound.y, aabb->lowerBound.y, aabb->upperBound.x, aabb->upperBound.y, clcolor);
	}

}
