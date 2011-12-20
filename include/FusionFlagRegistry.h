/*
*  Copyright (c) 2009-2010 Fusion Project Team
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

#ifndef H_FusionFlagRegistry
#define H_FusionFlagRegistry

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSingleton.h"
#include "FusionIDStack.h"

namespace FusionEngine
{

	//! Allows flags to be referenced by strings
	/*
	* \remarks
	* 0x00000001 is reserved for special cases, so only 31 tags can be registered at a time.
	*/
	class FlagRegistry
	{
	public:
		//! Ctor
		FlagRegistry();

		//! Associates a flag with the given tag
		/*!
		* \return
		* The flag that is now associated with the tag
		*/
		uint32_t RegisterTag(const std::string& tag);
		//! Frees up the flag associated with the given tag
		void UnregisterTag(const std::string& tag);
		//! Unregisters all associations
		void Clear();

		//! Returns the flag associated with the given tag
		uint32_t ToFlag(const std::string& tag) const;
		//! Returns and integer with each flag set that corresponds with the given tags
		uint32_t ToFlags(const std::vector<std::string>& tags) const;

	protected:
		std::unordered_map<std::string, uint32_t> m_TaggedFlags;
		uint32_t m_UnusedFlags;
	};

	//! Singleton access to a FlagRegistry object
	class StaticFlagRegistry : public FlagRegistry, protected Singleton<StaticFlagRegistry>
	{
	public:
		StaticFlagRegistry() {}

		static uint32_t RegisterTag(const std::string& tag)
		{
			FSN_ASSERT(getSingletonPtr() != nullptr);
			return getSingleton().FlagRegistry::RegisterTag(tag);
		}
		static void UnregisterTag(const std::string& tag)
		{
			FSN_ASSERT(getSingletonPtr() != nullptr);
			return getSingleton().FlagRegistry::UnregisterTag(tag);
		}
		static void Clear()
		{
			FSN_ASSERT(getSingletonPtr() != nullptr);
			return getSingleton().FlagRegistry::Clear();
		}

		static uint32_t ToFlag(const std::string& tag)
		{
			FSN_ASSERT(getSingletonPtr() != nullptr);
			return getSingleton().FlagRegistry::ToFlag(tag);
		}
		static uint32_t ToFlags(const std::vector<std::string>& tags)
		{
			FSN_ASSERT(getSingletonPtr() != nullptr);
			return getSingleton().FlagRegistry::ToFlags(tags);
		}
	};

}

#endif
