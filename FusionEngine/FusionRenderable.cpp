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

#include "FusionStableHeaders.h"

#include "FusionRenderable.h"

#include "FusionResourceManager.h"

namespace FusionEngine
{

	Renderable::Renderable()
		: m_Enabled(true),
		m_Depth(0),
		m_Angle(0.f),
		m_Colour(255, 255, 255, 255),
		m_Alpha(1.f)
	{
	}

	const CL_Rectf &Renderable::GetAABB() const
	{
		return m_AABB;
	}

	void Renderable::SetAlpha(float _alpha)
	{
		m_Alpha = _alpha;
	}

	float Renderable::GetAlpha() const
	{
		return m_Alpha;
	}

	void Renderable::SetColour(unsigned int r, unsigned int g, unsigned int b)
	{
		m_Colour.set_color(r, g, b);
	}

	void Renderable::SetColour(const CL_Color &colour)
	{
		m_Colour = colour;
	}

	const CL_Color &Renderable::GetColour() const
	{
		return m_Colour;
	}

	void Renderable::SetOrigin(CL_Origin origin)
	{
		m_Origin = origin;
	}

	CL_Origin Renderable::GetOrigin() const
	{
		return m_Origin;
	}

	void Renderable::SetOffset(float x, float y)
	{
		m_Offset.x = x;
		m_Offset.y = y;
	}

	void Renderable::SetOffset(const Vector2 &position)
	{
		m_Offset = position;
	}

	void Renderable::SetOffset(const CL_Vec2f &_position)
	{
		m_Offset.x = _position.x;
		m_Offset.y = _position.y;
	}

	const Vector2 &Renderable::GetOffset() const
	{
		return m_Offset;
	}

	void Renderable::SetAngle(float angle)
	{
		m_Angle = angle;
	}

	float Renderable::GetAngle() const
	{
		return m_Angle;
	}

	void Renderable::SetTags(const TagStringSet &tags)
	{
		m_Tags = tags;
	}

	void Renderable::AddTag(const std::string &tag)
	{
		m_Tags.insert(tag);
	}

	void Renderable::RemoveTag(const std::string &tag)
	{
		m_Tags.erase(tag);
	}

	bool Renderable::HasTag(const std::string &tag) const
	{
		return m_Tags.find(tag) != m_Tags.end();
	}

	void Renderable::SetDepth(int depth)
	{
		m_Depth = depth;
	}

	int Renderable::GetDepth() const
	{
		return m_Depth;
	}

	void Renderable::SetEnabled(bool enabled)
	{
		m_Enabled = enabled;
	}

	bool Renderable::IsEnabled() const
	{
		return m_Enabled;
	}

	unsigned int Renderable_GetRed(Renderable *obj)
	{
		return obj->GetColour().get_red();
	}
	unsigned int Renderable_GetGreen(Renderable *obj)
	{
		return obj->GetColour().get_green();
	}
	unsigned int Renderable_GetBlue(Renderable *obj)
	{
		return obj->GetColour().get_blue();
	}

	void Renderable::Register(asIScriptEngine *engine)
	{
		int r;

		RefCounted::RegisterType<Renderable>(engine, "Renderable");

		r = engine->RegisterObjectMethod("Renderable",
			"bool hasTag(const string &in) const",
			asMETHOD(Renderable, HasTag), asCALL_THISCALL);

		// Alpha
		r = engine->RegisterObjectMethod("Renderable",
			"void setAlpha(float)",
			asMETHOD(Renderable, SetAlpha), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"float getAlpha() const",
			asMETHOD(Renderable, GetAlpha), asCALL_THISCALL);
		// RGB
		r = engine->RegisterObjectMethod("Renderable",
			"void setColour(uint, uint, uint)",
			asMETHODPR(Renderable, SetColour, (unsigned int, unsigned int, unsigned int), void), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"uint getRed()",
			asFUNCTION(Renderable_GetRed), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("Renderable",
			"uint getGreen()",
			asFUNCTION(Renderable_GetGreen), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("Renderable",
			"uint getBlue()",
			asFUNCTION(Renderable_GetBlue), asCALL_CDECL_OBJLAST);

		r = engine->RegisterObjectMethod("Renderable",
			"void setOrigin(PointOrigin)",
			asMETHOD(Renderable, SetOrigin), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Renderable",
			"PointOrigin getOrigin() const",
			asMETHOD(Renderable, GetOrigin), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Renderable",
			"void setPosition(float, float)",
			asMETHODPR(Renderable, SetOffset, (float, float), void), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"void setPosition(const Vector &in)",
			asMETHODPR(Renderable, SetOffset, (const Vector2&), void), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Renderable",
			"const Vector &getPosition() const",
			asMETHOD(Renderable, GetOffset), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Renderable",
			"void setAngle(float)",
			asMETHOD(Renderable, SetAngle), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"float getAngle() const",
			asMETHOD(Renderable, GetAngle), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Renderable",
			"void setDepth(int)",
			asMETHOD(Renderable, SetDepth), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"int getDepth() const",
			asMETHOD(Renderable, GetDepth), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Renderable",
			"void setEnabled(bool)",
			asMETHOD(Renderable, SetEnabled), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"bool isEnabled() const",
			asMETHOD(Renderable, IsEnabled), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Renderable",
			"void startAnim(bool)",
			asMETHOD(Renderable, StartAnimation), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"bool stopAnim() const",
			asMETHOD(Renderable, StopAnimaion), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"bool pauseAnim() const",
			asMETHOD(Renderable, PauseAnimation), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"bool isPaused() const",
			asMETHOD(Renderable, IsPaused), asCALL_THISCALL);
	}


	RenderableSprite::RenderableSprite()
		: m_PreviousWidth(0),
		m_PreviousHeight(0),
		m_PositionChanged(false),
		m_ModifiedAlpha(false),
		m_ModifiedAngle(false),
		m_ModifiedColour(false),
		m_ModifiedOffset(false),
		m_ModifiedOrigin(false)
	{
	}

	RenderableSprite::RenderableSprite(ResourceManager *res_man, const std::string &sprite_path, int priority)
		: StreamedResourceUser(res_man, "SPRITE", sprite_path, priority),
		m_PreviousWidth(0),
		m_PreviousHeight(0),
		m_PositionChanged(false),
		m_ModifiedAlpha(false),
		m_ModifiedAngle(false),
		m_ModifiedColour(false),
		m_ModifiedOffset(false),
		m_ModifiedOrigin(false)
	{
	}

	RenderableSprite::~RenderableSprite()
	{
	}

	void RenderableSprite::UpdateAABB()
	{
		if (m_Sprite.IsLoaded())
		{
			bool bbChanged = false;

			// Check whether AABB needs to be upadated (frame width / height has changed)
			if (m_Sprite->get_height() != m_PreviousHeight)
			{
				bbChanged = true;
				m_PreviousHeight = m_Sprite->get_height();
			}
			if (m_Sprite->get_width() != m_PreviousWidth)
			{
				bbChanged = true;
				m_PreviousWidth = m_Sprite->get_width();
			}
			if (fe_fequal(m_Sprite->get_angle().to_radians(), m_PreviousAngle.to_radians(), 0.001f))
			{
				bbChanged = true;
				m_PreviousAngle = m_Sprite->get_angle();
			}

			if (bbChanged || m_PositionChanged)
			{
				CL_Rectf bb;
				bb.left = m_Offset.x;
				bb.top = m_Offset.y;
				bb.right = m_Offset.x + m_Sprite->get_width();
				bb.bottom = m_Offset.y + m_Sprite->get_height();

				CL_Origin origin;
				int x, y;
				m_Sprite->get_alignment(origin, x, y);
				bb.translate(-CL_Vec2f::calc_origin(origin, bb.get_size()));

				m_Sprite->get_rotation_hotspot(origin, x, y);
				m_AABB = bb.get_rot_bounds(origin, m_Offset.x + x, m_Offset.y + y, m_Sprite->get_angle());

				m_PositionChanged = false;
			}
		}
	}

	void RenderableSprite::SetAlpha(float _alpha)
	{
		if (m_Sprite.IsLoaded())
			m_Sprite->set_alpha(_alpha);
		m_Alpha = _alpha;
		m_ModifiedAlpha = true;
	}

	float RenderableSprite::GetAlpha() const
	{
		return m_Alpha;
	}

	void RenderableSprite::SetColour(unsigned int r, unsigned int g, unsigned int b)
	{
		m_Colour.set_color(r, g, b);
		m_ModifiedColour = true;

		if (m_Sprite.IsLoaded())
			m_Sprite->set_color(m_Colour);
	}

	void RenderableSprite::SetColour(const CL_Color &colour)
	{
		m_Colour = colour;
		m_ModifiedColour = true;

		if (m_Sprite.IsLoaded())
			m_Sprite->set_color(m_Colour);
	}

	const CL_Color &RenderableSprite::GetColour() const
	{
		return m_Colour;
	}

	void RenderableSprite::SetOrigin(CL_Origin origin)
	{
		if (m_Sprite.IsLoaded())
		{
			m_Sprite->set_alignment(origin);
			m_Sprite->set_rotation_hotspot(origin);
		}

		m_ModifiedOrigin = true;
		m_Origin = origin;
	}

	CL_Origin RenderableSprite::GetOrigin() const
	{
		return m_Origin;
	}

	void RenderableSprite::SetOffset(float x, float y)
	{
		m_Offset.x = x;
		m_Offset.y = y;
		m_ModifiedOffset = true;

		// Make sure the AABB is updated
		m_PositionChanged = true;
	}

	void RenderableSprite::SetOffset(const Vector2 &position)
	{
		SetOffset(position.x, position.y);
	}

	void RenderableSprite::SetOffset(const CL_Vec2f &position)
	{
		SetOffset(position.x, position.y);
	}

	const Vector2 &RenderableSprite::GetOffset() const
	{
		return m_Offset;
	}

	void RenderableSprite::SetAngle(float angle)
	{
		if (m_Sprite.IsLoaded())
		{
			m_Sprite->set_angle(CL_Angle(angle, cl_radians));

			if (!fe_fequal(angle, m_Angle))
			{
				CL_Origin origin;
				int x, y;
				m_Sprite->get_rotation_hotspot(origin, x, y);

				m_AABB = m_AABB.get_rot_bounds(origin, m_Offset.x + x, m_Offset.y + y, CL_Angle(angle, cl_radians));
			}
		}

		m_Angle = angle;
		m_ModifiedAngle = true;
	}

	float RenderableSprite::GetAngle() const
	{
		return m_Angle;
	}

	ResourcePointer<CL_Sprite> &RenderableSprite::GetSpriteResource()
	{
		return m_Sprite;
	}

	void RenderableSprite::OnResourceLoad(ResourceDataPtr resource)
	{
		m_Sprite.SetTarget(resource);
		if (m_ModifiedAlpha)
			m_Sprite->set_alpha(m_Alpha);
		else
			m_Alpha = m_Sprite->get_alpha();
		if (m_ModifiedColour)
			m_Sprite->set_color(m_Colour);
		else
			m_Colour = m_Sprite->get_color();

		if (m_ModifiedAngle)
			m_Sprite->set_angle(CL_Angle(m_Angle, cl_radians));
		else
			m_Angle = m_Sprite->get_angle().to_radians();

		CL_Origin alignment_origin, rotation_point;
		int ax, ay, rx, ry;
		m_Sprite->get_alignment(alignment_origin, ax, ay);
		m_Sprite->get_rotation_hotspot(rotation_point, rx, ry);
		// Set the origin
		if (m_ModifiedOrigin)
		{
			m_Sprite->set_alignment(m_Origin, ax, ay);
			m_Sprite->set_rotation_hotspot(m_Origin, rx, ry);
		}
		else
			m_Origin = alignment_origin;

		UpdateAABB();
	}

	void RenderableSprite::OnStreamOut()
	{
		m_Sprite.Release();
	}

	void RenderableSprite::Draw(CL_GraphicContext &gc)
	{
		if (m_Enabled && m_Sprite.IsLoaded())
		{
			m_Sprite->draw(gc, m_Offset.x, m_Offset.y);
		}
	}

	void RenderableSprite::Draw(CL_GraphicContext &gc, const Vector2 &origin)
	{
		if (m_Enabled && m_Sprite.IsLoaded())
		{
			m_Sprite->draw(gc, m_Offset.x + origin.x, m_Offset.y + origin.y);
		}
	}

	void RenderableSprite::StartAnimation()
	{
		if (m_Sprite.IsLoaded())
		{
			m_Sprite->restart();
		}
	}

	void RenderableSprite::StopAnimaion()
	{
		if (m_Sprite.IsLoaded())
		{
			m_Sprite->finish();
		}
	}

	void RenderableSprite::PauseAnimation()
	{
		m_Paused = true;
	}

	bool RenderableSprite::IsPaused() const
	{
		return m_Paused;
	}

	RenderableImage::RenderableImage()
	{
	}

	RenderableImage::RenderableImage(const CL_Image &image)
		: m_Image(image)
	{
	}

	RenderableImage::~RenderableImage()
	{
	}

	void RenderableImage::Draw(CL_GraphicContext &gc, const Vector2 &origin)
	{
		gc.push_modelview();
		gc.mult_rotate(CL_Angle(m_Angle, cl_radians));
		m_Image.draw(gc, m_Offset.x + origin.x, m_Offset.y + origin.y);
		gc.pop_modelview();
	}

	void RenderableImage::UpdateAABB()
	{
		bool bbChanged = false;

		// Check whether AABB needs to be upadated (frame width / height has changed)
		if (m_Image.get_height() != m_PreviousHeight)
		{
			bbChanged = true;
			m_PreviousHeight = m_Image.get_height();
		}
		if (m_Image.get_width() != m_PreviousWidth)
		{
			bbChanged = true;
			m_PreviousWidth = m_Image.get_width();
		}
		if (fe_fequal(m_Angle, m_PreviousAngle.to_radians(), 0.001f))
		{
			bbChanged = true;
			m_PreviousAngle = CL_Angle(m_Angle, cl_radians);
		}

		if (bbChanged || m_PositionChanged)
		{
			CL_Rectf bb;
			bb.left = m_Offset.x;
			bb.top = m_Offset.y;
			bb.right = m_Offset.x + m_Image.get_width();
			bb.bottom = m_Offset.y + m_Image.get_height();

			CL_Origin origin;
			int x, y;
			m_Image.get_alignment(origin, x, y);

			bb.translate(-CL_Vec2f::calc_origin(origin, bb.get_size()));

			m_AABB = bb.get_rot_bounds(origin, m_Offset.x, m_Offset.y, CL_Angle(m_Angle, cl_radians));

			m_PositionChanged = false;
		}
	}

	void RenderableImage::SetAlpha(float _alpha)
	{
		m_Image.set_alpha(_alpha);
		//m_Alpha = _alpha;
	}

	float RenderableImage::GetAlpha() const
	{
		return m_Alpha;
	}

	void RenderableImage::SetColour(unsigned int r, unsigned int g, unsigned int b)
	{
		//m_Colour.set_color(r, g, b);

		m_Image.set_color(m_Colour);
	}

	void RenderableImage::SetColour(const CL_Color &colour)
	{
		m_Colour = colour;

		m_Image.set_color(m_Colour);
	}

	const CL_Color &RenderableImage::GetColour() const
	{
		return m_Colour;
	}

	void RenderableImage::SetOrigin(CL_Origin origin)
	{
		//m_Origin = origin;
		m_Image.set_alignment(origin);
	}

	CL_Origin RenderableImage::GetOrigin() const
	{
		CL_Origin origin; int x, y;
		m_Image.get_alignment(origin, x, y);
		return origin;
	}

	void RenderableImage::SetOffset(float x, float y)
	{
		m_Offset.x = x;
		m_Offset.y = y;

		m_PositionChanged = true;
	}

	void RenderableImage::SetOffset(const Vector2 &position)
	{
		m_Offset = position;

		m_PositionChanged = true;
	}

	void RenderableImage::SetOffset(const CL_Vec2f &_position)
	{
		m_Offset.x = _position.x;
		m_Offset.y = _position.y;

		m_PositionChanged = true;
	}

	const Vector2 &RenderableImage::GetOffset() const
	{
		return m_Offset;
	}

	void RenderableImage::SetAngle(float angle)
	{
		if (!fe_fequal(angle, m_Angle))
		{
			CL_Origin origin;
			int x, y;
			m_Image.get_alignment(origin, x, y);

			m_AABB = m_AABB.get_rot_bounds(origin, (float)x, (float)y, CL_Angle(m_Angle-angle, cl_radians));
		}

		m_Angle = angle;
	}

	float RenderableImage::GetAngle() const
	{
		return m_Angle;
	}

}
