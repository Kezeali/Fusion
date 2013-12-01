/*
*  Copyright (c) 2009-2013 Fusion Project Team
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

#ifndef H_FusionEngine_Viewport
#define H_FusionEngine_Viewport

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

class asIScriptEngine;

#include <ClanLib/core.h>
#include <memory>

#include "FusionCamera.h"

namespace FusionEngine
{

	//! A render area
	/*!
	* \see Camera | Renderer
	*/
	class Viewport
	{
	public:
		Viewport();
		//! Sets the area of the viewport
		Viewport(const clan::Rectf &area_ratio);
		//! Sets the area of the viewport, and the camera
		Viewport(const clan::Rectf &area_ratio, const CameraPtr &camera);

		//! Sets the area of the viewport (relative to the graphics context)
		/*!
		* \see SetArea(float, float, float, float)
		*/
		void SetArea(const clan::Rectf &area);
		//! Sets the area of the viewport (relative to the graphics context)
		/*!
		* All parameters should be [0, 1) - these values will be multiplied
		* by the relavant screen dimension to find the area that the viewport
		* fills upon rendering.
		*/
		void SetArea(float left, float top, float right, float bottom);
		//! Sets the position within the graphics context
		void SetPosition(float left, float top);
		//! Sets the size of the render area
		void SetSize(float width, float height);

		const clan::Rectf &GetArea() const;
		clan::Pointf GetPosition() const;
		clan::Sizef GetSize() const;

		void SetCamera(const CameraPtr &camera);
		const CameraPtr &GetCamera() const;

		void CalculateScreenArea(const clan::GraphicContext& gc, clan::Rect &area, bool apply_camera_offset = false);
		void CalculateScreenArea(const clan::GraphicContext& gc, clan::Rectf &area, bool apply_camera_offset = false);

		static void Register(asIScriptEngine *engine);

	protected:
		clan::Rectf m_Area;
		CameraPtr m_Camera;
	};

	typedef std::shared_ptr<Viewport> ViewportPtr;

}

#endif
