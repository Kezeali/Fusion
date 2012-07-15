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

#include "FusionArchetypalEntityManager.h"
#include "FusionArchetype.h"
#include "FusionArchetypeFactory.h"
#include "FusionEntity.h"
#include "FusionEntitySerialisationUtils.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

namespace bio = boost::iostreams;

namespace FusionEngine
{

	ArchetypeFactory::ArchetypeFactory(EntityInstantiator* instantiator)
		: m_ComponentInstantiator(instantiator)
	{
	}

	ArchetypeFactory::~ArchetypeFactory()
	{
	}

	EntityPtr ArchetypeFactory::GetArchetype(const std::string& type_id) const
	{
		boost::mutex::scoped_lock lock(m_Mutex);

		auto entry = m_Archetypes.find(type_id);
		if (entry != m_Archetypes.end())
			return entry->second.Archetype;
		else
			return EntityPtr();
	}

	//EntityPtr ArchetypeFactory::CreateArchetype(ComponentFactory* factory, const std::string& type_id, const std::string& transform_type)
	//{
	//	boost::mutex::scoped_lock lock(m_Mutex);

	//	ArchetypeData& data = m_Archetypes[type_id];

	//	data.Archetype = std::make_shared<Entity>(nullptr, factory->InstantiateComponent(transform_type));
	//	// Generate the type definition
	//	data.Profile = std::make_shared<Archetype>(type_id);
	//	data.Profile->Define(data.Archetype);
	//	// Create and apply the definition agent
	//	data.Agent = std::make_shared<ArchetypeDefinitionAgent>();
	//	data.Agent->SetManagedEntity(data.Archetype);
	//	data.Archetype->SetArchetypeAgent(data.Agent);

	//	return data.Archetype;
	//}

	EntityPtr ArchetypeFactory::MakeInstance(ComponentFactory* factory, const std::string& type_id, const Vector2& pos, float angle)
	{
		EntityPtr entity;
		{
			boost::mutex::scoped_lock lock(m_Mutex);
			
			auto entry = m_Archetypes.find(type_id);
			if (entry != m_Archetypes.end())
			{
				// Since archetypes aren't updated regularly, make sure the properties are up to date (so they can be cloned accurately)
				entry->second.Archetype->SynchroniseParallelEdits();
				entity = entry->second.Archetype->Clone(factory);

				auto agent = std::make_shared<ArchetypalEntityManager>(entity, entry->second.Profile, m_ComponentInstantiator);
				// Set up signal handlers
				{
					using namespace std::placeholders;
					agent->m_ChangeConnection = entry->second.Agent->SignalChange.connect(std::bind(&ArchetypalEntityManager::OnSerialisedDataChanged, agent.get(), _1));
					agent->m_ComponentAddedConnection = entry->second.Agent->SignalAddComponent.connect(std::bind(&ArchetypalEntityManager::OnComponentAdded, agent.get(), _1, _2, _3));
					agent->m_ComponentRemovedConnection = entry->second.Agent->SignalRemoveComponent.connect(std::bind(&ArchetypalEntityManager::OnComponentRemoved, agent.get(), _1));
				}

				entity->SetArchetypeAgent(agent);

				entity->SetPosition(pos);
				entity->SetAngle(angle);
			}
		}
		return std::move(entity);
	}

	//void ArchetypeFactory::PushChange(const std::string& type_id, RakNet::BitStream& data)
	//{
	//	ArchetypeData& arc = m_Archetypes[type_id];
	//	arc.SignalChange(data);
	//}

	void ArchetypeFactory::DefineArchetypeFromEntity(ComponentFactory* factory, const std::string& type_id, const EntityPtr& entity)
	{
		boost::mutex::scoped_lock lock(m_Mutex);

		ArchetypeData& data = m_Archetypes[type_id];
		// Generate the archetype by cloning the given entity
		data.Archetype = entity->Clone(factory);
		data.Archetype->SetArchetype(type_id);
		// Generate the type definition
		data.Profile = std::make_shared<Archetypes::Profile>(type_id);
		auto componentIds = data.Profile->Define(data.Archetype);
		// Create and apply the definition agent
		data.Agent = std::make_shared<ArchetypeDefinitionAgent>(data.Archetype, data.Profile, std::move(componentIds));
		data.Archetype->SetArchetypeDefinitionAgent(data.Agent);

		// TODO: add archetypes to the entity manager or something to maintain them
		data.Archetype->StreamIn();
	}

}
