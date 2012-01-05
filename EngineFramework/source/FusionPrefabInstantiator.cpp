/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#include "FusionPrefabInstantiator.h"

#include <ScriptUtils/Inheritance/TypeTraits.h>

#include <boost/lexical_cast.hpp>

#include "FusionEntity.h"
#include "FusionExceptionFactory.h"
#include "FusionLog.h"
#include "FusionLogger.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionXML.h"

namespace FusionEngine
{

	class XMLPrefab : public EntityTemplate
	{
	public:
		XMLPrefab(ticpp::Document& document)
		{
			ticpp::Element *root = document.FirstChildElement();

			m_Type = root->GetAttribute("typename");

			// Create a sub-directory for this class in the 'temp' folder
			PHYSFS_mkdir((s_TempPath + m_Type).c_str());

			m_DefaultDomain = root->GetAttribute("domain");

			// Read each component
			ticpp::Iterator< ticpp::Element > comp_it;
			for (comp_it = comp_it.begin(root); comp_it != comp_it.end(); comp_it++)
			{
				std::string componentType = comp_it->GetAttribute("type");
				std::string componentIdentifier = comp_it->Value();

				if (componentType.empty())
					continue;

				ComponentData_t data;

				m_Composition.push_back(std::make_tuple(std::move(componentType), std::move(componentIdentifier), data));
			}
		}
	};

	template <typename T>
	bool parse_value(ticpp::Element *element, const std::string &attribute, T *target)
	{
		std::string val = element->GetAttribute(attribute);
		if (!val.empty())
		{
			*target = boost::lexical_cast<T>(val);
			return true;
		}
		else
			return false;
	}

	template <typename T>
	void parse_vector(const std::string &value, T *x, T *y)
	{
		std::string::size_type d = value.find(",");
		if (d != std::string::npos)
		{
			std::string xstr = value.substr(0, d), ystr = value.substr(d+1);
			boost::trim(xstr); boost::trim(ystr);
			*x = boost::lexical_cast<T>(xstr);
			*y = boost::lexical_cast<T>(xstr);
		}
		else
		{
			*x = *y = boost::lexical_cast<T>(boost::trim_copy(value));
		}
	}


	PrefabInstantiator::PrefabInstantiator(ComponentFactory* factory, EntityInstantiator* entity_instantiator)
		: m_ComponentFactory(factory),
		m_EntityInstantiator(entity_instantiator)
	{
		m_Log = Logger::getSingleton().OpenLog("EntityFactory");
	}

	PrefabInstantiator::~PrefabInstantiator()
	{
	}

	void PrefabInstantiator::InstantiatePrefab(const EntityPtr& entity, const std::string &prefab_type)
	{
		auto prefabEntry = m_PrefabTypes.find(prefab_type);
		if (prefabEntry == m_PrefabTypes.end())
		{
			FSN_EXCEPT(PrefabInstantiationException, "Unknown prefab type " + prefab_type);
		}

		auto& composition = prefabEntry->second->GetComposition();
		for (auto it = composition.begin(), end = composition.end(); it != end; ++it)
		{
			auto componentTypeName = std::get<0>(*it);
			auto componentIdentifier = std::get<1>(*it);
			auto& componentData = std::get<2>(*it);

			auto component = m_ComponentFactory->InstantiateComponent(componentTypeName);
			if (!component)
			{
				FSN_EXCEPT(PrefabInstantiationException, "Prefab (" + prefab_type + ") requires unknown component type: " + componentTypeName);
			}
				
			entity->AddComponent(component, componentIdentifier);
		}
	}

	void PrefabInstantiator::LoadXMLPrefabs(const std::string& path)
	{
		m_PrefabTypes.clear();

		auto prefabFiles = PhysFSHelp::regex_find(path, std::regex(".+\\.xml", std::regex::ECMAScript | std::regex::icase));

		for (auto it = prefabFiles.begin(), end = prefabFiles.end(); it != end; ++it)
		{
			const std::string& path = *it;
			try
			{
				ticpp::Document document(OpenXml_PhysFS(path));

				auto definition = std::make_shared<XMLPrefab>(/*fe_getbasepath(filename), */document);
				m_PrefabTypes[definition->GetTypeName()] = definition;
			}
			catch (ticpp::Exception &ex)
			{
				m_Log->AddEntry("The prefab definition file '" + path + "' has invalid XML: " + ex.what(), LOG_NORMAL);
			}
			catch (FileSystemException &ex)
			{
				m_Log->AddEntry("Failed to open prefab definition file '" + path + "': " + ex.ToString(), LOG_NORMAL);
			}
		}
	}

}
