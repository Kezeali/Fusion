#include "Common.h"

#include "FusionRenderer.h"

namespace FusionEngine
{

	Camera::Camera()
		: m_Mode(FixedPosition),
		m_Origin(origin_center),
		m_AutoRotate(FixedAngle),
		m_Angle(0.f),
		m_Scale(1.f),
		m_Body(NULL),
		m_Joint(NULL)
	{
		defineBody();
	}

	Camera::Camera(float x, float y)
		: m_Position(x, y),
		m_Origin(origin_center),
		m_Mode(FixedPosition),
		m_AutoRotate(FixedAngle),
		m_Angle(0.f),
		m_Scale(1.f),
		m_Body(NULL),
		m_Joint(NULL)
	{
		defineBody();
	}

	Camera::Camera(EntityPtr follow)
		: m_FollowEntity(follow),
		m_Origin(origin_center),
		m_Mode(FollowInstant),
		m_AutoRotate(FixedAngle),
		m_Angle(0.f),
		m_Scale(1.f),
		m_Body(NULL),
		m_Joint(NULL)
	{
		defineBody();
	}

	Camera::~Camera()
	{
		if (m_Body != NULL)
		{
			m_Body->GetWorld()->DestroyBody(m_Body);
		}
	}

	void Camera::SetOrigin(CL_Origin origin)
	{
		m_Origin = origin;
	}

	void Camera::SetPosition(float x, float y)
	{
		m_Position.x = x;
		m_Position.y = y;
	}

	void Camera::SetAngle(float angle)
	{
		m_Angle = angle;
	}

	//void Camera::SetOrientation(const Quaternion &orient)
	//{
	//}

	void Camera::SetZoom(float scale)
	{
		m_Scale = scale;
	}

	void Camera::SetFollowEntity(EntityPtr entity)
	{
		m_FollowEntity = entity;
	}

	void Camera::SetFollowMode(FollowMode mode)
	{
		m_Mode = mode;
	}

	void Camera::SetAutoRotate(RotateMode mode)
	{
		m_AutoRotate = mode;
	}

	b2Body *Camera::CreateBody(b2World *world)
	{
		return m_Body = world->CreateBody(&m_BodyDefinition);
	}

	void Camera::JoinToBody(b2Body *body)
	{
		createBody(body->GetWorld());
	}

	b2Body *Camera::GetBody() const
	{
		return m_Body;
	}

	void Camera::Update(float split)
	{
		if (m_Mode == FollowInstant)
		{
			const Vector2 &target = m_FollowEntity->GetPosition();
			m_Position.x = target.x;
			m_Position.y = target.y;
		}
		else if (m_Mode == FollowSmooth)
		{
			// Not implemented
		}
		if (m_Mode == Physical)
		{
			m_Position.x = m_Body->GetPosition().x;
			m_Position.y = m_Body->GetPosition().y;
		}

		if (m_AutoRotate == MatchEntity)
		{
			m_Angle = m_FollowEntity->GetAngle();
		}
	}

	const CL_Vec2f &Camera::GetPosition() const
	{
		return m_Position;
	}

	CL_Origin Camera::GetOrigin() const
	{
		return m_Origin;
	}

	float Camera::GetAngle() const
	{
		return m_Angle;
	}

	float Camera::GetZoom() const
	{
		return m_Scale;
	}

	void Camera::defineBody()
	{
		m_BodyDefinition.position.Set(m_Position.x, m_Position.y);
		m_BodyDefinition.massData.center.SetZero();
		m_BodyDefinition.massData.mass = 2;
	}

	void Camera::createBody(b2World *world)
	{
		if (m_Body == NULL)
			m_Body = world->CreateBody(&m_BodyDefinition);
	}


	Viewport::Viewport()
	{}

	Viewport::Viewport(CL_Rect area)
		: m_Area(area)
	{
	}

	Viewport::Viewport(CL_Rect area, CameraPtr camera)
		: m_Area(area),
		m_Camera(camera)
	{
	}

	void Viewport::SetPosition(int left, int top)
	{
		m_Area.left = left;
		m_Area.top = top;
	}

	void Viewport::SetSize(int width, int height)
	{
		m_Area.set_width(width);
		m_Area.set_height(height);
	}

	const CL_Rect &Viewport::GetArea() const
	{
		return m_Area;
	}
	CL_Point Viewport::GetPosition() const
	{
		return m_Area.get_top_left();
	}
	CL_Size Viewport::GetSize() const
	{
		return m_Area.get_size();
	}

	void Viewport::SetCamera(const CameraPtr &camera)
	{
		m_Camera = camera;
	}
	CameraPtr Viewport::GetCamera() const
	{
		return m_Camera;
	}

	Renderer::Renderer(const CL_GraphicContext &gc)
		: m_GC(gc),
		m_EntityAdded(false)
	{
	}

	Renderer::~Renderer()
	{
	}

	ViewportPtr Renderer::CreateViewport(ViewportArea area)
	{
		CL_Rect rect;
		if (area == ViewFull)
		{
			rect.bottom = m_GC.get_height();
			rect.right = m_GC.get_width();
		}
		else if (area == ViewVerticalHalf)
		{
			rect.bottom = fe_round<int>(m_GC.get_height() * 0.5);
			rect.right = m_GC.get_width();
		}
		else if (area == ViewHorizontalHalf)
		{
			rect.bottom = m_GC.get_height();;
			rect.right = fe_round<int>(m_GC.get_width() * 0.5);
		}
		else if (area == ViewQuarter)
		{
			rect.bottom = fe_round<int>(m_GC.get_height() * 0.5);
			rect.right = fe_round<int>(m_GC.get_width() * 0.5);
		}
		return ViewportPtr(new Viewport(rect));
	}

	bool lowerDepth(const EntityPtr &l, const EntityPtr &r)
	{
		return l->GetDepth() < r->GetDepth();
	}

	void Renderer::Add(const EntityPtr &entity)
	{
		//m_Entities.insert(entity);
		//if (!entity->IsHidden())
		//{
		//	m_EntitiesToDraw.insert(
		//		std::lower_bound(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), entity->GetDepth()),
		//		entity);
		//}

		std::sort(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), lowerDepth);

		m_EntitiesToDraw.insert(
			std::lower_bound(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), entity, lowerDepth),
			entity);

		//for (RenderableArray::iterator it = entity->GetRenderables().begin(), end = entity->GetRenderables().end(); it != end; ++it)
		//{
		//	m_Renderables.insert(
		//		std::lower_bound(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), entity->GetDepth() + it->GetDepth()),
		//		*it);
		//}

		//m_EntityAdded = true;
	}

	void Renderer::Remove(const EntityPtr &entity)
	{
		//m_Entities.erase(entity);
		//entity->SetHidden(true);

		std::sort(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), lowerDepth);

		EntityArray::iterator it = std::lower_bound(m_EntitiesToDraw.begin(), m_EntitiesToDraw.end(), entity, lowerDepth);
		for (EntityArray::iterator end = m_EntitiesToDraw.end(); it != end; ++it)
			if (*it == entity && (*it)->GetDepth() > entity->GetDepth())
				break;

		m_EntitiesToDraw.erase(it);

		//for (RenderableArray::iterator it = entity->GetRenderables().begin(), end = entity->GetRenderables().end(); it != end; ++it)
		//{
		//	if (it->GetEntity())
		//		m_Renderables.erase(it);
		//}

		//m_EntitiesChanged = true;
	}

	void Renderer::Clear()
	{
		//m_Entities.clear();
		m_EntitiesToDraw.clear();
		m_ChangedTags.clear();
		//m_Renderables.clear();
	}

	void Renderer::ShowTag(const std::string &tag)
	{
		m_ChangedTags.show(tag);
		m_HiddenTags.erase(tag);
	}

	void Renderer::HideTag(const std::string &tag)
	{
		m_ChangedTags.hide(tag);
		m_HiddenTags.insert(tag);
	}

	//void Renderer::AddViewport(ViewportPtr viewport)
	//{
	//	m_Viewports.push_back(viewport);
	//}

	void Renderer::Update(float split)
	{
		//for (EntityArray::iterator it = m_EntitiesToDraw.begin(), end = m_EntitiesToDraw.end(); it != end; ++it)
		//{
		//	RenderableArray &renderables = (*it)->GetRenderables();
		//	for (RenderableArray::iterator r_it = renderables.begin(), r_end = renderables.end(); r_it != r_end; ++r_it)
		//	{
		//		(*r_it)->Update(split);
		//	}
		//}
	}

	void Renderer::Draw(ViewportPtr viewport)
	{
		//if (m_ChangedTags.somethingWasShown())
		//{
		//	// Tags have been shown so Entities with those tags must be added to the depth-sorted draw list
		//	updateDrawArray();
		//	m_EntitiesAdded = false;
		//}

		CameraPtr &camera = viewport->GetCamera();

		const CL_Rect &viewportArea = viewport->GetArea();

		// Set the viewport
		m_GC.push_cliprect(viewportArea);

		const CL_Vec2f &camPosition = camera->GetPosition();
		CL_Origin camOrigin = camera->GetOrigin();

		//CL_Vec3f look(0.f, 0.f, 1.f), up(0.f, 1.f, 0.f), left(1.f, 0.f, 0.f);

		//CL_Mat4f rollMatrix = CL_Mat4f::rotate(CL_Angle(camera->GetAngle(), cl_radians), look.x, look.y, look.z);

		// Rotate up and left - by the camera angle - around the look vector (i.e. normal 2d rotation)
		//CL_Angle rotationAngle(camera->GetAngle(), cl_radians);
		//up.rotate(rotationAngle, look);
		//left.rotate(rotationAngle, look);

		CL_Vec2f viewportOffset;
		viewportOffset = camPosition - CL_Vec2f::calc_origin(camOrigin, CL_Sizef((float)viewportArea.get_width(), (float)viewportArea.get_height()));
		//if (camOrigin == origin_center)
		//{
		//	viewportOffset.x += viewportArea.get_width() * 0.5;
		//	viewportOffset.y += viewportArea.get_width() * 0.5;
		//}

		// Set up rotation, translation & scale matrix
		//CL_Mat4f cameraTransform = CL_Mat4f::translate(-viewportOffset.x, -viewportOffset.y, 0.f);
		//CL_Mat4f cameraTransform = CL_Mat4f::multiply(
		//	CL_Mat4f::translate(-viewportOffset.x, -viewportOffset.y, 0.f),
		//	CL_Mat4f::rotate(CL_Angle(camera->GetAngle(), cl_radians), 0.f, 0.f, 1.f) );
		// Scale
		//if ( !fe_fequal(camera->GetZoom(), 1.f) )
			//cameraTransform.multiply(CL_Mat4f::scale(camera->GetZoom(), camera->GetZoom(), 0.f));

		// Apply rotation, translation & scale
		m_GC.push_modelview();
		//m_GC.set_modelview(cameraTransform);
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

		drawNormally(drawArea);

		// By this point the draw list will have been updated to reflect the changed tags
		m_ChangedTags.clear();

		m_GC.pop_modelview();

		m_GC.pop_cliprect(); // the viewport cliprect
	}

	void Renderer::drawNormally(const CL_Rectf &draw_area)
	{
		int notRendered = 0;

		int previousDepth = INT_MIN; // Setting to max skips the first comparison, which would be invalid (since it-1 would be illegal)
		for (EntityArray::iterator it = m_EntitiesToDraw.begin(), end = m_EntitiesToDraw.end(); it != end; ++it)
		{
			//RenderablePtr &renderable = *it;
			EntityPtr &entity = *it;

			if (entity->IsHiddenByTag())
				continue;

			if (entity->IsHidden())
				continue;
			if (entity->IsStreamedOut())
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
