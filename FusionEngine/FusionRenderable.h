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

#ifndef Header_FusionRenderable
#define Header_FusionRenderable

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionStreamedResourceUser.h"

#include "FusionResourcePointer.h"
#include "FusionVector2.h"

#include <ClanLib/Display/2D/sprite.h>


namespace FusionEngine
{

	typedef std::tr1::unordered_set<std::string> TagStringSet;

	//! A thing that can be drawn by the renderer
	class Renderable
	{
		void OnStreamOut();
	};

	class RenderableSprite : public Renderable, public StreamedResourceUser
	{
	public:
		Renderable();
		Renderable(ResourceManager *res_man, const std::string &sprite_path, int priority);
		virtual ~Renderable();

		void SetTags(const TagStringSet &tags);
		void AddTag(const std::string &tag);
		void RemoveTag(const std::string &tag);
		bool HasTag(const std::string &tag) const;

		void SetAlpha(float _alpha);
		float GetAlpha() const;

		void SetColour(unsigned int r, unsigned int g, unsigned int b);

		void SetColour(const CL_Color & colour);
		const CL_Color &GetColour() const;

		void SetPosition(float x, float y);
		void SetPosition(const Vector2 &position);
		void SetPosition(const CL_Vec2f &_position);
		const Vector2 &GetPosition() const;

		void SetAngle(float angle);
		float GetAngle() const;

		void SetDepth(int depth);
		int GetDepth() const;

		void SetEnabled(bool enabled);
		bool IsEnabled() const;

		const CL_Rectf &GetAABB() const;

		void UpdateAABB();

		//void SetSpriteResource(ResourceManager *res_man, const std::string &path);
		ResourcePointer<CL_Sprite> &GetSpriteResource();

		void OnResourceLoad(ResourceDataPtr resource);
		//void OnStreamIn();
		void OnStreamOut();

		void Draw(CL_GraphicContext &gc);
		//! Draw the renderable at a position other than that of it's owning Entity
		void Draw(CL_GraphicContext &gc, const Vector2 &origin);

		void StartAnimation();
		void StopAnimaion();
		void PauseAnimation();

		bool IsPaused() const;

		static void Register(asIScriptEngine *engine);

	protected:
		bool m_Enabled;
		bool m_Paused;

		Vector2 m_Position;
		float m_Angle;
		float m_Alpha;
		CL_Color m_Colour;

		bool m_PositionChanged;

		Vector2 m_DerivedPosition;
		float m_DerivedAngle;

		CL_Rectf m_AABB;

		int m_Depth;

		ResourcePointer<CL_Sprite> m_Sprite;

		int m_PreviousWidth, m_PreviousHeight;
		CL_Angle m_PreviousAngle;

		TagStringSet m_Tags;
	};

	typedef boost::intrusive_ptr<Renderable> RenderablePtr;
	typedef std::vector<RenderablePtr> RenderableArray;

}

#endif
