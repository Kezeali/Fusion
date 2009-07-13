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

#include "FusionEntityFactory.h"

#include <boost/lexical_cast.hpp>


namespace FusionEngine
{

	//class update_state
	//{
	//public:
	//	update_state( )
	//	{
	//	}

	//	void operator() (EntityManager::InputStateMap::value_type &elem)
	//	{
	//		elem.second.m_Down = elem.second.m_Active;
	//		// So far the input has been active for the entire step, since
	//		//  this is the start of the step ;)
	//		elem.second.m_ActiveRatio = elem.m_Active ? 1.f : 0.f;
	//		elem.second.m_ActiveFirst = elem.m_Active;
	//	}
	//};

	EntityManager::EntityManager()
		: m_UpdateBlockedFlags(0),
		m_DrawBlockedFlags(0),
		m_EntitiesLocked(false)
	{
	}

	EntityManager::~EntityManager()
	{
	}

	EntityPtr EntityManager::InstanceEntity(const std::string &type, const std::string &name)
	{
		EntityPtr entity = m_EntityFactory->InstanceEntity(type, name);
		AddEntity(entity);
		return entity;
	}

	EntityFactory *EntityManager::GetFactory() const
	{
		return m_EntityFactory;
	}

	void EntityManager::AddEntity(EntityPtr entity)
	{
		entity->SetID(m_NextId++);

		if (m_EntitiesLocked)
			m_EntitiesToAdd.push_back(entity);

		else
		{
			if (entity->GetName() == "default")
				entity->_setName(generateName(entity));

			EntityMap::iterator _where = m_Entities.find(entity->GetID());
			if (_where != m_Entities.end())
				FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::AddEntity", "An entity with the ID " + boost::lexical_cast<std::string>(entity->GetID()) + " already exists");
			m_Entities[entity->GetID()] = entity;
		}
	}

	void EntityManager::RemoveEntity(EntityPtr entity)
	{
		if (m_EntitiesLocked)
		{
			entity->MarkToRemove(); // Mark this to make sure it isn't updated / drawn before it is removed
			m_EntitiesToRemove.push_back(entity);
		}
		else
			m_Entities.erase(entity->GetID());
	}

	void EntityManager::RemoveEntityNamed(const std::string &name)
	{
		RemoveEntity(GetEntity(name));
	}

	void EntityManager::RemoveEntityById(ObjectID id)
	{
		RemoveEntity(GetEntity(id));
	}

	bool isNamed(EntityManager::EntityMap::value_type &element, const std::string &name)
	{
		return element.second->GetName() == name;
	}

	EntityPtr EntityManager::GetEntity(const std::string &name)
	{
		EntityMap::iterator _where = std::find_if(m_Entities.begin(), m_Entities.end(), boost::bind(&isNamed, _1, name));
		if (_where == m_Entities.end())
			FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::GetEntity", std::string("There is no entity called: ") + name);
		return _where->second;
	}

	EntityPtr EntityManager::GetEntity(ObjectID id)
	{
		EntityMap::iterator _where = m_Entities.find(id);
		if (_where == m_Entities.end())
			FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::GetEntity", std::string("There is no entity with the ID: ") + boost::lexical_cast<std::string>(id));
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
		entity->AddTag(tag);
		return true;
	}

	void EntityManager::RemoveTag(const std::string &entity_name, const std::string &tag)
	{
		try
		{
			EntityPtr entity = GetEntity(entity_name);
			RemoveTag(entity, tag);
		}
		catch (InvalidArgumentException &ex)
		{
		}
	}

	void EntityManager::RemoveTag(EntityPtr entity, const std::string &tag)
	{
		try
		{
			entity->RemoveTag(tag);
		}
		catch (InvalidArgumentException &ex) // The given tag doesn't exist
		{
		}
	}

	bool EntityManager::CheckTag(const std::string &entity_name, const std::string &tag)
	{
		EntityPtr entity = GetEntity(entity_name);
		if (entity != NULL)
			return entity->CheckTag(tag);
		return false;
	}

	inline bool EntityManager::IsBlocked(EntityPtr entity, const BlockingChangeMap &tags)
	{
		for (BlockingChangeMap::const_iterator it = tags.begin(), end = tags.end(); it != end; ++it)
		{
			if (it->second && entity->CheckTag(it->first))
				return true;
		}
		return false;
	}

	void EntityManager::PauseEntitiesWithTag(const std::string &tag)
	{
		m_ChangedUpdateStateTags[tag] = false;
	}

	void EntityManager::ResumeEntitiesWithTag(const std::string &tag)
	{
		m_ChangedUpdateStateTags[tag] = true;
	}

	void EntityManager::HideEntitiesWithTag(const std::string &tag)
	{
		m_ChangedDrawStateTags[tag] = false;
	}

	void EntityManager::ShowEntitiesWithTag(const std::string &tag)
	{
		m_ChangedDrawStateTags[tag] = true;
	}

	void EntityManager::RemoveEntitiesWithTag(const std::string &tag)
	{
		for (EntityMap::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			RemoveEntity(it->second);
		}
	}

	void EntityManager::Clear()
	{
		m_EntitiesToAdd.clear();
		m_EntitiesToRemove.clear();
		m_Entities.clear();
	}

	void EntityManager::Update(float split)
	{
		m_EntitiesLocked = true;

		if (m_ChangedUpdateStateTags.empty())
		{
			for (EntityArray::iterator it = m_EntitiesToUpdate.begin(), end = m_EntitiesToUpdate.end(); it != end; ++it)
			{
				EntityPtr &entity = *it;

				if (entity->IsMarkedToRemove())
					continue;

				if (entity->GetTagFlags() & m_ToDeleteFlags)
					RemoveEntity(entity);

				else if ((entity->GetTagFlags() & m_UpdateBlockedFlags) || IsBlocked(entity, m_ChangedUpdateStateTags))
					m_EntitiesToUpdate.erase(it);
				else
					entity->Update(split);
				//updateEntity(entity, split);
			}
		}
		else
		{
			for (EntityMap::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
			{
				EntityPtr &entity = it->second;

				if (entity->IsMarkedToRemove())
					continue;

				if (entity->GetTagFlags() & m_ToDeleteFlags)
					RemoveEntity(entity);

				else if (!(entity->GetTagFlags() & m_UpdateBlockedFlags) && IsBlocked(entity, m_ChangedUpdateStateTags))
				{
					m_EntitiesToUpdate.push_back(entity);
					entity->Update(split);
				}
				//updateEntity(entity, split);
			}
			m_ChangedUpdateStateTags.clear();
		}
		m_EntitiesLocked = false;

		// Clear the ToDeleteFlags
		m_ToDeleteFlags = 0;

		// Actually add entities which were 'added' during the update
		for (EntityArray::iterator it = m_EntitiesToAdd.begin(), end = m_EntitiesToAdd.end(); it != end; ++it)
		{
			AddEntity(*it);
		}
		m_EntitiesToAdd.clear();

		// Actually remove entities which were marked for removal during update
		for (EntityArray::iterator it = m_EntitiesToRemove.begin(), end = m_EntitiesToRemove.end(); it != end; ++it)
		{
			m_Entities.erase((*it)->GetID());
		}
		m_EntitiesToRemove.clear();
	}

	void EntityManager::Draw()
	{
		m_EntitiesLocked = true;

		if (m_ChangedUpdateStateTags.empty())
		{
			for (EntityArray::iterator it = m_EntitiesToDraw.begin(), end = m_EntitiesToDraw.end(); it != end; ++it)
			{
				EntityPtr &entity = *it;

				if ((entity->GetTagFlags() & m_DrawBlockedFlags) && IsBlocked(entity, m_ChangedDrawStateTags))
					m_EntitiesToDraw.erase(it);
				else
					entity->Draw();
			}
		}
		else
		{
		}

		m_EntitiesLocked = false;
	}

	std::string EntityManager::generateName(EntityPtr entity)
	{
		std::stringstream stream;
		stream << "__entity_id_" << entity->GetID();
		return stream.str();
	}

	//void EntityManager::updateEntity(EntityPtr entity, float split)
	//{
	//	if (entity->IsMarkedToRemove())
	//		continue;

	//	if (entity->GetTagFlags() & m_ToDeleteFlags)
	//		RemoveEntity(entity);

	//	else if ((entity->GetTagFlags() & m_UpdateBlockedFlags) || IsBlocked(entity, m_ChangedUpdateStateTags))
	//		m_EntitiesToUpdate.erase(it);
	//	else
	//		entity->Update(split);
	//}

	//unsigned int EntityManager::getTagFlag(const std::string &tag, bool generate)
	//{
	//	unsigned int minFreeFlag = 1;
	//	TagFlagList::iterator _where = m_TagDictionary.begin();
	//	for (end = m_TagDictionary.end(); _where != end; ++_where)
	//	{
	//		if (_where->Flag == minFreeFlag)
	//			minFreeFlag << 1;

	//		if (it->Tag == tag)
	//			break;
	//	}
	//	//TagFlagMap::iterator _where = m_TagDictionary.find(tag);
	//	if (_where == m_TagDictionary.end())
	//	{
	//		if (!generate)
	//		{
	//			FSN_EXCEPT(ExCode::InvalidArgument, "EntityManager::getTagFlag",
	//				"Requested flag for the previously unused tag '" + tag + "' with new flag generation disabled");
	//		}

	//		m_TagDictionary.insert( TagDef(tag, minFreeFlag) );

	//		return minFreeFlag;
	//	}
	//	else
	//	{
	//		return _where->Flag;
	//	}
	//}

	//void EntityManager::updateInput(float step)
	//{
	//	for (PlayerInputStateMaps::iterator it = m_PlayerInputStates.begin(), end = m_PlayerInputStates.end();
	//		it != end; ++it)
	//	{
	//		std::for_each(it->begin(), it->end(), update_state);
	//	}
	//}

}
