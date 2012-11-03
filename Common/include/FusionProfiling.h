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
*  File Author:
*    Elliot Hayward
*/

#ifndef H_FusionProfiling
#define H_FusionProfiling

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSingleton.h"

#include "FusionAssert.h"

#include <cstdint>
#include <map>

#ifdef FSN_TBB_AVAILABLE
// Comment this out to use boost cpu_timer rather than tbb::tick_count (they both use perf. timer in windows)
#define FSN_PROFILER_USE_TBB_TIMER

#ifdef FSN_PROFILER_USE_TBB_TIMER
#include <tbb/tick_count.h>
#endif
#include <tbb/concurrent_queue.h>
#include <tbb/enumerable_thread_specific.h>

#else // TBB not available
#include <boost/thread/mutex.hpp>
#endif
#ifndef FSN_PROFILER_USE_TBB_TIMER
#include <boost/timer/timer.hpp>
#endif

namespace FusionEngine
{

	/*!
	 * \brief
	 * Stores profiling data
	 */
	class Profiling : public Singleton<Profiling>
	{
	public:
		//! Constructor
		Profiling();
		//! Destructor
		~Profiling();

		//! A time or other value. Entries with labels will be added together at the end of the frame.
		void AddTime(const std::string& label, const double seconds);
		//! Convenience: converts miliseconds to seconds and calls the other overload
		void AddTime(const std::string& label, const std::uint32_t miliseconds);

		//! Averaged over the frame
		void AddStat(const std::string& label, const double seconds);

		//! Returns the accumulated time recorded under the given label during the last tick
		double GetTime(const std::string& label) const;
		//! Returns the accumulated times recorded during the last tick
		std::map<std::string, double> GetTimes() const;

		//! Stores the times / stats recorded since the last time this method was called and clears them
		void StoreTick();

		std::string PushThreadScopeLabel(const std::string& suffix)
		{
#ifdef FSN_TBB_AVAILABLE
			bool exists;
			auto& current = m_ScopeLabel.local(exists);
			if (exists && !current.empty())
				return current += (":" + suffix);
			else
				return current += suffix;
#else
			return suffix;
#endif
		};

		void PopThreadScopeLabel()
		{
			auto pos = m_ScopeLabel.local().rfind(':');
			if (pos != std::string::npos)
				m_ScopeLabel.local().erase(pos);
			else
				m_ScopeLabel.local().clear();
		}

	private:
		std::map<std::string, double> m_TimesLastTick;
#ifdef FSN_TBB_AVAILABLE
		tbb::concurrent_queue<std::pair<std::string, double>> m_IncomingTimes;

		tbb::enumerable_thread_specific<std::string> m_ScopeLabel;
#else
		std::deque<std::pair<std::string, double>> m_IncomingTimes;
		boost::mutex m_Mutex;
#endif
	};

	//! Scoped timer
	class Profiler
	{
	public:
		Profiler(const std::string& label);
		~Profiler();

	private:
//#ifdef FSN_TBB_AVAILABLE
//		static tbb::enumerable_thread_specific<std::string> m_ScopeLabel;
//#endif

		std::string m_Label;

#ifdef FSN_PROFILER_USE_TBB_TIMER
		tbb::tick_count m_Start;
#else
		boost::timer::cpu_timer m_Timer;
#endif
	};

#ifdef FSN_PROFILING_ENABLED
#define FSN_PROFILE(name) Profiler scoped_profiler_(name);
#else
	#define FSN_PROFILE(name) FSN_UNUSED(name);
#endif

}

#endif
