/*
  Copyright (c) 2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward

*/

#include "Common.h"

#include "FusionEntityFactory.h"
#include "FusionScriptedEntity.h"
#include "FusionXml.h"
#include "FusionPhysFS.h"

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>


namespace FusionEngine
{

	EntityInstancer::EntityInstancer(const std::string &type)
		: m_Type(type)
	{}

	void EntityInstancer::SetType(const std::string &type)
	{
		m_Type = type;
	}

	const std::string &EntityInstancer::GetType() const
	{
		return m_Type;
	}


	//class RootEntity : public Entity
	//{
	//public:
	//	RootEntity(const std::string &name);
	//};

	////! Creates root entities
	//class RootEntityInstancer : public EntityInstancer
	//{
	//public:
	//	RootEntityInstancer();

	//	Entity *InstanceEntity(const std::string &name);
	//};

	//RootEntityInstancer::RootEntityInstancer()
	//	: EntityInstancer("root")
	//{}

	//Entity *RootEntityInstancer::InstanceEntity(const std::string &name)
	//{
	//	return new RootEntity(name);
	//}

	class EntityDefinition;

	typedef std::tr1::shared_ptr<EntityDefinition> EntityDefinitionPtr;

	//! Parsed Entity.xml data
	class EntityDefinition
	{
	public:
		struct Script
		{
			enum ScriptType { NoLanguage, AngelScript };

			ScriptType type;
			std::string fileName;
			std::string scriptData;

			static ScriptType ParseTypeID(const std::string &type_id);
		};

		typedef StringVector DependenciesMap;

	public:
		EntityDefinition();
		EntityDefinition(const std::string &current_folder, ticpp::Document &document);

		void Parse(const std::string &current_folder, ticpp::Document &document);

		std::string ResolvePath(const std::string &relative_path);

		void LoadScriptData(Script::ScriptType type, const std::string &filename);
		void SetScriptData(Script::ScriptType type, const std::string &data);

		const std::string &GetType() const;
	protected:
		void parseElement_Script(ticpp::Element *script_element);
		void parseElement_Dependencies(ticpp::Element *element);

		std::string m_WorkingDirectory;

		std::string m_TypeName;
		bool m_Abstract;

		Script m_Script;

		DependenciesMap m_EntityDependencies;
		DependenciesMap m_ScriptDependencies;
	};

	void EntityDefinition::Parse(const std::string &current_folder, ticpp::Document &document)
	{
		m_WorkingDirectory = current_folder;

		ticpp::Element *root = document.FirstChildElement();

		m_TypeName = root->GetAttribute("typename");

		ticpp::Iterator< ticpp::Element > child;
		for (child = child.begin( root ); child != child.end(); child++)
		{
			if (child->Value() == "Script")
				parseElement_Script(child.Get());

			else if (child->Value() == "Dependencies")
				parseElement_Dependencies(child.Get());
		}
	}

	void EntityDefinition::parseElement_Script(ticpp::Element *script_element)
	{
		// Catch no-such-attribute exception and throw an exception noting the lack of script-language definition
		std::string scriptType = script_element->GetAttribute("type");
		Script::ScriptType type = Script::ParseTypeID(scriptType);

		if (type == Script::NoLanguage)
			SendToConsole("Entity Factory", "The Script element of " + m_TypeName + " uses an unknown language ('" + scriptType + "')");

		std::string script = script_element->GetAttributeOrDefault("file", "");
		if (!script.empty())
		{
			LoadScriptData(type, script);
		}
		else
		{
			script_element->GetText(&script);
			SetScriptData(type, script);
		}
	}

	void EntityDefinition::parseElement_Dependencies(ticpp::Element *deps_element)
	{
		ticpp::Iterator< ticpp::Element > child;
		for (child = child.begin( deps_element ); child != child.end(); child++)
		{
			if (child->Value() == "Entity")
			{
			}

			else if (child->Value() == "UtilityScript")
			{
			}
		}
	}

	EntityDefinition::Script::ScriptType EntityDefinition::Script::ParseTypeID(const std::string &type_id)
	{
		if (type_id == "as" || type_id == "angelscript")
			return AngelScript;

		else
			return NoLanguage;
	}

	EntityDefinition::EntityDefinition()
	{
	}

	EntityDefinition::EntityDefinition(const std::string &current_folder, ticpp::Document &document)
	{
		Parse(current_folder, document);
	}

	std::string EntityDefinition::ResolvePath(const std::string &path)
	{
		if (path[0] == '/')
		{
			return path;
		}

		else
		{
			typedef std::vector<boost::iterator_range<std::string::iterator>> SplitResult;

			StringVector currentPath;
			boost::split(currentPath, m_WorkingDirectory, boost::is_any_of("/"));

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

	void EntityDefinition::LoadScriptData(Script::ScriptType type, const std::string &filename)
	{
		std::string absolutePath = ResolvePath(filename);

		m_Script.type = type;
		m_Script.fileName = absolutePath;

		OpenString_PhysFS(m_Script.scriptData, fe_widen(absolutePath));
	}

	void EntityDefinition::SetScriptData(FusionEngine::EntityDefinition::Script::ScriptType type, const std::string &data)
	{
		m_Script.type = type;
		m_Script.fileName = "inline";
		m_Script.scriptData = data;
	}

	const std::string &EntityDefinition::GetType() const
	{
		return m_TypeName;
	}

	//! Creates instances of a scripted entity type
	class ScriptedEntityInstancer : public EntityInstancer
	{
	public:
		//! Constructor
		ScriptedEntityInstancer();
		//! Constructor
		ScriptedEntityInstancer(const std::string &module, const EntityDefinition &definition);
		//! Constructor
		//ScriptedEntityInstancer(TiXmlDocument &document);

	public:
		Entity *InstanceEntity(const std::string &name);

		//void Parse();

	protected:
		//void parseDoc(TiXmlDocument *document);

		std::string m_Module;
	};

	ScriptedEntityInstancer::ScriptedEntityInstancer()
		: EntityInstancer("undefined_scripted_entity")
	{}

	ScriptedEntityInstancer::ScriptedEntityInstancer(const std::string &module, const EntityDefinition &definition)
		: EntityInstancer(definition.GetType()),
		m_Module(module)
	{}

	//ScriptedEntityInstancer::ScriptedEntityInstancer(TiXmlDocument &document)
	//	: EntityInstancer("")
	//{
	//	TiXmlElement *root = document.FirstChildElement();
	//	SetType(root->Attribute("typename"));
	//	parseDoc(document);
	//}

	Entity *ScriptedEntityInstancer::InstanceEntity(const std::string &name)
	{
		ScriptingEngine *manager = ScriptingEngine::getSingletonPtr();
		if (manager == NULL)
			return NULL;

		ScriptObject object = manager->CreateObject(m_Module.c_str(), GetType());
		ScriptedEntity *entity = new ScriptedEntity(object, name);
		return entity;
	}

	//void ScriptedEntityInstancer::Parse()
	//{
	//	TiXmlDocument *doc = OpenXml_PhysFS(fe_widen(m_Filename));
	//	parseDoc(doc);
	//	delete doc;
	//}

	//void ScriptedEntityInstancer::parseDoc(TiXmlDocument *document)
	//{
	//}


	EntityFactory::EntityFactory()
	{
	}

	EntityFactory::~EntityFactory()
	{
	}

	EntityPtr EntityFactory::InstanceEntity(const std::string &type, const std::string &name)
	{
		EntityInstancerMap::iterator _where = m_EntityInstancers.find(type);
		if (_where != m_EntityInstancers.end())
		{
			return EntityPtr( _where->second->InstanceEntity(name) );
		}
		return EntityPtr();
	}

	void EntityFactory::LoadScriptedEntity(const std::string &type)
	{
	}

	void EntityFactory::SetScriptedEntityPath(const std::string &path)
	{
		m_ScriptedEntityPath = path;

		parseScriptedEntities(m_ScriptedEntityPath.c_str());
	}

	void EntityFactory::ResetUsedTypesList()
	{
		m_UsedTypes.clear();
	}

	void EntityFactory::ClearUnusedInstancers()
	{
		for (EntityInstancerMap::iterator it = m_EntityInstancers.begin(), end = m_EntityInstancers.end(); it != end; ++it)
		{
			if (m_UsedTypes.find(it->first) == m_UsedTypes.end())
				it = m_EntityInstancers.erase(it);
		}
	}

	void EntityFactory::createScriptedEntityInstancer(TiXmlDocument *document)
	{
	}

	bool EntityFactory::getEntityType(TiXmlDocument *document, std::string &type)
	{
		TiXmlElement *root = document->FirstChildElement();
		if (root->ValueStr() == "Element")
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

	void EntityFactory::parseScriptedEntities(const char *path)
	{
		const char *dirsep = PHYSFS_getDirSeparator();

		const char *ext = "XML";
		const size_t extLength = 3;

		char *fileExt;

		char **files = PHYSFS_enumerateFiles(path);
		for (char **file = files; *file != NULL; file++)
		{
			size_t filenameLength = std::strlen(*file);

			// Check that this file has an extension
			if (filenameLength > extLength && (*file)[filenameLength - extLength - 1] == '.')
			{
				// Compare the file's extension to the expected extension for Entity definition files
				fileExt = *file + (filenameLength - extLength);
				if (fe_nocase_strcmp(fileExt, ext) == 0)
				{
					TiXmlDocument *doc = OpenXml_PhysFS(fe_widen(*file));
					std::string type;
					if (getEntityType(doc, type))
						m_ScriptEntityDefinitionFiles[type] = std::string(*file);
					delete doc;
				}
			}
		}

		PHYSFS_freeList(files);
	}

}
