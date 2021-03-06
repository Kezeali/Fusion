/*
*  Copyright (c) 2009-2010 Fusion Project Team
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

#ifndef H_FusionEngine_Renderer
#define H_FusionEngine_Renderer

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Fusion
#include "FusionCamera.h"
#include "FusionViewport.h"

#include <ClanLib/display.h>

namespace FusionEngine
{

	/*!
	 * \brief
	 * Renders Entities
	 *
	 * \see
	 * Viewport | Renderable | EntityManager
	 */
	class Renderer
	{
	protected:
		//typedef std::set<std::string> BlockedTagSet;

		//typedef std::set<EntityPtr> EntitySet;

	public:
		//! Constructor
		Renderer(const clan::Canvas &canvas);
		//! Destructor
		virtual ~Renderer();

		void CalculateScreenArea(clan::Rect &area, const ViewportPtr &viewport, bool apply_camera_offset = false);
		void CalculateScreenArea(clan::Rectf &area, const ViewportPtr &viewport, bool apply_camera_offset = false);
		static void CalculateScreenArea(const clan::GraphicContext& gc, clan::Rectf &area, const ViewportPtr &viewport, bool apply_camera_offset = false);

		//! Returns the GC object used by this renderer
		const clan::Canvas& GetCanvas() const;

		int GetContextWidth() const;
		int GetContextHeight() const;

		//! Sets up the GC to render within the given viewport
		void SetupDraw(const ViewportPtr& viewport, clan::Rectf* draw_area = nullptr);
		//! Resets the GC to as it was before SetupDraw was called, assuming SetupDraw has been called
		void PostDraw();

	protected:

		clan::Canvas m_Canvas;
	};

}

#endif
