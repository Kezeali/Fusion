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
#include "FusionComponentTypeInfo.h"

#include <tbb/parallel_for.h>

namespace FusionEngine
{

	ComponentUniverse::ComponentUniverse()
	{
		m_ComponentTypeInfoCache = std::make_shared<ComponentTypeInfoCache>();
	}

	ComponentUniverse::~ComponentUniverse()
	{
	}

	void ComponentUniverse::AddWorld(const std::shared_ptr<SystemWorldBase>& world)
	{
		const auto name = world->GetSystem()->GetName();

		const auto types = world->GetTypes();
		for (auto it = types.begin(), end = types.end(); it != end; ++it)
		{
			m_ComponentTypes.by<tag::type>().insert(std::make_pair(*it, world));
		}

		for (auto it = m_Worlds.begin(); it != m_Worlds.end(); ++it)
		{
			it->second->OnWorldAdded(name);
			world->OnWorldAdded(it->first);
		}

		m_Worlds[name] = world;
	}

	void ComponentUniverse::RemoveWorld(const std::shared_ptr<SystemWorldBase>& world)
	{
		const auto name = world->GetSystem()->GetName();
		m_Worlds.erase(name);

		for (auto it = m_Worlds.begin(); it != m_Worlds.end(); ++it)
		{
			world->OnWorldRemoved(it->first);
			it->second->OnWorldRemoved(name);
		}

		m_ComponentTypes.by<tag::world>().erase(world);
	}

	void ComponentUniverse::CheckMessages(const std::shared_ptr<SystemWorldBase>& world)
	{
		SystemWorldBase::Message message;
		while (world->TryPopOutgoingMessage(message))
		{
			switch (message.targetType)
			{
			case SystemWorldBase::Message::TargetType::System:
				if (!message.targetName.empty())
				{
					auto entry = m_Worlds.find(message.targetName);
					if (entry != m_Worlds.end())
					{
						entry->second->ReceiveMessage(message);
					}
				}
				else
				{
					for (const auto& targetWorld : m_Worlds)
						targetWorld.second->ReceiveMessage(message);
				}
				break;
			case SystemWorldBase::Message::TargetType::Engine:
				if (message.targetName.empty())
				{
					switch (boost::any_cast<SystemWorldBase::EngineMessage>(message.data))
					{
					case SystemWorldBase::EngineMessage::RefreshComponentTypes:
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
				else
				{
					// TODO?: move CheckMessages into the engine manager so it can pass messages to extensions
				}
				break;
			}
		}
	}

	std::shared_ptr<SystemWorldBase> ComponentUniverse::GetWorldByComponentType(const std::string& type)
	{
		const auto& types = m_ComponentTypes.by<tag::type>();
		auto _where = types.find(type);
		if (_where != types.end())
			return _where->second;
		else
			return std::shared_ptr<SystemWorldBase>();
	}

	std::map<std::string, std::shared_ptr<SystemWorldBase>> ComponentUniverse::GetWorlds() const
	{
		return m_Worlds;
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
