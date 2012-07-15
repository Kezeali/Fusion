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
#include "FusionEntityInstantiator.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionLogger.h"

#include <boost/lexical_cast.hpp>

#include <list>
#include <numeric>

namespace FusionEngine
{

	ArchetypalEntityManager::ArchetypalEntityManager(const EntityPtr& entity, const std::shared_ptr<Archetypes::Profile>& definition, EntityInstantiator* instantiator)
		: m_Profile(definition),
		m_ComponentInstantiator(instantiator)
	{
		m_ManagedEntity = entity;

		auto& components = entity->GetComponents();

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			auto& component = *it;
			auto arcId = m_Profile->FindComponent(component->GetType(), component->GetIdentifier());
			FSN_ASSERT(arcId != std::numeric_limits<Archetypes::ComponentID_t>::max());
			m_Components[arcId] = component.get();
		}

		// Add property listeners to add overrides
		for (auto it = components.begin(); it != components.end(); ++it)
		{
			AddPropertyListeners(*it);
		}
	}

	ArchetypalEntityManager::~ArchetypalEntityManager()
	{
	}

	void ArchetypalEntityManager::ComponentAddedToInstance(const ComponentPtr& component)
	{
		m_NonArchetypalComponents.insert(component);
	}

	void ArchetypalEntityManager::ComponentRemovedFromInstance(const ComponentPtr& component)
	{
		// Look for components that have been added to the instance but not the archetype
		//  (because it is a tree lookup, rather than linear)
		auto entry = m_NonArchetypalComponents.find(component.get());
		if (entry != m_NonArchetypalComponents.end())
			m_NonArchetypalComponents.erase(entry);
		else
		{
			auto arcEntry = std::find_if(m_Components.begin(), m_Components.end(), [component](const std::pair<Archetypes::ComponentID_t, IComponent*>& entry)
			{ return entry.second == component.get(); });
			if (arcEntry != m_Components.end())
				arcEntry->second = nullptr;
		}
	}
	
	void ArchetypalEntityManager::AutoOverride(const std::string& name, bool enable)
	{
		const auto id = m_Profile->FindProperty(name);
		if (enable)
			m_AutoOverride.insert(id);
		else
			m_AutoOverride.erase(id);
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
		if (m_ComponentInstantiator != nullptr)
		{
			if (auto entity = m_ManagedEntity.lock())
			{
				auto com = m_ComponentInstantiator->AddComponent(entity, type, identifier);
				m_Components[arch_id] = com.get();
			}
		}
	}

	void ArchetypalEntityManager::OnComponentRemoved(Archetypes::ComponentID_t arch_id)
	{
		if (m_ComponentInstantiator != nullptr)
		{
			auto entry = m_Components.find(arch_id);
			if (entry != m_Components.end())
			{
				if (auto entity = m_ManagedEntity.lock())
				{
					m_ComponentInstantiator->RemoveComponent(entity, entry->second);
				}
				m_Components.erase(entry);
			}
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
		// Write IDs of removed components
		{
			std::list<Archetypes::ComponentID_t> idsOfRemovedComponents;
			for (auto it = m_Components.begin(); it != m_Components.end(); ++it)
				if (it->second == nullptr)
					idsOfRemovedComponents.push_back(it->first);

			stream.Write((size_t)idsOfRemovedComponents.size());
			for (auto it = idsOfRemovedComponents.begin(); it != idsOfRemovedComponents.end(); ++it)
				stream.Write(*it);
		}
		// Write added components
		stream.Write((size_t)m_NonArchetypalComponents.size());
		for (auto it = m_NonArchetypalComponents.begin(); it != m_NonArchetypalComponents.end(); ++it)
		{
			const auto& component = *it;
			// Write type & id
			stream.Write(component->GetType());
			stream.Write(component->GetIdentifier());
			// Write state
			RakNet::BitStream tempStream;
			component->SerialiseContinuous(tempStream);
			EntitySerialisationUtils::WriteStateWithLength(stream, tempStream);
			tempStream.Reset();
			component->SerialiseOccasional(tempStream);
			EntitySerialisationUtils::WriteStateWithLength(stream, tempStream);
		}
		// Write modified properties
		stream.Write(m_ModifiedProperties.size());
		for (auto it = m_ModifiedProperties.begin(); it != m_ModifiedProperties.end(); ++it)
		{
			stream.Write(it->first);
			stream.Write(*it->second);
		}
	}

//#define DEFER_COMPONENT_OPERATIONS

	void ArchetypalEntityManager::Deserialise(RakNet::BitStream& stream)
	{
#ifdef DEFER_COMPONENT_OPERATIONS
		std::list<std::tuple<std::string, std::string, std::unique_ptr<RakNet::BitStream>>> addedComponents;
		std::list<std::pair<Archetypes::ComponentID_t, IComponent*>> removedComponents;
#else
		if (auto entity = m_ManagedEntity.lock())
#endif
		{
			// Read removed components
			{
				size_t numRemovedComponents = 0;
				stream.Read(numRemovedComponents);
				Archetypes::ComponentID_t id;
				for (size_t i = 0; i < numRemovedComponents; ++i)
				{
#ifdef DEFER_COMPONENT_OPERATIONS
					removedComponents.push_back(std::make_pair(id, m_Components[id]));
#else
					stream.Read(id);
					auto entry = m_Components.find(id);
					if (entry != m_Components.end())
					{
						m_ComponentInstantiator->RemoveComponent(entity, entry->second);
						entry->second = nullptr;
					}
#endif
				}
			}
			// Read added components
			size_t numAdded = 0;
			stream.Read(numAdded);
			for (size_t i = 0; i < numAdded; ++i)
			{
				std::string type, identifier;
				SerialisationUtils::read(stream, type);
				SerialisationUtils::read(stream, identifier);

#ifdef DEFER_COMPONENT_OPERATIONS
				auto stateLength = EntitySerialisationUtils::ReadStateLength(stream);
				auto conStream = new RakNet::BitStream(stream.GetData(), stateLength, true);
				stateLength = EntitySerialisationUtils::ReadStateLength(stream);
				auto occStream = new RakNet::BitStream(stream.GetData(), stateLength, true);
				addedComponents.push_back(std::make_tuple(type, identifier, ));
#else
				auto con = m_ComponentInstantiator->AddComponent(entity, type, identifier);

				auto stateLength = EntitySerialisationUtils::ReadStateLength(stream);
				auto startPos = stream.GetReadOffset();
				con->DeserialiseContinuous(stream);
				FSN_ASSERT(stream.GetReadOffset() - startPos == stateLength);

				stateLength = EntitySerialisationUtils::ReadStateLength(stream);
				startPos = stream.GetReadOffset();
				con->DeserialiseOccasional(stream);
				FSN_ASSERT(stream.GetReadOffset() - startPos == stateLength);
#endif
			}
		}

		// Write modified properties
		auto numModifiedProps = m_ModifiedProperties.size();
		stream.Read(numModifiedProps);
		for (size_t i = 0; i < numModifiedProps; ++i)
		{
			ModifiedProperties_t::value_type value;

			stream.Read(value.first);
			stream.Read(*value.second);

			m_ModifiedProperties.insert(std::move(value));
		}

#ifdef DEFER_COMPONENT_OPERATIONS
		PerformComponentOperations(addedComponents, removedComponents);
#endif
		PerformPropertyOverrides();
	}

	void ArchetypalEntityManager::PerformComponentOperations(const std::list<std::tuple<std::string, std::string, std::unique_ptr<RakNet::BitStream>>>& added, const std::list<std::pair<Archetypes::ComponentID_t, IComponent*>>& removed)
	{
		if (auto entity = m_ManagedEntity.lock())
		{
			for (auto it = removed.begin(); it != removed.end(); ++it)
			{
				m_ComponentInstantiator->RemoveComponent(entity, it->second);
				m_Components[it->first] = nullptr;
			}
			for (auto it = added.begin(); it != added.end(); ++it)
			{
				auto con = m_ComponentInstantiator->AddComponent(entity, std::get<0>(*it), std::get<1>(*it));
				const auto& state = std::get<2>(*it);
				con->DeserialiseContinuous(*state);
				con->DeserialiseOccasional(*state);
			}
		}
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

					auto entry = m_Components.find(componentId);
					if (entry != m_Components.end())
					{
						auto& props = entry->second->GetProperties();
						if (propertyIndex < props.size())
						{
							props[propertyIndex].second->Deserialise(*it->second);
							it->second->ResetReadPointer();
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
			const auto arcId = m_Profile->FindProperty(pit->first);
			FSN_ASSERT(arcId != std::numeric_limits<Archetypes::PropertyID_t>::max());
			const auto id = pit->second->GetID();
			m_PropertyListenerConnections[id] = 
				EvesdroppingManager::getSingleton().GetSignalingSystem().AddListener(id, std::bind(&ArchetypalEntityManager::OnInstancePropertyChanged, this, arcId));
		}
	}

	void ArchetypalEntityManager::OnInstancePropertyChanged(Archetypes::PropertyID_t id)
	{
		try
		{
			auto allowed = m_AutoOverride.find(id);
			if (allowed != m_AutoOverride.end())
			{
				m_AutoOverride.erase(allowed);

				auto locationData = m_Profile->GetPropertyLocation(id);
				auto entry = m_Components.find(locationData.first);
				if (entry != m_Components.end())
				{
					auto& properties = entry->second->GetProperties();
					if (locationData.second < properties.size())
					{
						RakNet::BitStream stream;
						properties[locationData.second].second->Serialise(stream);
						OverrideProperty(id, stream);
						return;
					}
				}
				SendToConsole("Failed to overwrite an archetype instance property: arc definition is wrong.");
			}
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
