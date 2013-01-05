/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionEditorShapeTool
#define H_FusionEditorShapeTool

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionVectorTypes.h"

#include <ClanLib/display.h>

namespace FusionEngine
{
	
	//! Shape tool interface
	class ShapeTool
	{
	public:
		virtual ~ShapeTool()
		{}

		//! Fin.
		virtual void Finish() = 0;
		//! Tool should reset to initial state
		virtual void Reset() = 0;
		//! Tool should reset and finish
		virtual void Cancel() = 0;
		
		//! Should return true if the tool is expecting input
		virtual bool IsActive() const = 0;

		enum MouseInput { None, LeftButton, ScrollUp, ScrollDown };

		//! Should handle key press / release inputs
		virtual void KeyChange(bool shift, bool ctrl, bool alt) = 0;
		//! Should handle mouse move inputs
		virtual void MouseMove(const Vector2& pos, bool shift, bool ctrl, bool alt) = 0;
		//! Should handle mouse press inputs
		virtual bool MousePress(const Vector2& pos, MouseInput input, bool shift, bool ctrl, bool alt) = 0;
		//! Should handle mouse release inputs
		virtual bool MouseRelease(const Vector2& pos, MouseInput input, bool shift, bool ctrl, bool alt) = 0;

		//! Should draw something useful
		virtual void Draw(clan::Canvas& gc) = 0;

	};

}

#endif
