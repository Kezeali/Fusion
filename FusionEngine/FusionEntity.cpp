
#include "FusionCommon.h"

// Fusion

// Class
#include "FusionEntity.h"

namespace FusionEngine
{

	bool TagFlagDictionary::AddTag(const std::string &tag, EntityPtr entity, bool add_flag)
	{
		TagFlagMap::iterator _where = m_Entries.find(tag);
		if (_where == m_Entries.end())
		{
			_where->second.Tag = tag;

			if (add_flag)
			{
				_where->second.Flag = m_MinFreeFlag; // If there are no flags left this will be zero, so Flag will be set correctly even in that case

				if (m_MinFreeFlag != 0)
					takeMinFreeFlag(); // Update m_MinFreeFlag and the FreeFlags mask
			}
		}

		_where->second.References.insert(entity->GetName());
		// If there is a flag for the given tag add it to the given entity
		if (add_flag && _where->second.Flag != 0)
			entity->AddTagFlag(_where->second.Flag);

		return add_flag && m_MinFreeFlag != 0; // Report whether a flag was given
	}

	void TagFlagDictionary::RemoveTag(const std::string &tag, EntityPtr entity)
	{
		TagFlagMap::iterator _where = m_Entries.find(tag);
		if (_where != m_Entries.end())
		{
			Entry &entry = _where->second;
			entry.References.erase(entity->GetName());
			entity->RemoveTagFlag(_where->second.Flag);

			// If there are no more entities using this flag
			if (entry.References.empty())
			{
				flagFreed(entry.Flag);
				m_Entries.erase(_where);
			}
		}
	}

	bool TagFlagDictionary::RequestFlagFor(const std::string &tag)
	{
		if (m_MinFreeFlag == 0)
			return false;

		TagFlagMap::iterator _where = m_Entries.find(tag);
		if (_where != m_Entries.end())
		{
			_where->second.Flag = m_MinFreeFlag;
		}

		return true;
	}

	void TagFlagDictionary::ForceFlagFor(const std::string &tag)
	{
		FSN_EXCEPT(ExCode::NotImplemented, "TagFlagDictionary::ForceFlagFor", "ForceFlagFor() is not implemented.");
	}

	unsigned int TagFlagDictionary::GetFlagFor(const std::string &tag)
	{
		TagFlagMap::iterator _where = m_Entries.find(tag);
		if (_where != m_Entries.end())
			return _where->second.Flag;
		else
			return 0; // No such tag
	}

	void TagFlagDictionary::takeMinFreeFlag()
	{
		m_FreeFlags &= ~m_MinFreeFlag; // remove the chosen flag from the FreeFlags mask

		// Set m_MinFreeFlag to the next free flag
		unsigned int checkFlag = m_MinFreeFlag;
		while (checkFlag != 0)
		{
			checkFlag = checkFlag << 1;
			if (m_FreeFlags & checkFlag)
			{
				m_MinFreeFlag = checkFlag;
				return;
			}
		}

		// No more free flags
		m_MinFreeFlag = 0;
	}

	void TagFlagDictionary::flagFreed(unsigned int flag)
	{
		if (flag < m_MinFreeFlag) m_MinFreeFlag = flag;
		m_FreeFlags |= flag;
	}

	Entity::Entity()
		: m_Name("default"),
		m_Flags(0),
		m_MarkedToRemove(false)
	{
	}

	Entity::Entity(const std::string &name)
		: m_Name(name),
		m_Flags(0),
		m_MarkedToRemove(false)
	{
	}


	Entity::~Entity()
	{
		// Nothing to do here
	}

	void Entity::_setName(const std::string &name)
	{
		m_Name = name;
	}

	const std::string &Entity::GetName() const
	{
		return m_Name;
	}

	void Entity::SetID(ObjectID id)
	{
		m_Id = id;
	}

	ObjectID Entity::GetID() const
	{
		return m_Id;
	}

	void Entity::AddTag(const std::string &tag)
	{
		m_Tags.insert(tag);
	}

	void Entity::RemoveTag(const std::string &tag)
	{
		m_Tags.erase(tag);
	}

	void Entity::ClearTags()
	{
		m_Tags.clear();
	}

	bool Entity::CheckTag(const std::string &tag) const
	{
		return m_Tags.find(tag) != m_Tags.end();
	}

	StringVector Entity::GetTags() const
	{
		StringVector tagArray;
		for (TagSet::const_iterator it = m_Tags.begin(), end = m_Tags.end(); it != end; ++it)
			tagArray.push_back(*it);

		return tagArray;
	}

	void Entity::_notifyPausedTag(const std::string &tag)
	{
		m_PausedTags.insert(tag);
	}

	void Entity::_notifyHiddenTag(const std::string &tag)
	{
		m_HiddenTags.insert(tag);
	}

	void Entity::SetTagFlags(unsigned int flags)
	{
		m_Flags = flags;
	}

	void Entity::AddTagFlag(unsigned int flag)
	{
		m_Flags |= flag;
	}

	void Entity::RemoveTagFlag(unsigned int flag)
	{
		m_Flags &= ~flag;
	}

	unsigned int Entity::GetTagFlags() const
	{
		return m_Flags;
	}

	void Entity::MarkToRemove()
	{
		m_MarkedToRemove = true;
	}

	bool Entity::IsMarkedToRemove() const
	{
		return m_MarkedToRemove;
	}

	std::string Entity::ToString() const
	{
		return GetType() + " - " + m_Name;
	}

}
