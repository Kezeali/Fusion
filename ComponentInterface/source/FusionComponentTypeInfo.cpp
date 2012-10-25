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

#include "FusionComponentTypeInfo.h"

#include "FusionEntityComponent.h"

namespace FusionEngine
{

	ComponentTypeInfoCache::ComponentTypeInfoCache()
	{
	}

	ComponentTypeInfoCache::~ComponentTypeInfoCache()
	{
	}

	std::shared_ptr<ComponentTypeInfoCache::ComponentPropertiesMap_t> GeneratePropertyIndexMap(const EntityComponent* instance)
	{
		const auto& properties = instance->GetProperties();

		auto propertyIndexMap = std::make_shared<ComponentTypeInfoCache::ComponentPropertiesMap_t>();
		{
			size_t index = 0;
			for (auto it = properties.begin(); it != properties.end(); ++it)
				propertyIndexMap->insert(std::make_pair(it->first, index++));
		}

		return std::move(propertyIndexMap);
	}

	const std::shared_ptr<ComponentTypeInfoCache::ComponentPropertiesMap_t> &ComponentTypeInfoCache::GetComponentTypeInfo(const EntityComponent* instance)
	{
		auto entry = m_ComponentTypes.find(instance->GetProfileType());
		if (entry != m_ComponentTypes.end())
			return entry->second;
		else
		{
			return m_ComponentTypes.insert(std::make_pair(instance->GetProfileType(), GeneratePropertyIndexMap(instance))).first->second;
		}
	}

	void ComponentTypeInfoCache::ClearCache()
	{
		m_ComponentTypes.clear();
	}

}