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

#ifndef H_FusionStreamingCameraComponent
#define H_FusionStreamingCameraComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionCommon.h"

#include "FusionRender2DComponent.h"
//#include "FusionSerialisationHelper.h"

#include "FusionCamera.h"
#include "FusionViewport.h"

#include <boost/signals2/connection.hpp>

namespace FusionEngine
{

	//! Camera component
	/*
	* \todo Add a property to EntityComponent(?) that indicates that the given component type must only have one instance per entity
	*/
	class StreamingCamera : public EntityComponent, public ICamera
	{
		friend class CLRenderWorld;
		friend class CLRenderTask;
	public:
		FSN_LIST_INTERFACES((ICamera))

		//struct PropsIdx { enum Names : size_t {
		//	SyncType = 0,
		//	NumProps
		//}; };
		//typedef SerialisationHelper<
		//	SyncTypes> // SyncType
		//	DeltaSerialiser_t;
		//static_assert(PropsIdx::NumProps == DeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		StreamingCamera();
		virtual ~StreamingCamera();

		void Update(float dt, float interp);

		void SetCamera(const CameraPtr& camera);

		ViewportPtr GetViewport() const { return m_Viewport; }

	private:
		void SetPosition(const Vector2& value);
		Vector2 GetPosition() const;

		void SetAngle(float angle);

		// EntityComponent
		std::string GetType() const { return "StreamingCamera"; }

		void OnSiblingAdded(const ComponentPtr& component);

		void SerialiseOccasional(RakNet::BitStream& stream);
		void DeserialiseOccasional(RakNet::BitStream& stream);

		// ICamera
		void SetSyncType(SyncTypes value);
		SyncTypes GetSyncType() const;

		void SetViewportEnabled(bool value);
		bool IsViewportEnabled() const;

		void SetViewportRect(const clan::Rectf& value);
		const clan::Rectf& GetViewportRect() const;

		void SetAngleEnabled(bool value);
		bool IsAngleEnabled() const;

		SyncTypes m_SyncType;

		bool m_ViewportEnabled;
		clan::Rectf m_ViewportRect;

		CameraPtr m_Camera;
		ViewportPtr m_Viewport;

		bool m_AngleEnabled;

		bool m_Interpolate;

		SyncSig::HandlerConnection_t m_PositionChangeConnection;
		Vector2 m_IncommingPosition; // Set by SetPosition

		Vector2 m_Position; // NewPosition is copied here when Update is called
		Vector2 m_InterpPosition;
		Vector2 m_LastPosition;

		SyncSig::HandlerConnection_t m_AngleChangeConnection;
		float m_IncommingAngle;

		float m_Angle;
		float m_InterpAngle;
		float m_LastAngle;
	};

}

#endif
