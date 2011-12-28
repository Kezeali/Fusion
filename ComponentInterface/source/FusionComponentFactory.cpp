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

#include "FusionComponentFactory.h"

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include <ScriptUtils/Inheritance/TypeTraits.h>

#include "FusionEntity.h"
#include "FusionExceptionFactory.h"
#include "FusionLog.h"
#include "FusionLogger.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionResourceManager.h"
//#include "FusionScriptedEntity.h"
#include "FusionScriptSound.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionXML.h"

namespace FusionEngine
{

	std::string ResolvePath(const std::string &working_directory, const std::string &path)
	{
		if (path[0] == '/')
		{
			return path;
		}

		else
		{
			typedef std::vector<boost::iterator_range<std::string::iterator>> SplitResult;

			StringVector currentPath;
			boost::split(currentPath, working_directory, boost::is_any_of("/"));

			StringVector pathTokens;
			boost::split(pathTokens, path, boost::is_any_of("/"));
			for (StringVector::iterator it = pathTokens.begin(), end = pathTokens.end(); it != end; ++it)
			{
				if (*it == "..")
					currentPath.pop_back();
				else
					currentPath.push_back(*it);
			}

			return boost::join(currentPath, "/");
		}
	}

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


	EntityFactory::EntityFactory()
	{
		m_Log = Logger::getSingleton().OpenLog("EntityFactory");
	}

	EntityFactory::~EntityFactory()
	{
	}

	ComponentPtr EntityFactory::InstanceComponent(const std::string& type, const Vector2& position, float angle)
	{
		auto _where = m_ComponentInstancers.find(type);
		if (_where != m_ComponentInstancers.end())
		{
			m_Log->AddEntry("Component instanciated: " + type, LOG_INFO);
			return _where->second->InstantiateComponent(type, position, angle);
		}
		return ComponentPtr();
	}

	EntityPtr EntityFactory::InstanceEntity(const std::string &prefab_type, const Vector2& position, float angle)
	{
		auto prefabEntry = m_PrefabTypes.find(prefab_type);
		if (prefabEntry == m_PrefabTypes.end())
			return EntityPtr();

		EntityPtr entity;

		auto& composition = prefabEntry->second->GetComposition();

		for (auto it = composition.begin(), end = composition.end(); it != end; ++it)
		{
			auto componentTypeName = std::get<0>(*it);
			auto componentIdentifier = std::get<1>(*it);
			auto& componentData = std::get<2>(*it);
			auto _where = m_ComponentInstancers.find(componentTypeName);
			if (_where != m_ComponentInstancers.end())
			{
				auto component = _where->second->InstantiateComponent(componentTypeName, position, angle);
				if (!component)
					return EntityPtr(); // or throw?
				
				entity->AddComponent(component, componentIdentifier);
			}
		}
		SignalEntityInstanced(entity);
		return entity;
	}

	EntityPtr EntityFactory::InstanceEntity(const std::vector<std::string>& composition, const Vector2& position, float angle)
	{
		EntityPtr entity;

		for (auto it = composition.begin(), end = composition.end(); it != end; ++it)
		{
			auto _where = m_ComponentInstancers.find(*it);
			if (_where != m_ComponentInstancers.end())
			{
				auto component = _where->second->InstantiateComponent(*it, position, angle);
				if (!component)
					return EntityPtr();
				
				entity->AddComponent(component);
			}
		}
		SignalEntityInstanced(entity);
		return entity;
	}

	//EntityPtr EntityFactory::InstanceEntity(const std::string &type, const std::string &name)
	//{
	//	return InstanceEntity(type, SupplementaryDefinitionData(), name);
	//}

	void EntityFactory::AddInstancer(const std::string &type, const ComponentInstancerPtr &instancer)
	{
		m_ComponentInstancers[type] = instancer;
	}

	void EntityFactory::AddInstancer(const ComponentInstancerPtr &instancer)
	{
		auto types = instancer->GetTypes();
		for (auto it = types.begin(), end = types.end(); it != end; ++it)
			AddInstancer(*it, instancer);
	}

	bool EntityFactory::LoadPrefabType(const std::string &type)
	{
		//auto _where = m_EntityDefinitionFileNames.find(type);
		//if (_where != m_EntityDefinitionFileNames.end())
		//{
		//	try
		//	{
		//		ticpp::Document document( OpenXml_PhysFS(_where->second) );
		//		loadAllDependencies(fe_getbasepath(_where->second), document);
		//	}
		//	catch (ticpp::Exception &)
		//	{
		//		return false;
		//	}
		//	catch (FileSystemException &)
		//	{
		//		return false;
		//	}
		//	return true;
		//}

		return false;
	}

	void EntityFactory::LoadAllPrefabTypes(const std::string& path)
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

	void EntityFactory::SetScriptedEntityPath(const std::string &path)
	{
		m_ScriptedEntityPath = path;
	}

	void EntityFactory::GetTypes(StringVector &types, bool sort)
	{
		types.reserve(m_ComponentInstancers.size());
		for (auto it = m_ComponentInstancers.begin(), end = m_ComponentInstancers.end(); it != end; ++it)
		{
			if (sort && !types.empty())
			{
				StringVector::iterator lowerBound = std::lower_bound(types.begin(), types.end(), it->first);
				types.insert(lowerBound, it->first);
			}
			else
				types.push_back(it->first);
		}
	}

	void EntityFactory::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSingletonType<EntityFactory>("EntityFactory", engine);

		r = engine->RegisterObjectMethod("EntityFactory",
			"Entity instance(const string &in, const Vector &in, float)",
			asMETHODPR(EntityFactory, InstanceEntity, (const std::string&, const Vector2&, float), EntityPtr), asCALL_THISCALL);
	}

	bool EntityFactory::getEntityType(TiXmlDocument *document, std::string &type)
	{
		TiXmlElement *root = document->FirstChildElement();
		if (root->ValueStr() == "Entity")
		{
			const char *typeCstr = root->Attribute("typename");
			if (typeCstr != NULL)
			{
				type = typeCstr;
				return true;
			}
		}

		return false;
	}

}
