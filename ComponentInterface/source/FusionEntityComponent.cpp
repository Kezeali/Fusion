/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionEntityComponent.h"

#include "FusionComponentProperty.h"
#include "FusionPropertySignalingSystem.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>

namespace FusionEngine
{

	void IComponent::AddProperty(IComponentProperty* prop)
	{
		FSN_ASSERT(std::find(m_Properties.begin(), m_Properties.end(), prop) == m_Properties.end());

		m_Properties.push_back(prop);

		// Add the property to the appropriate serialisation list
		PropertyListNode*& propListTail = prop->IsContinuous() ? m_LastContinuousProperty : m_LastOccasionalProperty;
		PropertyListNode* previousNewestProperty = propListTail;
		propListTail = new PropertyListNode(prop);
		propListTail->previous = previousNewestProperty;

		prop->AquireSignalGenerator(EvesdroppingManager::getSingleton().GetSignalingSystem());
	}

	void IComponent::OnPropertyChanged(IComponentProperty* prop)
	{
	}

	void IComponent::SynchronisePropertiesNow()
	{
		for (auto it = m_Properties.begin(), end = m_Properties.end(); it != end; ++it)
		{
			const auto& prop = *it;
			prop->Synchronise();
		}
	}

	void IComponent::SerialiseContinuous(RakNet::BitStream& stream)
	{
		for (auto it = m_LastContinuousProperty; it != nullptr; it = m_LastContinuousProperty->previous)
		{
			const auto& prop = it->prop;
			prop->Serialise(stream);
		}
	}

	void IComponent::DeserialiseContinuous(RakNet::BitStream& stream)
	{
		for (auto it = m_LastContinuousProperty; it != nullptr; it = m_LastContinuousProperty->previous)
		{
			const auto& prop = it->prop;
			prop->Deserialise(stream);
		}
	}

	void IComponent::SerialiseOccasional(RakNet::BitStream& stream)
	{
		for (auto it = m_LastOccasionalProperty; it != nullptr; it = m_LastOccasionalProperty->previous)
		{
			const auto& prop = it->prop;
			prop->Serialise(stream);
		}
	}

	void IComponent::DeserialiseOccasional(RakNet::BitStream& stream)
	{
		for (auto it = m_LastOccasionalProperty; it != nullptr; it = m_LastOccasionalProperty->previous)
		{
			const auto& prop = it->prop;
			prop->Deserialise(stream);
		}
	}

	void IComponent::SerialiseEditable(RakNet::BitStream& stream)
	{
		stream.Write(m_Properties.size());
		for (auto it = m_Properties.begin(), end = m_Properties.end(); it != end; ++it)
		{
			const auto& prop = *it;//it->second
			//stream.Write(it->first.length());
			//stream.Write(it->first); // Write the name
			prop->Serialise(stream);
		}
	}

	void IComponent::DeserialiseEditable(RakNet::BitStream& stream)
	{
		size_t numProps = 0;
		stream.Read(numProps);
		std::map<std::string, IComponentProperty*> propsMap;
		//propsMap.insert(m_Properties.begin(), m_Properties.end());
		for (size_t i = 0; i < numProps; ++i)
		{
			std::vector<char> d; size_t l;
			stream.Read(l); d.resize(l);
			stream.Read(d.data(), l);
			std::string name = d.data();

			auto it = propsMap.find(name);
			if (it != propsMap.end())
			{
				const auto& prop = it->second;
				prop->Deserialise(stream);
			}
		}
	}

}