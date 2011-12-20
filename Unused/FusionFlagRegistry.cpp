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

#include "FusionStableHeaders.h"

#include "FusionFlagRegistry.h"

#ifdef _WIN32
#include "intrin.h"
#pragma intrinsic(_BitScanForward)
#endif

namespace FusionEngine
{

	FlagRegistry::FlagRegistry()
		: m_UnusedFlags(0xFFFFFFFE)
	{
	}

	uint32_t FlagRegistry::RegisterTag(const std::string& tag)
	{
		unsigned long flagOffset;
#ifdef _WIN32
		bool flagsLeft = _BitScanForward(&flagOffset, m_UnusedFlags) != 0;
#else
		bool flagsLeft = m_UnusedFlags != 0;
		if (flagsLeft) flagOffset = __builtin_ctz(m_UnusedFlags);
#endif
		if (flagsLeft)
		{
			uint32_t flag = uint32_t(1) << flagOffset;
			m_UnusedFlags ^= flag;
			m_TaggedFlags[tag] = flag;
			return flag;
		}
		else
			return 0;
	}

	void FlagRegistry::UnregisterTag(const std::string& tag)
	{
		auto _where = m_TaggedFlags.find(tag);
		m_UnusedFlags |= _where->second;
		m_TaggedFlags.erase(_where);
	}

	void FlagRegistry::Clear()
	{
		m_UnusedFlags = 0xFFFFFFFE;
	}

	uint32_t FlagRegistry::ToFlag(const std::string& tag) const
	{
		auto _where = m_TaggedFlags.find(tag);
		if (_where != m_TaggedFlags.end())
			return _where->second;
		else
			return 0;
	}

	uint32_t FlagRegistry::ToFlags(const std::vector<std::string>& tags) const
	{
		uint32_t flags = 0;
		for (auto it = tags.begin(), end = tags.end(); it != end; ++it)
		{
			auto _where = m_TaggedFlags.find(*it);
			if (_where != m_TaggedFlags.end())
				flags |= _where->second;
		}
		return flags;
	}

}
