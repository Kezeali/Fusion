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
#include "FusionScriptSound.h"
#include "FusionXml.h"
#include "FusionPhysFS.h"
#include "FusionResourceManager.h"

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

	//! Stores parsed Entity.xml data
	class EntityDefinition
	{
	public:
		//! Stores script data info
		struct Script
		{
			enum ScriptType { NoLanguage, AngelScript };

			ScriptType type;
			std::string fileName;
			std::string scriptData;

			static ScriptType ParseTypeID(const std::string &type_id);
		};

		//! Stores data from the Dependencies element
		typedef StringVector DependenciesMap;

	public:
		//! Basic CTOR
		EntityDefinition();
		//! CTOR - calls Parse() on given params
		EntityDefinition(const std::string &current_folder, ticpp::Document &document);

		void Parse(const std::string &containing_folder, ticpp::Document &document);

		//! Resolves the given path relative to the working directory
		std::string ResolvePath(const std::string &relative_path);

		void LoadScriptData(Script::ScriptType type, const std::string &filename);
		void SetScriptData(Script::ScriptType type, const std::string &data);

		//! Returns the type of entity defined by this object
		const std::string &GetType() const;
		//! Returns the default domain value of the entity
		EntityDomain GetDefaultDomain() const;
		//! Gets the directory where the XML file was located
		const std::string &GetWorkingDirectory() const;
		//! Gets script data
		Script &GetScript();
		//! Gets script data (const)
		const Script &GetScript() const;
		//! Returns Entity types that were listed in the Dependencies element
		const DependenciesMap &GetEntityDependencies() const;
		//! Returns UtilityScript filenames that were listed in the Dependencies element
		const DependenciesMap &GetScriptDependencies() const;
		ScriptedEntity::PropertiesMap &GetSyncProperties();
		ResourcesMap &GetStreamedResources();

		static EntityDomain ToDomainIndex(const std::string &domain);
	protected:
		void parseElement_Script(ticpp::Element *element);
		void parseElement_Dependencies(ticpp::Element *element);
		void parseElement_Sync(ticpp::Element *element);
		void parseElement_Streaming(ticpp::Element *element);

		std::string m_WorkingDirectory;

		std::string m_TypeName;
		bool m_Abstract;

		EntityDomain m_DefaultDomain;

		Script m_Script;

		DependenciesMap m_EntityDependencies;
		DependenciesMap m_ScriptDependencies;

		ScriptedEntity::PropertiesMap m_SyncProperties;

		ResourcesMap m_Resources;
	};

	EntityDomain EntityDefinition::ToDomainIndex(const std::string &domain)
	{
		if (domain.empty())
			return SYSTEM_DOMAIN;
		else if (domain == "system")
			return SYSTEM_DOMAIN;
		else if (domain == "game")
			return GAME_DOMAIN;
		else if (domain == "temp")
			return TEMP_DOMAIN;
		else
			return UNRESERVED_DOMAIN;
	}

	void EntityDefinition::Parse(const std::string &current_folder, ticpp::Document &document)
	{
		m_WorkingDirectory = current_folder;

		ticpp::Element *root = document.FirstChildElement();

		m_TypeName = root->GetAttribute("typename");

		std::string domainString = root->GetAttribute("defaultdomain");
		m_DefaultDomain = ToDomainIndex(domainString);

		ticpp::Iterator< ticpp::Element > child;
		for (child = child.begin( root ); child != child.end(); child++)
		{
			if (child->Value() == "Script")
				parseElement_Script(child.Get());

			else if (child->Value() == "Dependencies")
				parseElement_Dependencies(child.Get());

			else if (child->Value() == "Sync")
				parseElement_Sync(child.Get());

			else if (child->Value() == "Streaming")
				parseElement_Streaming(child.Get());
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
			//LoadScriptData(type, script);
			m_Script.type = type;
			m_Script.fileName = ResolvePath(script);
		}
		else
		{
			// Since it's already allocated, used the filename string object to load the inline script text
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
				m_EntityDependencies.push_back(child->GetAttribute("typename"));
			}

			else if (child->Value() == "UtilityScript")
			{
				std::string filename = child->GetAttribute("file");
				m_ScriptDependencies.push_back( ResolvePath(filename) );
			}
		}
	}

	void EntityDefinition::parseElement_Sync(ticpp::Element *sync_element)
	{
		std::string attribute;
		ticpp::Iterator< ticpp::Element > child;
		for (child = child.begin( sync_element ); child != child.end(); child++)
		{
			ScriptedEntity::Property propertyDefinition;

			propertyDefinition.type = child->GetAttribute("type");
			propertyDefinition.name = child->GetAttribute("name");

			attribute = child->GetAttributeOrDefault("arbitrated", "0");
			propertyDefinition.arbitrated = CL_StringHelp::local8_to_bool(attribute.c_str());

			attribute = child->GetAttributeOrDefault("local", "0");
			propertyDefinition.localOnly = CL_StringHelp::local8_to_bool(attribute.c_str());

			m_SyncProperties[propertyDefinition.name] = propertyDefinition;
		}
	}

	void EntityDefinition::parseElement_Streaming(ticpp::Element *element)
	{
		std::string attribute;
		ticpp::Iterator< ticpp::Element > child;
		for (child = child.begin( element ); child != child.end(); child++)
		{
			ResourceDescription resource;

			resource.SetType( child->Value() );

			std::string propertyName = child->GetAttribute("property");
			resource.SetPropertyName( propertyName );
			resource.SetResourceName( ResolvePath(child->GetAttribute("resource")) );
			int priority; child->GetAttributeOrDefault("priority", &priority, 0);
			resource.SetPriority(priority);

			m_Resources[propertyName] = resource;
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

	EntityDomain EntityDefinition::GetDefaultDomain() const
	{
		return m_DefaultDomain;
	}

	const std::string &EntityDefinition::GetWorkingDirectory() const
	{
		return m_WorkingDirectory;
	}

	EntityDefinition::Script &EntityDefinition::GetScript()
	{
		return m_Script;
	}

	const EntityDefinition::Script &EntityDefinition::GetScript() const
	{
		return m_Script;
	}

	const EntityDefinition::DependenciesMap &EntityDefinition::GetEntityDependencies() const
	{
		return m_EntityDependencies;
	}

	const EntityDefinition::DependenciesMap &EntityDefinition::GetScriptDependencies() const
	{
		return m_ScriptDependencies;
	}

	ScriptedEntity::PropertiesMap &EntityDefinition::GetSyncProperties()
	{
		return m_SyncProperties;
	}

	ResourcesMap &EntityDefinition::GetStreamedResources()
	{
		return m_Resources;
	}

	//! Creates instances of a scripted entity type
	class ScriptedEntityInstancer : public EntityInstancer
	{
	public:
		//! Constructor
		ScriptedEntityInstancer();
		//! Constructor
		ScriptedEntityInstancer(ScriptingEngine *manager, const std::string &module, EntityDefinitionPtr definition);
		//! Constructor
		//ScriptedEntityInstancer(TiXmlDocument &document);

	public:
		Entity *InstanceEntity(const std::string &name);

		//void Parse();

	protected:
		//void parseDoc(TiXmlDocument *document);

		ScriptingEngine *m_ScriptingManager;
		std::string m_Module;
		EntityDefinitionPtr m_Definition;
	};

	ScriptedEntityInstancer::ScriptedEntityInstancer()
		: EntityInstancer("undefined_scripted_entity")
	{}

	ScriptedEntityInstancer::ScriptedEntityInstancer(ScriptingEngine *manager, const std::string &module, EntityDefinitionPtr definition)
		: EntityInstancer(definition->GetType()),
		m_ScriptingManager(manager),
		m_Module(module),
		m_Definition(definition)
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
		if (m_ScriptingManager == NULL)
			return NULL;

		ResourceManager *resMan = ResourceManager::getSingletonPtr();
		if (resMan == NULL)
			return NULL;

		ScriptObject object = m_ScriptingManager->CreateObject(m_Module.c_str(), m_Definition->GetType());
		asIScriptObject *scrObj = object.GetScriptObject();

		ScriptedEntity *entity = new ScriptedEntity(object, name);
		entity->_setDomain(m_Definition->GetDefaultDomain());
		//entity->SetPath(m_Definition->GetWorkingDirectory());
		entity->SetSyncProperties(m_Definition->GetSyncProperties());

		ScriptUtils::Calling::Caller f = object.GetCaller("void _setAppObject(Entity@ obj)");
		if (f.ok())
			f(entity);
		else
			return NULL;

		//StreamedResourceUserPtr resourceUser;

		const ResourcesMap &resources = m_Definition->GetStreamedResources();
		for (ResourcesMap::const_iterator it = resources.begin(), end = resources.end(); it != end; ++it)
		{
			const ResourceDescription &desc = it->second;
			if (desc.GetType() == "Sprite")
			{
				RenderablePtr renderable( new Renderable(resMan, fe_widen(desc.GetResourceName()), desc.GetPriority()) );

				entity->AddRenderable(renderable);

				// Add the object to the entity for automatic streaming
				entity->AddStreamedResource( renderable );

				if (desc.GetPropertyIndex() >= 0)
				{
					void *prop = scrObj->GetPropertyPointer(desc.GetPropertyIndex());
					Renderable **renderableProperty = static_cast<Renderable**>( prop );
					*renderableProperty = renderable.get();
				}
			}
			else if (desc.GetType() == "Image")
			{
				if (desc.GetPropertyIndex() >= 0)
				{
				}
			}
			else if (desc.GetType() == "Polygon")
			{
				if (desc.GetPropertyIndex() >= 0)
				{
				}
			}
			else if (desc.GetType() == "Sound")
			{
				// Check that the property listed in the description is correct
				FSN_ASSERT( desc.GetPropertyIndex() < scrObj->GetPropertyCount() &&
					desc.GetPropertyName() == std::string(scrObj->GetPropertyName(desc.GetPropertyIndex())) );

				void *prop = scrObj->GetPropertyPointer(desc.GetPropertyIndex());
				SoundSample **soundProp = static_cast<SoundSample**>( prop );
				*soundProp = new SoundSample(resMan, fe_widen(desc.GetResourceName()), desc.GetPriority(), false);

				entity->AddStreamedResource( StreamedResourceUserPtr(*soundProp) );
			}
			else if (desc.GetType() == "SoundStream")
			{
				// Check that the property listed in the description is correct
				FSN_ASSERT( desc.GetPropertyIndex() < scrObj->GetPropertyCount() &&
					desc.GetPropertyName() == std::string(scrObj->GetPropertyName(desc.GetPropertyIndex())) );

				void *prop = scrObj->GetPropertyPointer(desc.GetPropertyIndex());
				SoundSample **soundProp = static_cast<SoundSample**>( prop );
				*soundProp = new SoundSample(resMan, fe_widen(desc.GetResourceName()), desc.GetPriority(), true);

				entity->AddStreamedResource( StreamedResourceUserPtr(*soundProp) );
			}

			//if (resourceUser.get() != NULL)
			//{
			//	entity->AddStreamedResource(resourceUser);
			//	resourceUser.reset();
			//}
		}

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
		else
		{
			// Check for a scripted entity type that hasn't been loaded
			StringMap::iterator _whereFile = m_EntityDefinitionFileNames.find(type);
			if (_whereFile != m_EntityDefinitionFileNames.end())
			{
				SendToConsole("Entity Factory",
					"Tried to instance an Entity for which there is a known definition file that hasn't been compiled: "
					"Please add all required Entities to the <Dependencies> element of your definition file.");
			}
		}
		return EntityPtr();
	}

	std::string fe_getbasepath(const std::string &path)
	{
		std::string::size_type pathEnd = path.find_last_of("/");
		if (pathEnd != std::string::npos)
			return path.substr(0, pathEnd);
		else
			return "/";
	}

	bool EntityFactory::LoadScriptedType(const std::string &type)
	{
		StringMap::iterator _where = m_EntityDefinitionFileNames.find(type);
		if (_where != m_EntityDefinitionFileNames.end())
		{
			ticpp::Document document( OpenXml_PhysFS(fe_widen(_where->second)) );
			loadAllDependencies(fe_getbasepath(_where->second), document);
			return true;
		}

		return false;
	}

	void EntityFactory::SetScriptedEntityPath(const std::string &path)
	{
		m_ScriptedEntityPath = path;

		parseScriptedEntities(m_ScriptedEntityPath.c_str());
	}

	void EntityFactory::SetScriptingManager(ScriptingEngine *manager, const std::string &module_name)
	{
		m_ScriptingManager = manager;
		m_ModuleName = module_name;

		m_ModuleConnection.disconnect();
		m_ModuleConnection = m_ScriptingManager->GetModule(module_name.c_str())->ConnectToBuild(boost::bind(&EntityFactory::OnModuleRebuild, this, _1));
		//m_ModuleConnection = m_ScriptingManager->SubscribeToModule(module_name.c_str(), boost::bind(&EntityFactory::OnModuleRebuild, this, _1));
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

	void EntityFactory::OnModuleRebuild(BuildModuleEvent &ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
		{
			for (EntityDefinitionArray::iterator it = m_LoadedEntityDefinitions.begin(), end = m_LoadedEntityDefinitions.end();
				it != end; ++it)
			{
				EntityDefinitionPtr &def = *it;
				EntityDefinition::Script &script = def->GetScript();
				if (script.fileName == "inline")
					ev.manager->AddCode(script.scriptData, m_ModuleName.c_str(), (def->GetType() + "_inline").c_str());
				else
					ev.manager->AddFile(script.fileName, m_ModuleName.c_str());

				const EntityDefinition::DependenciesMap &deps = def->GetScriptDependencies();
				for (EntityDefinition::DependenciesMap::const_iterator it = deps.begin(), end = deps.end(); it != end; ++it)
				{
					bool success = ev.manager->AddFile(*it, m_ModuleName.c_str());
					if (!success)
						SendToConsole("Couldn't load script file '" + *it + "', which is a required UtilityScript for " + def->GetType());
				}
			}
		}

		else if (ev.type == BuildModuleEvent::PostBuild)
		{
			for (EntityDefinitionArray::iterator it = m_LoadedEntityDefinitions.begin(), end = m_LoadedEntityDefinitions.end(); it != end; ++it)
				createScriptedEntityInstancer(*it);
		}
	}

	void EntityFactory::loadAllDependencies(const std::string &working_directory, ticpp::Document &document)
	{
		// Load the Entity definition from the given document (this is the root of the dependency tree)
		EntityDefinitionPtr definition( new EntityDefinition(working_directory, document) );
		m_LoadedEntityDefinitions.push_back(definition);

		std::deque<std::string> *depsToLoad = new std::deque<std::string>();

		// Push all the dependencies of the root onto the stack
		const EntityDefinition::DependenciesMap &deps = definition->GetEntityDependencies();
		depsToLoad->insert(depsToLoad->end(), deps.begin(), deps.end());

		// Load dependencies until the stack is empty
		while (!depsToLoad->empty())
		{
			// Get the file of the next dependency on the stack
			StringMap::iterator _where = m_EntityDefinitionFileNames.find(depsToLoad->back());
			depsToLoad->pop_back();
			if (_where != m_EntityDefinitionFileNames.end())
			{
				// Figure out the working directory for this file (i.e. the path up to the file name)
				std::string depPath;
				std::string::size_type pathEnd = _where->second.find_last_of("/");
				if (pathEnd != std::string::npos)
					depPath = _where->second.substr(0, pathEnd);
				else
					depPath = "/";
				ticpp::Document depDocument( OpenXml_PhysFS(fe_widen(_where->second)) );

				// Parse the Entity definition document
				definition = EntityDefinitionPtr( new EntityDefinition(depPath, depDocument) );
				m_LoadedEntityDefinitions.push_back(definition);

				// Push the dep.s for this Entity on to the stack
				const EntityDefinition::DependenciesMap &deps = definition->GetEntityDependencies();
				depsToLoad->insert(depsToLoad->end(), deps.begin(), deps.end());
			}
		}
		/*for (EntityDefinition::DependencyMap::const_iterator it = deps.begin(), end = deps.end(); it != end; ++it)
		{
			StringMap::iterator _where = m_EntityDefinitionFileNames.find(*it);
			if (_where != m_EntityDefinitionFileNames.end())
			{
				std::string depPath;
				std::string::size_type pathEnd = _where->second.find_last_of("/");
				if (pathEnd != std::string::npos)
					depPath = _where->second.substr(0, pathEnd);
				else
					depPath = "/";
				TiXmlDocument *depDocument = OpenXml_PhysFS(fe_widen(_where->second));
				
				loadAllDependencies(depPath, depDocument);
			}
		}*/
	}

	void EntityFactory::createScriptedEntityInstancer(EntityDefinitionPtr definition)
	{
		// Find the index for each script property listed in the Sync section of the Entity definition file
		//  Note that it is important that this is done after all the definitions have been loaded and
		//  the script module has been built
		ScriptedEntity::PropertiesMap &syncProperties = definition->GetSyncProperties();
		// Create an instance of the script object
		asIScriptEngine *engine = m_ScriptingManager->GetEnginePtr();
		int typeId = engine->GetModule(m_ModuleName.c_str())->GetTypeIdByDecl(definition->GetType().c_str());
		if (typeId < 0)
		{
			SendToConsole("Couldn't create instancer for '" + definition->GetType() +
				"' type Entities because the type doesn't exist in the script module (most likely due to a compilation error).");
			return;
		}
		asIScriptObject *object = (asIScriptObject*)engine->CreateScriptObject(typeId);
		// Iterate through all of the script object's properties
		for (int i = 0; i < object->GetPropertyCount(); i++)
		{
			ScriptedEntity::PropertiesMap::iterator _where = syncProperties.find( object->GetPropertyName(i) );
			if (_where != syncProperties.end())
			{
				_where->second.scriptPropertyIndex = i;
			}
		}
		// Erase synced-property defs that are missing from the script type
		{
			ScriptedEntity::PropertiesMap::iterator it = syncProperties.begin(), end = syncProperties.end();
			while (it != end)
			{
				if (it->second.scriptPropertyIndex < 0)
				{
					SendToConsole("Creating instancer for a scripted entity: There is no property called '"
						+ it->first + "' in " + definition->GetType() +
						" as indicated in the <Sync> element of the xml definition file - i.e. the definition is incorrect.");
					syncProperties.erase(it++);
				}
				else
					++it;
			}
		}

		// Same as above for Streaming section
		ResourcesMap &resources = definition->GetStreamedResources();
		for (int i = 0; i < object->GetPropertyCount(); i++)
		{
			ResourcesMap::iterator _where = resources.find( object->GetPropertyName(i) );
			if (_where != resources.end())
			{
				_where->second.SetPropertyIndex(i);
			}
		}
		// Erase resource-property defs that are missing from the script type
		{
			ResourcesMap::iterator it = resources.begin(), end = resources.end();
			while (it != end)
			{
				// Script-class properties for Sprite, Image, Polygon types are not necessary
				//  (these define Renderable objects, which are held by the Entity baseclass)
				if (it->second.GetPropertyIndex() < 0 &&
					it->second.GetType() != "Sprite" && it->second.GetType() != "Image" && it->second.GetType() != "Polygon")
				{
					SendToConsole("Creating instancer for a scripted entity: There is no property called '"
						+ it->second.GetPropertyName() + "' in " + definition->GetType() +
						" as indicated in the <Streaming> element of the xml definition file - i.e. the definition is incorrect.");
					resources.erase(it++);
				}
				else
					++it;
			}
		}
		object->Release();

		m_EntityInstancers[definition->GetType()] = EntityInstancerPtr( new ScriptedEntityInstancer(m_ScriptingManager, m_ModuleName, definition) ); 
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

	void EntityFactory::parseScriptedEntities(const char *path, unsigned int current_recursion)
	{
		// Limit recursion depth to prevent stack issues - 25 folders deep seems like more than enough
		if (current_recursion > 25)
			return;

		const char *ext = "XML";
		const size_t extLength = 3;

		char *fileExt;

		std::string fullPath(path);

		char **files = PHYSFS_enumerateFiles(path);
		for (char **file = files; *file != NULL; file++)
		{
			size_t filenameLength = std::strlen(*file);

			fullPath = path;
			fullPath += *file;
			// Recursively search directories
			if (PHYSFS_isDirectory(fullPath.c_str()))
			{
				parseScriptedEntities((fullPath + '/').c_str(), current_recursion+1);
			}

			// Check that this file has an extension
			else if (filenameLength > extLength && (*file)[filenameLength - extLength - 1] == '.')
			{
				// Compare the file's extension to the expected extension for Entity definition files
				fileExt = *file + (filenameLength - extLength);
				if (fe_nocase_strcmp(fileExt, ext) == 0)
				{
					TiXmlDocument *doc = OpenXml_PhysFS(fe_widen(fullPath));
					std::string type;
					if (getEntityType(doc, type))
						m_EntityDefinitionFileNames[type] = fullPath;
					delete doc;
				}
			}
		}

		PHYSFS_freeList(files);
	}

}
