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

#ifndef H_FusionEditorPolygonTool
#define H_FusionEditorPolygonTool

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionVectorTypes.h"
#include <functional>
#include <vector>

#include <ClanLib/display.h>

namespace FusionEngine
{

	typedef std::function<void (const std::vector<Vector2>&)> PolygonToolCallback_t;
	typedef std::function<void (const std::vector<Vector2>&, const PolygonToolCallback_t&)> PolygonToolExecutor_t;

	class EditorPolygonTool
	{
	public:
		EditorPolygonTool();
		
		enum Mode { Freeform, Convex, Line };

		void Start(const std::vector<Vector2>& verts, const PolygonToolCallback_t& done_callback, Mode mode);
		void Finish();

		bool IsActive() const { return m_Active; }

		void MouseMove(const Vector2& pos, int key, bool shift, bool ctrl, bool alt);
		void MousePress(const Vector2& pos, int key, bool shift, bool ctrl, bool alt);

		void Draw(CL_GraphicContext& gc);

	private:
		Mode m_Mode;
		std::vector<Vector2> m_Verts;
		PolygonToolCallback_t m_DoneCallback;

		bool m_Active;

		Vector2 m_FeedbackPoint;

		size_t m_GrabbedPoint;

		void AddFreeformPoint(const Vector2& pos, bool to_nearest_edge);
		void RemoveNearestPoint(const Vector2& pos);
		void GrabNearestPoint(const Vector2& pos);

		void UpdateFeedbackPoint(const Vector2& pos, bool to_nearest_edge);
	};

}

#endif