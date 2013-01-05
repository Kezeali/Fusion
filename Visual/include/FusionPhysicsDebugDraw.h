/*
  Copyright (c) 2006 Fusion Project Team

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

#ifndef H_FusionEngine_Physics_DebugDraw
#define H_FusionEngine_Physics_DebugDraw
#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <Box2D/Box2D.h>

#include "FusionRenderer.h"

namespace FusionEngine
{

	class B2DebugDraw : public b2Draw
	{
	public:
		B2DebugDraw() : b2Draw() {}
		B2DebugDraw(clan::GraphicContext gc);

		void SetGraphicContext(clan::GraphicContext gc);
		void SetViewport(const ViewportPtr &viewport);

		void SetupView();
		void ResetView();
		
		void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

		void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

		void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);

		void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);

		void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);

		void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);

		void DrawTransform(const b2Transform& xf);

	protected:
		clan::GraphicContext m_gc;
		ViewportPtr m_Viewport;
	};

}

#endif