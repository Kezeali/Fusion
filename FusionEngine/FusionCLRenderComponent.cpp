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

#include <functional>

namespace FusionEngine
{

	CLSprite::CLSprite()
		: m_Reload(true)
	{
	}

	CLSprite::~CLSprite()
	{
		m_ResourceLoadConnection.disconnect();
	}

	void CLSprite::Update(const float elapsed)
	{
		if (m_Reload && !m_FilePath.empty())
		{
			using namespace std::placeholders;
			m_ResourceLoadConnection.disconnect();
			m_ResourceLoadConnection = ResourceManager::getSingleton().GetResource("SPRITE", m_FilePath, std::bind(&CLSprite::OnResourceLoaded, this, _1));
		}

		m_LastPosition = m_Position;
		m_Position = m_NewPosition;

		// TODO: interpolate (probably in a separate method., taking a 'm' param)
		m_InterpPosition = m_Position;

		if (m_SpriteResource.IsLoaded())
		{
			m_SpriteResource->update(int(elapsed * 1000));
		}
	}

	void CLSprite::Draw(CL_GraphicContext& gc)
	{
		if (m_SpriteResource.IsLoaded())
		{
			m_SpriteResource->draw(gc, m_InterpPosition.x, m_InterpPosition.y);
		}
	}

	void CLSprite::OnResourceLoaded(ResourceDataPtr data)
	{
		m_SpriteResource.SetTarget(data);
	}

	void CLSprite::OnSiblingAdded(const std::set<std::string>& interfaces, const std::shared_ptr<IComponent>& component)
	{
		if (interfaces.find(ITransform::GetTypeName()) != interfaces.end())
		{
			auto transform = dynamic_cast<ITransform*>(component.get());
			transform->Position.Connect(std::bind(&CLSprite::SetPosition, this, std::placeholders::_1));
			transform->Depth.Connect([this](int depth) { m_LocalDepth = depth; });
		}
	}

	void CLSprite::SynchroniseParallelEdits()
	{
		ISprite::SynchroniseInterface();
	}

	bool CLSprite::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		//return m_SerialisationHelper.writeChanges(force_all, stream, std::tie(m_Offset, m_FilePath, m_Reload));
		return m_SerialisationHelper.writeChanges(force_all, stream, m_Offset, m_LocalDepth, m_FilePath, m_Reload);
	}

	void CLSprite::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		std::bitset<4> changed;
		if (!all)
		{
			m_SerialisationHelper.readChanges(stream, changed, m_Offset, m_LocalDepth, m_FilePath, m_Reload);
			if (changed[PropsOrder::FilePath]) // file path changed
				m_Reload = true;
		}
		else
		{
			m_SerialisationHelper.readAll(stream, m_Offset, m_LocalDepth, m_FilePath, m_Reload);
		}
	}

	void CLSprite::SetPosition(const Vector2& value)
	{
		m_NewPosition = value;
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
