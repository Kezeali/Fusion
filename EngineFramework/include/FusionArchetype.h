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

#ifndef H_FusionArchetype
#define H_FusionArchetype

#include "FusionPrerequisites.h"

#include "FusionTypes.h"

#include "FusionArchetypalEntityManager.h"

#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include <boost/signals2/signal.hpp>

namespace FusionEngine
{

	namespace Archetypes
	{
		extern const int s_ArchetypeFileVersion;
	}

	//! Data defining an entity archetype
	class Archetype
	{
	public:
		Archetype(const std::string& name);
		~Archetype();

		void Load(std::istream& data);
		void Save(std::ostream& data);

		void Define(const EntityPtr& definition);

		std::tuple<std::string, std::string, size_t> GetPropertyLocation(Archetypes::PropertyID_t id);
		std::string GetComponentLocation(Archetypes::ComponentID_t id);

	private:
		struct ComponentData
		{
			struct PropertyData
			{
				Archetypes::PropertyID_t id;
			};

			std::string type;
			std::string identifier;
			std::vector<PropertyData> properties;
		};

		struct ReversePropertyData
		{
			std::string type;
			std::string identifier;
			size_t index;
		};

		std::string m_Name;

		typedef std::map<Archetypes::ComponentID_t, ComponentData> ComponentDataMap;
		typedef std::map<Archetypes::PropertyID_t, ReversePropertyData> PropertyDataMap;
		ComponentDataMap m_Components;
		PropertyDataMap m_Properties;

		//struct PropertyIDData
		//{
		//	std::string component_identifier;
		//	size_t index;
		//};
		//typedef std::map<Archetypes::PropertyID_t, PropertyIDData> PropertyIDMap_t;
		//PropertyIDMap_t m_PropertyIDMap;
		//struct ComponentIDData
		//{
		//	std::string identifier;
		//	PropertyIDMap_t properties;
		//};
		//// Defines the property locations for this instance
		//std::map<Archetypes::ComponentID_t, ComponentIDData> m_ComponentIDMap;
	};

}

#endif
