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
#include "FusionLogger.h"

#include <boost/lexical_cast.hpp>

namespace FusionEngine
{

	ArchetypalEntityManager::ArchetypalEntityManager(const std::shared_ptr<Archetypes::Profile>& definition)
		: m_Profile(definition)
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

	void ArchetypalEntityManager::RemoveOverride(const std::string& property_name)
	{
		const auto id = m_Profile->FindProperty(property_name);
		m_ModifiedProperties.erase(id);
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

	void ArchetypalEntityManager::OnSerialisedDataChanged(RakNet::BitStream& data)
	{
		if (auto entity = m_ManagedEntity.lock())
		{
			EntitySerialisationUtils::DeserialiseContinuous(data, entity, EntitySerialisationUtils::All);
			EntitySerialisationUtils::DeserialiseOccasional(data, entity, EntitySerialisationUtils::All);
			data.ResetReadPointer();
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
			Archetypes::ComponentID_t componentId;
			size_t propertyIndex;
			for (auto it = m_ModifiedProperties.begin(); it != m_ModifiedProperties.end(); ++it)
			{
				try
				{
					std::tie(componentId, propertyIndex) = m_Profile->GetPropertyLocation(it->first);

#ifdef _DEBUG
					std::tie(componentType, componentIdentifier) = m_Profile->GetComponentInfo(componentId);
					FSN_ASSERT(entity->GetComponent(componentType, componentIdentifier));
#endif

					auto entry = m_Components.find(componentId);
					if (entry != m_Components.end())
					{
						auto& props = entry->second->GetProperties();
						if (propertyIndex < props.size())
						{
							props[propertyIndex].second->Deserialise(*it->second);
						}
						else
						{
							std::tie(componentType, componentIdentifier) = m_Profile->GetComponentInfo(componentId);
							SendToConsole("Invalid property index in archetypal entity component: component=" + componentType + "/" + componentIdentifier + " index=" + boost::lexical_cast<std::string>(propertyIndex) + ". Type '" + entity->GetArchetype() + "'");
						}
					}
					else
					{
						std::tie(componentType, componentIdentifier) = m_Profile->GetComponentInfo(componentId);
						SendToConsole("Missing expected component in archetypal entity: " + componentType + "/" + componentIdentifier + ". Type '" + entity->GetArchetype() + "'");
					}
				}
				catch (InvalidArgumentException&)
				{
					SendToConsole("Missing location data for a property in archetypal entity. Type '" + entity->GetArchetype() + "'");
				}
			}
		}
	}

	void ArchetypalEntityManager::AddPropertyListeners(const ComponentPtr& component)
	{
		auto& properties = component->GetProperties();
		for (auto pit = properties.begin(); pit != properties.end(); ++pit)
		{
			const auto id = pit->second->GetID();
			m_PropertyListenerConnections[id] = 
				EvesdroppingManager::getSingleton().GetSignalingSystem().AddListener(id, std::bind(&ArchetypalEntityManager::OnInstancePropertyChanged, this, id));
		}
	}

	void ArchetypalEntityManager::OnInstancePropertyChanged(Archetypes::PropertyID_t id)
	{
		try
		{
			auto locationData = m_Profile->GetPropertyLocation(id);
			auto entry = m_Components.find(locationData.first);
			if (entry != m_Components.end())
			{
				auto& properties = entry->second->GetProperties();
				if (properties.size() < locationData.second)
				{
					RakNet::BitStream stream;
					properties[locationData.second].second->Serialise(stream);
					OverrideProperty(id, stream);
				}
			}
			SendToConsole("Failed to overwrite an archetype instance property: arc definition is wrong.");
		}
		catch (InvalidArgumentException&)
		{
			SendToConsole("Failed to overwrite an archetype instance property: couldn't locate the property in the arc. definition.");
		}
	}

	ArchetypeDefinitionAgent::ArchetypeDefinitionAgent(const EntityPtr& entity, const std::shared_ptr<Archetypes::Profile>& profile, std::map<ComponentPtr, Archetypes::ComponentID_t> ids)
		: m_DefinitionEntity(entity),
		m_Profile(profile),
		m_ComponentIdMap(ids)
	{
		// Add property listeners to push changes
		auto& components = entity->GetComponents();
		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AddPropertyListeners(*it);
		}
	}

	//! Save the local overrides
	void ArchetypeDefinitionAgent::Serialise(RakNet::BitStream& stream)
	{
	}

	//! Load the local overrides
	void ArchetypeDefinitionAgent::Deserialise(RakNet::BitStream& stream)
	{
	}

	void ArchetypeDefinitionAgent::ComponentAdded(const ComponentPtr& component)
	{
		auto id = m_Profile->AddComponent(component);
		m_ComponentIdMap[component] = id;

		AddPropertyListeners(component);

		SignalAddComponent(id, component->GetType(), component->GetIdentifier());
	}

	void ArchetypeDefinitionAgent::ComponentRemoved(const ComponentPtr& component)
	{
		auto entry = m_ComponentIdMap.find(component);
		if (entry != m_ComponentIdMap.end())
		{
			m_Profile->RemoveComponent(entry->second);
			SignalRemoveComponent(entry->second);
		}
	}

	void ArchetypeDefinitionAgent::PushState()
	{
		if (auto entity = m_DefinitionEntity.lock())
		{
			RakNet::BitStream stream;
			std::vector<std::uint32_t> checksums;
			EntitySerialisationUtils::SerialiseContinuous(stream, entity, EntitySerialisationUtils::All);
			EntitySerialisationUtils::SerialiseOccasional(stream, checksums, entity, EntitySerialisationUtils::All);
			SignalChange(stream);
		}
	}

	void ArchetypeDefinitionAgent::AddPropertyListeners(const ComponentPtr& component)
	{
		auto& properties = component->GetProperties();
		for (auto pit = properties.begin(); pit != properties.end(); ++pit)
		{
			const auto id = pit->second->GetID();
			m_PropertyListenerConnections[id] = 
				EvesdroppingManager::getSingleton().GetSignalingSystem().AddListener(id, std::bind(&ArchetypeDefinitionAgent::PushState, this));
		}
	}

}
