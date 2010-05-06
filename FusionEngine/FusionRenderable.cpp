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
		: m_Angle(0.f),
		m_DerivedAngle(0.f),
		m_Colour(255, 255, 255, 255),
		m_Alpha(1.f),
		m_Depth(0),
		m_PreviousWidth(0),
		m_PreviousHeight(0),
		m_PositionChanged(false)
	{
	}

	Renderable::Renderable(ResourceManager *res_man, const std::string &sprite_path, int priority)
		: StreamedResourceUser(res_man, "SPRITE", sprite_path, priority),
		m_Angle(0.f),
		m_DerivedAngle(0.f),
		m_Colour(255, 255, 255, 255),
		m_Alpha(1.f),
		m_Depth(0),
		m_PreviousWidth(0),
		m_PreviousHeight(0),
		m_PositionChanged(false)
	{
	}

	Renderable::~Renderable()
	{
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

	void Renderable::SetAlpha(float _alpha)
	{
		if (m_Sprite.IsLoaded())
			m_Sprite->set_alpha(_alpha);
		m_Alpha = _alpha;
	}

	float Renderable::GetAlpha() const
	{
		return m_Alpha;
	}

	void Renderable::SetColour(unsigned int r, unsigned int g, unsigned int b)
	{
		m_Colour.set_color(r, g, b);

		if (m_Sprite.IsLoaded())
			m_Sprite->set_color(m_Colour);
	}

	void Renderable::SetColour(const CL_Color &colour)
	{
		m_Colour = colour;

		if (m_Sprite.IsLoaded())
			m_Sprite->set_color(m_Colour);
	}

	const CL_Color &Renderable::GetColour() const
	{
		return m_Colour;
	}

	void Renderable::SetPosition(float x, float y)
	{
		m_Position.x = x;
		m_Position.y = y;

		m_PositionChanged = true;
	}

	void Renderable::SetPosition(const Vector2 &position)
	{
		m_Position = position;

		m_PositionChanged = true;
	}

	void Renderable::SetPosition(const CL_Vec2f &_position)
	{
		m_Position.x = _position.x;
		m_Position.y = _position.y;

		m_PositionChanged = true;
	}

	const Vector2 &Renderable::GetPosition() const
	{
		return m_Position;
	}

	void Renderable::SetAngle(float angle)
	{
		if (m_Sprite.IsLoaded())
			m_Sprite->set_angle(CL_Angle(m_Angle, cl_radians));

		if (!fe_fequal(angle, m_Angle))
		{
			CL_Origin origin;
			int x, y;
			m_Sprite->get_alignment(origin, x, y);

			m_AABB = m_AABB.get_rot_bounds(origin, (float)x, (float)y, CL_Angle(m_Angle-angle, cl_radians));
		}

		m_Angle = angle;
	}

	float Renderable::GetAngle() const
	{
		return m_Angle;
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

	const CL_Rectf &Renderable::GetAABB() const
	{
		return m_AABB;
	}

	void Renderable::UpdateAABB()
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
				bb.left = m_Position.x;
				bb.top = m_Position.y;
				bb.right = m_Position.x + m_Sprite->get_width();
				bb.bottom = m_Position.y + m_Sprite->get_height();

				CL_Origin origin;
				int x, y;
				m_Sprite->get_alignment(origin, x, y);

				bb.translate(-CL_Vec2f::calc_origin(origin, bb.get_size()));

				m_AABB = bb.get_rot_bounds(origin, (float)x, (float)y, m_Sprite->get_angle());

				m_PositionChanged = false;
			}
		}
	}

	ResourcePointer<CL_Sprite> &Renderable::GetSpriteResource()
	{
		return m_Sprite;
	}

	void Renderable::OnResourceLoad(ResourceDataPtr resource)
	{
		m_Sprite.SetTarget(resource);
		m_Sprite->set_angle(CL_Angle(m_Angle/*m_DerivedAngle*/, cl_radians));
		m_Sprite->set_alpha(m_Alpha);
		m_Sprite->set_color(m_Colour);
	}

	void Renderable::OnStreamOut()
	{
		m_Sprite.Release();
	}

	void Renderable::Draw(CL_GraphicContext &gc)
	{
		if (m_Enabled && m_Sprite.IsLoaded())
		{
			m_Sprite->draw(gc, m_DerivedPosition.x, m_DerivedPosition.y);
		}
	}

	void Renderable::Draw(CL_GraphicContext &gc, const Vector2 &origin)
	{
		if (m_Enabled && m_Sprite.IsLoaded())
		{
			m_Sprite->draw(gc, m_Position.x + origin.x, m_Position.y + origin.y);
		}
	}

	void Renderable::StartAnimation()
	{
		if (m_Sprite.IsLoaded())
		{
			m_Sprite->restart();
		}
	}

	void Renderable::StopAnimaion()
	{
		if (m_Sprite.IsLoaded())
		{
			m_Sprite->finish();
		}
	}

	void Renderable::PauseAnimation()
	{
		m_Paused = true;
	}

	bool Renderable::IsPaused() const
	{
		return m_Paused;
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
			"void setPosition(float, float)",
			asMETHODPR(Renderable, SetPosition, (float, float), void), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"void setPosition(const Vector &in)",
			asMETHODPR(Renderable, SetPosition, (const Vector2&), void), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Renderable",
			"const Vector &getPosition() const",
			asMETHOD(Renderable, GetPosition), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Renderable",
			"void setAngle(float)",
			asMETHOD(Renderable, SetAngle), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Renderable",
			"float getAngle() const",
			asMETHOD(Renderable, SetAngle), asCALL_THISCALL);

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

}
