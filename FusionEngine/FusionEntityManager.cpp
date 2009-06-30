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

#include "FusionEntityManager.h"

namespace FusionEngine
{

	class update_state
	{
	public:
		update_state( )
		{
		}

		void operator() (EntityManager::InputStateMap::value_type &elem)
		{
			elem.second.m_Down = elem.second.m_Active;
			// So far the input has been active for the entire step, since
			//  this is the start of the step ;)
			elem.second.m_ActiveRatio = elem.m_Active ? 1.f : 0.f;
			elem.second.m_ActiveFirst = elem.m_Active;
		}
	};

	EntityManager::EntityManager()
		: m_UpdateBlockedTags(0),
		m_DrawBlockedTags(0),
		m_EntitiesLocked(false)
	{
	}

	EntityManager::~EntityManager()
	{
	}

	void EntityManager::AddEntity(EntityPtr entity)
	{
		if (m_EntitiesLocked)
			m_EntitiesToAdd.push_back(entity);

		else
		{
			if (entity->GetName() == "default")
				entity->_setName(generateName());

			EntityMap::iterator _where = m_Entities.find(entity->GetName());
			if (_where != m_Entities.end())
				FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::AddEntity", "An entity named " + entity->GetName() + " already exists");
			m_Entities[entity->GetName()] = entity;
		}
	}

	void EntityManager::RemoveEntity(EntityPtr entity)
	{
		if (m_EntitiesLocked)
			m_EntitiesToRemove.push_back(entity);
		else
			m_Entities.erase(entity->GetName());
	}

	EntityPtr EntityManager::GetEntity(const std::string &name)
	{
		EntityMap::iterator _where = m_Entities.find(name);
		if (_where == m_Entities.end())
			FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::GetEntity", std::string("There is no entity called: ") + name);
		return _where->second;
	}

	bool EntityManager::AddTag(const std::string &entity_name, const std::string &tag)
	{
		try
		{
			EntityPtr entity = GetEntity(entity_name);
			return AddTag(entity, tag);
		}
		catch (InvalidArgumentException &ex)
		{
			return false;
		}
	}

	bool EntityManager::AddTag(EntityPtr entity, const std::string &tag)
	{
		entity->AddTagFlag( getTagFlag(tag, true) );
		return true;
	}

	void EntityManager::RemoveTag(const std::string &entity_name, const std::string &tag)
	{
		try
		{
			EntityPtr entity = GetEntity(entity_name);
			RemoveTag(entity);
		}
		catch (InvalidArgumentException &ex)
		{
		}
	}

	void EntityManager::RemoveTag(EntityPtr entity, const std::string &tag)
	{
		try
		{
			entity->RemoveTagFlag( getTagFlag(tag, false) );
		}
		catch (InvalidArgumentException &ex) // The given tag doesn't exist
		{
		}
	}

	void EntityManager::Clear()
	{
		m_EntitiesToRemove.clear();
		m_Entities.clear();
	}

	void EntityManager::Update(float split)
	{
		m_EntitiesLocked = true;

		for (EntityMap::iterator it = m_Entities.begin(), end = m_Entities.end();
			it != end; ++it)
		{
			EntityPtr &entity = it->second;

			if (entity->GetTagFlags() & m_ToDeleteFlags)
				RemoveEntity(entity);
			if (!(entity->GetTagFlags() & m_UpdateBlockedTags))
				entity->Update(split);
		}
		// Clear the ToDeleteFlags
		m_ToDeleteFlags = 0;

		m_EntitiesLocked = false;

		for (EntityList::iterator it = m_EntitiesToRemove.begin(), end = m_EntitiesToRemove.end();
			it != end; ++it)
		{
			m_Entities.erase((*it)->GetName());
		}
		m_EntitiesToRemove.clear();
	}

	void EntityManager::Draw()
	{
		m_EntitiesLocked = true;

		for (EntityMap::iterator it = m_Entities.begin(), end = m_Entities.end();
			it != end; ++it)
		{
			EntityPtr &entity = it->second;

			if (!(entity->GetTagFlags() & m_DrawBlockedTags))
				entity->Draw();
		}

		m_EntitiesLocked = false;
	}

	unsigned int EntityManager::getTagFlag(const std::string &tag, bool generate)
	{
		unsigned int minFreeFlag = 1;
		TagFlagList::iterator _where = m_TagDictionary.begin();
		for (end = m_TagDictionary.end(); _where != end; ++_where)
		{
			if (_where->Flag == minFreeFlag)
				minFreeFlag << 1;

			if (it->Tag == tag)
				break;
		}
		//TagFlagMap::iterator _where = m_TagDictionary.find(tag);
		if (_where == m_TagDictionary.end())
		{
			if (!generate)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::getTagFlag",
					"Requested flag for the previously unused tag '" + tag + "' with new flag generation disabled");
			}

			m_TagDictionary.insert( TagDef(tag, minFreeFlag) );

			return minFreeFlag;
		}
		else
		{
			return _where->Flag;
		}
	}

	void EntityManager::updateInput(float step)
	{
		for (PlayerInputStateMaps::iterator it = m_PlayerInputStates.begin(), end = m_PlayerInputStates.end();
			it != end; ++it)
		{
			std::for_each(it->begin(), it->end(), update_state);
		}
	}

}
