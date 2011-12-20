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
	//typedef ISystemWorld* ComponentInstancerPtr;

	//! Prefab base class
	class EntityTemplate
	{
	public:
		EntityTemplate() {}

		//! CTOR
		EntityTemplate(const std::string &type)
			: m_Type(type)
		{}

		typedef std::shared_ptr<std::vector<char>> ComponentData_t;

		const std::pair<std::string, ComponentData_t>& GetTransform() const { return m_Transform; }

		typedef std::vector<std::tuple<std::string, std::string, ComponentData_t>> Composition_t;
		const Composition_t& GetComposition() const { return m_Composition; }

		//! Sets the type of this instancer
		void SetTypeName(const std::string &type) { m_Type = type; }
		//! Gets the type of this instancer
		const std::string &GetTypeName() const { return m_Type; }

		std::string m_Type;
		std::string m_DefaultDomain;
		Composition_t m_Composition;
		std::pair<std::string, ComponentData_t> m_Transform;
	};

	typedef std::tr1::shared_ptr<EntityTemplate> EntityTemplatePtr;

	//! Creates entities
	/*!
	 * Creates built-in entities using registered instancers or scripted
	 * entities loaded from files.
	 */
	class EntityFactory
	{
		// TEMP (access to m_ComponentInstancers):
		friend class EntityManager;

	public:
		//! Constructor
		EntityFactory();

		//! Destructor
		~EntityFactory();

	public:
		//! Instantiates a component of the given type
		ComponentPtr InstanceComponent(const std::string& type, const Vector2& position = Vector2::zero(), float angle = 0.f);

		//! Instances Entity
		/*!
		* Returns an entity object of the requested type, or NULL.
		*/
		EntityPtr InstanceEntity(const std::string &template_type, const Vector2& position, float angle);

		//! Instantiates an Entity composed of the given components
		EntityPtr InstanceEntity(const std::vector<std::string>& composition, const Vector2& position, float angle);

		//! Adds an instancer object for the given type
		void AddInstancer(const std::string &type, const ComponentInstancerPtr &instancer);

		void AddInstancer(const ComponentInstancerPtr &instancer);

		std::pair<bool, Vector2> DeserialisePosition(RakNet::BitStream& in, const Vector2& origin, const float radius);
		void SerialisePosition(RakNet::BitStream& out, ComponentPtr tf, const Vector2& origin, const float radius);

		//! Creates an instancer for the the given scripted type
		bool LoadPrefabType(const std::string &type);

		//! Loads all the scripted types within the current ScriptedEntityPath
		void LoadAllPrefabTypes(const std::string &type);

		//! Sets the path where scripted entity files can be found
		void SetScriptedEntityPath(const std::string &path);

		//! Returns the names of types with instancers available to the factory
		void GetTypes(StringVector &types, bool sort = false);

		//! Fired whenever an entity is instanced
		boost::signals2::signal<void (EntityPtr &)> SignalEntityInstanced;

		static void Register(asIScriptEngine *engine);

	protected:
		bool getEntityType(TiXmlDocument *document, std::string &type);

		LogPtr m_Log;

		std::map<std::string, ComponentInstancerPtr> m_ComponentInstancers;

		std::unordered_map<std::string, EntityTemplatePtr> m_PrefabTypes;

		typedef std::tr1::unordered_set<std::string> StringSet;
		StringSet m_UsedTypes;

		std::string m_ScriptedEntityPath;

	};

}

#endif
