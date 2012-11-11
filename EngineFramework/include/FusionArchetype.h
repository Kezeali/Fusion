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

		//! Data defining the layout of an entity archetype
		class Profile
		{
		public:
			Profile(const std::string& name);
			~Profile();

			const std::string& GetName() const { return m_Name; }

			void Load(std::istream& data);
			void Save(std::ostream& data);

			std::map<ComponentPtr, ComponentID_t> Define(const EntityPtr& definition);

			ComponentID_t AddComponent(const ComponentPtr& component);
			void RemoveComponent(ComponentID_t component);

			//! Returns the component ID & offset which can be used to find the given property
			std::pair<Archetypes::ComponentID_t, size_t> GetPropertyLocation(Archetypes::PropertyID_t id) const;
			std::tuple<std::string, std::string, Archetypes::ComponentID_t, size_t> GetPropertyLocationAndComponentInfo(Archetypes::PropertyID_t id) const;
			//! Returns the type and identifier of the given component
			std::pair<std::string, std::string> GetComponentInfo(Archetypes::ComponentID_t id) const;
			std::pair<std::string, std::string> GetComponentInfoViaProperty(Archetypes::PropertyID_t id) const;

			//! Returns the archetype ID of the property with the given name
			PropertyID_t FindProperty(const std::string& name) const;
			//! Returns the archetype ID of the component with the given type & id
			ComponentID_t FindComponent(const std::string& type, const std::string& identifier) const;

		private:
			struct ComponentData
			{
				struct PropertyData
				{
					Archetypes::PropertyID_t id;
					std::string name;
				};

				std::string type;
				std::string identifier;
				std::vector<PropertyData> properties;
			};

			struct ReversePropertyData
			{
				// About the component that has this property:
				Archetypes::ComponentID_t component_id;
				std::string component_type;
				std::string component_identifier;
				// About the property itself:
				std::string name;
				size_t index;
			};

			std::string m_Name;

			typedef std::map<Archetypes::ComponentID_t, ComponentData> ComponentDataMap;
			typedef std::map<Archetypes::PropertyID_t, ReversePropertyData> PropertyDataMap;
			ComponentDataMap m_Components;
			PropertyDataMap m_Properties;

			Archetypes::ComponentID_t m_NextComId;
			Archetypes::PropertyID_t m_NextPropId;

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

}

#endif
