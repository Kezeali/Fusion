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
#include "FusionSystemWorld.h"
#include "FusionComponentTypeInfo.h"

#include <tbb/parallel_for.h>

namespace FusionEngine
{

	using namespace System;

	ComponentUniverse::ComponentUniverse()
		: Router("Universe")
	{
		m_ComponentTypeInfoCache = std::make_shared<ComponentTypeInfoCache>();
	}

	ComponentUniverse::~ComponentUniverse()
	{
	}

	void ComponentUniverse::AddWorld(const std::shared_ptr<WorldBase>& world)
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

		AddDownstream(world->GetRouter());

		m_Worlds[name] = world;
	}

	void ComponentUniverse::RemoveWorld(const std::shared_ptr<WorldBase>& world)
	{
		const auto name = world->GetSystem()->GetName();
		m_Worlds.erase(name);

		RemoveDownstream(world->GetRouter());

		for (auto it = m_Worlds.begin(); it != m_Worlds.end(); ++it)
		{
			world->OnWorldRemoved(it->first);
			it->second->OnWorldRemoved(name);
		}

		m_ComponentTypes.by<tag::world>().erase(world);
	}

	void ComponentUniverse::ProcessMessage(Messaging::Message message)
	{
		auto request = boost::any_cast<WorldBase::ComponentDispatchRequest>(message.data);
		switch (request.type)
		{
		case WorldBase::ComponentDispatchRequest::RefreshComponentTypes:
			{
				m_ComponentTypes.by<tag::world>().erase(request.world->GetShared());

				auto types = request.world->GetTypes();
				for (auto it = types.begin(), end = types.end(); it != end; ++it)
				{
					m_ComponentTypes.by<tag::type>().insert(std::make_pair(*it, request.world->GetShared()));
				}
			}
			break;
		}
	}

	std::shared_ptr<WorldBase> ComponentUniverse::GetWorldByComponentType(const std::string& type)
	{
		const auto& types = m_ComponentTypes.by<tag::type>();
		auto _where = types.find(type);
		if (_where != types.end())
			return _where->second;
		else
			return std::shared_ptr<WorldBase>();
	}

	std::map<std::string, std::shared_ptr<WorldBase>> ComponentUniverse::GetWorlds() const
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
