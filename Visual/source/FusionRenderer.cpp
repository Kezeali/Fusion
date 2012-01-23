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
		CalculateScreenArea(m_GC, area, viewport, apply_camera_offset);
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

			auto camZoom = (1.f / camera->GetZoom());
			const CL_Sizef zoomedSize(area.get_width() * camZoom, area.get_height() * camZoom);

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			CL_Vec2f viewportOffset =
				camera->GetPosition() - CL_Vec2f::calc_origin( origin_center, zoomedSize);

			area.left = viewportOffset.x;
			area.top = viewportOffset.y;
			area.set_size(zoomedSize);
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
		//m_GC.clear_stencil();
		//CL_BufferControl bufferControl = m_GC.get_buffer_control();
		//bufferControl.enable_stencil_test(true);
		//bufferControl.enable_color_write(false);
		//m_GC.set_buffer_control(bufferControl);
		//CL_Draw::fill(m_GC, viewportArea, CL_Colorf(0.f, 0.f, 0.f, 1.f));

		//bufferControl.enable_color_write(true);
		//m_GC.set_buffer_control(bufferControl);

		const CL_Vec2f &camPosition = camera->GetPosition();
		const CL_Origin camOrigin = origin_center;

		const CL_Sizef sizef(viewportArea.get_size());
		// Get the top-left pixel of the viewport (unscaled) in relative to the origin point of the world
		const CL_Vec2f viewportOffsetInWorld = camPosition * camera->GetZoom() - CL_Vec2f::calc_origin(camOrigin, sizef);

		// Apply rotation, translation & scale
		gc.push_modelview();
		gc.set_translate(-viewportOffsetInWorld.x + viewportArea.left, -viewportOffsetInWorld.y + viewportArea.top);
		gc.mult_rotate(CL_Angle(-camera->GetAngle(), cl_radians));
		if ( !fe_fequal(camera->GetZoom(), 1.f) )
			gc.mult_scale(camera->GetZoom(), camera->GetZoom());

		if (draw_area != nullptr)
		{
			float drawAreaScale = 0.001f;
			if (!fe_fzero(camera->GetZoom()))
				drawAreaScale = 1.f / camera->GetZoom();
			const CL_Sizef scaledSize(viewportArea.get_width() * drawAreaScale, viewportArea.get_height() * drawAreaScale);

			const CL_Vec2f scaledViewportOffsetInWorld = camPosition - CL_Vec2f::calc_origin(camOrigin, scaledSize);

			draw_area->left = scaledViewportOffsetInWorld.x;
			draw_area->top = scaledViewportOffsetInWorld.y;
			draw_area->set_size(scaledSize);
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
