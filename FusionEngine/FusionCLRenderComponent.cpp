/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionCLRenderComponent.h"

#include "FusionResourceManager.h"

#include "FusionPhysicalComponent.h"
#include "FusionSpriteDefinition.h"

#include <functional>

namespace FusionEngine
{

	CLSprite::CLSprite()
		: m_ReloadImage(false),
		m_ReloadAnimation(false),
		m_RecreateSprite(false),
		m_EntityDepth(0),
		m_LocalDepth(0),
		//m_Interpolate(true),
		m_AnimationFrame(0),
		//m_InterpAnimationFrame(0),
		//m_LastAnimationFrame(0),
		m_DeltaTime(0.f),
		m_ElapsedTime(0.f),
		m_NewAngle(0.f),
		m_Angle(0.f)
		//m_InterpAngle(0.f),
		//m_LastAngle(0.f)
	{
	}

	CLSprite::~CLSprite()
	{
		m_ImageLoadConnection.disconnect();
		m_AnimationLoadConnection.disconnect();

		m_PositionChangeConnection.disconnect();
		m_AngleChangeConnection.disconnect();
		m_DepthChangeConnection.disconnect();
	}

	void CLSprite::redefineSprite()
	{
		if (m_ImageResource.IsLoaded() && (m_AnimationResource.IsLoaded() || !m_AnimationLoadConnection.connected()))
		{
			m_SpriteDef.reset(new SpriteDefinition2(m_ImageResource, m_AnimationResource));
			m_RecreateSprite = true;
		}
		else
			SendToConsole("Failed to load?");
	}

	void CLSprite::Update(unsigned int tick, const float elapsed, const float alpha)
	{
		auto onImageLoaded = [this](ResourceDataPtr& data)
		{
			m_ImageResource.SetTarget(data);
			redefineSprite();
		};

		auto onAnimationLoaded = [this](ResourceDataPtr& data)
		{
			m_AnimationResource.SetTarget(data);
			redefineSprite();
		};

		using namespace std::placeholders;
		if (m_ReloadImage && !m_ImagePath.empty())
		{
			m_ImageLoadConnection.disconnect();
			m_ImageLoadConnection = ResourceManager::getSingleton().GetResource("TEXTURE", m_ImagePath, onImageLoaded);
			m_ReloadImage = false;
		}

		if (m_ReloadAnimation && !m_AnimationPath.empty())
		{
			m_AnimationLoadConnection.disconnect();
			m_AnimationLoadConnection = ResourceManager::getSingleton().GetResource("ANIMATION", m_AnimationPath, onAnimationLoaded);
			m_ReloadAnimation = false;
		}

		m_DeltaTime = elapsed;
		m_ElapsedTime = 0.f;

		m_CurrentTick = tick;

		//m_LastPosition = m_Position;
		//m_Position = m_NewPosition;

		//m_LastAngle = m_Angle;
		//m_Angle = m_NewAngle;

		//m_InterpPosition = m_LastPosition;
		//m_InterpAngle = m_LastAngle;

		//if (m_Interpolate && m_CurrentTick == m_InterpTick)
		//{
		//	if ((m_Position - m_LastPosition).squared_length() > ToSimUnits(1.0f))
		//		m_LastPosition = m_Position;

		//	Lerp(m_InterpPosition, m_LastPosition, m_Position, alpha);

		//	Lerp(m_InterpAngle, m_LastAngle, m_Angle, alpha);
		//}
		//else
		//{
		//	m_InterpPosition = m_Position;
		//	m_InterpAngle = m_Angle;
		//}

		if (!m_Sprite.is_null())
		{
			if (m_Sprite.get_angle().to_radians() != m_Angle)
			{
				auto normalisedAngle = CL_Angle(m_Angle, cl_radians).normalize();
				m_Sprite.set_angle(normalisedAngle);
				m_Angle = normalisedAngle.to_radians();
			}

			//if (!m_Sprite.is_finished())
			{
				// update animation
				m_Sprite.update(int(elapsed * 1000));

				if (m_Sprite.is_finished() != AnimationFinished.Get())
					AnimationFinished.MarkChanged();
			}

			// update AABB for the current frame / transform
			m_AABB.left = ToRender(m_Position.x);
			m_AABB.top = ToRender(m_Position.y);
			m_AABB.set_size(CL_Sizef(m_Sprite.get_size()));

			CL_Origin origin;
			int x, y;
			m_Sprite.get_alignment(origin, x, y);

			m_AABB.translate(-CL_Vec2f::calc_origin(origin, m_AABB.get_size()) - CL_Vec2f((float)x, (float)y));
			//m_AABB.apply_alignment(origin, (float)x, (float)y);

			CL_Angle draw_angle = m_Sprite.get_angle() - m_Sprite.get_base_angle();
			//draw_angle.normalize();
			if (!fe_fzero(draw_angle.to_degrees()))
			{
				m_Sprite.get_rotation_hotspot(origin, x, y);

				CL_Quadf bb(m_AABB);
				bb.rotate(CL_Vec2f::calc_origin(origin, m_AABB.get_size()) + m_AABB.get_top_left() + CL_Vec2f((float)x, (float)y), draw_angle);

				m_AABB = bb.get_bounds();
			}
		}
	}

	void CLSprite::Draw(CL_GraphicContext& gc, const Vector2& camera_pos)
	{
		if (m_RecreateSprite && m_SpriteDef)
		{
			m_Sprite = m_SpriteDef->CreateSprite(gc);
			m_Sprite.set_alignment(origin_center);
			m_Sprite.set_rotation_hotspot(origin_center);
			//m_Sprite.set_alignment(AlignmentOrigin.Get(), AlignmentOffset.Get().x, AlignmentOffset.Get().y);
			//m_Sprite.set_rotation_hotspot(RotationOrigin.Get(), RotationOffset.Get().x, RotationOffset.Get().y);
			//m_Sprite.set_color(Colour.Get());
			//m_Sprite.set_alpha(Alpha.Get());
			//m_Sprite.set_scale(Scale.Get().x, Scale.Get().y);
			m_Sprite.set_base_angle(CL_Angle(BaseAngle.Get(), cl_radians));

			m_Sprite.set_angle(CL_Angle(m_Angle, cl_radians));

			m_RecreateSprite = false;
		}
		if (!m_Sprite.is_null())
		{
			Vector2 draw_pos = ToRender(m_Position);// - camera_pos;
			m_Sprite.draw(gc, draw_pos.x, draw_pos.y);

			//auto size = m_AABB.get_size();
			//if (size.width > 0.0f)
			//{
			//	auto drawAABB = m_AABB;
			//	drawAABB.translate(-camera_pos.x, -camera_pos.y);
			//	CL_Draw::box(gc, drawAABB, CL_Colorf::purple);
			//}

			//if (m_Interpolate)
			//{
			//	if (m_DebugFont.is_null())
			//		m_DebugFont = CL_Font(gc, "Lucida Console", 12);

			//	if (m_CurrentTick == m_InterpTick)
			//		m_DebugFont.draw_text(gc, draw_pos.x, draw_pos.y, "Interpolating", CL_Colorf::deepskyblue);

			//	//std::stringstream str;
			//	//str << m_LastAngle << " | " << m_InterpAngle << " | " << m_Angle;
			//	//m_DebugFont.draw_text(gc, draw_pos.x, draw_pos.y + 14.f, str.str(), CL_Colorf::cyan);
			//}
		}
	}

	void CLSprite::OnSiblingAdded(const std::shared_ptr<IComponent>& component)
	{
		const auto& interfaces = component->GetInterfaces();
		if (interfaces.find(ITransform::GetTypeName()) != interfaces.end())
		{
			auto transform = dynamic_cast<ITransform*>(component.get());

			FSN_ASSERT(transform);

			m_PositionChangeConnection.disconnect();
			m_AngleChangeConnection.disconnect();
			m_DepthChangeConnection.disconnect();

			m_PositionChangeConnection = transform->Position.Connect(std::bind(&CLSprite::SetPosition, this, std::placeholders::_1));
			m_AngleChangeConnection = transform->Angle.Connect(std::bind(&CLSprite::SetAngle, this, std::placeholders::_1));
			m_DepthChangeConnection = transform->Depth.Connect([this](int depth) { m_EntityDepth = depth; });
		}
	}

	void CLSprite::SynchroniseParallelEdits()
	{
		ISprite::SynchroniseInterface();
	}

	void CLSprite::FireSignals()
	{
		ISprite::FireInterfaceSignals();
	}

	bool CLSprite::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		//return m_SerialisationHelper.writeChanges(force_all, stream, std::tie(m_Offset, m_FilePath, m_Reload));
		return m_SerialisationHelper.writeChanges(force_all, stream,
			m_Offset, m_LocalDepth,
			m_ImagePath, m_ReloadImage, m_AnimationPath, m_ReloadAnimation,
			GetAlignmentOrigin(), GetAlignmentOffset(),
			GetRotationOrigin(), GetRotationOffset(),
			GetColour(),
			GetAlpha(),
			GetScale(),
			GetBaseAngle());
	}

	void CLSprite::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		std::bitset<PropsIdx::NumProps> changed;

		m_SerialisationHelper.readChanges(stream, all, changed,
			m_Offset, m_LocalDepth,
			m_ImagePath, m_ReloadImage, m_AnimationPath, m_ReloadAnimation,
			AlignmentOrigin.m_Value, AlignmentOffset.m_Value,
			RotationOrigin.m_Value, RotationOffset.m_Value,
			Colour.m_Value,
			Alpha.m_Value,
			Scale.m_Value,
			BaseAngle.m_Value
			);
		if (changed[PropsIdx::ImagePath]) // file path changed
			m_ReloadImage = true;
		if (changed[PropsIdx::AnimationPath])
			m_ReloadAnimation = true;

		if (!m_Sprite.is_null()) // Copy the changes to the CL_Sprite, if it is loaded
		{
			if (changed[PropsIdx::AlignmentOrigin] || changed[PropsIdx::AlignmentOffset])
				m_Sprite.set_alignment(AlignmentOrigin.Get(), AlignmentOffset.Get().x, AlignmentOffset.Get().y);
			if (changed[PropsIdx::RotationOrigin] || changed[PropsIdx::RotationOffset])
				m_Sprite.set_rotation_hotspot(RotationOrigin.Get(), RotationOffset.Get().x, RotationOffset.Get().y);
			if (changed[PropsIdx::Colour])
				m_Sprite.set_color(Colour.Get());
			if (changed[PropsIdx::Alpha])
				m_Sprite.set_alpha(Alpha.Get());
			if (changed[PropsIdx::Scale])
				m_Sprite.set_scale(Scale.Get().x, Scale.Get().y);
			if (changed[PropsIdx::BaseAngle])
				m_Sprite.set_base_angle(CL_Angle(BaseAngle.Get(), cl_radians));
		}
	}

	void CLSprite::SetPosition(const Vector2& value)
	{
		//m_NewPosition = value;

		//if (m_PositionSet)
		//	m_LastPosition = m_Position;
		//else
		//{
		//	m_LastPosition = value;
		//	m_PositionSet = true;
		//}
		//m_Position = value;

		//m_InterpTick = m_CurrentTick;

		m_Position = value;
	}
	
	void CLSprite::SetAngle(float angle)
	{
		//m_NewAngle = angle;

		//if (m_AngleSet)
		//	m_LastAngle = m_Angle;
		//else
		//{
		//	m_LastAngle = angle;
		//	m_AngleSet = true;
		//}
		//m_Angle = angle;

		//m_InterpTick = m_CurrentTick;

		m_Angle = angle;
	}

	Vector2 CLSprite::GetPosition() const
	{
		return m_Position;
	}

	void CLSprite::SetOffset(const Vector2& value)
	{
		m_Offset = value;
		m_SerialisationHelper.markChanged(PropsIdx::Offset);
	}

	void CLSprite::SetLocalDepth(int value)
	{
		m_LocalDepth = value;
		m_SerialisationHelper.markChanged(PropsIdx::LocalDepth);
	}

	//void CLSprite::SetInterpolate(bool value)
	//{
	//	m_Interpolate = value;
	//}

	void CLSprite::SetImagePath(const std::string& value)
	{
		m_ImagePath = value;
		m_ReloadImage = true;
		m_SerialisationHelper.markChanged(PropsIdx::ImagePath);
		m_SerialisationHelper.markChanged(PropsIdx::ReloadImage);
	}

	const std::string &CLSprite::GetImagePath() const
	{
		return m_ImagePath;
	}

	void CLSprite::SetAnimationPath(const std::string& value)
	{
		m_AnimationPath = value;
		m_ReloadAnimation = true;
		m_SerialisationHelper.markChanged(PropsIdx::AnimationPath);
		m_SerialisationHelper.markChanged(PropsIdx::ReloadAnimation);
	}

	const std::string &CLSprite::GetAnimationPath() const
	{
		return m_AnimationPath;
	}

	void CLSprite::SetAlignmentOrigin(CL_Origin origin)
	{
		if (!m_Sprite.is_null())
		{
			CL_Origin ignore; int x, y;
			m_Sprite.get_alignment(ignore, x, y);
			m_Sprite.set_alignment(origin, x, y);
		}

		m_SerialisationHelper.markChanged(PropsIdx::AlignmentOrigin);
	}
	
	CL_Origin CLSprite::GetAlignmentOrigin() const
	{
		if (!m_Sprite.is_null())
		{
			CL_Origin origin; int x, y;
			m_Sprite.get_alignment(origin, x, y);
			return origin;
		}
		else
			return AlignmentOrigin.Get();
	}

	void CLSprite::SetAlignmentOffset(const Vector2i& offset)
	{
		if (!m_Sprite.is_null())
		{
			CL_Origin origin; int x, y;
			m_Sprite.get_alignment(origin, x, y);
			m_Sprite.set_alignment(origin, offset.x, offset.y);
		}

		m_SerialisationHelper.markChanged(PropsIdx::AlignmentOffset);
	}

	Vector2i CLSprite::GetAlignmentOffset() const
	{
		if (!m_Sprite.is_null())
		{
			Vector2i off; CL_Origin ignore;
			m_Sprite.get_alignment(ignore, off.x, off.y);
			return off;
		}
		else
			return AlignmentOffset.Get();
	}

	void CLSprite::SetRotationOrigin(CL_Origin origin)
	{
		if (!m_Sprite.is_null())
		{
			CL_Origin ignore; int x, y;
			m_Sprite.get_rotation_hotspot(ignore, x, y);
			m_Sprite.set_rotation_hotspot(origin, x, y);
		}

		m_SerialisationHelper.markChanged(PropsIdx::RotationOrigin);
	}

	CL_Origin CLSprite::GetRotationOrigin() const
	{
		if (!m_Sprite.is_null())
		{
			CL_Origin origin; int x, y;
			m_Sprite.get_rotation_hotspot(origin, x, y);
			return origin;
		}
		else
			return RotationOrigin.Get();
	}

	void CLSprite::SetRotationOffset(const Vector2i& offset)
	{
		if (!m_Sprite.is_null())
		{
			CL_Origin origin; int x, y;
			m_Sprite.get_rotation_hotspot(origin, x, y);
			m_Sprite.set_rotation_hotspot(origin, offset.x, offset.y);
		}

		m_SerialisationHelper.markChanged(PropsIdx::RotationOffset);
	}

	Vector2i CLSprite::GetRotationOffset() const
	{
		if (!m_Sprite.is_null())
		{
			Vector2i off; CL_Origin ignore;
			m_Sprite.get_rotation_hotspot(ignore, off.x, off.y);
			return off;
		}
		else
			return RotationOffset.Get();
	}

	void CLSprite::SetColour(const CL_Colorf& val)
	{
		if (!m_Sprite.is_null())
			m_Sprite.set_color(val);

		m_SerialisationHelper.markChanged(PropsIdx::Colour);
	}

	const CL_Colorf &CLSprite::GetColour() const
	{
		if (!m_Sprite.is_null())
		{
			m_Colour = m_Sprite.get_color();
			return m_Colour;
		}
		else
			return Colour.Get();
	}

	void CLSprite::SetAlpha(float alpha)
	{
		if (!m_Sprite.is_null())
			m_Sprite.set_alpha(alpha);

		m_SerialisationHelper.markChanged(PropsIdx::Alpha);
	}

	float CLSprite::GetAlpha() const
	{
		if (!m_Sprite.is_null())
			return m_Sprite.get_alpha();
		else
			return Alpha.Get();
	}

	void CLSprite::SetScale(const Vector2& val)
	{
		if (!m_Sprite.is_null())
			m_Sprite.set_scale(val.x, val.y);

		m_SerialisationHelper.markChanged(PropsIdx::Scale);
	}

	Vector2 CLSprite::GetScale() const
	{
		if (!m_Sprite.is_null())
		{
			Vector2 scale;
			m_Sprite.get_scale(scale.x, scale.y);
			return scale;
		}
		else
			return Scale.Get();
	}

	void CLSprite::SetBaseAngle(float val)
	{
		if (!m_Sprite.is_null())
			m_Sprite.set_base_angle(CL_Angle(val, cl_radians));

		m_SerialisationHelper.markChanged(PropsIdx::BaseAngle);
	}

	float CLSprite::GetBaseAngle() const
	{
		if (!m_Sprite.is_null())
		{
			CL_Angle angle = m_Sprite.get_base_angle();
			return angle.to_radians();
		}
		else
			return BaseAngle.Get();
	}

	bool CLSprite::IsAnimationFinished() const
	{
		if (!m_Sprite.is_null())
			return m_Sprite.is_finished();
		else
			return AnimationFinished.Get();
	}

}
