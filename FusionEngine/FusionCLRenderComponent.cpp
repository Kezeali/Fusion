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
		: m_Reload(true)
	{
	}

	CLSprite::~CLSprite()
	{
		m_ImageLoadConnection.disconnect();
		m_AnimationLoadConnection.disconnect();
	}

	void CLSprite::redefineSprite()
	{
		if (m_ImageResource.IsLoaded() && m_AnimationResource.IsLoaded())
		{
			m_SpriteDef.reset(new SpriteDefinition(m_ImageResource, m_AnimationResource));
			m_RecreateSprite = true;
		}
	}

	void CLSprite::Update(const float elapsed)
	{
		auto onImageLoaded = [this](ResourceDataPtr& data)
		{
			m_ImageResource.SetTarget(data);
			if (m_AnimationResource.IsLoaded())
			{
				redefineSprite();
			}
		};

		auto onAnimationLoaded = [this](ResourceDataPtr& data)
		{
			m_AnimationResource.SetTarget(data);
			if (m_ImageResource.IsLoaded())
			{
				m_SpriteDef.reset(new SpriteDefinition(/*m_ImageResource, m_AnimationResource*/));
				m_RecreateSprite = true;
			}
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

		if (!m_Sprite.is_null() && !m_Sprite.is_finished())
		{
			m_Sprite.update(int(elapsed * 1000));

			if (m_Sprite.is_finished())
				AnimationFinished.MarkChanged();
		}
	}

	void CLSprite::Draw(CL_GraphicContext& gc, const Vector2& camera_pos)
	{
		if (m_RecreateSprite)
		{
			m_Sprite = m_SpriteDef->CreateSprite(gc);
			m_RecreateSprite = false;
		}
		if (!m_Sprite.is_null())
		{
			Vector2 draw_pos = m_InterpPosition - camera_pos;
			m_Sprite.draw(gc, draw_pos.x, draw_pos.y);
		}
	}

	void CLSprite::OnSiblingAdded(const std::set<std::string>& interfaces, const std::shared_ptr<IComponent>& component)
	{
		if (interfaces.find(ITransform::GetTypeName()) != interfaces.end())
		{
			auto transform = dynamic_cast<ITransform*>(component.get());
			transform->Position.Connect(std::bind(&CLSprite::SetPosition, this, std::placeholders::_1));
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
		return m_SerialisationHelper.writeChanges(force_all, stream, m_Offset, m_LocalDepth, m_ImagePath, m_ReloadImage, m_AnimationPath, m_ReloadAnimation);
	}

	void CLSprite::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		std::bitset<6> changed;
		if (!all)
		{
			m_SerialisationHelper.readChanges(stream, changed, m_Offset, m_LocalDepth, m_ImagePath, m_ReloadImage, m_AnimationPath, m_ReloadAnimation);
			if (changed[PropsOrder::ImagePath]) // file path changed
				m_ReloadImage = true;
			if (changed[PropsOrder::AnimationPath])
				m_ReloadAnimation = true;
		}
		else
		{
			//m_SerialisationHelper.readAll(stream, m_Offset, m_LocalDepth, m_ImagePath, m_ReloadImage);
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
		m_SerialisationHelper.markChanged(0);
	}

	void CLSprite::SetLocalDepth(int value)
	{
		m_LocalDepth = value;
		m_SerialisationHelper.markChanged(1);
	}

	void CLSprite::SetFilePath(const std::string& value)
	{
		m_FilePath = value;
		m_Reload = true;
		m_SerialisationHelper.markChanged(2);
		m_SerialisationHelper.markChanged(3);
	}

}
