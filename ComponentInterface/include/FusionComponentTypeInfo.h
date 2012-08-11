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

#ifndef H_FusionComponentTypeInfo
#define H_FusionComponentTypeInfo

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSingleton.h"

#include <map>
#include <tbb/concurrent_unordered_map.h>

namespace FusionEngine
{

	class EntityComponent;

	class ComponentTypeInfoCache : public Singleton<ComponentTypeInfoCache>
	{
	public:
		ComponentTypeInfoCache();
		~ComponentTypeInfoCache();

		typedef std::map<std::string, size_t> ComponentPropertiesMap_t;

		const std::shared_ptr<ComponentPropertiesMap_t> &GetComponentTypeInfo(const EntityComponent* instance);

	private:
		tbb::concurrent_unordered_map<std::string, std::shared_ptr<ComponentPropertiesMap_t>> m_ComponentTypes;
	};

}

#endif