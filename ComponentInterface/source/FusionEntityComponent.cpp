/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#include "FusionComponentFactory.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>

namespace FusionEngine
{

	ComponentPtr IComponent::Clone(ComponentFactory* factory)
	{
		ComponentPtr component = factory->InstantiateComponent(GetType());
		RakNet::BitStream stream;
		this->SerialiseContinuous(stream);
		component->DeserialiseContinuous(stream);
		stream.Reset();
		this->SerialiseOccasional(stream);
		component->DeserialiseOccasional(stream);
		return component;
	}

	boost::intrusive_ptr<ComponentProperty> IComponent::AddProperty(const std::string& name, IComponentProperty* impl)
	{
#ifdef _DEBUG
		auto isContainer = [impl](const std::pair<std::string, PropertyPtr>& existing)->bool { return existing.second->GetImpl() == impl; };
		FSN_ASSERT(std::find_if(m_Properties.begin(), m_Properties.end(), isContainer) == m_Properties.end());
#endif

		boost::intrusive_ptr<ComponentProperty> prop(new ComponentProperty(impl, EvesdroppingManager::getSingleton().GenerateNextId()));

		m_Properties.push_back(std::make_pair(name, prop));

		// Add the property to the appropriate serialisation list
		PropertyListNode*& propListTail = prop->IsContinuous() ? m_LastContinuousProperty : m_LastOccasionalProperty;
		PropertyListNode* previousNewestProperty = propListTail;
		propListTail = new PropertyListNode(prop.get());
		propListTail->previous = previousNewestProperty;

		prop->AquireSignalGenerator(EvesdroppingManager::getSingleton().GetSignalingSystem());

		return prop;
	}

	void IComponent::SynchronisePropertiesNow()
	{
		for (auto it = m_Properties.begin(), end = m_Properties.end(); it != end; ++it)
		{
			it->second->Synchronise();
		}
	}

	void IComponent::SerialiseContinuous(RakNet::BitStream& stream)
	{
		for (auto it = m_LastContinuousProperty; it != nullptr; it = it->previous)
		{
			const auto& prop = it->prop;
			prop->Serialise(stream);
		}
	}

	void IComponent::DeserialiseContinuous(RakNet::BitStream& stream)
	{
		for (auto it = m_LastContinuousProperty; it != nullptr; it = it->previous)
		{
			const auto& prop = it->prop;
			prop->Deserialise(stream);
		}

		OnPostDeserialisation();
	}

	void IComponent::SerialiseOccasional(RakNet::BitStream& stream)
	{
		for (auto it = m_LastOccasionalProperty; it != nullptr; it = it->previous)
		{
			const auto& prop = it->prop;
			prop->Serialise(stream);
		}
	}

	void IComponent::DeserialiseOccasional(RakNet::BitStream& stream)
	{
		for (auto it = m_LastOccasionalProperty; it != nullptr; it = it->previous)
		{
			const auto& prop = it->prop;
			prop->Deserialise(stream);
		}

		OnPostDeserialisation();
	}

	void IComponent::SerialiseEditable(RakNet::BitStream& stream)
	{
		stream.Write(m_Properties.size());
		for (auto it = m_Properties.begin(), end = m_Properties.end(); it != end; ++it)
		{
			const auto& prop = it->second;
			FSN_ASSERT(it->first.length() > 0);
			stream.Write(it->first.length());
			if (it->first.length() > 0)
			{
				stream.Write(it->first.c_str(), it->first.length()); // Write the name
				prop->Serialise(stream);
			}
		}
	}

	void IComponent::DeserialiseEditable(RakNet::BitStream& stream)
	{
		size_t numProps = 0;
		stream.Read(numProps);
		std::map<std::string, PropertyPtr> propsMap;
		propsMap.insert(m_Properties.begin(), m_Properties.end());
		for (size_t i = 0; i < numProps; ++i)
		{
			// Read the name
			std::vector<char> d; size_t l;
			stream.Read(l); d.resize(l);
			if (l > 0)
			{
				stream.Read(d.data(), l);
				std::string name(d.begin(), d.end());

				// Deserialise the prop
				auto it = propsMap.find(name);
				if (it != propsMap.end())
				{
					const auto& prop = it->second;
					prop->Deserialise(stream);
				}
			}
		}

		OnPostDeserialisation();
	}

}