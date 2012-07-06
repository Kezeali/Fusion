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
#include "FusionComponentFactory.h"
#include "FusionEntity.h"
#include "FusionEntitySerialisationUtils.h"

#include <boost/lexical_cast.hpp>

namespace FusionEngine
{

	ArchetypalEntityManager::ArchetypalEntityManager(const std::shared_ptr<Archetype>& definition)
		: m_Definition(definition)
	{
	}

	ArchetypalEntityManager::~ArchetypalEntityManager()
	{
	}

	void ArchetypalEntityManager::SetManagedEntity(const EntityPtr& entity)
	{
		m_ManagedEntity = entity;
	}

	void ArchetypalEntityManager::OverrideProperty(Archetypes::PropertyID_t id, RakNet::BitStream& str)
	{
		m_ModifiedProperties[id].reset(new RakNet::BitStream(str.GetData(), str.GetNumberOfBytesUsed(), true));
	}

	void ArchetypalEntityManager::OnComponentAdded(Archetypes::ComponentID_t arch_id, const std::string& type, const std::string& identifier)
	{
		if (auto entity = m_ManagedEntity.lock())
		{
			auto com = m_ComponentFactory->InstantiateComponent(type);
			entity->AddComponent(com, identifier);
			m_Components[arch_id] = com.get();
		}
	}

	void ArchetypalEntityManager::OnComponentRemoved(Archetypes::ComponentID_t arch_id)
	{
		auto entry = m_Components.find(arch_id);
		if (entry != m_Components.end())
		{
			if (auto entity = m_ManagedEntity.lock())
			{
				entity->RemoveComponent(entry->second);
			}
			m_Components.erase(entry);
		}
	}

	void ArchetypalEntityManager::OnPropertyChanged(Archetypes::PropertyID_t id, RakNet::BitStream& data)
	{
		if (auto entity = m_ManagedEntity.lock())
		{
		}
	}

	void ArchetypalEntityManager::OnSerialisedDataChanged(RakNet::BitStream& data)
	{
		if (auto entity = m_ManagedEntity.lock())
		{
			EntitySerialisationUtils::DeserialiseContinuous(data, entity, EntitySerialisationUtils::All);
			EntitySerialisationUtils::DeserialiseOccasional(data, entity, EntitySerialisationUtils::All);
		}
		PerformPropertyOverrides();
	}

	void ArchetypalEntityManager::Serialise(RakNet::BitStream& stream)
	{
		stream.Write(m_ModifiedProperties.size());
		for (auto it = m_ModifiedProperties.begin(); it != m_ModifiedProperties.end(); ++it)
		{
			stream.Write(it->first);
			stream.Write(*it->second);
		}
	}

	void ArchetypalEntityManager::Deserialise(RakNet::BitStream& stream)
	{
		auto numModifiedProps = m_ModifiedProperties.size();
		stream.Read(numModifiedProps);
		for (size_t i = numModifiedProps; i < numModifiedProps; ++i)
		{
			ModifiedProperties_t::value_type value;

			stream.Read(value.first);
			stream.Read(*value.second);

			m_ModifiedProperties.insert(std::move(value));
		}

		PerformPropertyOverrides();
	}

	void ArchetypalEntityManager::PerformPropertyOverrides()
	{
		if (auto entity = m_ManagedEntity.lock())
		{
			std::string componentType;
			std::string componentIdentifier;
			size_t propertyIndex;
			for (auto it = m_ModifiedProperties.begin(); it != m_ModifiedProperties.end(); ++it)
			{
				std::tie(componentType, componentIdentifier, propertyIndex) = m_Definition->GetPropertyLocation(it->first);
				
				auto component = entity->GetComponent(componentType, componentIdentifier);
				if (component)
				{
					auto& props = component->GetProperties();
					if (propertyIndex < props.size())
					{
						props[propertyIndex].second->Deserialise(*it->second);
					}
					else
					{
						SendToConsole("Invalid property index in archetypal entity component: component=" + componentType + "/" + componentIdentifier + " index=" + boost::lexical_cast<std::string>(propertyIndex));
					}
				}
				else
				{
					SendToConsole("Missing expected component in archetypal entity: " + componentType + "/" + componentIdentifier);
				}
			}
		}
	}

	void ArchetypeDefinitionAgent::SetManagedEntity(const EntityPtr& entity)
	{
		m_ManagedEntity = entity;
	}

	//! Save the local overrides
	void ArchetypeDefinitionAgent::Serialise(RakNet::BitStream& stream)
	{
	}

	//! Load the local overrides
	void ArchetypeDefinitionAgent::Deserialise(RakNet::BitStream& stream)
	{
	}

	void ArchetypeDefinitionAgent::PushConfiguration()
	{
		if (auto entity = m_ManagedEntity.lock())
		{
			RakNet::BitStream stream;
			std::vector<std::uint32_t> checksums;
			EntitySerialisationUtils::SerialiseContinuous(stream, entity, EntitySerialisationUtils::All);
			EntitySerialisationUtils::SerialiseOccasional(stream, checksums, entity, EntitySerialisationUtils::All);
			SignalChange(stream);
		}
	}

}
