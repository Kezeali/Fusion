#include "PrecompiledHeaders.h"

#include "FusionRenderer.h"

#include "FusionExceptionFactory.h"

namespace FusionEngine
{

	Renderer::Renderer(const clan::Canvas &canvas)
		: m_Canvas(canvas)
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::CalculateScreenArea(clan::Rect &area, const ViewportPtr &viewport, bool apply_camera_offset)
	{
		const clan::Rectf &proportions = viewport->GetArea();

		area.left = (int)floor(proportions.left * m_Canvas.get_width());
		area.top = (int)floor(proportions.top * m_Canvas.get_height());
		area.right = (int)ceil(proportions.right * m_Canvas.get_width());
		area.bottom = (int)ceil(proportions.bottom * m_Canvas.get_height());

		if (apply_camera_offset)
		{
			const CameraPtr &camera = viewport->GetCamera();
			if (!camera)
				FSN_EXCEPT(ExCode::InvalidArgument, "Cannot apply camera offset if the viewport has no camera associated with it");

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			clan::Vec2i viewportOffset =
				camera->GetPosition() - clan::Vec2f::calc_origin(clan::origin_center, clan::Sizef((float)area.get_width(), (float)area.get_height()));

			area.translate(viewportOffset);
		}
	}

	void Renderer::CalculateScreenArea(clan::Rectf &area, const ViewportPtr &viewport, bool apply_camera_offset)
	{
		CalculateScreenArea(m_Canvas.get_gc(), area, viewport, apply_camera_offset);
	}

	void Renderer::CalculateScreenArea(const clan::GraphicContext& gc, clan::Rectf &area, const ViewportPtr &viewport, bool apply_camera_offset)
	{
		const clan::Rectf &proportions = viewport->GetArea();

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
			const clan::Sizef zoomedSize(area.get_width() * camZoom, area.get_height() * camZoom);

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			clan::Vec2f viewportOffset =
				camera->GetPosition() - clan::Vec2f::calc_origin( clan::origin_center, zoomedSize);

			area.left = viewportOffset.x;
			area.top = viewportOffset.y;
			area.set_size(zoomedSize);
		}
	}

	const clan::Canvas& Renderer::GetCanvas() const
	{
		return m_Canvas;
	}

	int Renderer::GetContextWidth() const
	{
		return m_Canvas.get_width();
	}

	int Renderer::GetContextHeight() const
	{
		return m_Canvas.get_height();
	}

	void Renderer::SetupDraw(const ViewportPtr& viewport, clan::Rectf* draw_area)
	{
		const CameraPtr &camera = viewport->GetCamera();

		clan::Rect viewportArea;
		CalculateScreenArea(viewportArea, viewport);

		// Set the viewport
		m_Canvas.set_cliprect(viewportArea);

		const clan::Vec2f &camPosition = camera->GetPosition();
		const clan::Origin camOrigin = clan::origin_center;

		const clan::Sizef sizef(viewportArea.get_size());
		// Get the top-left pixel of the viewport (unscaled) in relative to the origin point of the world
		const clan::Vec2f viewportOffsetInWorld = camPosition * camera->GetZoom() - clan::Vec2f::calc_origin(camOrigin, sizef);

		// Apply rotation, translation & scale
		m_Canvas.push_modelview();
		m_Canvas.set_translate(-viewportOffsetInWorld.x + viewportArea.left, -viewportOffsetInWorld.y + viewportArea.top);
		m_Canvas.mult_rotate(clan::Angle(-camera->GetAngle(), clan::angle_radians));
		if (!fe_fequal(camera->GetZoom(), 1.f))
			m_Canvas.mult_scale(camera->GetZoom(), camera->GetZoom());

		if (draw_area != nullptr)
		{
			float drawAreaScale = 0.001f;
			if (!fe_fzero(camera->GetZoom()))
				drawAreaScale = 1.f / camera->GetZoom();
			const clan::Sizef scaledSize(viewportArea.get_width() * drawAreaScale, viewportArea.get_height() * drawAreaScale);

			const clan::Vec2f scaledViewportOffsetInWorld = camPosition - clan::Vec2f::calc_origin(camOrigin, scaledSize);

			draw_area->left = scaledViewportOffsetInWorld.x;
			draw_area->top = scaledViewportOffsetInWorld.y;
			draw_area->set_size(scaledSize);
		}
	}

	void Renderer::PostDraw()
	{
		m_Canvas.pop_modelview();
		m_Canvas.reset_cliprect(); // the viewport cliprect
	}

}
