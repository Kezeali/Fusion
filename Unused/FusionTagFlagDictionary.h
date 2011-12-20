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

#ifndef H_FusionTagFlagDictionary
#define H_FusionTagFlagDictionary

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionTypes.h"

namespace FusionEngine
{

	class TagFlagDictionary
	{
	public:
		//! Adds the given tag to the given entity
		/*!
		* \returns True if a flag was given to the tag
		*/
		bool AddTag(const std::string &tag, EntityPtr to_entity, bool add_flag);
		//! Removes the given tag from the given entity
		void RemoveTag(const std::string &tag, EntityPtr from_entity);

		//! Defines a flag for the given tag if one is available
		bool RequestFlagFor(const std::string &tag);
		//! Defines a flag for the given tag, taking one from another tag if necessary
		void ForceFlagFor(const std::string &tag);

		//! Returns the flag currently defined for the given tag
		unsigned int GetFlagFor(const std::string &tag);

	private:
		//! Updates the free flags state
		/*!
		* Removes the current m_MinFreeFlag from the m_FreeFlags mask and
		* sets m_MinFreeFlag to the next free flag.
		*/
		void takeMinFreeFlag();

		//! Called when a flag becomes free
		void flagFreed(unsigned int flag);

		struct Entry
		{
			std::string Tag;
			unsigned int Flag;
			std::tr1::unordered_set<std::string> References;
		};
		// Could use a boost::multi_index_container here - i.e. index by
		//  unique Tag AND Flag - but that's too much of a hassle and I
		//  doubt it'd be worth it.
		typedef std::tr1::unordered_map<std::string, Entry> TagFlagMap;
		TagFlagMap m_Entries;

		unsigned int m_MinFreeFlag;
		unsigned int m_FreeFlags;
	};

	typedef std::tr1::shared_ptr<TagFlagDictionary> TagFlagDictionaryPtr;

}

#endif