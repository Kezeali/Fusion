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
#include "FusionSpriteDefinition.h"
#include "FusionVector2.h"

#include <ClanLib/Display/2D/image.h>
#include <ClanLib/Display/2D/sprite.h>


namespace FusionEngine
{

	typedef std::tr1::unordered_set<std::string> TagStringSet;

	//! A thing that can be drawn by the renderer
	class Renderable : public RefCounted, noncopyable
	{
	public:
		//! CTOR
		Renderable();
		//! Virtual DTOR
		virtual ~Renderable() {};

		//! Implementation should draw something to the given GC
		virtual void Draw(CL_GraphicContext &gc, const Vector2 &origin) = 0;

		//! Returns the AABB
		virtual const CL_Rectf &GetAABB() const;
		//! Updates the AABB
		virtual void UpdateAABB() {};

		//! Sets the opacity of the renderable
		/*!
		* May be equivilant to setting a transparent colour. In that case it is a
		* convinience function.
		*/
		virtual void SetAlpha(float _alpha);
		//! Gets the opacity of the renderable
		virtual float GetAlpha() const;
		virtual void SetColour(unsigned int r, unsigned int g, unsigned int b);
		virtual void SetColour(const CL_Color & colour);
		virtual const CL_Color &GetColour() const;

		//! Sets the point that should be considered 0,0 on the object
		virtual void SetOrigin(CL_Origin origin);
		//! Gets the origin alignment
		virtual CL_Origin GetOrigin() const;

		//! Sets the offset from the Entity
		/*!
		* This will be the position relative to the Entity that
		* the Origin point of this renderable will be positioned
		* at for rendering.
		*
		* \see SetOrigin()
		*/
		virtual void SetOffset(float x, float y);
		//! Sets the offset
		virtual void SetOffset(const Vector2 &position);
		//! Sets the offset
		virtual void SetOffset(const CL_Vec2f &_position);
		//! Returns the offset from the Entity
		virtual const Vector2 &GetOffset() const;

		//! Sets the angle to render at
		virtual void SetAngle(float angle);
		//! Gets the angle
		virtual float GetAngle() const;

		//! Should start any animation
		virtual void StartAnimation() {};
		//! Should stop any animation
		virtual void StopAnimaion() {};
		//! Should pause any animation, maintaining the state for restarting
		virtual void PauseAnimation() {};
		//! Returns true if the animation is paused
		virtual bool IsPaused() const { return false; };

		//! Sets the tags that this renderable has
		void SetTags(const TagStringSet &tags);
		//! Adds a tag to this renderable
		/*!
		* Tags can be used to identify a renderable's purpose.
		*/
		void AddTag(const std::string &tag);
		//! Removes the given tag
		void RemoveTag(const std::string &tag);
		//! Returns true if the given tag is attached to this renderable
		bool HasTag(const std::string &tag) const;

		uint32_t RefreshFlags();
		uint32_t GetFlags() const;

		//! Sets the relative depth of this renderable
		/*!
		* Depth is relative to other renderables attached to the same Entity.
		*/
		void SetDepth(int depth);
		//! Gets the relative depth of this renderable
		int GetDepth() const;

		//! Enables this renderable
		/*!
		* Renderables that aren't enabled wont be rendered.
		*/
		void SetEnabled(bool enabled);
		//! Returns true if this renderable is enabled
		bool IsEnabled() const;

		//! Registers the script interface
		static void Register(asIScriptEngine *engine);

	protected:
		bool m_Enabled;
		int m_Depth;
		TagStringSet m_Tags;
		uint32_t m_FlagsCache;

		CL_Origin m_Origin;
		Vector2 m_Offset;
		float m_Angle;
		float m_Alpha;
		CL_Color m_Colour;

		CL_Rectf m_AABB;
	};

	//! Renderable Sprite resource
	/*!
	* This also implements StreamedResourceUser, so it the sprite can be automatically loaded
	* when needed.
	*
	* \todo Support seperate translation and rotation origins (at the moment, if SetOrigin is called it sets both).
	* Also, there is no support for setting the specific co-ords of the origin, only the alignment (center, left, etc.)
	*/
	class RenderableSprite : public Renderable, public StreamedResourceUser
	{
	public:
		RenderableSprite(CL_GraphicContext& gc);
		RenderableSprite(ResourceManager *res_man, const std::string &sprite_path, int priority);
		virtual ~RenderableSprite();

		//void SetSpriteResource(ResourceManager *res_man, const std::string &path);
		ResourcePointer<SpriteDefinition> &GetSpriteDefinition();
		CL_Sprite* GetSprite();

		void OnResourceLoad(ResourceDataPtr resource);
		//void OnStreamIn();
		void OnStreamOut();

		void Draw(CL_GraphicContext &gc);
		void Draw(CL_GraphicContext &gc, const Vector2 &origin);

		virtual void UpdateAABB();

		virtual void SetAlpha(float _alpha);
		virtual float GetAlpha() const;

		virtual void SetColour(unsigned int r, unsigned int g, unsigned int b);
		virtual void SetColour(const CL_Color & colour);
		virtual const CL_Color &GetColour() const;

		virtual void SetOrigin(CL_Origin origin);
		virtual CL_Origin GetOrigin() const;

		virtual void SetOffset(float x, float y);
		virtual void SetOffset(const Vector2 &position);
		virtual void SetOffset(const CL_Vec2f &_position);
		virtual const Vector2 &GetOffset() const;

		virtual void SetAngle(float angle);
		virtual float GetAngle() const;

		virtual void StartAnimation();
		virtual void StopAnimaion();
		virtual void PauseAnimation();
		virtual bool IsPaused() const;

	protected:
		bool m_Paused;

		// Indicates that the AABB needs to be updated
		bool m_PositionChanged;

		//Vector2 m_DerivedPosition;
		//float m_DerivedAngle;

		// These are used to indicate that the properties have been changed
		//  from the defaults in the sprite definition
		bool m_ModifiedAlpha;
		bool m_ModifiedColour;
		bool m_ModifiedOrigin;
		bool m_ModifiedOffset;
		bool m_ModifiedAngle;

		CL_GraphicContext m_GC;

		ResourcePointer<SpriteDefinition> m_SpriteDefinition;
		CL_Sprite m_Sprite;

		int m_PreviousWidth, m_PreviousHeight;
		CL_Angle m_PreviousAngle;
	};

	//! Renders a CL_Sprite object
	/*!
	* This class is for special cases where a sprite that isn't loaded as a Resource needs to be attached
	* to an Entity. For example, the selection overlays in the Editor.
	* \see RenderableSprite
	*/
	class RenderableGeneratedSprite : public Renderable
	{
	public:
		RenderableGeneratedSprite();
		RenderableGeneratedSprite(const CL_Sprite &sprite);
		virtual ~RenderableGeneratedSprite();

		void Draw(CL_GraphicContext &gc, const Vector2 &origin);

		virtual void UpdateAABB();

		virtual void SetAlpha(float _alpha);
		virtual float GetAlpha() const;

		virtual void SetColour(unsigned int r, unsigned int g, unsigned int b);
		virtual void SetColour(const CL_Color & colour);
		virtual const CL_Color &GetColour() const;

		virtual void SetOrigin(CL_Origin origin);
		virtual CL_Origin GetOrigin() const;

		virtual void SetOffset(float x, float y);
		virtual void SetOffset(const Vector2 &position);
		virtual void SetOffset(const CL_Vec2f &_position);
		virtual const Vector2 &GetOffset() const;

		virtual void SetAngle(float angle);
		virtual float GetAngle() const;

	protected:
		bool m_PositionChanged;

		CL_Sprite m_Sprite;

		int m_PreviousWidth, m_PreviousHeight;
		CL_Angle m_PreviousAngle;
	};

	//! Renders a CL_Image object
	class RenderableGeneratedImage : public Renderable
	{
	public:
		RenderableGeneratedImage();
		RenderableGeneratedImage(const CL_Image &image);
		virtual ~RenderableGeneratedImage();

		void Draw(CL_GraphicContext &gc, const Vector2 &origin);

		virtual void UpdateAABB();

		virtual void SetAlpha(float _alpha);
		virtual float GetAlpha() const;

		virtual void SetColour(unsigned int r, unsigned int g, unsigned int b);
		virtual void SetColour(const CL_Color & colour);
		virtual const CL_Color &GetColour() const;

		virtual void SetOrigin(CL_Origin origin);
		virtual CL_Origin GetOrigin() const;

		virtual void SetOffset(float x, float y);
		virtual void SetOffset(const Vector2 &position);
		virtual void SetOffset(const CL_Vec2f &_position);
		virtual const Vector2 &GetOffset() const;

		virtual void SetAngle(float angle);
		virtual float GetAngle() const;

	protected:
		bool m_PositionChanged;

		Vector2 m_DerivedPosition;
		float m_DerivedAngle;

		CL_Image m_Image;

		int m_PreviousWidth, m_PreviousHeight;
		CL_Angle m_PreviousAngle;
	};

	typedef boost::intrusive_ptr<Renderable> RenderablePtr;
	typedef boost::intrusive_ptr<RenderableSprite> RenderableSpritePtr;
	typedef std::vector<RenderablePtr> RenderableArray;

}

#endif
