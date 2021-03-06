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

#ifndef H_FusionCamera
#define H_FusionCamera

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <boost/noncopyable.hpp>

namespace FusionEngine
{
	
	typedef std::shared_ptr<Camera> CameraPtr;

	//! Defines a 2D camera
	class Camera
	{
	public:
		Camera();
		Camera(float x, float y);
		~Camera();

		Vector2 GetSimPosition() const;
		const clan::Vec2f &GetPosition() const;
		float GetAngle() const;
		// TODO: rename this GetScale (perhaps add GetZoom method that returns 1/GetScale())
		//! Camera scale
		float GetZoom() const;

		void SetSimPosition(const Vector2& position);
		void SetPosition(float x, float y);
		void SetAngle(float angle);
		void SetZoom(float scale);

		static void Register(asIScriptEngine *engine);

	private:
		Camera( const Camera& );
		const Camera& operator=( const Camera& );

		clan::Vec2f m_Position;
		float m_Angle;
		float m_Scale;
	};

}

#endif
