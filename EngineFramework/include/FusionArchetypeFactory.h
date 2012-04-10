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

#ifndef H_FusionArchetypeFactory
#define H_FusionArchetypeFactory

#include "FusionPrerequisites.h"

#include "FusionVectorTypes.h"
#include "FusionTypes.h"

#include <string>
#include <map>
#include <memory>

namespace FusionEngine
{

	class Archetype;

	//! Used to define archetypes
	class ArchetypeFactory
	{
	public:
		ArchetypeFactory();
		virtual ~ArchetypeFactory();

		//! Returns an existing archetype
		std::shared_ptr<Archetype> GetArchetype(const std::string& type_id) const;

		//! Creates a new archetype
		std::shared_ptr<Archetype> CreateArchetype(const std::string& type_id);

		//! Makes the given entity an instance of the given archetype
		void MakeInstance(const EntityPtr& entity, const std::string& type_id, const Vector2& pos, float angle);

		//! Defines the given archetype using the given entity
		void DefineArchetypeFromEntity(const std::string& type_id, const EntityPtr& entity);

	private:
		std::map<std::string, std::shared_ptr<Archetype>> m_Archetypes;
	};

}

#endif