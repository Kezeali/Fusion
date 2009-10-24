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

#ifndef Header_FusionEngine_Viewport
#define Header_FusionEngine_Viewport

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Fusion
#include "FusionCamera.h"


namespace FusionEngine
{

	//! A render area
	/*!
	* \see Camera | Renderer
	*/
	class Viewport : public RefCounted
	{
	public:
		Viewport();
		//! Sets the area of the viewport
		Viewport(const CL_Rectf &area_ratio);
		//! Sets the area of the viewport, and the camera
		Viewport(const CL_Rectf &area_ratio, const CameraPtr &camera);

		//! Sets the position within the graphics context
		void SetPosition(float left, float top);
		//! Sets the size of the render area
		void SetSize(float width, float height);

		const CL_Rectf &GetArea() const;
		CL_Pointf GetPosition() const;
		CL_Sizef GetSize() const;

		void SetCamera(const CameraPtr &camera);
		const CameraPtr &GetCamera() const;

		Vector2* ToScreenCoords(const Vector2 &entity_position) const;
		Vector2* ToEntityCoords(const Vector2 &screen_position) const;

		static void Register(asIScriptEngine *engine);

	protected:
		CL_Rectf m_Area;
		CameraPtr m_Camera;
	};

	typedef boost::intrusive_ptr<Viewport> ViewportPtr;

}

#endif
