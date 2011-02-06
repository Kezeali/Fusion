/*
  Copyright (c) 2009-2010 Fusion Project Team

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

#include "FusionStableHeaders.h"

#include "FusionEntityFactory.h"

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include <ScriptUtils/Inheritance/TypeTraits.h>

#include "FusionExceptionFactory.h"
#include "FusionLog.h"
#include "FusionLogger.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionPhysicalEntityManager.h"
#include "FusionResourceManager.h"
#include "FusionScriptedEntity.h"
#include "FusionScriptSound.h"
#include "FusionXml.h"

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

		typedef std::tr1::unordered_map<std::string, ScriptedEntity::Property> PropertiesMap;

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

		//! Sets the type ID of the class in the script module
		/*!
		* This is the result of 'module->GetTypeIdByDecl(GetType())'
		*/
		void SetTypeId(int id);
		int GetTypeId() const;

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
		//! Returns streamed-resources collection
		ResourcesMap &GetStreamedResources();
		//! Properties array that is passed to Entities created
		ScriptedEntity::PropertiesArray &GetSyncProperties();

		//! Returns synchronised-properties collection
		/*!
		* Used while building the definition (merging parent properties, listing
		* property indexes) - GetSyncProperties returns the completed list.
		*
		* \see GetStreamedResources()
		* 
		*/
		PropertiesMap &GetSyncPropertiesMap();

		//! Returns true if the entity has a body definition
		bool HasBody() const;
		const b2BodyDef &GetBodyDef() const;
		typedef std::vector<FixtureDefinition> FixtureArray;
		const FixtureArray &GetFixtures() const;

		//! Sets the base type of this definition
		void SetBaseType(const EntityDefinitionPtr &base_type_def);
		const EntityDefinitionPtr &GetBaseType() const;

		static EntityDomain ToDomainIndex(const std::string &domain);
	protected:
		void parseElement_Script(ticpp::Element *element);
		void parseElement_Dependencies(ticpp::Element *element);
		void parseElement_Sync(ticpp::Element *element);
		void parseElement_Streaming(ticpp::Element *element);
		// Physics stuff
		void parseElement_Body(ticpp::Element *element);
		void parseElement_Body_Fixtures(ticpp::Element *element);

		std::string m_WorkingDirectory;

		std::string m_TypeName;
		int m_TypeId;
		bool m_Abstract;

		EntityDefinitionPtr m_BaseType;

		EntityDomain m_DefaultDomain;

		Script m_Script;

		DependenciesMap m_EntityDependencies;
		DependenciesMap m_ScriptDependencies;

		PropertiesMap m_SyncPropertiesMap;
		ScriptedEntity::PropertiesArray m_SyncProperties;

		ResourcesMap m_Resources;

		bool m_HasBody;
		b2BodyDef m_BodyDef;
		FixtureArray m_Fixtures;
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
			return USER_DOMAIN;
	}

	void EntityDefinition::Parse(const std::string &current_folder, ticpp::Document &document)
	{
		m_WorkingDirectory = current_folder;

		ticpp::Element *root = document.FirstChildElement();

		m_TypeName = root->GetAttribute("typename");

		// Create a sub-directory for this class in the 'temp' folder
		PHYSFS_mkdir((s_TempPath + m_TypeName).c_str());

		std::string domainString = root->GetAttribute("domain");
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

			else if (child->Value() == "Body")
				parseElement_Body(child.Get());
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

			m_SyncPropertiesMap[propertyDefinition.name] = propertyDefinition;
		}
	}

	std::string getXmlRootTagForResourceType(ticpp::Element *inline_resource_element)
	{
		if (inline_resource_element->Value() == "Sprite")
			return "sprite";
		else if (inline_resource_element->Value() == "Polygon")
			return "polygon";
		else if (inline_resource_element->Value() == "Image")
			return "sprite";
		else
			return fe_newlower( inline_resource_element->Value() );
	}

	void createInlineResourceFile_Xml(std::string &file_name, ticpp::Element *resource_element, const std::string &entity_typename)
	{
		if (resource_element->NoChildren())
			return; // Throw?

		file_name = s_TempPath + entity_typename + '/' + resource_element->GetAttribute("property") + ".xml";

		ticpp::Document document;
		ticpp::Declaration *decl = new ticpp::Declaration(XML_STANDARD, "", "");
		document.LinkEndChild(decl);

		// Check whether the given inline-resource-XML includes a root element (indicated by there only being one child)
		if (*resource_element->FirstChild() != *resource_element->LastChild())
		{
			ticpp::Element *root = new ticpp::Element( getXmlRootTagForResourceType(resource_element) );
			document.LinkEndChild(root);
			root->InsertEndChild(*resource_element);
		}
		else
			document.InsertEndChild(*resource_element->FirstChild());

		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		CL_IODevice ioDevice = vdir.open_file(file_name, CL_File::create_new, CL_File::access_write);
		if (ioDevice.is_null())
			return; // Throw?
		ClanLibTiXmlFile xmlFile(ioDevice);
		document.SaveFile(&xmlFile);
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
			std::string resourceFileName = child->GetAttribute("resource");
			// Read inline-resource
			if (resourceFileName.compare(0, 8, "\\inline:") == 0)
			{
				if (resourceFileName.compare(8, 3, "xml") == 0)
					createInlineResourceFile_Xml(resourceFileName, child.Get(), m_TypeName);

				resource.SetResourceName(resourceFileName);
			}
			// Read normal resource (file name)
			else
				resource.SetResourceName( ResolvePath(resourceFileName) );
			// Tags
			resource.ParseTags(child->GetAttribute("tags"));
			// Priority - defaults to zero
			int priority; child->GetAttributeOrDefault("priority", &priority, 0);
			resource.SetPriority(priority);

			m_Resources[propertyName] = resource;
		}
	}

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

	void EntityDefinition::parseElement_Body(ticpp::Element *element)
	{
		m_HasBody = true;

		std::string attribute = element->GetAttribute("fixed_rotation");
		if (!attribute.empty())
		{
			m_BodyDef.fixedRotation = true;
			m_BodyDef.angle = boost::lexical_cast<float32>(attribute);
		}
		attribute = element->GetAttribute("is_bullet");
		m_BodyDef.isBullet = (attribute == "t" || attribute == "1" || attribute == "true");

		attribute = element->GetAttribute("allow_sleep");
		m_BodyDef.allowSleep = (attribute == "t" || attribute == "1" || attribute == "true");

		element->GetAttribute("linear_damping", &m_BodyDef.linearDamping, false);
		element->GetAttribute("angular_damping", &m_BodyDef.angularDamping, false);

		ticpp::Iterator< ticpp::Element > child;
		for (child = child.begin( element ); child != child.end(); child++)
		{
			FixtureDefinition fixtureDef;
			typedef std::tr1::shared_ptr<b2CircleShape> CircleShapePtr;
			typedef std::tr1::shared_ptr<b2PolygonShape> PolyShapePtr;

			if (child->Value() == "CircleFixture")
			{
				CircleShapePtr circle(new b2CircleShape());
				attribute = child->GetAttribute("position");
				if (!attribute.empty())
					parse_vector(attribute, &circle->m_p.x, &circle->m_p.y);

				child->GetAttribute("x", &circle->m_p.x, false);
				child->GetAttribute("y", &circle->m_p.y, false);

				child->GetAttribute("radius", &circle->m_radius, false);
				child->GetAttribute("r", &circle->m_radius, false);

				circle->m_p *= s_SimUnitsPerGameUnit;
				circle->m_radius *= s_SimUnitsPerGameUnit;

				fixtureDef.SetShape(circle);
			}
			else if (child->Value() == "RectFixture")
			{
				PolyShapePtr poly(new b2PolygonShape());
				float hx = 0.f, hy = 0.f, angle = 0.f;
				b2Vec2 center(0.f, 0.f);

				attribute = child->GetAttribute("size");
				if (!attribute.empty())
				{
					parse_vector(attribute, &hx, &hy);
				}
				else
				{
					child->GetAttribute("half_width", &hx, false);
					child->GetAttribute("half_height", &hy, false);
					child->GetAttribute("hw", &hx, false);
					child->GetAttribute("hh", &hy, false);
				}

				if (hx == 0.f || hy == 0.f)
					continue;

				attribute = child->GetAttribute("center");
				if (!attribute.empty())
				{
					parse_vector(attribute, &center.x, &center.y);
				}
				else
				{
					child->GetAttribute("x", &center.x, false);
					child->GetAttribute("y", &center.y, false);
				}

				child->GetAttribute("angle", &angle, false);

				hx *= s_SimUnitsPerGameUnit;
				hy *= s_SimUnitsPerGameUnit;

				if (fe_fzero(angle) && fe_fzero(center.LengthSquared()))
					poly->SetAsBox(hx, hy);
				else
				{
					center *= s_SimUnitsPerGameUnit;
					poly->SetAsBox(hx, hy, center, angle);
				}

				fixtureDef.SetShape(poly);
			}
			else if (child->Value() == "PolygonFixture")
			{
				FSN_EXCEPT(ExCode::NotImplemented, "EntityDefinition::parseElement_Body", "PolygonFixture type not yet implemented");
			}
			else if (child->Value() == "EdgeFixture")
			{
				PolyShapePtr poly(new b2PolygonShape());
				b2Vec2 vertex1, vertex2;
				attribute = child->GetAttribute("first");
				if (!attribute.empty())
					parse_vector(attribute, &vertex1.x, &vertex1.y);
				attribute = child->GetAttribute("second");
				if (!attribute.empty())
					parse_vector(attribute, &vertex2.x, &vertex2.y);

				vertex1 *= s_SimUnitsPerGameUnit;
				vertex2 *= s_SimUnitsPerGameUnit;

				poly->SetAsEdge(vertex1, vertex2);

				fixtureDef.SetShape(poly);
			}

			else
				continue; // Unknown fixture type, ignore

			child->GetAttribute("friction", &fixtureDef.definition.friction, false);
			child->GetAttribute("restitution", &fixtureDef.definition.restitution, false);
			child->GetAttribute("density", &fixtureDef.definition.density, false);
			attribute = child->GetAttribute("is_sensor");
			fixtureDef.definition.isSensor = (attribute == "t" || attribute == "1" || attribute == "true");
			// TODO: seperate <Filter> element?
			child->GetAttribute("group_index", &fixtureDef.definition.filter.groupIndex, false);

			m_Fixtures.push_back(fixtureDef);
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
		: m_Abstract(false),
		m_HasBody(false)
	{
	}

	EntityDefinition::EntityDefinition(const std::string &current_folder, ticpp::Document &document)
		: m_Abstract(false),
		m_HasBody(false)
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

		OpenString_PhysFS(m_Script.scriptData, absolutePath);
	}

	void EntityDefinition::SetScriptData(FusionEngine::EntityDefinition::Script::ScriptType type, const std::string &data)
	{
		m_Script.type = type;
		m_Script.fileName = "inline";
		m_Script.scriptData = data;
	}

	void EntityDefinition::SetTypeId(int id)
	{
		m_TypeId = id;
	}

	int EntityDefinition::GetTypeId() const
	{
		return m_TypeId;
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

	ResourcesMap &EntityDefinition::GetStreamedResources()
	{
		return m_Resources;
	}

	ScriptedEntity::PropertiesArray &EntityDefinition::GetSyncProperties()
	{
		return m_SyncProperties;
	}

	EntityDefinition::PropertiesMap &EntityDefinition::GetSyncPropertiesMap()
	{
		return m_SyncPropertiesMap;
	}

	bool EntityDefinition::HasBody() const
	{
		return m_HasBody;
	}

	const b2BodyDef &EntityDefinition::GetBodyDef() const
	{
		return m_BodyDef;
	}

	const EntityDefinition::FixtureArray &EntityDefinition::GetFixtures() const
	{
		return m_Fixtures;
	}

	void EntityDefinition::SetBaseType(const EntityDefinitionPtr &base_type_def)
	{
		m_BaseType = base_type_def;
	}

	const EntityDefinitionPtr &EntityDefinition::GetBaseType() const
	{
		return m_BaseType;
	}

	//! Creates instances of a scripted entity type
	class ScriptedEntityInstancer : public EntityInstancer
	{
	public:
		//! Constructor
		ScriptedEntityInstancer();
		//! Constructor
		ScriptedEntityInstancer(ScriptManager *manager, EntityDefinitionPtr definition);
		//! Constructor for entity defs with phys. bodies
		ScriptedEntityInstancer(ScriptManager *manager, EntityDefinitionPtr definition, PhysicalWorld *world);

	public:
		Entity *InstanceEntity(const SupplementaryDefinitionData &sup_data, const std::string &name);

	protected:

		PhysicalWorld *m_PhysicsWorld;

		ScriptManager *m_ScriptingManager;
		std::string m_Module;
		EntityDefinitionPtr m_Definition;
	};

	ScriptedEntityInstancer::ScriptedEntityInstancer()
		: EntityInstancer("undefined_scripted_entity")
	{}

	ScriptedEntityInstancer::ScriptedEntityInstancer(ScriptManager *manager, EntityDefinitionPtr definition)
		: EntityInstancer(definition->GetType()),
		m_ScriptingManager(manager),
		m_Definition(definition),
		m_PhysicsWorld(NULL)
	{}

	ScriptedEntityInstancer::ScriptedEntityInstancer(ScriptManager *manager, EntityDefinitionPtr definition, PhysicalWorld *world)
		: EntityInstancer(definition->GetType()),
		m_ScriptingManager(manager),
		m_Definition(definition),
		m_PhysicsWorld(world)
	{}

	Entity *ScriptedEntityInstancer::InstanceEntity(const SupplementaryDefinitionData &sup_data, const std::string &name)
	{
		if (m_ScriptingManager == NULL)
			return nullptr;

		ResourceManager *resMan = ResourceManager::getSingletonPtr();
		if (resMan == NULL)
			return nullptr;

		asIScriptEngine *engine = m_ScriptingManager->GetEnginePtr();

		asIScriptObject *scrObj = static_cast<asIScriptObject*>( engine->CreateScriptObject(m_Definition->GetTypeId()) );
		ScriptObject object(scrObj, false);
		// Make sure the entity was successfully instanciated
		if (!object.IsValid())
			return nullptr;

		ScriptedEntity *entity = new ScriptedEntity(object, name);
		entity->_setDomain(m_Definition->GetDefaultDomain());
		entity->SetPath(m_Definition->GetWorkingDirectory());

		entity->SetSyncProperties(m_Definition->GetSyncProperties());

		ScriptUtils::Calling::Caller f = object.GetCaller("void _setAppObject(Entity@ obj)");
		if (f.ok())
		{
			entity->addRef();
			f(entity);
		}
		else
			return nullptr;

		// Create phys body
		if (m_Definition->HasBody())
		{
			FSN_ASSERT_MSG(m_PhysicsWorld != NULL, "The physics-world ptr for this ScriptedEntityInstancer is invalid, so physical Entities cannot be created with it.");
			b2Body *body = m_PhysicsWorld->GetB2World()->CreateBody(&m_Definition->GetBodyDef());
			// Point the entity to the body and the body to the entity
			entity->_setBody(body);
			body->SetUserData((void*)entity);
			// Set up the entity to delete the body when it is deleted
			m_PhysicsWorld->PrepareEntity(entity);

			// Add fixtures
			const EntityDefinition::FixtureArray &fixtures = m_Definition->GetFixtures();
			for (EntityDefinition::FixtureArray::const_iterator it = fixtures.begin(), end = fixtures.end();
				it != end; ++it)
			{
				const FixtureDefinition &fixtureDef = *it;
				entity->CreateFixture(&fixtureDef.definition);
			}
			// ... And supplemental fixtures
			for (SupplementaryDefinitionData::FixtureList::const_iterator it = sup_data.fixtures.begin(), end = sup_data.fixtures.end();
				it != end; ++it)
			{
				const FixtureDefinition &fixtureDef = *it;
				entity->CreateFixture(&fixtureDef.definition);
			}
		}

		// Add resource refs
		//StreamedResourceUserPtr resourceUser;
		const ResourcesMap &resources = m_Definition->GetStreamedResources();
		for (ResourcesMap::const_iterator it = resources.begin(), end = resources.end(); it != end; ++it)
		{
			const ResourceDescription &desc = it->second;
			std::string resourceName;
			// Check for an overriding property in the supplemental data
			SupplementaryDefinitionData::PropertyOverrideMap::const_iterator _where = sup_data.properties.find(desc.GetPropertyName());
			if (_where != sup_data.properties.end())
				resourceName = _where->second;
			else
				resourceName = desc.GetResourceName();

			if (desc.GetType() == "Sprite")
			{
				RenderableSpritePtr renderable( new RenderableSprite(resMan, resourceName, desc.GetPriority()) );
				renderable->SetTags(desc.GetTags());

				entity->AddRenderable(renderable);

				// Add the object to the entity for automatic streaming
				entity->AddStreamedResource( renderable.get() );

				if (desc.GetPropertyIndex() >= 0)
				{
					void *prop = scrObj->GetAddressOfProperty(desc.GetPropertyIndex());
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

				void *prop = scrObj->GetAddressOfProperty(desc.GetPropertyIndex());
				SoundSample **soundProp = static_cast<SoundSample**>( prop );
				*soundProp = new SoundSample(resMan, resourceName, desc.GetPriority(), false);

				entity->AddStreamedResource( *soundProp );
			}
			else if (desc.GetType() == "SoundStream")
			{
				// Check that the property listed in the description is correct
				FSN_ASSERT( desc.GetPropertyIndex() < scrObj->GetPropertyCount() &&
					desc.GetPropertyName() == std::string(scrObj->GetPropertyName(desc.GetPropertyIndex())) );

				void *prop = scrObj->GetAddressOfProperty(desc.GetPropertyIndex());
				SoundSample **soundProp = static_cast<SoundSample**>( prop );
				*soundProp = new SoundSample(resMan, resourceName, desc.GetPriority(), true);

				entity->AddStreamedResource( *soundProp );
			}
		}

		return entity;
	}


	EntityFactory::EntityFactory()
	{
		m_Log = Logger::getSingleton().OpenLog("entity_factory");
	}

	EntityFactory::~EntityFactory()
	{
	}

	EntityPtr EntityFactory::InstanceEntity(const std::string &type, const SupplementaryDefinitionData &sup_data, const std::string &name)
	{
		EntityInstancerMap::iterator _where = m_EntityInstancers.find(type);
		if (_where != m_EntityInstancers.end())
		{
			EntityPtr entity( _where->second->InstanceEntity(sup_data, name.empty() ? "default" : name), false );
			if (entity)
				SignalEntityInstanced(entity);
			return entity;
		}
		else
		{
			// Check for a scripted entity type that hasn't been loaded
			StringMap::iterator _whereFile = m_EntityDefinitionFileNames.find(type);
			if (_whereFile != m_EntityDefinitionFileNames.end())
			{
				SendToConsole("Entity Factory",
					"Tried to instance an Entity for which there is a known definition file that hasn't been compiled: "
					"Please add all required Entities to the <Dependencies> element of your definition file, and check "
					" that there are no compile errors.");
			}
		}
		return EntityPtr();
	}

	EntityPtr EntityFactory::InstanceEntity(const std::string &type, const std::string &name)
	{
		return InstanceEntity(type, SupplementaryDefinitionData(), name);
	}

	void EntityFactory::AddInstancer(const std::string &type, const FusionEngine::EntityInstancerPtr &instancer)
	{
		m_EntityInstancers[type] = instancer;
	}

	bool EntityFactory::LoadScriptedType(const std::string &type)
	{
		StringMap::iterator _where = m_EntityDefinitionFileNames.find(type);
		if (_where != m_EntityDefinitionFileNames.end())
		{
			try
			{
				ticpp::Document document( OpenXml_PhysFS(_where->second) );
				loadAllDependencies(fe_getbasepath(_where->second), document);
			}
			catch (ticpp::Exception &)
			{
				return false;
			}
			catch (FileSystemException &)
			{
				return false;
			}
			return true;
		}

		return false;
	}

	void EntityFactory::LoadAllScriptedTypes()
	{
		for (StringMap::const_iterator it = m_EntityDefinitionFileNames.begin(), end = m_EntityDefinitionFileNames.end(); it != end; ++it)
		{
			const std::string &filename = it->second;
			try
			{
				ticpp::Document document( OpenXml_PhysFS(filename) );

				EntityDefinitionPtr definition( new EntityDefinition(fe_getbasepath(filename), document) );
				m_LoadedEntityDefinitions.push_back(definition);
				m_EntityDefinitionsByType[definition->GetType()] = definition;
			}
			catch (ticpp::Exception &ex)
			{
				m_Log->AddEntry("The Entity definition file '" + filename + "' has invalid XML: " + ex.what(), LOG_NORMAL);
			}
			catch (FileSystemException &ex)
			{
				m_Log->AddEntry("Failed to open Entity definition file '" + filename + "': " + ex.ToString(), LOG_NORMAL);
			}
		}
	}

	void EntityFactory::SetScriptedEntityPath(const std::string &path)
	{
		m_ScriptedEntityPath = path;

		parseScriptedEntities(m_ScriptedEntityPath.c_str());
	}

	void EntityFactory::SetScriptingManager(ScriptManager *manager)
	{
		m_ScriptingManager = manager;
	}

	void EntityFactory::SetModule(const ModulePtr &module)
	{
		m_Module = module;

		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild(boost::bind(&EntityFactory::OnModuleRebuild, this, _1));
	}

	void EntityFactory::GetTypes(StringVector &types, bool sort)
	{
		types.reserve(m_EntityInstancers.size());
		for (EntityInstancerMap::iterator it = m_EntityInstancers.begin(), end = m_EntityInstancers.end(); it != end; ++it)
		{
			if (sort && !types.empty())
			{
				StringVector::iterator lowerBound = std::lower_bound(types.begin(), types.end(), it->first);
				types.insert(lowerBound, it->first);
				/*for (StringVector::iterator s_it = types.begin(), s_end = types.end()-1; s_it != s_end; ++s_it)
				{
					if (it->first.compare(*s_it) < 0)
						types.insert(s_it, it->first);
				}*/
			}
			else
				types.push_back(it->first);
		}
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

	bool register_unwrap_type(ScriptManager *manager, const std::string &module_name, const std::string &type)
	{
		std::string script =
			type + "@ unwrap_" + type + "(Entity@ entity) {\n" +
			" return cast<" + type + ">(unwrap(entity));\n}";
		return manager->AddCode(script, module_name.c_str(), (type + "_unwrap_type").c_str());
	}

	void EntityFactory::OnModuleRebuild(BuildModuleEvent &ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
		{
			// Add the script-Entity base-class
			ev.manager->AddFile("core/entity/ScriptEntity.as", ev.module_name);

			for (EntityDefinitionArray::iterator it = m_LoadedEntityDefinitions.begin(), end = m_LoadedEntityDefinitions.end();
				it != end; ++it)
			{
				EntityDefinitionPtr &def = *it;
				EntityDefinition::Script &script = def->GetScript();
				if (script.fileName == "inline")
					ev.manager->AddCode(script.scriptData, ev.module_name, (def->GetType() + "_inline").c_str());
				else
					ev.manager->AddFile(script.fileName, ev.module_name);

				register_unwrap_type(ev.manager, ev.module_name, def->GetType());

				const EntityDefinition::DependenciesMap &deps = def->GetScriptDependencies();
				for (EntityDefinition::DependenciesMap::const_iterator it = deps.begin(), end = deps.end(); it != end; ++it)
				{
					bool success = ev.manager->AddFile(*it, ev.module_name);
					if (!success)
						SendToConsole("Couldn't load script file '" + *it + "', which is a required UtilityScript for " + def->GetType());
				}
			}
		}

		else if (ev.type == BuildModuleEvent::PostBuild)
		{
			//verifyTypes();
			asIScriptModule *mod = ev.manager->GetEnginePtr()->GetModule(ev.module_name);
			ScriptedEntity::SetScriptEntityTypeId(mod->GetTypeIdByDecl("ScriptEntity"));
			for (EntityDefinitionArray::iterator it = m_LoadedEntityDefinitions.begin(), end = m_LoadedEntityDefinitions.end(); it != end; ++it)
				createScriptedEntityInstancer(*it);
		}
	}

	void EntityFactory::loadAllDependencies(const std::string &working_directory, ticpp::Document &document)
	{
		// Load the Entity definition from the given document (this is the root of the dependency tree)
		EntityDefinitionPtr definition( new EntityDefinition(working_directory, document) );
		m_LoadedEntityDefinitions.push_back(definition);
		m_EntityDefinitionsByType[definition->GetType()] = definition;

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

				try
				{
					ticpp::Document depDocument( OpenXml_PhysFS(_where->second) );

					// Parse the Entity definition document
					definition = EntityDefinitionPtr( new EntityDefinition(depPath, depDocument) );
				}
				catch (ticpp::Exception &)
				{
					continue;
				}

				m_LoadedEntityDefinitions.push_back(definition);
				m_EntityDefinitionsByType[definition->GetType()] = definition;

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
				TiXmlDocument *depDocument = OpenXml_PhysFS(_where->second);
				
				loadAllDependencies(depPath, depDocument);
			}
		}*/
	}

	bool isSyncableType(int type_id, ScriptManager *script_manager)
	{
		int valueTypeId = type_id & ~asTYPEID_OBJHANDLE;

		// Check for array types (an array type is supported as long as the sub type is)
		if (type_id & asTYPEID_TEMPLATE)
		{
			asIObjectType *arrayType = script_manager->GetEnginePtr()->GetObjectTypeById(type_id);

			std::string name = script_manager->GetEnginePtr()->GetTypeDeclaration(arrayType->GetSubTypeId());
			name += "& opIndex(uint)";
			bool hasIndexOp = false;
			for (int i = 0, count = arrayType->GetMethodCount(); i < count; ++i)
			{
				asIScriptFunction* method = arrayType->GetMethodDescriptorByIndex(i);
				if (name.compare(method->GetDeclaration(false)) == 0)
				{
					hasIndexOp = true;
					break;
				}
			}

			if (!hasIndexOp)
				return false;
			valueTypeId = arrayType->GetSubTypeId() & ~asTYPEID_OBJHANDLE;

			//if (arrayType->GetMethodIdByDecl(name.c_str()) < 0)
			//	return false;
			//if (strcmp(arrayType->GetName(), defaultArrayType->GetName()) == 0)

			//switch (valueTypeId)
			//{
			//	// Primatives
			//case asTYPEID_BOOL:
			//	name = "bool";
			//case asTYPEID_INT8:
			//	name = "int8";
			//case asTYPEID_INT16:
			//case asTYPEID_INT32:
			//case asTYPEID_INT64:
			//case asTYPEID_UINT8:
			//case asTYPEID_UINT16:
			//case asTYPEID_UINT32:
			//case asTYPEID_UINT64:
			//case asTYPEID_FLOAT:
			//case asTYPEID_DOUBLE:

			//default:
			//	asIObjectType *subType = script_manager->GetEnginePtr()->GetObjectTypeById(arrayType->GetSubTypeId());
			//};
		}

		switch (valueTypeId)
		{
			// Primatives
		case asTYPEID_BOOL:
		case asTYPEID_INT8:
		case asTYPEID_INT16:
		case asTYPEID_INT32:
		case asTYPEID_INT64:
		case asTYPEID_UINT8:
		case asTYPEID_UINT16:
		case asTYPEID_UINT32:
		case asTYPEID_UINT64:
		case asTYPEID_FLOAT:
		case asTYPEID_DOUBLE:
			return true;

		default:
			// Non-constant type IDs (for app. registered types)
			if (valueTypeId == ScriptedEntity::s_EntityTypeId ||
				valueTypeId == script_manager->GetStringTypeId() ||
				valueTypeId == script_manager->GetVector2DTypeId())
				return true;
			else
				return false;
		}
	}

	void EntityFactory::createScriptedEntityInstancer(EntityDefinitionPtr definition)
	{
		// Create an instance of the script object
		int typeId = m_Module->GetASModule()->GetTypeIdByDecl(definition->GetType().c_str());
		if (typeId < 0)
		{
			SendToConsole("Couldn't create an Entity instancer for the type '" + definition->GetType() +
				"' because it doesn't exist in the script module (most likely due to a compilation error).");
			return;
		}
		definition->SetTypeId(typeId);

		asIScriptEngine *engine = m_ScriptingManager->GetEnginePtr();

		// Verify type
		asIObjectType *entityInterface = engine->GetObjectTypeById(engine->GetTypeIdByDecl("IEntity"));
		asIObjectType *objectType = engine->GetObjectTypeById(typeId);
		if (!ScriptUtils::Inheritance::base_implements(objectType, entityInterface))
		{
			SendToConsole("The Entity type '" + definition->GetType() + "' does not implement IEntity, so no instancer was created.");
			return;
		}

		// Dynamically build the definition base-type links
		{
			typedef std::vector<EntityDefinitionPtr> BaseDefsT;
			BaseDefsT baseDefs;
			{
				EntityDefinitionPtr currentDefinition = definition;
				asIObjectType *baseType = objectType->GetBaseType();
				while (baseType != NULL)
				{
					if (currentDefinition->GetBaseType()) // base type has already been set (this means all base types of this one have also been set)
						break;

					ScriptEntityDefinitionMap::iterator _where = m_EntityDefinitionsByType.find(baseType->GetName());
					if (_where != m_EntityDefinitionsByType.end())
					{
						currentDefinition->SetBaseType(_where->second);
						currentDefinition = _where->second;
					}
					baseType = baseType->GetBaseType();
				}
			}

			if (baseDefs.size() > 1)
			{
				for (BaseDefsT::iterator it = baseDefs.end()-1, end = baseDefs.begin(), base_it = baseDefs.end();
					it != end;
					--it, --base_it)
				{
					EntityDefinitionPtr &derrived = *it;
					const EntityDefinitionPtr &base = *base_it;

					EntityDefinition::PropertiesMap &baseProps = base->GetSyncPropertiesMap();
					derrived->GetSyncPropertiesMap().insert(baseProps.begin(), baseProps.end());

					ResourcesMap &baseResources = base->GetStreamedResources();
					derrived->GetStreamedResources().insert(baseResources.begin(), baseResources.end());
				}
			}
		}

		// Find the index for each script property listed in the Sync section of the Entity definition file
		//  Note that it is important that this is done after all the definitions have been loaded and
		//  the script module has been built
		EntityDefinition::PropertiesMap &syncPropertiesByName = definition->GetSyncPropertiesMap();
		ScriptedEntity::PropertiesArray &syncProperties = definition->GetSyncProperties();
		// Iterate through all of the script object's properties
		for (int i = 0; i < objectType->GetPropertyCount(); ++i)
		{
			const char* name;
			int typeId;
			objectType->GetProperty(i, &name, &typeId);
			EntityDefinition::PropertiesMap::iterator _where = syncPropertiesByName.find(name);
			if (_where != syncPropertiesByName.end())
			{
				ScriptedEntity::Property &prop = _where->second;

				int propertyType = ScriptedEntity::ScriptTypeIdToPropertyType(typeId);
				if (propertyType != 0)
				{
					prop.typeFlags = propertyType;
					prop.scriptPropertyIndex = i;
					// Add the property def to the array (which will be passed to ScriptedEntities when they are created)
					syncProperties.push_back(prop);
				}
				else
				{
					prop.scriptPropertyIndex = -1;
					std::string type = engine->GetTypeDeclaration(typeId);
					SendToConsole("Creating instancer for a scripted entity: The property '"
						+ prop.name + "' in " + definition->GetType() +
						" given in a <sync> element is of type " + type + " - objects of that type cannot be synchronized.");
				}
			}
		}
		// Erase synced-property defs that are missing from the script type
		{
			ScriptedEntity::PropertiesArray::iterator it = syncProperties.begin(), end = syncProperties.end();
			while (it != end)
			{
				if (it->scriptPropertyIndex < 0)
				{
					SendToConsole("Creating instancer for a scripted entity: There is no property called '"
						+ it->name + "' in " + definition->GetType() +
						" as indicated in the <Sync> element of the xml definition file - i.e. the definition is incorrect.");
					syncProperties.erase(it++);
					end = syncProperties.end();
				}
				else
					++it;
			}
		}

		// Same as above for Streaming section
		ResourcesMap &resources = definition->GetStreamedResources();
		for (int i = 0; i < objectType->GetPropertyCount(); ++i)
		{
			const char* name;
			objectType->GetProperty(i, &name);
			ResourcesMap::iterator _where = resources.find(name);
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
					end = resources.end();
				}
				else
					++it;
			}
		}

		EntityInstancerPtr instancer;
		if (definition->HasBody())
			instancer.reset( new ScriptedEntityInstancer(m_ScriptingManager, definition, PhysicalWorld::getSingletonPtr()) );
		else
			instancer.reset( new ScriptedEntityInstancer(m_ScriptingManager, definition) );
		m_EntityInstancers[definition->GetType()] = instancer; 
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
					TiXmlDocument *doc = OpenXml_PhysFS(fullPath);
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
