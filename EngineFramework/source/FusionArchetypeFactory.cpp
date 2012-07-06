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

	ArchetypeFactory::ArchetypeFactory()
	{
	}

	ArchetypeFactory::~ArchetypeFactory()
	{
	}

	EntityPtr ArchetypeFactory::GetArchetype(const std::string& type_id) const
	{
		auto entry = m_Archetypes.find(type_id);
		if (entry != m_Archetypes.end())
			return entry->second.Archetype;
		else
			return EntityPtr();
	}

	EntityPtr ArchetypeFactory::CreateArchetype(ComponentFactory* factory, const std::string& type_id, const std::string& transform_type)
	{
		auto entity = std::make_shared<Entity>(nullptr, factory->InstantiateComponent(transform_type));
		ArchetypeData& data = m_Archetypes[type_id];
		data.Archetype = entity;
		return entity;
	}

	EntityPtr ArchetypeFactory::MakeInstance(ComponentFactory* factory, const std::string& type_id, const Vector2& pos, float angle)
	{
		EntityPtr entity;
		auto entry = m_Archetypes.find(type_id);
		if (entry != m_Archetypes.end())
		{
			entry->second.Archetype->SynchroniseParallelEdits();
			entity = entry->second.Archetype->Clone(factory);

			auto agent = std::make_shared<ArchetypalEntityManager>(entry->second.Definition);
			agent->m_ChangeConnection = entry->second.Agent->SignalChange.connect(std::bind(&ArchetypalEntityManager::OnSerialisedDataChanged, agent.get(), std::placeholders::_1));
			agent->SetManagedEntity(entity);

			entity->SetArchetypeAgent(agent);
		}
		entity->SetPosition(pos);
		entity->SetAngle(angle);
		return std::move(entity);
	}

	//void ArchetypeFactory::PushChange(const std::string& type_id, RakNet::BitStream& data)
	//{
	//	ArchetypeData& arc = m_Archetypes[type_id];
	//	arc.SignalChange(data);
	//}

	void ArchetypeFactory::DefineArchetypeFromEntity(ComponentFactory* factory, const std::string& type_id, const EntityPtr& entity)
	{
		ArchetypeData& data = m_Archetypes[type_id];
		data.Archetype = entity->Clone(factory);
		data.Definition = std::make_shared<Archetype>(type_id);
		data.Definition->Define(entity);
		data.Agent = std::make_shared<ArchetypeDefinitionAgent>();
		data.Agent->SetManagedEntity(data.Archetype);
		data.Archetype->SetArchetypeAgent(data.Agent);
	}

	EntityPtr EditorArchetypeFactory::MakeInstance(ComponentFactory* factory, const std::string& type_id, const Vector2& pos, float angle)
	{
		auto archetype = GetArchetype(type_id);
		if (archetype)
			return archetype->Clone(factory);
		else
			return EntityPtr();
	}

}
