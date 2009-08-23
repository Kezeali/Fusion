#include "Common.h"

#include "FusionRenderer.h"

#include "FusionGUI.h"
#include "FusionScriptedEntity.h"

namespace FusionEngine
{

	Camera::Camera(asIScriptEngine *engine)
		: GarbageCollected(engine),
		m_Mode(FixedPosition),
		m_Origin(origin_center),
		m_AutoRotate(FixedAngle),
		m_Angle(0.f),
		m_Scale(1.f),
		m_Body(NULL),
		m_Joint(NULL)
	{
		defineBody();
	}

	Camera::Camera(asIScriptEngine *engine, float x, float y)
		: GarbageCollected(engine),
		m_Position(x, y),
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

	Camera::Camera(asIScriptEngine *engine, EntityPtr follow)
		: GarbageCollected(engine),
		m_FollowEntity(follow),
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

	void Camera::SetParallaxCamera(const CameraPtr &main_camera, float distance)
	{
		m_MainCamera = main_camera;
		m_ParallaxDistance = distance;
	}

	void Camera::SetFollowEntity(const EntityPtr &entity)
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
		if (m_FollowEntity)
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

			if (m_AutoRotate == MatchEntity)
			{
				m_Angle = m_FollowEntity->GetAngle();
			}
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

	void Camera::EnumReferences(asIScriptEngine *engine)
	{
		engine->GCEnumCallback((void*)m_FollowEntity.get());
	}

	void Camera::ReleaseAllReferences(asIScriptEngine *engine)
	{
		m_FollowEntity.reset();
	}

	Camera* Camera_Factory()
	{
		return new Camera(asGetActiveContext()->GetEngine());
	}

	Camera* Camera_Factory(float x, float y)
	{
		return new Camera(asGetActiveContext()->GetEngine(), x, y);
	}

	Camera* Camera_Factory(asIScriptObject *follow)
	{
		return new Camera(asGetActiveContext()->GetEngine(), ScriptedEntity::GetAppObject(follow));
	}

	Scripting::ScriptVector* Camera_GetPosition(Camera *obj)
	{
		return new Scripting::ScriptVector(obj->GetPosition().x, obj->GetPosition().y);
	}

	void Camera_SetParallaxCamera(Camera *camera, float distance, Camera *obj)
	{
		obj->SetParallaxCamera(CameraPtr(camera), distance);
	}

	void Camera_SetFollowEntity(asIScriptObject *entity, Camera *obj)
	{
		obj->SetFollowEntity(ScriptedEntity::GetAppObject(entity));
	}

	void Camera::Register(asIScriptEngine *engine)
	{
		int r;

		r = engine->RegisterEnum("CamFollowMode");
		r = engine->RegisterEnumValue("CamFollowMode", "FixedPosition", FixedPosition);
		r = engine->RegisterEnumValue("CamFollowMode", "FollowInstant", FollowInstant);
		r = engine->RegisterEnumValue("CamFollowMode", "FollowSmooth", FollowSmooth);

		r = engine->RegisterEnum("PointOrigin");
		r = engine->RegisterEnumValue("PointOrigin", "top_left", origin_top_left);
		r = engine->RegisterEnumValue("PointOrigin", "top_center", origin_top_center);
		r = engine->RegisterEnumValue("PointOrigin", "top_right", origin_top_right);
		r = engine->RegisterEnumValue("PointOrigin", "center_left", origin_center_left);
		r = engine->RegisterEnumValue("PointOrigin", "center", origin_center);
		r = engine->RegisterEnumValue("PointOrigin", "center_right", origin_center_right);
		r = engine->RegisterEnumValue("PointOrigin", "bottom_left", origin_bottom_left);
		r = engine->RegisterEnumValue("PointOrigin", "bottom_center", origin_bottom_center);
		r = engine->RegisterEnumValue("PointOrigin", "bottom_right", origin_bottom_right);

		Camera::RegisterGCType(engine, "Camera");
		r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_FACTORY,
			"Camera@ f()",
			asFUNCTIONPR(Camera_Factory, (void), Camera*), asCALL_CDECL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_FACTORY,
			"Camera@ f(float, float)",
			asFUNCTIONPR(Camera_Factory, (float, float), Camera*), asCALL_CDECL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_FACTORY,
			"Camera@ f(IEntity@)",
			asFUNCTIONPR(Camera_Factory, (asIScriptObject*), Camera*), asCALL_CDECL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Camera",
			"void setOrigin(PointOrigin)",
			asMETHOD(Camera, SetPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Camera",
			"PointOrigin getOrigin() const",
			asMETHOD(Camera, GetPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Camera",
			"void setPosition(int, int)",
			asMETHOD(Camera, SetPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Camera",
			"Vector@ getPosition() const",
			asFUNCTION(Camera_GetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Camera",
			"void setAngle(float)",
			asMETHOD(Camera, SetAngle), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Camera",
			"float getAngle() const",
			asMETHOD(Camera, SetAngle), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Camera",
			"void setScale(float)",
			asMETHOD(Camera, SetZoom), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Camera",
			"float getScale() const",
			asMETHOD(Camera, GetZoom), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Camera",
			"void setParallaxCamera(Camera@, float)",
			asMETHOD(Camera, SetParallaxCamera), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Camera",
			"void setFollowEntity(IEntity@)",
			asFUNCTION(Camera_SetFollowEntity), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Camera",
			"void setFollowMode(CamFollowMode)",
			asMETHOD(Camera, SetFollowMode), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}


	Viewport::Viewport()
	{}

	Viewport::Viewport(const CL_Rectf &area)
		: m_Area(area)
	{
	}

	Viewport::Viewport(const CL_Rectf &area, const CameraPtr &camera)
		: m_Area(area),
		m_Camera(camera)
	{
	}

	void Viewport::SetPosition(float left, float top)
	{
		m_Area.left = left;
		m_Area.top = top;
	}

	void Viewport::SetSize(float width, float height)
	{
		m_Area.set_width(width);
		m_Area.set_height(height);
	}

	const CL_Rectf &Viewport::GetArea() const
	{
		return m_Area;
	}
	CL_Pointf Viewport::GetPosition() const
	{
		return m_Area.get_top_left();
	}
	CL_Sizef Viewport::GetSize() const
	{
		return m_Area.get_size();
	}

	void Viewport::SetCamera(const CameraPtr &camera)
	{
		m_Camera = camera;
	}
	const CameraPtr &Viewport::GetCamera() const
	{
		return m_Camera;
	}

	Vector2* Viewport::ToScreenCoords(const Vector2 &entity_position) const
	{
		Vector2 *position = new Vector2();
		position->x = entity_position.x - m_Camera->GetPosition().x + m_Area.left;
		position->y = entity_position.y - m_Camera->GetPosition().y + m_Area.top;
		return position;
	}

	Vector2* Viewport::ToEntityCoords(const Vector2 &screen_position) const
	{
		Vector2 *position = new Vector2();
		position->x = screen_position.x + m_Camera->GetPosition().x + m_Area.left;
		position->y = screen_position.y + m_Camera->GetPosition().y + m_Area.top;
		return position;
	}

	Viewport* Viewport_Factory()
	{
		return new Viewport();
	}

	Viewport* Viewport_Factory(float left, float top, float right, float bottom)
	{
		return new Viewport(CL_Rectf(left, top, right, bottom));
	}

	Viewport* Viewport_Factory(float left, float top, float right, float bottom, Camera* camera)
	{
		return new Viewport(CL_Rectf(left, top, right, bottom), camera);
	}

	void Viewport_SetCamera(Camera *camera, Viewport *viewport)
	{
		viewport->SetCamera(camera);
	}

	void Viewport::Register(asIScriptEngine *engine)
	{
		int r;
		RefCounted::RegisterType<Viewport>(engine, "Viewport");
		r = engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY,
			"Viewport@ f()",
			asFUNCTIONPR(Viewport_Factory, (void), Viewport*), asCALL_CDECL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY,
			"Viewport@ f(float, float, float, float)",
			asFUNCTIONPR(Viewport_Factory, (float, float, float, float), Viewport*), asCALL_CDECL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY,
			"Viewport@ f(float, float, float, float, Camera@)",
			asFUNCTIONPR(Viewport_Factory, (float, float, float, float, Camera*), Viewport*), asCALL_CDECL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Viewport",
			"void setPosition(float, float)",
			asMETHOD(Viewport, SetPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"void setSize(float, float)",
			asMETHOD(Viewport, SetSize), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"void setCamera(Camera@)",
			asFUNCTION(Viewport_SetCamera), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"Camera& getCamera()",
			asMETHOD(Viewport, GetCamera), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Viewport",
			"Vector@ toScreenCoords(const Vector &in) const",
			asMETHOD(Viewport, ToScreenCoords), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"Vector@ toEntityCoords(const Vector &in) const",
			asMETHOD(Viewport, ToEntityCoords), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

	Renderer::Renderer(const CL_GraphicContext &gc)
		: m_GC(gc),
		m_EntityAdded(false)
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::CalculateScreenArea(CL_Rect &area, const ViewportPtr &viewport)
	{
		const CL_Rectf &proportions = viewport->GetArea();
		area.left = (int)floor(proportions.left * m_GC.get_width());
		area.top = (int)floor(proportions.top * m_GC.get_height());
		area.right = (int)ceil(proportions.right * m_GC.get_width());
		area.bottom = (int)ceil(proportions.bottom * m_GC.get_height());
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

		const CameraPtr &camera = viewport->GetCamera();

		CL_Rect viewportArea;
		CalculateScreenArea(viewportArea, viewport);

		// Set the viewport
		m_GC.set_cliprect(viewportArea);

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

		m_GC.pop_modelview();

		GUI::getSingleton().Draw();

		m_GC.reset_cliprect(); // the viewport cliprect

		// By this point the draw list will have been updated to reflect the changed tags
		m_ChangedTags.clear();

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
