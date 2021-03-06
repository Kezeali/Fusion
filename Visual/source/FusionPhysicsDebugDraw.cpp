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

#include "PrecompiledHeaders.h"

#include "FusionPhysicsDebugDraw.h"

#include <Box2D/Box2D.h>
#include <ClanLib/display.h>

namespace FusionEngine
{

	B2DebugDraw::B2DebugDraw(clan::GraphicContext gc)
		: b2Draw(),
		m_gc(gc)
	{
	}

	void B2DebugDraw::SetGraphicContext(clan::GraphicContext gc)
	{
		m_gc = gc;
	}

	void B2DebugDraw::SetViewport(const ViewportPtr &viewport)
	{
		m_Viewport = viewport;
	}

	void B2DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		//clan::Colorf clcolor(color.r, color.g, color.b, 1.0f);

		//clan::Vec2f *positions = new clan::Vec2f[vertexCount];
		//for (int32 i = 0; i < vertexCount; i++)
		//{
		//	positions[i].x = vertices[i].x * s_GameUnitsPerSimUnit;
		//	positions[i].y = vertices[i].y * s_GameUnitsPerSimUnit;
		//}

		//clan::PrimitivesArray vertex_data(m_gc);
		//vertex_data.set_attributes(0, positions);
		//vertex_data.set_attribute(1, clcolor);
		//m_gc.draw_primitives(cl_line_loop, vertexCount, vertex_data);

		//delete[] positions;
	}

	void B2DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		//clan::Colorf clcolor(color.r, color.g, color.b, 0.8f);

		//for (int32 i = 0; i < vertexCount; i++)
		//{
		//	positions[i].x = vertices[i].x * s_GameUnitsPerSimUnit;
		//	positions[i].y = vertices[i].y * s_GameUnitsPerSimUnit;
		//}

		//clan::PrimitivesArray vertex_data(m_gc);
		//vertex_data.set_attributes(0, positions);
		//vertex_data.set_attribute(1, clcolor);
		//m_gc.draw_primitives(cl_triangle_fan, vertexCount, vertex_data); // draw fill
		//m_gc.draw_primitives(cl_line_loop, vertexCount, vertex_data); // draw outline

		//delete[] positions;
	}

	void B2DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
	{
		//clan::Colorf clcolor(color.r, color.g, color.b, 1.0f);

		//clan::Vec2f renderCenter(center.x * s_GameUnitsPerSimUnit, center.y * s_GameUnitsPerSimUnit);
		//radius *= s_GameUnitsPerSimUnit;

		//const int segs = 16;
		//const float32 coef = 2.0f*s_pi / (float32)segs;

		//clan::Vec2f *positions = new clan::Vec2f[segs + 1];
		//for(int n = 0; n <= segs; n++)
		//{
		//	float32 rads = n*coef;
		//	positions[n].x = radius * cos(rads) + renderCenter.x;
		//	positions[n].y = radius * sin(rads) + renderCenter.y;
		//}

		//clan::PrimitivesArray vertex_data(m_gc);
		//vertex_data.set_attributes(0, positions);
		//vertex_data.set_attribute(1, clcolor);
		//m_gc.draw_primitives(cl_line_strip, segs, vertex_data);

		//delete[] positions;
	}

	void B2DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
	{
		//clan::Colorf clcolor(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, 0.5f);
		//clan::Colorf linecolour(color.r, color.g, color.b, 1.f);

		//clan::Vec2f renderCenter(center.x * s_GameUnitsPerSimUnit, center.y * s_GameUnitsPerSimUnit);
		//radius *= s_GameUnitsPerSimUnit;

		//const int segs = 16;
		//const float32 coef = 2.0f*s_pi / (float32)segs;

		//const float32 angle = atanf(axis.y / axis.x);

		//clan::Vec2f *positions = new clan::Vec2f[segs + 1];
		//for(int n = 0; n <= segs; n++)
		//{
		//	float32 rads = n*coef;
		//	positions[n].x = radius * cos(rads) + renderCenter.x;
		//	positions[n].y = radius * sin(rads) + renderCenter.y;
		//}

		//clan::PrimitivesArray vertex_data(m_gc);
		//vertex_data.set_attributes(0, positions);
		//vertex_data.set_attribute(1, clcolor);
		//m_gc.draw_primitives(cl_triangle_fan, segs, vertex_data);

		//for(int n = 0; n <= segs; n++)
		//{
		//	float32 rads = n*coef;
		//	positions[n].x = radius * cos(rads) + renderCenter.x;
		//	positions[n].y = radius * sin(rads) + renderCenter.y;
		//}
		//vertex_data.set_attributes(0, positions);
		//vertex_data.set_attribute(1, clcolor);
		//m_gc.draw_primitives(cl_line_strip, segs, vertex_data);

		//const b2Vec2 b2renderCenter = s_GameUnitsPerSimUnit * center;
		//auto axisPoint1 = b2renderCenter + (radius * 0.89f) * axis;
		//auto axisPoint2 = b2renderCenter + (radius * 1.05f) * axis;

		//clan::Vec2f f(axisPoint1.x, axisPoint1.y);
		//clan::Vec2f t(axisPoint2.x, axisPoint2.y);
		//clan::Draw::line(m_gc, f, t, linecolour);
		//m_gc.set_program_object(cl_program_color_only);

		//delete[] positions;
	}

	void B2DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
		//clan::Colorf clcolor(color.r, color.g, color.b, 1.0f);

		//clan::Vec2i positions[] =
		//{
		//	clan::Vec2i((int)(p1.x * s_GameUnitsPerSimUnit), (int)(p1.y * s_GameUnitsPerSimUnit)),
		//	clan::Vec2i((int)(p2.x * s_GameUnitsPerSimUnit), (int)(p2.y * s_GameUnitsPerSimUnit))
		//};

		//clan::PrimitivesArray vertex_data(m_gc);
		//vertex_data.set_attributes(0, positions);
		//vertex_data.set_attribute(1, clcolor);
		//m_gc.draw_primitives(cl_lines, 2, vertex_data);
	}

	void B2DebugDraw::DrawTransform(const b2Transform& xf)
	{
		//clan::Vec2f mid(xf.p.x * s_GameUnitsPerSimUnit, xf.p.y * s_GameUnitsPerSimUnit);
		//const float length = 10.f;
		//
		//// X
		//clan::Vec2f from = mid + clan::Vec2f(length, 0.f);
		//clan::Vec2f to = mid;// + clan::Vec2f(length, 0.f);

		//from.rotate(mid, clan::Angle(xf.q.GetAngle(), cl_radians));
		////to.rotate(mid, clan::Angle(xf.q.GetAngle(), cl_radians));

		//clan::Draw::line(m_gc, from, to, clan::Colorf::magenta);

		//// Y
		//from = mid - clan::Vec2f(0.f, length);
		////to = mid + clan::Vec2f(0.f, length);

		//from.rotate(mid, clan::Angle(xf.q.GetAngle(), cl_radians));
		////to.rotate(mid, clan::Angle(xf.q.GetAngle(), cl_radians));

		//clan::Draw::line(m_gc, from, to, clan::Colorf::aqua);

		//m_gc.set_program_object(cl_program_color_only);
	}

	void B2DebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
	{
		//clan::Colorf clcolor(color.r, color.g, color.b, 1.0f);

		//if (size <= 1)
		//{
		//	clan::Draw::point(m_gc, p.x * s_GameUnitsPerSimUnit, p.y * s_GameUnitsPerSimUnit, clcolor);
		//	m_gc.set_program_object(cl_program_color_only);
		//}
		//else
		//	DrawSolidCircle(p, size, b2Vec2_zero, color);
	}

	void B2DebugDraw::SetupView()
	{
		//if (m_Viewport)
		//{
			m_gc.set_program_object(clan::program_color_only);

			//const CameraPtr &camera = m_Viewport->GetCamera();

			//clan::Rect viewportArea;
			//const clan::Rectf &proportions = m_Viewport->GetArea();
			//viewportArea.left = (int)floor(proportions.left * m_gc.get_width());
			//viewportArea.top = (int)floor(proportions.top * m_gc.get_height());
			//viewportArea.right = (int)ceil(proportions.right * m_gc.get_width());
			//viewportArea.bottom = (int)ceil(proportions.bottom * m_gc.get_height());

			////m_gc.set_cliprect(viewportArea);

			//clan::Vec2f camPosition = camera->GetPosition();
			//clan::Origin camOrigin = origin_center;//camera->GetOrigin();

			////camPosition.x = ToRenderUnits(camPosition.x);
			////camPosition.y = ToRenderUnits(camPosition.y);

			//clan::Vec2f viewportOffset;
			//viewportOffset = camPosition - clan::Vec2f::calc_origin(camOrigin, clan::Sizef((float)viewportArea.get_width(), (float)viewportArea.get_height()));

			//m_gc.push_modelview();
			//m_gc.set_translate(-viewportOffset.x, -viewportOffset.y);
			////m_gc.set_translate(0.f, 0.f);
			////m_gc.mult_rotate(clan::Angle(-camera->GetAngle(), cl_radians));
			//if (!fe_fequal(camera->GetZoom(), 1.0f))
			//	m_gc.mult_scale(camera->GetZoom(), camera->GetZoom());
		//}
	}

	void B2DebugDraw::ResetView()
	{
		//if (m_Viewport)
		//{
			//m_gc.pop_modelview();
			//m_gc.reset_cliprect();

			m_gc.reset_program_object();
		//}
	}

}
