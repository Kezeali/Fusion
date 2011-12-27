/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionStreamingCameraComponent.h"

#include "FusionMaths.h"
#include "FusionTransformComponent.h"
#include "FusionResourceManager.h"

#include <functional>

namespace FusionEngine
{

	StreamingCamera::StreamingCamera()
		: m_Interpolate(false),
		m_SyncType(NoSync),
		m_ViewportEnabled(true),
		m_ViewportRect(0.f, 0.f, 1.f, 1.f)
	{
	}

	StreamingCamera::~StreamingCamera()
	{
		m_PositionChangeConnection.disconnect();
		m_AngleChangeConnection.disconnect();
	}

	void StreamingCamera::SetCamera(const CameraPtr& camera)
	{
		m_Camera = camera;
	}

	void StreamingCamera::Update(float dt, float interp_a)
	{
		m_LastPosition = m_Position;
		m_LastAngle = m_Angle;

		m_Position = m_IncommingPosition;
		m_Angle = m_IncommingAngle;

		if (m_Interpolate)
		{
			Maths::Lerp(m_InterpPosition, m_LastPosition, m_Position, interp_a);
			Maths::AngleInterp(m_InterpAngle, m_LastAngle, m_Angle, interp_a);
		}
		else
		{
			m_InterpPosition = m_Position;
			m_InterpAngle = m_Angle;
		}

		if (m_Camera)
		{
			m_Camera->SetSimPosition(m_InterpPosition);
			if (m_AngleEnabled)
				m_Camera->SetAngle(m_InterpAngle);
		}
	}

	void StreamingCamera::SetPosition(const Vector2& value)
	{
		m_IncommingPosition = value;
	}

	Vector2 StreamingCamera::GetPosition() const
	{
		return m_Position;
	}

	void StreamingCamera::SetAngle(float angle)
	{
		m_IncommingAngle = angle;
	}

	// IComponent
	void StreamingCamera::OnSiblingAdded(const ComponentPtr& component)
	{
		if (auto transform = dynamic_cast<ITransform*>(component.get()))
		{
			m_PositionChangeConnection.disconnect();
			m_AngleChangeConnection.disconnect();

			m_PositionChangeConnection = transform->Position.Connect(std::bind(&StreamingCamera::SetPosition, this, std::placeholders::_1));
			m_AngleChangeConnection = transform->Angle.Connect(std::bind(&StreamingCamera::SetAngle, this, std::placeholders::_1));

			m_Position = transform->Position.Get();
			m_Angle = transform->Angle.Get();
		}
	}

	bool StreamingCamera::SerialiseOccasional(RakNet::BitStream& stream, const SerialiseMode mode)
	{
		stream.Write((uint8_t)m_SyncType);

		stream.Write(m_ViewportEnabled);
		stream.Write(m_ViewportRect.left);
		stream.Write(m_ViewportRect.top);
		stream.Write(m_ViewportRect.right);
		stream.Write(m_ViewportRect.bottom);

		stream.Write(m_AngleEnabled);

		return true;
	}

	void StreamingCamera::DeserialiseOccasional(RakNet::BitStream& stream, const SerialiseMode mode)
	{
		uint8_t syncType;
		stream.Read(syncType);
		m_SyncType = (SyncTypes)syncType;

		stream.Read(m_ViewportEnabled);
		stream.Read(m_ViewportRect.left);
		stream.Read(m_ViewportRect.top);
		stream.Read(m_ViewportRect.right);
		stream.Read(m_ViewportRect.bottom);

		stream.Read(m_AngleEnabled);
	}

	// ICamera
	void StreamingCamera::SetSyncType(SyncTypes value)
	{
		m_SyncType = value;
	}

	StreamingCamera::SyncTypes StreamingCamera::GetSyncType() const
	{
		return m_SyncType;
	}

	void StreamingCamera::SetViewportEnabled(bool value)
	{
		m_ViewportEnabled = value;
	}

	bool StreamingCamera::IsViewportEnabled() const
	{
		return m_ViewportEnabled;
	}

	void StreamingCamera::SetViewportRect(const CL_Rectf& value)
	{
		m_ViewportRect = value;
		if (m_Viewport)
			m_Viewport->SetArea(m_ViewportRect);
	}

	const CL_Rectf& StreamingCamera::GetViewportRect() const
	{
		return m_ViewportRect;
	}

	void StreamingCamera::SetAngleEnabled(bool value)
	{
		m_AngleEnabled = value;
	}

	bool StreamingCamera::IsAngleEnabled() const
	{
		return m_AngleEnabled;
	}

}
