/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionCLRenderComponent.h"

#include "FusionLogger.h"
#include "FusionResourceManager.h"

#include "FusionEntity.h"
#include "FusionTransformComponent.h"
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
		m_Colour = clan::Colorf::white;
		m_AlignmentOrigin = clan::origin_center;
		m_RotationOrigin = clan::origin_center;
		m_Alpha = 1.f;
		m_Scale = Vector2(1.f, 1.0f);
		m_BaseAngle = 0.f;

		m_AnimationFrame = 0;

		m_Looping = true;
	}

	CLSprite::~CLSprite()
	{
		m_ImageLoadConnection.disconnect();
		m_AnimationLoadConnection.disconnect();

		//m_PositionChangeConnection.disconnect();
		//m_AngleChangeConnection.disconnect();
		//m_DepthChangeConnection.disconnect();
	}

	std::shared_ptr<SpriteDefinition> SpriteDefinitionCache::GetSpriteDefinition(const ResourcePointer<clan::Texture2D>& texture, const ResourcePointer<SpriteAnimation>& animation)
	{
		std::shared_ptr<SpriteDefinition> spriteDefinition;

		if (texture.IsLoaded())
		{
			const auto key = std::make_pair(texture.GetTarget()->GetPath(), animation.IsLoaded() ? animation.GetTarget()->GetPath() : "");

			auto& defMap = getSingleton().m_SpriteDefinitions;

			SpriteDefinitionCache::SpriteDefinitionMap_t::accessor accessor;
			if (defMap.insert(accessor, key))
			{
				spriteDefinition = std::make_shared<SpriteDefinition>(texture, animation);
				spriteDefinition->m_UnusedCallback = [key]()
				{
					if (getSingletonPtr())
						getSingleton().m_SpriteDefinitions.erase(key);
				};

				accessor->second = spriteDefinition;
			}
			else
				spriteDefinition = accessor->second.lock();
		}

		return spriteDefinition;
	}

	void CLSprite::redefineSprite()
	{
		// This checks if m_AnimationLoadConnection is connected because if it is this method will
		//  be called again when the animation resource is done loading. Otherwise, the animation
		//  resource path simply hasn't been set.
		if (m_ImageResource.IsLoaded() && (m_AnimationResource.IsLoaded() || !m_AnimationLoadConnection.connected()))
		{
			//m_SpriteDef.reset(new SpriteDefinition(m_ImageResource, m_AnimationResource));
			m_SpriteDef = SpriteDefinitionCache::GetSpriteDefinition(m_ImageResource, m_AnimationResource);
			m_RecreateSprite = true;
		}
		if (!m_ImageResource.IsLoaded() && !m_AnimationResource.IsLoaded())
			AddLogEntry("CLSprite: Seems like a sprite resource failed to load. Image: " + m_ImagePath + " Animation: " + m_AnimationPath);
	}

	bool CLSprite::ImageHotReloadEvents(ResourceDataPtr resource, ResourceContainer::HotReloadEvent ev)
	{
		switch (ev)
		{
		case ResourceContainer::HotReloadEvent::Validate:
			//m_Sprite = clan::Sprite(); // This seems to cause a crash
			m_SpriteDef.reset();
			m_ImageResource.Release();
			break;
		case ResourceContainer::HotReloadEvent::PostReload:
			ImageLoaded(resource);
			break;
		}

		return true;
	}

	bool CLSprite::AnimationHotReloadEvents(ResourceDataPtr resource, ResourceContainer::HotReloadEvent ev)
	{
		switch (ev)
		{
		case ResourceContainer::HotReloadEvent::Validate:
			m_AnimationResource.Release();
			if (!m_Sprite.is_null())
				redefineSprite();
			break;
		case ResourceContainer::HotReloadEvent::PostReload:
			AnimationLoaded(resource);
			break;
		}

		return true;
	}

	void CLSprite::ImageLoaded(ResourceDataPtr data)
	{
		using namespace std::placeholders;

		m_ImageResource.SetTarget(data);
		// Allow hot-reload
		m_ImageLoadConnection = data->SigHotReloadEvents.connect(std::bind(&CLSprite::ImageHotReloadEvents, this, _1, _2));

		m_RecreateSprite = true;
		//redefineSprite();
	}

	void CLSprite::AnimationLoaded(ResourceDataPtr data)
	{
		using namespace std::placeholders;

		m_AnimationResource.SetTarget(data);
		// Allow hot-reload
		m_AnimationLoadConnection = data->SigHotReloadEvents.connect(std::bind(&CLSprite::AnimationHotReloadEvents, this, _1, _2));

		m_RecreateSprite = true;
		//redefineSprite();
	}

	void CLSprite::DefineSpriteIfNecessary()
	{
		using namespace std::placeholders;
		if (m_ReloadImage)
		{
			if (!m_ImagePath.empty())
			{
				m_ImageLoadConnection.disconnect();
				m_ImageLoadConnection = ResourceManager::getSingleton().GetResource("TEXTURE", m_ImagePath, std::bind(&CLSprite::ImageLoaded, this, _1));
			}
			else
			{
				m_Sprite = clan::Sprite();
				m_SpriteDef.reset();
				m_ImageResource.Release();
			}
			m_ReloadImage = false;
		}

		if (m_ReloadAnimation)
		{
			if (!m_AnimationPath.empty())
			{
				m_AnimationLoadConnection.disconnect();
				m_AnimationLoadConnection = ResourceManager::getSingleton().GetResource("ANIMATION", m_AnimationPath, std::bind(&CLSprite::AnimationLoaded, this, _1));
			}
			else
			{
				m_AnimationResource.Release();
				if (!m_Sprite.is_null())
					redefineSprite();
			}
			m_ReloadAnimation = false;
		}

		if (m_RecreateSprite/* && !m_SpriteDef*/)
			redefineSprite();
	}

	void CLSprite::Update(unsigned int, const float elapsed, const float)
	{
		DefineSpriteIfNecessary();

		m_DeltaTime = elapsed;
		m_ElapsedTime = 0.f;

		//m_CurrentTick = tick;

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
			// Update the colour property
			m_Colour = m_Sprite.get_color();

			// Update the angle prop
			if (m_Sprite.get_angle().to_radians() != m_Angle)
			{
				auto normalisedAngle = clan::Angle(m_Angle, clan::angle_radians).normalize();
				m_Sprite.set_angle(normalisedAngle);
				m_Angle = normalisedAngle.to_radians();
			}

			//if (!m_Sprite.is_finished())
			{
				const auto frameBefore = m_Sprite.get_current_frame();

				// update animation
				m_Sprite.update(int(elapsed * 1000));

				if (m_Sprite.is_finished() == Playing.Get())
					Playing.MarkChanged();
				if (m_Sprite.is_finished() != AnimationFinished.Get())
					AnimationFinished.MarkChanged();

				if (frameBefore != m_Sprite.get_current_frame())
					AnimationFrame.MarkChanged();
			}

			// update AABB for the current frame / transform
			m_AABB.left = ToRender(m_Position.x);
			m_AABB.top = ToRender(m_Position.y);
			m_AABB.set_size(clan::Sizef(m_Sprite.get_size()));

			clan::Origin origin;
			int x, y;
			m_Sprite.get_alignment(origin, x, y);

			m_AABB.translate(-clan::Vec2f::calc_origin(origin, m_AABB.get_size()) - clan::Vec2f((float)x, (float)y));
			//m_AABB.apply_alignment(origin, (float)x, (float)y);

			clan::Angle draw_angle = m_Sprite.get_angle() - m_Sprite.get_base_angle();
			//draw_angle.normalize();
			if (!fe_fzero(draw_angle.to_degrees()))
			{
				m_Sprite.get_rotation_hotspot(origin, x, y);

				clan::Quadf bb(m_AABB);
				bb.rotate(clan::Vec2f::calc_origin(origin, m_AABB.get_size()) + m_AABB.get_top_left() + clan::Vec2f((float)x, (float)y), draw_angle);

				m_AABB = bb.get_bounds();
			}
		}
	}

	void CLSprite::CreateSpriteIfNecessary(clan::Canvas& canvas)
	{
		if (m_RecreateSprite && m_SpriteDef)
		{
			m_Sprite = m_SpriteDef->CreateSprite(canvas);

			m_Sprite.set_alignment(m_AlignmentOrigin, m_AlignmentOffset.x, m_AlignmentOffset.y);
			m_Sprite.set_rotation_hotspot(m_RotationOrigin, m_RotationOffset.x, m_RotationOffset.y);
			m_Sprite.set_color(m_Colour);
			m_Sprite.set_alpha(m_Alpha);
			m_Sprite.set_scale(m_Scale.x, m_Scale.y);
			m_Sprite.set_base_angle(clan::Angle(m_BaseAngle, clan::angle_radians));

			m_Sprite.set_angle(clan::Angle(m_Angle, clan::angle_radians));
			
			m_Sprite.set_frame(m_AnimationFrame);

			m_RecreateSprite = false;
		}
	}

	void CLSprite::Draw(clan::Canvas& canvas, const Vector2& camera_pos)
	{
		CreateSpriteIfNecessary(canvas);

		if (!m_Sprite.is_null())
		{
//#ifdef _DEBUG
//			const clan::Colorf authColours[] = {
//				clan::Colorf::white,
//				clan::Colorf::blue,
//				clan::Colorf::red,
//				clan::Colorf::yellow,
//				clan::Colorf::green,
//				clan::Colorf::brown,
//				clan::Colorf::purple,
//				clan::Colorf::orange
//			};
//			PlayerID auth = GetParent()->GetAuthority();
//			if (auth < 8)
//				m_Sprite.set_color(authColours[auth]);
//#endif

			Vector2 draw_pos = ToRender(m_Position) + m_Offset;
			m_Sprite.draw(canvas, draw_pos.x, draw_pos.y);

			//auto size = m_AABB.get_size();
			//if (size.width > 0.0f)
			//{
			//	auto drawAABB = m_AABB;
			//	drawAABB.translate(-camera_pos.x, -camera_pos.y);
			//	clan::Draw::box(gc, drawAABB, clan::Colorf::purple);
			//}

			//if (m_Interpolate)
			//{
			//	if (m_DebugFont.is_null())
			//		m_DebugFont = clan::Font(gc, "Lucida Console", 12);

			//	if (m_CurrentTick == m_InterpTick)
			//		m_DebugFont.draw_text(gc, draw_pos.x, draw_pos.y, "Interpolating", clan::Colorf::deepskyblue);

			//	//std::stringstream str;
			//	//str << m_LastAngle << " | " << m_InterpAngle << " | " << m_Angle;
			//	//m_DebugFont.draw_text(gc, draw_pos.x, draw_pos.y + 14.f, str.str(), clan::Colorf::cyan);
			//}
		}
	}

	void CLSprite::OnSiblingAdded(const ComponentPtr& component)
	{
		const auto& interfaces = component->GetInterfaces();
		if (interfaces.find(ITransform::GetTypeName()) != interfaces.end())
		{
			auto transform = dynamic_cast<ITransform*>(component.get());

			FSN_ASSERT(transform);

			if (!GetParent()->IsTerrain())
			{
				using namespace std::placeholders;

				auto& system = EvesdroppingManager::getSingleton().GetSignalingSystem();
				m_PositionChangeConnection = system.AddHandler<const Vector2&>(transform->Position.GetID(), std::bind(&CLSprite::SetPosition, this, _1));
				m_AngleChangeConnection = system.AddHandler<const float&>(transform->Angle.GetID(), std::bind(&CLSprite::SetAngle, this, _1));
				m_DepthChangeConnection = system.AddHandler<const int&>(transform->Depth.GetID(), [this](const int& depth) { m_EntityDepth = depth; });
			}

			m_Position = transform->Position.Get();
			m_Angle = transform->Angle.Get();
			m_EntityDepth = transform->Depth.Get();
		}
	}

	void CLSprite::SerialiseOccasional(RakNet::BitStream& stream)
	{
		SerialisationUtils::write(stream, GetOffset());
		SerialisationUtils::write(stream, GetLocalDepth());
		
		SerialisationUtils::write(stream, GetImagePath());
		SerialisationUtils::write(stream, GetAnimationPath());

		SerialisationUtils::write(stream, GetAlignmentOrigin());
		SerialisationUtils::write(stream, GetAlignmentOffset());
		SerialisationUtils::write(stream, GetRotationOrigin());
		SerialisationUtils::write(stream, GetRotationOffset());

		SerialisationUtils::write(stream, GetColour());
		SerialisationUtils::write(stream, GetAlpha());
		SerialisationUtils::write(stream, GetScale());
		SerialisationUtils::write(stream, GetBaseAngle());
		SerialisationUtils::write(stream, GetAnimationFrame());
	}

	void CLSprite::DeserialiseOccasional(RakNet::BitStream& stream)
	{
		SerialisationUtils::read(stream, m_Offset);
		SerialisationUtils::read(stream, m_LocalDepth);
		
		std::string imagePath;
		SerialisationUtils::read(stream, imagePath);
		std::string animationPath;
		SerialisationUtils::read(stream, animationPath);

		SerialisationUtils::read(stream, m_AlignmentOrigin);
		SerialisationUtils::read(stream, m_AlignmentOffset);
		SerialisationUtils::read(stream, m_RotationOrigin);
		SerialisationUtils::read(stream, m_RotationOffset);

		SerialisationUtils::read(stream, m_Colour);
		SerialisationUtils::read(stream, m_Alpha);
		SerialisationUtils::read(stream, m_Scale);
		SerialisationUtils::read(stream, m_BaseAngle);
		SerialisationUtils::read(stream, m_AnimationFrame);

		if (imagePath != m_ImagePath) // file path changed
		{
			m_ImagePath = imagePath;
			m_ReloadImage = true;
		}
		if (animationPath != m_AnimationPath)
		{
			m_AnimationPath = animationPath;
			m_ReloadAnimation = true;
		}

		//if (m_Offset != Offset.Get())
		//	Offset.MarkChanged();

		//if (m_LocalDepth != LocalDepth.Get())
		//	LocalDepth.MarkChanged();

		//if (m_ReloadImage)
		//	ImagePath.MarkChanged();

		//if (m_ReloadAnimation)
		//	AnimationPath.MarkChanged();

		if (!m_Sprite.is_null()) // Copy the changes to the clan::Sprite, if it is loaded
		{
			m_Sprite.set_alignment(m_AlignmentOrigin, m_AlignmentOffset.x, m_AlignmentOffset.y);
			m_Sprite.set_rotation_hotspot(m_RotationOrigin, m_RotationOffset.x, m_RotationOffset.y);
			m_Sprite.set_color(m_Colour);
			m_Sprite.set_alpha(m_Alpha);
			m_Sprite.set_scale(m_Scale.x, m_Scale.y);
			m_Sprite.set_base_angle(clan::Angle(m_BaseAngle, clan::angle_radians));

			m_Sprite.set_frame(m_AnimationFrame);
		}
	}

	void CLSprite::OnPostDeserialisation()
	{
		//if (m_ImagePath != ImagePath.Get())
		//{
		//	m_ReloadImage = true;
		//}
		//if (m_AnimationPath != AnimationPath.Get())
		//{
		//	m_ReloadAnimation = true;
		//}

		//if (!m_Sprite.is_null()) // Copy the changes to the clan::Sprite, if it is loaded
		//{
		//	m_Sprite.set_alignment(AlignmentOrigin.Get(), AlignmentOffset.Get().x, AlignmentOffset.Get().y);
		//	m_Sprite.set_rotation_hotspot(RotationOrigin.Get(), RotationOffset.Get().x, RotationOffset.Get().y);
		//	m_Sprite.set_color(Colour.Get());
		//	m_Sprite.set_alpha(Alpha.Get());
		//	m_Sprite.set_scale(Scale.Get().x, Scale.Get().y);
		//	m_Sprite.set_base_angle(clan::Angle(BaseAngle.Get(), clan::angle_radians));

		//	m_Sprite.set_frame(AnimationFrame.Get());
		//}
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
	}

	Vector2 CLSprite::GetOffset() const
	{
		return m_Offset;
	}

	void CLSprite::SetLocalDepth(int value)
	{
		m_LocalDepth = value;
	}

	//void CLSprite::SetInterpolate(bool value)
	//{
	//	m_Interpolate = value;
	//}

	void CLSprite::SetImagePath(const std::string& value)
	{
		m_ImagePath = value;
		m_ReloadImage = true;
	}

	const std::string &CLSprite::GetImagePath() const
	{
		return m_ImagePath;
	}

	void CLSprite::SetAnimationPath(const std::string& value)
	{
		m_AnimationPath = value;
		m_ReloadAnimation = true;
	}

	const std::string &CLSprite::GetAnimationPath() const
	{
		return m_AnimationPath;
	}

	void CLSprite::SetAlignmentOrigin(clan::Origin origin)
	{
		m_AlignmentOrigin = origin;
		if (!m_Sprite.is_null())
		{
			clan::Origin ignore; int x, y;
			m_Sprite.get_alignment(ignore, x, y);
			m_Sprite.set_alignment(origin, x, y);
		}
	}
	
	clan::Origin CLSprite::GetAlignmentOrigin() const
	{
		if (!m_Sprite.is_null())
		{
			clan::Origin origin; int x, y;
			m_Sprite.get_alignment(origin, x, y);
			return origin;
		}
		else
			return m_AlignmentOrigin;
	}

	void CLSprite::SetAlignmentOffset(const Vector2i& offset)
	{
		m_AlignmentOffset = offset;
		if (!m_Sprite.is_null())
		{
			clan::Origin origin; int x, y;
			m_Sprite.get_alignment(origin, x, y);
			m_Sprite.set_alignment(origin, offset.x, offset.y);
		}
	}

	Vector2i CLSprite::GetAlignmentOffset() const
	{
		if (!m_Sprite.is_null())
		{
			Vector2i off; clan::Origin ignore;
			m_Sprite.get_alignment(ignore, off.x, off.y);
			return off;
		}
		else
			return m_AlignmentOffset;
	}

	void CLSprite::SetRotationOrigin(clan::Origin origin)
	{
		m_RotationOrigin = origin;
		if (!m_Sprite.is_null())
		{
			clan::Origin ignore; int x, y;
			m_Sprite.get_rotation_hotspot(ignore, x, y);
			m_Sprite.set_rotation_hotspot(origin, x, y);
		}
	}

	clan::Origin CLSprite::GetRotationOrigin() const
	{
		if (!m_Sprite.is_null())
		{
			clan::Origin origin; int x, y;
			m_Sprite.get_rotation_hotspot(origin, x, y);
			return origin;
		}
		else
			return m_RotationOrigin;
	}

	void CLSprite::SetRotationOffset(const Vector2i& offset)
	{
		m_RotationOffset = offset;
		if (!m_Sprite.is_null())
		{
			clan::Origin origin; int x, y;
			m_Sprite.get_rotation_hotspot(origin, x, y);
			m_Sprite.set_rotation_hotspot(origin, offset.x, offset.y);
		}
	}

	Vector2i CLSprite::GetRotationOffset() const
	{
		if (!m_Sprite.is_null())
		{
			Vector2i off; clan::Origin ignore;
			m_Sprite.get_rotation_hotspot(ignore, off.x, off.y);
			return off;
		}
		else
			return m_RotationOffset;
	}

	void CLSprite::SetColour(const clan::Colorf& val)
	{
		if (!m_Sprite.is_null())
			m_Sprite.set_color(val);

		m_Colour = val;
	}

	const clan::Colorf &CLSprite::GetColour() const
	{
		if (!m_Sprite.is_null())
		{
			return m_Colour;
		}
		else
			return m_Colour;
	}

	void CLSprite::SetAlpha(float alpha)
	{
		m_Alpha = alpha;
		if (!m_Sprite.is_null())
			m_Sprite.set_alpha(alpha);
	}

	float CLSprite::GetAlpha() const
	{
		if (!m_Sprite.is_null())
			return m_Sprite.get_alpha();
		else
			return m_Alpha;
	}

	void CLSprite::SetScale(const Vector2& val)
	{
		m_Scale = val;
		if (!m_Sprite.is_null())
			m_Sprite.set_scale(val.x, val.y);
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
			return m_Scale;
	}

	void CLSprite::SetBaseAngle(float val)
	{
		m_BaseAngle = val;
		if (!m_Sprite.is_null())
			m_Sprite.set_base_angle(clan::Angle(val, clan::angle_radians));
	}

	float CLSprite::GetBaseAngle() const
	{
		if (!m_Sprite.is_null())
		{
			clan::Angle angle = m_Sprite.get_base_angle();
			return angle.to_radians();
		}
		else
			return m_BaseAngle;
	}

	bool CLSprite::IsAnimationFinished() const
	{
		if (!m_Sprite.is_null())
			return m_Sprite.is_finished();
		else
			return false;
	}

	void CLSprite::SetAnimationFrame(int frame)
	{
		m_AnimationFrame = frame;
		if (!m_Sprite.is_null())
			m_Sprite.set_frame(frame);
	}
	
	int CLSprite::GetAnimationFrame() const
	{
		if (!m_Sprite.is_null())
			return m_Sprite.get_current_frame();
		else
			return m_AnimationFrame;
	}

	void CLSprite::SetPlaying(bool play)
	{
		if (!m_Sprite.is_null())
		{
			if (play)
				m_Sprite.restart();
			else
				m_Sprite.finish();
		}
	}
	
	bool CLSprite::IsPlaying() const
	{
		if (!m_Sprite.is_null())
			return !m_Sprite.is_finished();
		else
			return false;
	}

	void CLSprite::SetLooping(bool loop)
	{
		m_Looping = loop;
		if (!m_Sprite.is_null())
			m_Sprite.set_play_loop(loop);
	}
	
	bool CLSprite::IsLooping() const
	{
		if (!m_Sprite.is_null())
			return m_Sprite.is_play_loop();
		else
			return m_Looping;
	}

	void CLSprite::Finish()
	{
		boost::mutex::scoped_lock lock(m_Mutex);
		if (!m_Sprite.is_null())
			return m_Sprite.finish();
	}

}
