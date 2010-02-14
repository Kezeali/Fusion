#include "Common.h"

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
				FSN_EXCEPT(ExCode::InvalidArgument, "Renderer::CalculateScreenArea", "Cannot apply camera offset if the viewport has no camera associated with it");

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			CL_Vec2i viewportOffset =
				camera->GetPosition() - CL_Vec2f::calc_origin( camera->GetOrigin(), CL_Sizef((float)area.get_width(), (float)area.get_height()) );

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
				FSN_EXCEPT(ExCode::InvalidArgument, "Renderer::CalculateScreenArea", "Cannot apply camera offset if the viewport has no camera associated with it");

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			CL_Vec2f viewportOffset =
				camera->GetPosition() - CL_Vec2f::calc_origin( camera->GetOrigin(), CL_Sizef((float)area.get_width(), (float)area.get_height()) );

			area.translate(viewportOffset);
		}
	}

	const CL_GraphicContext &Renderer::GetGraphicContext() const
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

	bool lowerDepth(const EntityPtr &l, const EntityPtr &r)
	{
		return l->GetDepth() < r->GetDepth();
	}

	//void Renderer::Add(const EntityPtr &entity)
	//{
	//	//m_Entities.insert(entity);
	//	//if (!entity->IsHidden())
	//	//{
	//	//	m_EntitiesToDraw.insert(
	//	//		std::lower_bound(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), entity->GetDepth()),
	//	//		entity);
	//	//}

	//	std::sort(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), lowerDepth);

	//	m_EntitiesToDraw.insert(
	//		std::lower_bound(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), entity, lowerDepth),
	//		entity);

	//	//for (RenderableArray::iterator it = entity->GetRenderables().begin(), end = entity->GetRenderables().end(); it != end; ++it)
	//	//{
	//	//	m_Renderables.insert(
	//	//		std::lower_bound(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), entity->GetDepth() + it->GetDepth()),
	//	//		*it);
	//	//}

	//	//m_EntityAdded = true;
	//}

	//void Renderer::Remove(const EntityPtr &entity)
	//{
	//	//m_Entities.erase(entity);
	//	//entity->SetHidden(true);

	//	std::sort(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), lowerDepth);

	//	EntityArray::iterator it = std::lower_bound(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), entity, lowerDepth);
	//	//for (EntityArray::iterator end = m_EntitiesToDraw.end(); it != end; ++it)
	//	//	if (*it == entity || (*it)->GetDepth() > entity->GetDepth())
	//	//		break;
	//	if (it != m_EntitiesToDraw.end() && *it == entity)
	//		m_EntitiesToDraw.erase(it);

	//	//for (RenderableArray::iterator it = entity->GetRenderables().begin(), end = entity->GetRenderables().end(); it != end; ++it)
	//	//{
	//	//	if (it->GetEntity())
	//	//		m_Renderables.erase(it);
	//	//}

	//	//m_EntitiesChanged = true;
	//}

	//void Renderer::Clear()
	//{
	//	//m_Entities.clear();
	//	m_EntitiesToDraw.clear();
	//	m_ChangedTags.clear();
	//	//m_Renderables.clear();
	//}

	//void Renderer::ShowTag(const std::string &tag)
	//{
	//	m_ChangedTags.show(tag);
	//	m_HiddenTags.erase(tag);
	//}

	//void Renderer::HideTag(const std::string &tag)
	//{
	//	m_ChangedTags.hide(tag);
	//	m_HiddenTags.insert(tag);
	//}

	//void Renderer::AddViewport(ViewportPtr viewport)
	//{
	//	m_Viewports.push_back(viewport);
	//}

	//void Renderer::Update(float split)
	//{
	//	//for (EntityArray::iterator it = m_EntitiesToDraw.begin(), end = m_EntitiesToDraw.end(); it != end; ++it)
	//	//{
	//	//	RenderableArray &renderables = (*it)->GetRenderables();
	//	//	for (RenderableArray::iterator r_it = renderables.begin(), r_end = renderables.end(); r_it != r_end; ++r_it)
	//	//	{
	//	//		(*r_it)->Update(split);
	//	//	}
	//	//}
	//}

	void Renderer::Draw(EntityArray &entities, const ViewportPtr &viewport, size_t layer)
	{
		const CameraPtr &camera = viewport->GetCamera();

		CL_Rect viewportArea;
		CalculateScreenArea(viewportArea, viewport);

		// Set the viewport
		m_GC.set_cliprect(viewportArea);

		const CL_Vec2f &camPosition = camera->GetPosition();
		CL_Origin camOrigin = camera->GetOrigin();

		CL_Vec2f viewportOffset;
		viewportOffset = camPosition - CL_Vec2f::calc_origin(camOrigin, CL_Sizef((float)viewportArea.get_width(), (float)viewportArea.get_height()));

		// Apply rotation, translation & scale
		m_GC.push_modelview();
		m_GC.set_translate(-viewportOffset.x, -viewportOffset.y);
		m_GC.mult_rotate(CL_Angle(-camera->GetAngle(), cl_radians));
		if ( !fe_fequal(camera->GetZoom(), 1.f) )
			m_GC.mult_scale(camera->GetZoom(), camera->GetZoom());

		// Draw the entities within the camera area for this viewport
		float drawAreaScale = 0.001f;
		if (!fe_fzero(camera->GetZoom()))
			drawAreaScale = 1.f / camera->GetZoom();
		CL_Size size = viewportArea.get_size();
		CL_Rectf drawArea(viewportOffset.x, viewportOffset.y, CL_Sizef(size.width * drawAreaScale, size.height * drawAreaScale));

		drawNormally(entities, drawArea, layer);

		m_GC.pop_modelview();

		GUI::getSingleton().Draw();

		m_GC.reset_cliprect(); // the viewport cliprect

		// By this point the draw list will have been updated to reflect the changed tags
		m_ChangedTags.clear();

	}

	void Renderer::drawNormally(EntityArray &entities, const CL_Rectf &draw_area, size_t layer)
	{
		int notRendered = 0;

		int previousDepth = INT_MIN; // Setting to max skips the first comparison, which would be invalid (since it-1 would be illegal)
		for (EntityArray::iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			//RenderablePtr &renderable = *it;
			EntityPtr &entity = *it;

			if (entity->IsHiddenByTag())
				continue;

			if (entity->IsHidden())
				continue;
			if (entity->IsStreamedOut())
				continue;

			if (entity->GetLayer() != layer)
				continue;

			const Vector2 &entityPosition = entity->GetPosition();

			CL_Mat4f entityTransform = CL_Mat4f::translate(entityPosition.x, entityPosition.y, 0.f);
			entityTransform.multiply(CL_Mat4f::rotate(CL_Angle(entity->GetAngle(), cl_radians), 0.f, 0.f, 1.f));

			// Draw_area translated by -entityPosition (since renderable AABBs are relative to entity position)
			CL_Rectf normDrawArea(draw_area.left - entityPosition.x, draw_area.top - entityPosition.y, draw_area.right - entityPosition.x, draw_area.bottom - entityPosition.y);

			m_GC.push_modelview();
			//m_GC.mult_modelview(entityTransform);
			m_GC.mult_translate(entityPosition.x, entityPosition.y);
			m_GC.mult_rotate(CL_Angle(entity->GetAngle(), cl_radians));

			//drawRenderables(entity, draw_area);
			RenderableArray &entityRenderables = entity->GetRenderables();
			for (RenderableArray::iterator r_it = entityRenderables.begin(), r_end = entityRenderables.end(); r_it != r_end; ++r_it)
			{
				RenderablePtr &renderable = *r_it;
				if ( normDrawArea.is_overlapped(renderable->GetAABB()) )
					renderable->Draw(m_GC, Vector2()/*entityPosition*/);
				else
					++notRendered;
			}

			m_GC.pop_modelview();

			// Bubble up previous Entity if incorrectly depth-sorted
			if (entity->GetDepth() < previousDepth)
			{
				std::swap(*it, *(it-1));
			}
			else
				previousDepth = entity->GetDepth();
		}
	}

	void Renderer::updateDrawArray()
	{
		//for (EntitySet::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		//{
		//	const EntityPtr &entity = *it;
		//	
		//	if (m_ChangedTags.wasShown(entity))
		//	{
		//		m_EntitiesToDraw.insert(
		//			std::lower_bound(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), entity->GetDepth()),
		//			entity);
		//	}
		//}
	}

	void Renderer::drawRenderables(EntityPtr &entity, const CL_Rectf &draw_area)
	{

		//int previousDepth = INT_MAX; // Setting to max skips the first comparison, which would be invalid (since it-1 would be illegal)
		//for (EntityArray::iterator it = m_EntitiesToDraw.begin(), end = m_EntitiesToDraw.end(); it != end; ++it)
		//{
		//	EntityPtr &entity = *it;

		//	if (m_ChangedTags.wasHidden(entity))
		//	{
		//		it = m_EntitiesToDraw.erase(it);
		//		if (it == m_EntitiesToDraw.end())
		//			break;
		//	}

		//	if (entity->IsHidden())
		//		continue;
		//	if (entity->IsStreamedOut())
		//		continue;

		//	const Vector2 &entityPosition = entity->GetPosition();

		//	m_GC.push_translate(entityPosition.x, entityPosition.y);

		//	Entity::RenderableArray &entityRenderables = entity->GetRenderables();
		//	for (Entity::RenderableArray::iterator it = entityRenderables.begin(), end = entityRenderables.end(); it != end; ++it)
		//	{
		//		RenderablePtr renderable = *it;
		//		ResourcePointer<CL_Sprite> &spriteResource = renderable->sprite;

		//		float x = entityPosition.x + renderable->position.x;
		//		float y = entityPosition.y + renderable->position.y;
		//		CL_Rectf spriteRect(x, y, x + spriteResource->get_width(), y +spriteResource->get_height());
		//		if (spriteResource.IsValid() && draw_area.is_overlapped(spriteRect))
		//		{
		//			spriteResource->draw(m_GC, renderable->GetPosition().x, renderable->GetPosition().y);
		//		}
		//	}

		//	m_GC.pop_modelview();

		//	// Bubble up previous Entity if incorrectly depth-sorted
		//	if (entity->GetDepth() < previousDepth)
		//	{
		//		std::swap(*it, *(it-1));
		//	}
		//	else
		//		previousDepth = entity->GetDepth();
		//}
	}

}
