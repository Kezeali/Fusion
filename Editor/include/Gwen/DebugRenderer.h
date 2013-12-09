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
#ifndef H_Fusion_Editor_Gwen_DebugRenderer
#define H_Fusion_Editor_Gwen_DebugRenderer

#include <Gwen/Gwen.h>
#include <Gwen/BaseRender.h>
#include <ClanLib/display.h>
#include "Visual/FusionDebugDraw.h"

namespace Gwen
{
	namespace Renderer
	{

		class DebugRenderer : public ::Gwen::Renderer::Base
		{
		public:

			DebugRenderer(const clan::Canvas& canvas, const clan::FileSystem& filesystem);
			virtual ~DebugRenderer();

			virtual void Begin();
			virtual void End();

			virtual void StartClip();
			virtual void EndClip();

			virtual void SetDrawColor( Gwen::Color color );
			virtual void DrawPixel( int x, int y );
			virtual void DrawLinedRect( Gwen::Rect rect );
			virtual void DrawFilledRect( Gwen::Rect rect );
			virtual void DrawShavedCornerRect( Gwen::Rect rect, bool bSlight = false );
			virtual void DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2 );

			virtual void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text );
			virtual Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text );
			virtual void LoadFont( Gwen::Font* font );
			virtual void FreeFont( Gwen::Font* font );

			virtual void LoadTexture( Gwen::Texture* pTexture );
			virtual void FreeTexture( Gwen::Texture* pTexture );
			virtual Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default );

		protected:
			clan::Canvas m_Canvas;
			FusionEngine::DebugDraw m_Target;
			clan::Color m_Color;
			//sf::RenderStates m_RenderStates;
			//clan::View m_OriginalView;
			int m_Height;
			clan::Font defaultFont;
			clan::FileSystem m_FileSystem;
		};
	}
}

#endif
