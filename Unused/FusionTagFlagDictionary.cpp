/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionEntity.h"

#include "FusionExceptionFactory.h"
#include "FusionResourceManager.h"

#include <boost/lexical_cast.hpp>

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
		FSN_EXCEPT(ExCode::NotImplemented, "ForceFlagFor() is not implemented.");
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

}