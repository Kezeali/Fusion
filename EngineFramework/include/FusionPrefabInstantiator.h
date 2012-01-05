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

#ifndef H_FusionPrefabInstantiator
#define H_FusionPrefabInstantiator

#include "FusionPrerequisites.h"

#include "FusionEntityInstantiator.h"
#include "FusionComponentFactory.h"

#include <tuple>

namespace FusionEngine
{

	class ISystemWorld;

	class PrefabInstantiationException : public Exception
	{
	public:
		PrefabInstantiationException(const std::string& description, const std::string& origin, const char* file, long line)
			: Exception(description, origin, file, line) {}
		virtual ~PrefabInstantiationException()
		{}
	};

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

	//! Instantiates pre-composed entities
	class PrefabInstantiator
	{
	public:
		PrefabInstantiator(ComponentFactory* factory, EntityInstantiator* entity_instantiator);
		virtual ~PrefabInstantiator();

		void AddPrefab(const std::shared_ptr<EntityTemplate>& type);
		void LoadXMLPrefabs(const std::string& path);

		void InstantiatePrefab(const EntityPtr& entity, const std::string& type);

	private:
		std::unordered_map<std::string, std::shared_ptr<EntityTemplate>> m_PrefabTypes;

		ComponentFactory* m_ComponentFactory;
		EntityInstantiator* m_EntityInstantiator;

		LogPtr m_Log;
	};

}

#endif
