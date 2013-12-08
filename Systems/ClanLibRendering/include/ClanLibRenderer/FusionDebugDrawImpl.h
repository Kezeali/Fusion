/*
  Copyright (c) 2013 Fusion Project Team

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

#pragma once
#ifndef H_Fusion_ClanLibRenderer_DebugDrawImpl
#define H_Fusion_ClanLibRenderer_DebugDrawImpl

#include "Visual/FusionDebugDrawImpl.h"

#include "FusionRenderAction.h"

#include <ClanLib/display.h>
#include <tbb/concurrent_queue.h>

namespace FusionEngine
{
	namespace ClanLibRenderer
	{

		class DebugDrawImpl : public FusionEngine::DebugDrawImpl
		{
		public:
			virtual ~DebugDrawImpl();

			typedef std::vector<RenderActionFunctor> RenderActionFrame_t;

			virtual bool TryGetFrame(RenderActionFrame_t& out_frame);

		private:
			virtual void Flip() override;

			virtual void SetCliprect(const clan::Rect& rect) override;

			virtual void ResetCliprect() override;

			virtual void DrawRectangle(const clan::Rectf& rect, const clan::Color& color) override;

			virtual void DrawSolidRectangle(const clan::Rectf& rect, const clan::Color& color) override;

			virtual void DrawTexturedRectangle(clan::Image texture, clan::Rect source, clan::Rectf dest) override;

			virtual void DrawTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color) override;

			virtual void DrawSolidTriangle(Vector2 vert_a, Vector2 vert_b, Vector2 vert_c, const clan::Color& color) override;

			virtual void DrawPolygon(std::vector<Vector2> vertices, const clan::Color& color) override;

			virtual void DrawSolidPolygon(std::vector<Vector2> vertices, const clan::Color& color) override;

			virtual void DrawCircle(const Vector2& center, float radius, const clan::Color& color) override;

			virtual void DrawSolidCircle(const Vector2& center, float radius, const Vector2& axis, const clan::Color& color) override;

			virtual void DrawSegment(const Vector2& p1, const Vector2& p2, const clan::Color& color) override;

			virtual void DrawPoint(const Vector2& p, float size, const clan::Color& color) override;

			virtual void RenderText(clan::Font font, clan::Pointf pos, const std::string& text, const clan::Color& color) override;

		protected:
			virtual void AddAction(RenderActionFunctor action);

			typedef tbb::concurrent_queue<RenderActionFrame_t> RenderActionFrameQueue_t;

			RenderActionFrame_t m_IncommingRenderActions;
			RenderActionFrameQueue_t m_QueuedFrames;

		};

	}
}

#endif
