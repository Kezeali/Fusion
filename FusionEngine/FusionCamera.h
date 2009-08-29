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

#ifndef Header_FusionEngine_Camera
#define Header_FusionEngine_Camera

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Fusion
#include "FusionEntity.h"


namespace FusionEngine
{
	
	typedef boost::intrusive_ptr<Camera> CameraPtr;

	//! Defines a camera point
	/*!
	* \todo Interpolated rotation toward movement direction
	* \todo Interpolated smooth movement
	*/
	class Camera : public GarbageCollected<Camera>, noncopyable
	{
	public:
		Camera(asIScriptEngine *engine);
		Camera(asIScriptEngine *engine, float x, float y);
		Camera(asIScriptEngine *engine, EntityPtr follow);
		~Camera();

		void SetMass(float mass);

		void SetOrigin(CL_Origin origin);

		void SetPosition(float x, float y);

		void SetAngle(float angle);

		//void SetOrientation(const Quaternion &orientation);

		void SetZoom(float scale);

		void SetParallaxCamera(const CameraPtr &main_camera, float distance);

		void SetFollowEntity(const EntityPtr &follow);

		enum FollowMode
		{
			FixedPosition,
			FollowInstant,
			FollowSmooth,
			Physical
		};

		void SetFollowMode(FollowMode mode);

		enum RotateMode
		{
			FixedAngle,
			MatchEntity,
			SlerpToMovementDirection,
			Spline
		};

		void SetAutoRotate(RotateMode mode);

		void JoinToBody(b2Body *body);

		b2Body *CreateBody(b2World *world);
		b2Body *GetBody() const;

		void Update(float split);

		CL_Origin GetOrigin() const;

		const CL_Vec2f &GetPosition() const;

		float GetAngle() const;

		float GetZoom() const;

		void EnumReferences(asIScriptEngine *engine);
		void ReleaseAllReferences(asIScriptEngine *engine);

		static void Register(asIScriptEngine *engine);

	protected:
		void defineBody();
		void createBody(b2World *world);

		FollowMode m_Mode;
		RotateMode m_AutoRotate;

		CL_Origin m_Origin;
		CL_Vec2f m_Position;
		float m_Angle;
		float m_Scale;

		// Main camera for paralax effect (i.e. the
		//  camera that this one leads / follows)
		CameraPtr m_MainCamera;
		float m_ParallaxDistance;

		EntityPtr m_FollowEntity;

		b2BodyDef m_BodyDefinition;
		b2Body *m_Body;
		b2Joint *m_Joint;
	};

}

#endif
