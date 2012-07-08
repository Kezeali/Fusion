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

#include "FusionArchetype.h"

#include "FusionBinaryStream.h"
#include "FusionEntity.h"
#include "FusionEntityComponent.h"

namespace FusionEngine
{

	namespace Archetypes
	{
		const int s_ArchetypeFileVersion = 2;

		Profile::Profile(const std::string& name)
			: m_Name(name),
			m_NextComId(0),
			m_NextPropId(0)
		{
		}

		Profile::~Profile()
		{
		}

		void Profile::Load(std::istream& stream)
		{
			IO::Streams::CellStreamReader reader(&stream);
		
			auto version = Archetypes::s_ArchetypeFileVersion;
			reader.Read(version);

			if (version == Archetypes::s_ArchetypeFileVersion)
			{
				m_Components.clear();

				m_Name = reader.ReadString();

				reader.Read(m_NextComId);
				reader.Read(m_NextPropId);

				auto numComponents = reader.ReadValue<std::size_t>();
				for (size_t i = 0; i < numComponents; ++i)
				{
					auto archComponentId = reader.ReadValue<Archetypes::ComponentID_t>();

					auto& componentData = m_Components[archComponentId];

					componentData.identifier = reader.ReadString();
					componentData.type = reader.ReadString();

					auto numProperties = reader.ReadValue<std::size_t>();
					componentData.properties.resize(numProperties);
					for (auto pit = componentData.properties.begin(); pit != componentData.properties.end(); ++pit)
					{
						pit->id = reader.ReadValue<Archetypes::PropertyID_t>();
					}
				}
			}
			else // unknown version
			{
				FSN_EXCEPT(FileTypeException, "Unable to load archetype file (version code is incorrect)");
			}
		}

		void Profile::Save(std::ostream& stream)
		{
			IO::Streams::CellStreamWriter writer(&stream);
			FSN_ASSERT(Archetypes::s_ArchetypeFileVersion == 1);
			writer.Write(Archetypes::s_ArchetypeFileVersion);
			writer.WriteString(m_Name);

			writer.Write(m_NextComId);
			writer.Write(m_NextPropId);

			writer.WriteAs<std::size_t>(m_Components.size());
			for (auto it = m_Components.cbegin(); it != m_Components.cend(); ++it)
			{
				writer.Write(it->first);

				const auto& componentData = it->second;

				writer.WriteString(componentData.identifier);
				writer.WriteString(componentData.type);

				writer.WriteAs<std::size_t>(componentData.properties.size());
				for (auto pit = componentData.properties.begin(); pit != componentData.properties.end(); ++pit)
				{
					writer.Write(pit->id);
				}
			}
		}

		std::map<ComponentPtr, ComponentID_t> Profile::Define(const EntityPtr& definition)
		{
			std::map<ComponentPtr, ComponentID_t> resultHackityHack;

			m_Components.clear();

			const auto& components = definition->GetComponents();
			for (auto it = components.begin(); it != components.end(); ++it)
			{
				const auto& component = *it;
				resultHackityHack[component] = AddComponent(component);
			}

			return std::move(resultHackityHack);
		}

		ComponentID_t Profile::AddComponent(const ComponentPtr& component)
		{
			const auto type = component->GetType();
			const auto identifier = component->GetIdentifier();

			const auto comId = m_NextComId++;
			ComponentData& comda = m_Components[comId];

			comda.type = type;
			comda.identifier = identifier;

			const auto& properties = component->GetProperties();
			size_t propIndex = 0;
			for (auto pit = properties.begin(); pit != properties.end(); ++pit)
			{
				const auto& prop = *pit;

				const auto propId = m_NextPropId++;

				{
					ComponentData::PropertyData propda;
					propda.id = propId;
					comda.properties.push_back(propda);
				}

				{
					ReversePropertyData propda;
					propda.type = type;
					propda.identifier = identifier;
					propda.index = propIndex++;
					m_Properties[propId] = propda;
				}
			}

			return comId;
		}

		void Profile::RemoveComponent(ComponentID_t component)
		{
			auto entry = m_Components.find(component);
			if (entry != m_Components.end())
			{
				// Remove all the entries for this component's properties
				auto& properties = entry->second.properties;
				for (auto it = properties.begin(); it != properties.end(); ++it)
				{
					m_Properties.erase(it->id);
				}
				// Remove the component entry itself
				m_Components.erase(entry);
			}
		}

		std::tuple<std::string, std::string, size_t> Profile::GetPropertyLocation(Archetypes::PropertyID_t id)
		{
			auto entry = m_Properties.find(id);
			if (entry != m_Properties.end())
				return std::make_tuple(entry->second.type, entry->second.identifier, entry->second.index);
			else
			{
				FSN_EXCEPT(InvalidArgumentException, "Unknown property ID");
			}
		}

		std::string Profile::GetComponentLocation(Archetypes::ComponentID_t id)
		{
			auto entry = m_Components.find(id);
			if (entry != m_Components.end())
				return entry->second.identifier;
			else
			{
				FSN_EXCEPT(InvalidArgumentException, "Unknown component ID");
			}
		}

	}

}
