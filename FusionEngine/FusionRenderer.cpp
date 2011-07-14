#include "FusionStableHeaders.h"

#include "FusionRenderer.h"

#include "FusionExceptionFactory.h"
#include "FusionGUI.h"
#include "FusionScriptedEntity.h"

namespace FusionEngine
{

	Renderer::Renderer(const CL_GraphicContext &gc)
		: m_GC(gc),
		m_EntityAdded(false)
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
		gc.set_translate(-viewportOffset.x, -viewportOffset.y);
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

	/*
	void Renderer::Draw(EntityArray &entities, const ViewportPtr &viewport, size_t layer)
	{
		drawImpl(entities, viewport, layer, std::string(), 0xFFFFFFFF);
	}

	void Renderer::Draw(EntityArray &entities, const ViewportPtr &viewport, size_t layer, const std::string& renderable_tag)
	{
		drawImpl(entities, viewport, layer, renderable_tag, 0xFFFFFFFF);
	}

	void Renderer::Draw(EntityArray &entities, const ViewportPtr &viewport, size_t layer, uint32_t filter_flags)
	{
		drawImpl(entities, viewport, layer, std::string(), filter_flags);
	}

	void Renderer::DrawEntity(const EntityPtr& entity)
	{
		drawEntity(entity, [&](const RenderablePtr& renderable)->bool { return true; });
	}

	void Renderer::DrawEntity(const EntityPtr& entity, const std::string& renderable_tag, uint32_t filter_flags)
	{
		drawEntity(entity, [&](const RenderablePtr& renderable)->bool
		{
			return (renderable->GetFlags() & filter_flags) != 0 || (!renderable_tag.empty() && renderable->HasTag(renderable_tag));
		});
	}

	void Renderer::drawImpl(EntityArray &entities, const ViewportPtr &viewport, size_t layer, const std::string& renderable_tag, uint32_t filter_flags)
	{
		CL_Rectf drawArea;
		SetupDraw(m_GC, viewport, &drawArea);

		actuallyDraw(entities, drawArea, layer, renderable_tag, filter_flags);

		m_GC.pop_modelview();

		GUI::getSingleton().Draw();

		m_GC.reset_cliprect(); // the viewport cliprect

		// By this point the draw list will have been updated to reflect the changed tags
		//m_ChangedTags.clear();
	}

	void Renderer::actuallyDraw(EntityArray &entities, const CL_Rectf &draw_area, size_t layer, const std::string& renderable_tag, uint32_t filter_flags)
	{
		int previousDepth = INT_MIN; // Setting to max skips the first comparison, which would be invalid (since it-1 would be illegal)
		for (EntityArray::iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			//RenderablePtr &renderable = *it;
			EntityPtr &entity = *it;

			if (entity->IsHiddenByTag())
				continue;
			if (entity->IsHidden())
				continue;
			if (!entity->IsStreamedIn())
				continue;

			if (entity->GetLayer() != layer)
				continue;

			if (entity->GetRenderables().empty())
				continue;

			const Vector2 &entityPosition = entity->GetPosition();

			CL_Mat4f entityTransform = CL_Mat4f::translate(entityPosition.x, entityPosition.y, 0.f);
			entityTransform.multiply(CL_Mat4f::rotate(CL_Angle(entity->GetAngle(), cl_radians), 0.f, 0.f, 1.f));

			// draw_area translated by -entityPosition (since Renderable positions are relative to entity position)
			CL_Rectf relativeDrawRect(draw_area.left - entityPosition.x, draw_area.top - entityPosition.y, draw_area.right - entityPosition.x, draw_area.bottom - entityPosition.y);

			m_GC.push_modelview();
			//m_GC.mult_modelview(entityTransform);
			m_GC.mult_translate(entityPosition.x, entityPosition.y);
			m_GC.mult_rotate(CL_Angle(entity->GetAngle(), cl_radians));

			// Draw the entities renderables
			drawEntity(entity, relativeDrawRect, renderable_tag, filter_flags);

			m_GC.pop_modelview();

			// Bubble up previous Entity if incorrectly depth-sorted
			if (entity->GetDepth() < previousDepth)
				std::swap(*it, *(it-1));
			else
				previousDepth = entity->GetDepth();
		}
	}

	inline void Renderer::drawEntity(const EntityPtr& entity, const CL_Rectf& draw_area, const std::string& renderable_tag, uint32_t filter_flags)
	{
		drawEntity(entity, [&](const RenderablePtr& renderable)->bool
		{
			return ((renderable->GetFlags() & filter_flags) != 0 || (!renderable_tag.empty() && renderable->HasTag(renderable_tag)))
				&& draw_area.is_overlapped(renderable->GetAABB());
		});
	}

	inline void Renderer::drawEntity(const EntityPtr& entity, std::function<bool (const RenderablePtr&)> filter_fn)
	{
		RenderableArray &entityRenderables = entity->GetRenderables();
		int previousRenderableDepth = 0;
		for (RenderableArray::iterator r_it = entityRenderables.begin(), r_end = entityRenderables.end(); r_it != r_end; ++r_it)
		{
			RenderablePtr &renderable = *r_it;
			if (filter_fn(renderable))
				renderable->Draw(m_GC, Vector2());

			// Bubble-sort by depth
			if (renderable->GetDepth() < previousRenderableDepth)
				std::swap(*r_it, *(r_it-1));
			else
				previousRenderableDepth = renderable->GetDepth();
		}
	}
	*/

}
