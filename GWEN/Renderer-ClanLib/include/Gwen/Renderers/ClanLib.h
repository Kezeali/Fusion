#pragma once
#ifndef GWEN_RENDERERS_CLANLIB_H
#define GWEN_RENDERERS_CLANLIB_H

#include <Gwen/Gwen.h>
#include <Gwen/BaseRender.h>
#include <ClanLib/display.h>

namespace Gwen
{
	namespace Renderer
	{

		class ClanLib : public Gwen::Renderer::Base
		{
		public:

			ClanLib(const clan::Canvas& target, const clan::FileSystem& filesystem);
			virtual ~ClanLib();

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
			virtual void FreeFont( Gwen::Font* pFont );

			virtual void LoadTexture( Gwen::Texture* pTexture );
			virtual void FreeTexture( Gwen::Texture* pTexture );
			virtual Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default );

		protected:
			clan::Canvas m_Target;
			clan::Color m_Color;
			//sf::RenderStates m_RenderStates;
			//clan::View m_OriginalView;
			int m_Height;
			clan::FileSystem m_FileSystem;
		};
	}
}
#endif
