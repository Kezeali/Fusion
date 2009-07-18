#include "Common.h"

#include "FusionRenderer.h"

namespace FusionEngine
{

	Camera::Camera()
		: m_Body(NULL),
		m_Joint(NULL)
	{
		defineBody();
	}

	Camera::Camera(float x, float y)
		: m_Position(x, y),
		m_Body(NULL),
		m_Joint(NULL)
	{
		defineBody();
	}

	Camera::Camera(EntityPtr follow)
		: m_FollowEntity(follow),
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

	void Camera::CreateBody(b2World *world)
	{
		m_Body = world->CreateBody(&m_BodyDefinition);
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
			m_Position = m_Entity->GetPosition();
		}
		else if (m_Mode == FollowSmooth)
		{
			// Not implemented
		}
	}

	const CL_Vec2f &Camera::GetPosition() const
	{
		if (m_Mode == Physical)
		{
			m_Position.x = m_Body->GetPosition().x;
			m_Position.y = m_Body->GetPosition().y;
		}
		return m_Position;
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
		m_EntitiesChanged(false)
	{
	}

	void Renderer::Add(EntityPtr entity)
	{
		m_Entities.push_back(entity);
		m_EntityAdded = true;
	}

	void Renderer::Remove(EntityPtr entity)
	{
		m_Entities.erase(entity);
		//m_EntitiesChanged = true;
	}

	void Renderer::ShowTag(const std::string &tag)
	{
		m_ChangedTags.show(tag);
	}

	void Renderer::HideTag(const std::string &tag)
	{
		m_ChangedTags.hide(tag);
	}

	//void Renderer::AddViewport(ViewportPtr viewport)
	//{
	//	m_Viewports.push_back(viewport);
	//}

	void Renderer::Draw(ViewportPtr viewport)
	{

		if (m_EntitiesAdded || m_ChangedTags.somethingWasShown())
		{
			// Entities have been added / shown so the depth list must be rebuilt
			updateDrawArray();
			m_EntitiesAdded = false;
		}

		CameraPtr &camera = viewport->GetCamera();

		const CL_Rect &viewportArea = viewport->GetArea();

		// Set the viewport
		m_GC.push_cliprect(viewportArea);

		const CL_Vec2f &camPosition = camera->GetPosition();

		// Set up rotation, translation & scale matrix
		CL_Mat4f cameraTransform = CL_Mat4f::multiply(
			CL_Mat4f::translate(-camPosition.x, -camPosition.y, 0.f),
			CL_Mat4f::rotate(CL_Angle(camera->GetAngle(), cl_radians), 0.f, 0.f, 1.f) );
		// Scale
		if ( !fe_fzero(camera->GetZoom()) )
			cameraTransform.multiply(CL_Mat4f::scale(camera->GetZoom(), camera->GetZoom(), 0.f));

		// Apply rotation, translation & scale
		m_GC.push_modelview();
		m_GC.set_modelview(cameraTransform);

		// Draw the entities within the camera area for this viewport
		CL_Rectf drawArea(camPosition.x, camPosition.y, viewportArea.get_size() * camera->GetZoom());
		drawNormally(drawArea);

		// By this point the draw list will have been updated to reflect the changed tags
		m_ChangedTags.clear();

		m_GC.pop_modelview();

		m_GC.pop_cliprect(); // the viewport cliprect
	}

	void Renderer::updateDrawArray()
	{
		for (EntitySet::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			const EntityPtr &entity = *it;
			for (StringSet::iterator ch = m_ShownTags.begin(), ch_end = m_ShownTags.end(); it != end; ++it)
				if (entity->CheckTag(*ch))
					m_EntitiesToDraw.push_back(entity);
		}
	}

	void Renderer::drawNormally(const CL_Rectf &draw_area)
	{
		int previousDepth = INT_MAX; // Setting to max skips the first comparison, which would be invalid (since it-1 would be illegal)
		for (EntityArray::iterator it = m_EntitiesToDraw.begin(), end = m_EntitiesToDraw.end(); it != end; ++it)
		{
			EntityPtr &entity = *it;

			if (m_ChangedTags.wasHidden(entity))
				m_EntitiesToDraw.erase(it);

			if (entity->IsHidden())
				continue;
			if (entity->IsStreamedOut())
				continue;

			const Vector2 &entityPosition = entity->GetPosition();

			m_GC.push_translate(entityPosition.x, entityPosition.y);

			Entity::RenderableArray &entityRenderables = entity->GetRenderables();
			for (Entity::RenderableArray::iterator it = entityRenderables.begin(), end = entityRenderables.end(); it != end; ++it)
			{
				RenderablePtr renderable = *it;
				ResourcePointer<CL_Sprite> &spriteResource = renderable->sprite;

				float x = entityPosition.x + renderable->position.x;
				float y = entityPosition.y + renderable->position.y;
				CL_Rectf spriteRect(x, y, x + spriteResource->get_width(), y +spriteResource->get_height());
				if (spriteResource.IsValid() && draw_area.is_overlapped(spriteRect))
				{
					spriteResource->draw(m_GC, renderable->GetPosition().x, renderable->GetPosition().y);
				}
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

}
