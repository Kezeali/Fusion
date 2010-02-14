/*
  Copyright (c) 2009 Fusion Project Team

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

#include "FusionStableHeaders.h"

// Class
#include "FusionCamera.h"

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

	Vector2* Camera_GetPosition(Camera *obj)
	{
		return new Vector2(obj->GetPosition().x, obj->GetPosition().y);
	}

	void Camera_SetParallaxCamera(Camera *camera, float distance, Camera *obj)
	{
		obj->SetParallaxCamera(CameraPtr(camera), distance);
	}

	void Camera_SetFollowEntity(asIScriptObject *entity, Camera *obj)
	{
		obj->SetFollowEntity( EntityPtr(ScriptedEntity::GetAppObject(entity)) );
		entity->Release();
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
			asMETHOD(Camera, GetPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
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

}
