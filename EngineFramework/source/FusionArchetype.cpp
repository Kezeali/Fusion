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
		const int s_ArchetypeFileVersion = 1;
	}

	Archetype::Archetype(const std::string& name)
		: m_Name(name)
	{
	}

	Archetype::~Archetype()
	{
	}

	void Archetype::Load(std::istream& stream)
	{
		IO::Streams::CellStreamReader reader(&stream);
		
		auto version = Archetypes::s_ArchetypeFileVersion;
		reader.Read(version);

		if (version == Archetypes::s_ArchetypeFileVersion)
		{
			m_Name = reader.ReadString();

			auto numComponents = reader.ReadValue<std::size_t>();
			m_Components.resize(numComponents);
			for (auto it = m_Components.begin(); it != m_Components.end(); ++it)
			{
				it->identifier = reader.ReadString();
				it->type = reader.ReadString();

				auto numProperties = reader.ReadValue<std::size_t>();
				it->properties.resize(numProperties);
				for (auto pit = it->properties.begin(); pit != it->properties.end(); ++pit)
				{
					pit->id = reader.ReadValue<Archetypes::PropertyID_t>();

					auto dataLength = pit->data.size();
					reader.Read(dataLength);
					pit->data.resize(dataLength);
					stream.read(pit->data.data(), pit->data.size());
				}
			}
		}
		else // unknown version
		{
			FSN_EXCEPT(FileTypeException, "Invalid archetype file (version code unknown)");
		}
	}

	void Archetype::Save(std::ostream& stream)
	{
		IO::Streams::CellStreamWriter writer(&stream);
		FSN_ASSERT(Archetypes::s_ArchetypeFileVersion == 1);
		writer.Write(Archetypes::s_ArchetypeFileVersion);
		writer.WriteString(m_Name);

		writer.WriteAs<std::size_t>(m_Components.size());
		for (auto it = m_Components.begin(); it != m_Components.end(); ++it)
		{
			writer.WriteString(it->identifier);
			writer.WriteString(it->type);

			writer.WriteAs<std::size_t>(it->properties.size());
			for (auto pit = it->properties.begin(); pit != it->properties.end(); ++pit)
			{
				writer.Write(pit->id);

				writer.Write(pit->data.size());
				stream.write(pit->data.data(), pit->data.size());
			}
		}
	}

	void Archetype::Define(const EntityPtr& definition)
	{
		const auto& components = definition->GetComponents();
		for (auto it = components.begin(); it != components.end(); ++it)
		{
			const auto& component = *it;
			const auto type = component->GetType();
			const auto identifier = component->GetIdentifier();

			auto entry = std::find_if(m_Components.begin(), m_Components.end(), [type, identifier](ComponentData& comda) { return comda.type == type && comda.identifier == identifier; });
			ComponentData newComda;
			ComponentData& comda = entry != m_Components.end() ? *entry : newComda;

			comda.type = type;
			comda.identifier = identifier;
			
			auto comdaPropEntry = comda.properties.begin();
			const auto& properties = component->GetProperties();
			for (auto pit = properties.begin(); pit != properties.end(); ++pit)
			{
				const auto& prop = *pit;

				RakNet::BitStream stream;
				prop->Serialise(stream);

				if (comdaPropEntry != comda.properties.end())
				{
					comdaPropEntry->data.assign(stream.GetData(), stream.GetData() + stream.GetNumberOfBytesUsed());
					++comdaPropEntry;
				}
			}
		}
	}

}
