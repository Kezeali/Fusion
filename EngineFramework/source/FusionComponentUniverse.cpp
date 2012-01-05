/*
*  Copyright (c) 2012 Fusion Project Team
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

#include "FusionComponentUniverse.h"

#include "FusionComponentSystem.h"

namespace FusionEngine
{

	ComponentUniverse::ComponentUniverse()
	{
	}

	ComponentUniverse::~ComponentUniverse()
	{
	}

	void ComponentUniverse::AddWorld(const std::shared_ptr<ISystemWorld>& world)
	{
		m_Worlds.insert(world);

		auto types = world->GetTypes();
		for (auto it = types.begin(), end = types.end(); it != end; ++it)
		{
			m_ComponentTypes.by<tag::type>().insert(std::make_pair(*it, world));
		}
	}

	void ComponentUniverse::RemoveWorld(const std::shared_ptr<ISystemWorld>& world)
	{
		m_Worlds.erase(world);

		m_ComponentTypes.by<tag::world>().erase(world);
	}

	void ComponentUniverse::OnBeginStep()
	{
		m_Mutex.lock();
	}

	void ComponentUniverse::OnEndStep()
	{
		m_Mutex.unlock();
	}

	void ComponentUniverse::CheckMessages()
	{
		for (auto it = m_Worlds.begin(), end = m_Worlds.end(); it != end; ++it)
		{
			const auto& world = (*it);
			while (const auto message = world->PopSystemMessage())
			{
				switch (message)
				{
				case ISystemWorld::MessageType::NewTypes:
					{
						m_ComponentTypes.by<tag::world>().erase(world);

						auto types = world->GetTypes();
						for (auto it = types.begin(), end = types.end(); it != end; ++it)
						{
							m_ComponentTypes.by<tag::type>().insert(std::make_pair(*it, world));
						}
					}
					break;
				}
			}
		}
	}

	std::shared_ptr<ISystemWorld> ComponentUniverse::GetWorldByComponentType(const std::string& type)
	{
		const auto& types = m_ComponentTypes.by<tag::type>();
		auto _where = types.find(type);
		if (_where != types.end())
			return _where->second;
		else
			return std::shared_ptr<ISystemWorld>();
	}

	ComponentPtr ComponentUniverse::InstantiateComponent(const std::string& type)
	{
		if (auto world = GetWorldByComponentType(type))
			return world->InstantiateComponent(type);
		else
			return ComponentPtr();
	}

	void ComponentUniverse::OnActivated(const ComponentPtr& component)
	{
		auto world = GetWorldByComponentType(component->GetType());
		FSN_ASSERT(world);
		world->OnActivation(component);
	}

	void ComponentUniverse::OnDeactivated(const ComponentPtr& component)
	{
		auto world = GetWorldByComponentType(component->GetType());
		FSN_ASSERT(world);
		world->OnDeactivation(component);
	}

}
