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

#include "FusionProfiling.h"

#include <tbb/tick_count.h>

namespace FusionEngine
{

	Profiling::Profiling()
	{
	}

	Profiling::~Profiling()
	{
	}

	void Profiling::AddTime(const std::string& label, double seconds)
	{
		AddTime(label, unsigned long(seconds * 1000));
	}

	void Profiling::AddTime(const std::string& label, unsigned long miliseconds)
	{
		m_IncomingTimes.push(std::make_pair(label, miliseconds));
	}

	unsigned long Profiling::GetTime(const std::string& label) const
	{
		return m_TimesLastTick.at(label);
	}

	std::map<std::string, unsigned long> Profiling::GetTimes() const
	{
		return m_TimesLastTick;
	}

	void Profiling::StoreTick()
	{
		m_TimesLastTick.clear();

		std::pair<std::string, unsigned long> entry;
		while (m_IncomingTimes.try_pop(entry))
		{
			auto result = m_TimesLastTick.insert(entry);
			if (!result.second) // Add time to existing labels:
				result.first->second += entry.second;
		}
	}

	Profiler::Profiler(const std::string& label)
		: m_Label(label),
		m_Start(tbb::tick_count::now())
	{
	}

	Profiler::~Profiler()
	{
		auto doneTime = tbb::tick_count::now() - m_Start;
		Profiling::getSingleton().AddTime(m_Label, doneTime.seconds());
	}

}
