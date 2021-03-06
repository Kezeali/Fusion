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

#ifndef H_FusionEditorCircleTool
#define H_FusionEditorCircleTool

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionVectorTypes.h"
#include <array>
#include <boost/intrusive_ptr.hpp>
#include <functional>
#include <set>
#include <vector>

#include <ClanLib/display.h>

#include "FusionEditorShapeTool.h"

#include "Visual/FusionDebugDraw.h"

namespace Rocket { namespace Core {
	class ElementDocument;
} }

namespace FusionEngine
{

	typedef std::function<void (const Vector2&, float)> CircleToolCallback_t;
	typedef std::function<void (const Vector2&, float, const CircleToolCallback_t&)> CircleToolExecutor_t;

	//! Circle tool (creates circles, is a tool)
	class EditorCircleTool : public ShapeTool
	{
	public:
		EditorCircleTool();

		void Start(const Vector2& c, float r, const CircleToolCallback_t& done_callback);
		void Finish();
		void Reset();
		void Cancel();

		bool IsActive() const { return m_Active; }

		void KeyChange(bool shift, bool ctrl, bool alt);
		void MouseMove(const Vector2& pos, bool shift, bool ctrl, bool alt);
		bool MousePress(const Vector2& pos, MouseInput key, bool shift, bool ctrl, bool alt);
		bool MouseRelease(const Vector2& pos, MouseInput key, bool shift, bool ctrl, bool alt);

		void Draw() override;

	private:
		Vector2 m_Center;
		float m_Radius;
		CircleToolCallback_t m_DoneCallback;

		Vector2 m_InitialCenter;
		float m_InitialRadius;

		bool m_Active;

		Vector2 m_FeedbackPoint;

		Vector2 m_FeedbackCenter;
		float m_FeedbackRadius;

		Vector2 m_LastMousePos;

		enum Action { None, Resize, ResizeRelative, Move } m_Action;
		bool m_MouseDown;
		Vector2 m_DragFrom;

		DebugDraw m_DebugDraw;

		void CreateGui();

	};

}

#endif