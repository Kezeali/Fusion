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

#ifndef H_FusionEntityInstantiator
#define H_FusionEntityInstantiator

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntity.h"
#include <iostream>

namespace FusionEngine
{

	class EntityInstantiator
	{
	public:
		virtual ~EntityInstantiator()
		{}

		//! Resets the used ID lists, setting the min world id to the one given.
		virtual void Reset(ObjectID min_unused = 0) = 0;

		virtual void SaveState(std::ostream& stream) = 0;
		virtual void LoadState(std::istream& stream) = 0;

		//! Removes the given ID from the available pool
		virtual void TakeID(ObjectID id) = 0;
		//! Puts the given ID back in the pool
		virtual void FreeID(ObjectID id) = 0;
		//! Returns a free global ID (used when creating entities in the editor)
		virtual ObjectID GetFreeGlobalID() = 0;

		//! Instance (named)
		virtual EntityPtr RequestInstance(EntityPtr &requester, bool synced, const std::string &transform_type, const std::string &name, Vector2 pos, float angle, PlayerID instance_owner = 0) = 0;
		//! Instance
		virtual EntityPtr RequestInstance(EntityPtr &requester, bool synced, const std::string &transform_type, Vector2 pos, float angle, PlayerID instance_owner = 0) = 0;

		//! Returns the given Entity's ID to the pool
		virtual void RemoveInstance(EntityPtr& entity) = 0;

		// TODO: ?move to ComponentInstantiator
		virtual ComponentPtr AddComponent(const EntityPtr& entity, const std::string& type, const std::string& identifier) = 0;
		virtual bool RemoveComponent(const EntityPtr& entity, const ComponentPtr& component) = 0;
		virtual bool RemoveComponent(const EntityPtr& entity, const std::string& type, const std::string& identifier)
		{
			auto com = entity->GetComponent(type, identifier);
			if (com)
			{
				return RemoveComponent(entity, com);
			}
			else
			{
				//FSN_EXCEPT(InvalidArgumentException,
				//	"Tried to remove non-existant component of type '" + type + "'" + (identifier.empty() ? "" : " with id '" + identifier + "'") + " from an entity");
				return false;
			}
		}
	};

}

#endif
