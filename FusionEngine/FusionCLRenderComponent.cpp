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
		: m_ReloadImage(true),
		m_ReloadAnimation(true)
	{
	}

	CLSprite::~CLSprite()
	{
		m_ImageLoadConnection.disconnect();
		m_AnimationLoadConnection.disconnect();
	}

	void CLSprite::redefineSprite()
	{
		if (m_ImageResource.IsLoaded() && (m_AnimationResource.IsLoaded() || !m_AnimationLoadConnection.connected()))
		{
			m_SpriteDef.reset(new SpriteDefinition2(m_ImageResource, m_AnimationResource));
			m_RecreateSprite = true;
		}
	}

	void CLSprite::Update(const float elapsed)
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
			m_ImageLoadConnection = ResourceManager::getSingleton().GetResource("IMAGE", m_ImagePath, onImageLoaded);
			m_ReloadImage = false;
		}

		if (m_ReloadAnimation && !m_AnimationPath.empty())
		{
			m_AnimationLoadConnection.disconnect();
			m_AnimationLoadConnection = ResourceManager::getSingleton().GetResource("ANIMATION", m_AnimationPath, onAnimationLoaded);
			m_ReloadAnimation = false;
		}

		m_LastPosition = m_Position;
		m_Position = m_NewPosition;

		// TODO: interpolate (probably in a separate method., taking a 'm' param)
		m_InterpPosition = m_Position;

		if (!m_Sprite.is_null())
		{
			// update animation
			m_Sprite.update(int(elapsed * 1000));

			if (!m_Sprite.is_finished())
			{
				

				if (m_Sprite.is_finished())
					AnimationFinished.MarkChanged();
			}

			// update AABB for the current frame / transform
			m_AABB.left = m_InterpPosition.x;
			m_AABB.top = m_InterpPosition.y;
			m_AABB.set_size(CL_Sizef(m_Sprite.get_size()));

			CL_Origin origin;
			int x, y;
			m_Sprite.get_alignment(origin, x, y);

			m_AABB.apply_alignment(origin, (float)x, (float)y);

			CL_Angle draw_angle = m_Sprite.get_angle() - m_Sprite.get_base_angle();
			draw_angle.normalize();
			if (!fe_fzero(draw_angle.to_degrees()))
			{
				m_Sprite.get_rotation_hotspot(origin, x, y);

				CL_Quadf bb(m_AABB);
				bb.rotate(CL_Vec2f::calc_origin(origin, m_AABB.get_size()) + CL_Vec2f((float)x, (float)y), draw_angle);

				m_AABB = bb.get_bounds();
			}
		}
	}

	void CLSprite::Draw(CL_GraphicContext& gc, const Vector2& camera_pos)
	{
		if (m_RecreateSprite)
		{
			m_Sprite = m_SpriteDef->CreateSprite(gc);
			m_Sprite.set_alignment(AlignmentOrigin.Get(), AlignmentOffset.Get().x, AlignmentOffset.Get().y);
			m_Sprite.set_rotation_hotspot(RotationOrigin.Get(), RotationOffset.Get().x, RotationOffset.Get().y);
			m_Sprite.set_color(Colour.Get());
			m_Sprite.set_alpha(Alpha.Get());
			m_Sprite.set_scale(Scale.Get().x, Scale.Get().y);
			m_Sprite.set_base_angle(CL_Angle(BaseAngle.Get(), cl_radians));
			m_RecreateSprite = false;
		}
		if (!m_Sprite.is_null())
		{
			Vector2 draw_pos = m_InterpPosition - camera_pos;
			m_Sprite.draw(gc, draw_pos.x, draw_pos.y);

			CL_Draw::box(gc, m_AABB, CL_Colorf::purple);
		}
	}

	void CLSprite::OnSiblingAdded(const std::shared_ptr<IComponent>& component)
	{
		const auto& interfaces = component->GetInterfaces();
		if (interfaces.find(ITransform::GetTypeName()) != interfaces.end())
		{
			auto transform = dynamic_cast<ITransform*>(component.get());
			transform->Position.Connect(std::bind(&CLSprite::SetPosition, this, std::placeholders::_1));
			transform->Angle.Connect(std::bind(&CLSprite::SetAngle, this, std::placeholders::_1));
			transform->Depth.Connect([this](int depth) { m_EntityDepth = depth; });
		}
	}

	void CLSprite::SynchroniseParallelEdits()
	{
		ISprite::SynchroniseInterface();
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
		m_NewPosition = value;
	}

	Vector2 CLSprite::GetPosition() const
	{
		return m_InterpPosition;
	}

	void CLSprite::SetOffset(const Vector2& value)
	{
		m_Offset = value;
		m_SerialisationHelper.markChanged(PropsIdx::Offset);
	}

	void CLSprite::SetAngle(float angle)
	{
		m_Sprite.set_angle(CL_Angle(angle, cl_radians).normalize());
	}

	void CLSprite::SetLocalDepth(int value)
	{
		m_LocalDepth = value;
		m_SerialisationHelper.markChanged(PropsIdx::LocalDepth);
	}

	void CLSprite::SetImagePath(const std::string& value)
	{
		m_ImagePath = value;
		m_ReloadImage = true;
		m_SerialisationHelper.markChanged(PropsIdx::ImagePath);
		m_SerialisationHelper.markChanged(PropsIdx::ReloadImage);
	}

	std::string CLSprite::GetImagePath() const
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

	std::string CLSprite::GetAnimationPath() const
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

	CL_Colorf CLSprite::GetColour() const
	{
		if (!m_Sprite.is_null())
			return m_Sprite.get_color();
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
