#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "Gwen/Renderers/ClanLib.h"
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <GL/gl.h>
#include <cmath>

namespace Gwen
{
	namespace Renderer
	{
		struct TextureData
		{
			TextureData( clan::Image* img ) : image( img ), texture( NULL ) { }
			TextureData( clan::Texture* text ) : texture( text ), image( NULL ) { }
			~TextureData() { if ( texture != NULL ) { delete texture; } if ( image != NULL ) { delete image; } }

			clan::Texture* texture;
			clan::Image*   image;
		};


		ClanLib::ClanLib(const clan::Canvas& target, const clan::FileSystem& fileSystem) : m_Target(target), m_FileSystem(fileSystem)
		{
		}

		ClanLib::~ClanLib()
		{
		}

		void ClanLib::Begin()
		{
			//m_OriginalView = m_Target.getView();
			//sf::FloatRect vrect;
			//vrect.left = 0;	vrect.top = 0;
			//vrect.width = m_Target.getSize().x;	vrect.height = m_Height = m_Target.getSize().y;
			//sf::FloatRect vprect;
			//vprect.left = 0; vprect.top = 0;
			//vprect.width = 1.0f; vprect.height = 1.0f;
			//sf::View view( vrect );
			//view.setViewport( vprect );
			//m_Target.setView( view );
		}

		void ClanLib::End()
		{
			//m_Target.setView( m_OriginalView );
		}

		void ClanLib::StartClip()
		{
			Gwen::Rect rect = ClipRegion();

			m_Target.set_cliprect(clan::Rect(rect.x * Scale(), rect.y * Scale(), rect.w * Scale(), rect.h * Scale()));
		};


		void ClanLib::EndClip()
		{
			m_Target.reset_cliprect();
		};

		void ClanLib::SetDrawColor( Gwen::Color color )
		{
			m_Color.r = color.r;
			m_Color.g = color.g;
			m_Color.b = color.b;
			m_Color.a = color.a;
		}

		void ClanLib::DrawFilledRect( Gwen::Rect rect )
		{
			Translate( rect );
			m_Target.fill_rect(clan::Rectf(clan::Pointf(rect.x, rect.y), clan::Sizef(rect.w, rect.h)), clan::Colorf(m_Color));
		}

		void ClanLib::DrawShavedCornerRect( Gwen::Rect rect, bool bSlight )
		{
			//TODO: Implement this
			Base::DrawShavedCornerRect( rect, bSlight );
		}

		void ClanLib::DrawLinedRect( Gwen::Rect rect )
		{
			Base::DrawLinedRect( rect );
		}

		void ClanLib::DrawPixel( int x, int y )
		{
			Base::DrawPixel( x, y );
		}

		void ClanLib::LoadFont( Gwen::Font* font )
		{
			font->realsize = font->size * Scale();
			clan::Font* pFont = new clan::Font(m_Target, Utility::UnicodeToString(font->facename), font->realsize);
			
			if (!pFont->is_null())
			{
				// Ideally here we should be setting the font to a system default font here.
				delete pFont;

				//static clan::Font defaultFont = clan::Font();
				//pFont = &defaultFont;
				pFont = NULL;
			}

			font->data = pFont;
		}

		void ClanLib::FreeFont( Gwen::Font* pFont )
		{
			if ( !pFont->data ) return;
			clan::Font* font = reinterpret_cast<clan::Font*>( pFont->data );
			delete font;
			pFont->data = NULL;
		}

		void ClanLib::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString & text )
		{
			Translate( pos.x, pos.y );

			// If the font doesn't exist, or the font size should be changed
			if ( !pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
			{
				FreeFont( pFont );
				LoadFont( pFont );
			}

			const clan::Font* pCLFont = ( clan::Font* )( pFont->data );

			if ( !pCLFont )
			{
				//static clan::Font defaultFont = clan::Font();
				//pCLFont = &defaultFont;
				pCLFont = NULL;
			}

			pCLFont->draw_text(m_Target, clan::Pointf(pos.x, pos.y), Gwen::Utility::UnicodeToString(text), clan::Colorf(m_Color));
		}

		Gwen::Point ClanLib::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString & text )
		{
			// If the font doesn't exist, or the font size should be changed
			if ( !pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
			{
				FreeFont( pFont );
				LoadFont( pFont );
			}

			const clan::Font* pCLFont = ( clan::Font* )( pFont->data );

			if ( !pCLFont )
			{
				//static clan::Font defaultFont = clan::Font();
				//pCLFont = &defaultFont;
				pCLFont = NULL;
			}

			auto size = pCLFont->get_text_size(m_Target, Gwen::Utility::UnicodeToString(text));
			
			return Gwen::Point(size.width, size.height);			
		}

		void ClanLib::LoadTexture( Gwen::Texture* pTexture )
		{
			if ( !pTexture ) { return; }
			if ( pTexture->data ) { FreeTexture( pTexture ); }

			clan::Image* tex = new clan::Image(m_Target, pTexture->name.Get(), m_FileSystem);
			//tex->SetSmooth( true );

			//if ( !tex->LoadFromFile( pTexture->name.Get() ) )
			if (tex->is_null())
			{
				delete( tex );
				pTexture->failed = true;
				return;
			}
			
			pTexture->height = tex->get_height();
			pTexture->width = tex->get_width();
			pTexture->data = new TextureData( tex );
			
		};

		void ClanLib::FreeTexture( Gwen::Texture* pTexture )
		{
			TextureData* data = static_cast<TextureData*>( pTexture->data );

			if ( data )
			{
				delete data;
			}

			pTexture->data = NULL;
		}

		void ClanLib::DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2 )
		{
			TextureData* data = static_cast<TextureData*>( pTexture->data );

			if ( !data )
			{ return DrawMissingImage( rect ); }

			const clan::Image* tex = data->image;

			if ( !tex )
			{ return DrawMissingImage( rect ); }

			Translate( rect );
			
			tex->draw(m_Target, clan::Rectf(u1, v1, u2, v2), clan::Rect(rect.x, rect.y, rect.w, rect.h));
		}

		Gwen::Color ClanLib::PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color & col_default )
		{
			TextureData* data = static_cast<TextureData*>( pTexture->data );

			const clan::Image* tex = data->image;
			if ( !tex ) return col_default;

			auto pixelData = tex->get_texture().get_texture().get_pixeldata(m_Target.get_gc());

			clan::Color col = pixelData.get_pixel( x, y );
			return Gwen::Color( col.r, col.g, col.b, col.a );
		}

	}
}
