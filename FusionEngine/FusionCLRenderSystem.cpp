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

#include "FusionCLRenderSystem.h"

#include "FusionCLRenderComponent.h"
#include "FusionRenderer.h"

namespace FusionEngine
{
	CLRenderSystem::CLRenderSystem(const CL_GraphicContext& gc)
		: m_GraphicContext(gc)
	{
	}

	ISystemWorld* CLRenderSystem::CreateWorld()
	{
		return new CLRenderWorld(m_GraphicContext);
	}

	CLRenderWorld::CLRenderWorld(const CL_GraphicContext& gc)
	{
		m_Renderer = new Renderer(gc);
	}

	std::vector<std::string> CLRenderWorld::GetTypes() const
	{
		static const std::string types[] = { "CLSprite", "" };
		return std::vector<std::string>(types, types + sizeof(types));
	}

	std::shared_ptr<IComponent> CLRenderWorld::InstantiateComponent(const std::string& type, const Vector2& pos, float angle)
	{
		std::shared_ptr<IComponent> com;
		if (type == "CLSprite")
		{
			com = std::make_shared<CLSprite>();
		}
		return com;
	}

	void CLRenderWorld::OnActivation(const std::shared_ptr<IComponent>& component)
	{
		if (component->GetType() == "CLSprite")
		{
			auto drawable = std::dynamic_pointer_cast<IDrawable>(component);
			if (drawable)
			{
				m_Drawables.push_back(drawable);
				// In task update: tbb::parallel_sort(drawables, depth cmp)
				//  I suspect most of the time there will be so few things to draw that TBB will just
				//  let the sort run serialy, and it will take negligable time (on modern hardware). Away with ye, bubble sort!
			}
		}
	}

	void CLRenderWorld::MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
	{
		if (type == "CLSprite")
		{
			CLSprite::DeltaSerialiser_t::copyChanges(result, current_data, new_data);
		}
	}

}
