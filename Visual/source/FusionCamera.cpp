/*
*  Copyright (c) 2009-2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "PrecompiledHeaders.h"

// Class
#include "FusionCamera.h"

#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{

	Camera::Camera()
		: m_Angle(0.f),
		m_Scale(1.f)
	{
	}

	Camera::Camera(float x, float y)
		: m_Position(x, y),
		m_Angle(0.f),
		m_Scale(1.f)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::SetPosition(float x, float y)
	{
		m_Position.x = x;
		m_Position.y = y;
	}

	void Camera::SetSimPosition(const Vector2& position)
	{
		m_Position.x = ToRenderUnits(position.x);
		m_Position.y = ToRenderUnits(position.y);
	}

	Vector2 Camera::GetSimPosition() const
	{
		return Vector2(ToSimUnits(m_Position.x), ToSimUnits(m_Position.y));;
	}

	void Camera::SetAngle(float angle)
	{
		m_Angle = angle;
	}

	void Camera::SetZoom(float scale)
	{
		m_Scale = scale;
	}

	const CL_Vec2f &Camera::GetPosition() const
	{
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

	void Camera_Ctor2(float x, float y, void* ptr)
	{
		new (ptr) std::shared_ptr<Camera>(new Camera(x, y));
	}

	void Camera_Ctor3(Vector2& pos, void* ptr)
	{
		new (ptr) std::shared_ptr<Camera>(new Camera(pos.x, pos.y));
	}

	void Camera_SetPosition(Vector2& pos, CameraPtr *obj)
	{
		return (*obj)->SetPosition(ToRenderUnits(pos.x), ToRenderUnits(pos.y));
	}

	Vector2 Camera_GetPosition(CameraPtr *obj)
	{
		return Vector2((*obj)->GetPosition().x, (*obj)->GetPosition().y);
	}

	void Camera::Register(asIScriptEngine *engine)
	{
		int r;

		RegisterSharedPtrType<Camera>("Camera", engine);
		// The other ctor overload (sets position)
		r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION(Camera_Ctor2), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_CONSTRUCT, "void f(Vector &in)", asFUNCTION(Camera_Ctor3), asCALL_CDECL_OBJLAST);

		r = engine->RegisterObjectMethod("Camera",
			"void setPosition(Vector &in)",
			asFUNCTION(Camera_SetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Camera",
			"Vector getPosition() const",
			asFUNCTION(Camera_GetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		/*r = engine->RegisterObjectMethod("Camera",
			"void setPosition(float, float)",
			asMETHOD(Camera, SetPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Camera",
			"Vector getPosition() const",
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
			asMETHOD(Camera, GetZoom), asCALL_THISCALL); FSN_ASSERT(r >= 0);*/
	}

}
