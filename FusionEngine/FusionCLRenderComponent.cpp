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

#include <functional>

namespace FusionEngine
{

	CLSprite::CLSprite()
		: m_Reload(true)
	{
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
		if (m_SpriteResource.IsLoaded())
		{
			m_SpriteResource->update(int(elapsed * 1000));
		}
	}

	void CLSprite::OnResourceLoaded(ResourceDataPtr data)
	{
		m_SpriteResource.SetTarget(data);
	}

	void CLSprite::SynchroniseParallelEdits()
	{
		ISprite::SynchroniseInterface();
	}

	void CLSprite::SetPosition(const Vector2& value)
	{
		m_NewPosition = value;
	}

	void CLSprite::SetOffset(const Vector2& value)
	{
		m_Offset = value;
	}

	void CLSprite::SetFilePath(const std::string& value)
	{
		m_FilePath = value;
		m_Reload = true;
	}

}
