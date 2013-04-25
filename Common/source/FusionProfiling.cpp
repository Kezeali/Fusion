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

#include "PrecompiledHeaders.h"

#include "FusionProfiling.h"

namespace FusionEngine
{

	Profiling::Profiling()
	{
	}

	Profiling::~Profiling()
	{
	}

	void Profiling::AddTime(const std::string& label, const TimeValue_t seconds)
	{
#ifdef FSN_TBB_AVAILABLE
		m_IncomingTimes.push(std::make_pair(label, seconds));
#else
		boost::mutex::scoped_lock lock(m_Mutex);
		m_IncomingTimes.push_back(std::make_pair(label, seconds));
#endif
	}

	void Profiling::AddTime(const std::string& label, const uint32_t miliseconds)
	{
		AddTime(label, TimeValue_t(miliseconds) * 0.001);
	}

	void Profiling::AddStat(const std::string& label, const TimeValue_t seconds)
	{
		AddTime("@" + label, seconds);
	}

	Profiling::TimeValue_t Profiling::GetTime(const std::string& label) const
	{
		return m_TimesLastTick->at(label);
	}

	Profiling::Times_t Profiling::GetTimes() const
	{
		return *m_TimesLastTick;
	}

	void Profiling::StoreTick()
	{
		m_ScopeLabel.clear();

		std::unique_ptr<Times_t> newTimes(new Times_t); // get with them, man.
		std::map<std::string, size_t> numValues;

		Entry_t entry;
#ifdef FSN_TBB_AVAILABLE
		while (m_IncomingTimes.try_pop(entry))
		{
#else
		boost::mutex::scoped_lock lock(m_Mutex);
		while (!m_IncomingTimes.empty())
		{
			entry = m_IncomingTimes.front();
			m_IncomingTimes.pop_front();
#endif
			auto result = newTimes->insert(std::move(entry));
			FSN_ASSERT(!result.first->first.empty());
			if (result.first->first[0] == '@')
				++numValues[result.first->first];
			if (!result.second) // Add time to existing labels:
				result.first->second += entry.second;
		}
		for (auto it = numValues.begin(); it != numValues.end(); ++it)
			(*newTimes)[it->first] /= it->second;

		m_TimesLastTick.swap(newTimes); // drop a bomb on it
	}

	std::string Profiling::PushThreadScopeLabel(const std::string& suffix)
	{
#ifdef FSN_TBB_AVAILABLE
		auto& current = m_ScopeLabel.local();
		return current += ("/" + suffix);
#else
		return suffix;
#endif
	}

	void Profiling::PopThreadScopeLabel()
	{
#ifdef FSN_TBB_AVAILABLE
		const auto pos = m_ScopeLabel.local().rfind('/');
		if (pos != std::string::npos && pos != 0)
			m_ScopeLabel.local().erase(pos);
		else
			m_ScopeLabel.local().clear();
#endif
	}

	Profiler::Profiler(const std::string& label)
		: m_Label(Profiling::getSingleton().PushThreadScopeLabel(label))
#ifdef FSN_PROFILER_USE_TBB_TIMER
		, m_Start(tbb::tick_count::now())
#endif
	{
	}

	Profiler::~Profiler()
	{
		Profiling::getSingleton().PopThreadScopeLabel();
#ifdef FSN_PROFILER_USE_TBB_TIMER
		auto doneTime = tbb::tick_count::now() - m_Start;
		Profiling::getSingleton().AddTime(m_Label, doneTime.seconds());
#else
		boost::timer::nanosecond_type nanoseconds = m_Timer.elapsed().wall;
		Profiling::getSingleton().AddTime(m_Label, Profiling::TimeValue_t(nanoseconds * 0.000000001));
#endif
	}

}
