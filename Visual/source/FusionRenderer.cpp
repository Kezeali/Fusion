#include "PrecompiledHeaders.h"

#include "FusionRenderer.h"

#include "FusionExceptionFactory.h"

namespace FusionEngine
{

	Renderer::Renderer(const CL_GraphicContext &gc)
		: m_GC(gc)
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::CalculateScreenArea(CL_Rect &area, const ViewportPtr &viewport, bool apply_camera_offset)
	{
		const CL_Rectf &proportions = viewport->GetArea();

		area.left = (int)floor(proportions.left * m_GC.get_width());
		area.top = (int)floor(proportions.top * m_GC.get_height());
		area.right = (int)ceil(proportions.right * m_GC.get_width());
		area.bottom = (int)ceil(proportions.bottom * m_GC.get_height());

		if (apply_camera_offset)
		{
			const CameraPtr &camera = viewport->GetCamera();
			if (!camera)
				FSN_EXCEPT(ExCode::InvalidArgument, "Cannot apply camera offset if the viewport has no camera associated with it");

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			CL_Vec2i viewportOffset =
				camera->GetPosition() - CL_Vec2f::calc_origin( origin_center, CL_Sizef((float)area.get_width(), (float)area.get_height()) );

			area.translate(viewportOffset);
		}
	}

	void Renderer::CalculateScreenArea(CL_Rectf &area, const ViewportPtr &viewport, bool apply_camera_offset)
	{
		const CL_Rectf &proportions = viewport->GetArea();

		area.left = proportions.left * m_GC.get_width();
		area.top = proportions.top * m_GC.get_height();
		area.right = proportions.right * m_GC.get_width();
		area.bottom = proportions.bottom * m_GC.get_height();

		if (apply_camera_offset)
		{
			const CameraPtr &camera = viewport->GetCamera();
			if (!camera)
				FSN_EXCEPT(ExCode::InvalidArgument, "Cannot apply camera offset if the viewport has no camera associated with it");

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			CL_Vec2f viewportOffset =
				camera->GetPosition() - CL_Vec2f::calc_origin( origin_center, CL_Sizef((float)area.get_width(), (float)area.get_height()) );

			area.translate(viewportOffset);
		}
	}

	void Renderer::CalculateScreenArea(const CL_GraphicContext& gc, CL_Rectf &area, const ViewportPtr &viewport, bool apply_camera_offset)
	{
		const CL_Rectf &proportions = viewport->GetArea();

		area.left = proportions.left * gc.get_width();
		area.top = proportions.top * gc.get_height();
		area.right = proportions.right * gc.get_width();
		area.bottom = proportions.bottom * gc.get_height();

		if (apply_camera_offset)
		{
			const CameraPtr &camera = viewport->GetCamera();
			if (!camera)
				FSN_EXCEPT(ExCode::InvalidArgument, "Cannot apply camera offset if the viewport has no camera associated with it");

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			CL_Vec2f viewportOffset =
				camera->GetPosition() - CL_Vec2f::calc_origin( origin_center, CL_Sizef((float)area.get_width(), (float)area.get_height()) );

			area.translate(viewportOffset);
		}
	}

	const CL_GraphicContext& Renderer::GetGraphicContext() const
	{
		return m_GC;
	}

	int Renderer::GetContextWidth() const
	{
		return m_GC.get_width();
	}

	int Renderer::GetContextHeight() const
	{
		return m_GC.get_height();
	}

	CL_GraphicContext& Renderer::SetupDraw(CL_GraphicContext& gc, const ViewportPtr& viewport, CL_Rectf* draw_area)
	{
		const CameraPtr &camera = viewport->GetCamera();

		CL_Rect viewportArea;
		CalculateScreenArea(viewportArea, viewport);

		// Set the viewport
		m_GC.set_cliprect(viewportArea);

		const CL_Vec2f &camPosition = camera->GetPosition();
		const CL_Origin camOrigin = origin_center;

		CL_Vec2f viewportOffset;
		viewportOffset = camPosition - CL_Vec2f::calc_origin(camOrigin, CL_Sizef((float)viewportArea.get_width(), (float)viewportArea.get_height()));

		// Apply rotation, translation & scale
		gc.push_modelview();
		gc.set_translate(-viewportOffset.x + viewportArea.left, -viewportOffset.y + viewportArea.top);
		gc.mult_rotate(CL_Angle(-camera->GetAngle(), cl_radians));
		if ( !fe_fequal(camera->GetZoom(), 1.f) )
			gc.mult_scale(camera->GetZoom(), camera->GetZoom());

		if (draw_area != nullptr)
		{
			// Get & scale the draw area
			float drawAreaScale = 0.001f;
			if (!fe_fzero(camera->GetZoom()))
				drawAreaScale = 1.f / camera->GetZoom();
			CL_Size size = viewportArea.get_size();
			draw_area->left = viewportOffset.x;
			draw_area->top = viewportOffset.y;
			draw_area->set_size(CL_Sizef(size.width * drawAreaScale, size.height * drawAreaScale));
		}

		return gc;
	}

	void Renderer::PostDraw(CL_GraphicContext& gc)
	{
		gc.pop_modelview();
		gc.reset_cliprect(); // the viewport cliprect
	}

	CL_GraphicContext& Renderer::SetupDraw(const ViewportPtr& viewport, CL_Rectf* draw_area)
	{
		return SetupDraw(m_GC, viewport, draw_area);
	}

	void Renderer::PostDraw()
	{
		PostDraw(m_GC);
	}

}
