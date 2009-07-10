/*
  Copyright (c) 2007 Fusion Project Team

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

#ifndef Header_FusionEngine_EntityFactory
#define Header_FusionEngine_EntityFactory

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

namespace FusionEngine
{

	class EntityInstancer
	{
	public:
		EntityInstancer(const std::string &type);

		Entity *InstanceEntity(const std::string &name) = 0;

		const std::string &GetType() const;

	private:
		std::string m_Type;
	};

	typedef std::tr1::shared_ptr<EntityInstancer> EntityInstancerPtr;

	EntityInstancer::EntityInstancer(const std::string &type)
		: m_Type(type)
	{}

	const std::string &EntityInstancer::GetType() const
	{
		return m_Type;
	}

	//! Creates root entities
	class RootEntityInstancer : public EntityInstancer
	{
	public:
		RootEntityInstancer();

		Entity *InstanceEntity(const std::string &name)
	};

	RootEntityInstancer::RootEntityInstancer()
		: EntityInstancer("root")
	{}

	Entity *RootEntityInstancer::InstanceEntity(const std::string &name)
	{
	}

	// TODO: this should be in the source file
	//! Creates instances of a scripted entity type
	class ScriptedEntityInstancer : public EntityInstancer
	{
	public:
		//! Constructor
		ScriptedEntityInstancer();
		//! Constructor
		ScriptedEntityInstancer(const std::string &type);

	public:
		Entity *InstanceEntity(const std::string &name);
	};

	ScriptedEntityInstancer::ScriptedEntityInstancer()
		: EntityInstancer("generic_scripted_entity")
	{}

	ScriptedEntityInstancer::ScriptedEntityInstancer(const std::string &type)
		: EntityInstancer(type)
	{}

	ScriptedEntityInstancer::InstanceEntity(const std::string &name)
	{
	}

	//! Creates entities
	/*!
	 * Creates built-in entities using registered instancers or scripted
	 * entities loaded from files.
	 */
	class EntityFactory
	{
	public:
		//! Constructor
		EntityFactory();

		//! Destructor
		~EntityFactory();

	public:
		//!
		/*!
		* Returns an entity object of the requested type, or NULL. The type will be
		* added to the Used Type List, so so when ClearUnusedInstancers is called the
		* relavant instancer will not be removed
		*/
		EntityPtr InstanceEntity(const std::string &type);

		//! Creates an instancer for the the given scripted type
		void LoadScriptedEntity(const std::string &type);

		//! Sets the path where scripted entity files can be found
		void SetScriptedEntityPath(const std::string &path);

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
		* still require loading, but you can't get everything).
		* If an instancer hasn't been used since ResetUsedTypesList was last
		* called it is considered unused and will be removed by this method.
		*/
		void ClearUnusedInstancers();

	protected:
		//! Maps tags to entity definitions
		typedef std::tr1::unordered_map<std::string, EntityInstancerPtr> EntityInstancerList;

		EntityInstancerList m_EntityInstancers;

	};

}

#endif
