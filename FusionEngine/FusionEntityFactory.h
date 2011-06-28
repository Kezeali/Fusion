/*
*  Copyright (c) 2007-2011 Fusion Project Team
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

#ifndef H_FusionEntityFactory
#define H_FusionEntityFactory

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <boost/signals2.hpp>

#include "FusionSingleton.h"

#include "FusionComponentSystem.h"
#include "FusionTypes.h"
#include "FusionXML.h"

namespace FusionEngine
{

	typedef std::shared_ptr<ISystemWorld> ComponentInstancerPtr;

	//! Prefab base class
	class Prefab
	{
	public:
		Prefab() {}

		//! CTOR
		Prefab(const std::string &type)
			: m_Type(type)
		{}

		typedef std::vector<std::pair<std::string, ComponentStaticProps>> Composition;
		const Composition& GetComposition() const { return m_Composition; }

		//! Sets the type of this instancer
		void SetTypeName(const std::string &type) { m_Type = type; }
		//! Gets the type of this instancer
		const std::string &GetTypeName() const { return m_Type; }

		std::string m_Type;
		std::string m_DefaultDomain;
		Composition m_Composition;
	};

	typedef std::tr1::shared_ptr<Prefab> PrefabPtr;

	class EntityDefinition;

	typedef std::tr1::shared_ptr<EntityDefinition> EntityDefinitionPtr;

	//! Creates entities
	/*!
	 * Creates built-in entities using registered instancers or scripted
	 * entities loaded from files.
	 */
	class EntityFactory
	{
	protected:
		//! Maps tags to entity definitions
		typedef std::tr1::unordered_map<std::string, PrefabPtr> PrefabMap;

	public:
		//! Constructor
		EntityFactory();

		//! Destructor
		~EntityFactory();

	public:
		//! Instanciates a component of the given type
		std::shared_ptr<IComponent> InstanceComponent(const std::string& type);

		//! Instances Entity
		/*!
		* Returns an entity object of the requested type, or NULL.
		*/
		EntityPtr InstanceEntity(const std::string &prefab_type);

		//! Instantiates an Entity composed of the given components
		EntityPtr InstanceEntity(const std::vector<std::string>& composition, const Vector2& position, float angle);

		//! Adds an instancer object for the given type
		void AddInstancer(const std::string &type, const ComponentInstancerPtr &instancer);

		//! Creates an instancer for the the given scripted type
		bool LoadPrefabType(const std::string &type);

		//! Loads all the scripted types within the current ScriptedEntityPath
		void LoadAllPrefabTypes();

		//! Unloads all scripted entity descriptions
		void UnloadAllScriptedTypes();

		//! Sets the path where scripted entity files can be found
		void SetScriptedEntityPath(const std::string &path);
		////! Sets the scripting manager and module used to add script sections
		//void SetScriptingManager(ScriptManager *manager);
		////! Sets the scripting module used to add script sections
		//void SetModule(const ModulePtr &module);

		//! Returns the names of types with instancers available to the factory
		void GetTypes(StringVector &types, bool sort = false);

		//! Clears the Used Types List
		/*!
		* Should be called before a new environment is loaded.
		*/
		void ResetUsedTypesList();

		//! Removes unused instancers
		/*!
		* Should be called after a new environment is loaded - this way
		* loading save games can be done fairly quickly without just keeping
		* all instancers in memory (of course, instancing types later will
		* still require loading, but this generally shouldn't happen and
		* anyway you can't win everything).
		* If an instancer hasn't been used since ResetUsedTypesList was last
		* called it is considered unused and will be removed by this method.
		*/
		void ClearUnusedInstancers();

		//! Adds entity script sections
		void OnModuleRebuild(BuildModuleEvent &ev);

		//! Fired whenever an entity is instanced
		boost::signals2::signal<void (EntityPtr &)> SignalEntityInstanced;

		static void Register(asIScriptEngine *engine);

	protected:
		void loadAllDependencies(const std::string &working_directory, ticpp::Document &document);

		void createScriptedEntityInstancer(EntityDefinitionPtr definition);

		bool getEntityType(TiXmlDocument *document, std::string &type);

		//! Recursively parses the scripted entity files below the given path
		void parseScriptedEntities(const char *path, unsigned int current_recursion = 0);

		LogPtr m_Log;

		std::map<std::string, ComponentInstancerPtr> m_ComponentInstancers;

		typedef std::tr1::unordered_set<std::string> StringSet;
		StringSet m_UsedTypes;

		std::string m_ScriptedEntityPath;

		typedef std::tr1::unordered_map<std::string, std::string> StringMap;
		StringMap m_EntityDefinitionFileNames;

		typedef std::vector<EntityDefinitionPtr> EntityDefinitionArray;
		EntityDefinitionArray m_LoadedEntityDefinitions;

		typedef std::tr1::unordered_map<std::string, EntityDefinitionPtr> ScriptEntityDefinitionMap;
		ScriptEntityDefinitionMap m_EntityDefinitionsByType;

		ScriptManager *m_ScriptingManager;
		ModulePtr m_Module;

		boost::signals2::connection m_ModuleConnection;
	};

}

#endif
