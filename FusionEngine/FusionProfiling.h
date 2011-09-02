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

// TODO: For profiler: (could be in another header)
#include <tbb/tick_count.h>
#include <tbb/concurrent_queue.h>

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

		void AddTime(const std::string& label, double seconds);
		void AddTime(const std::string& label, unsigned long miliseconds);

		//! Returns the accumulated time recoreded under the given label during the last tick
		unsigned long GetTime(const std::string& label) const;
		//! Returns the accumulated times recorded during the last tick
		std::map<std::string, unsigned long> GetTimes() const;

		void StoreTick();

	private:
		std::map<std::string, unsigned long> m_TimesLastTick;
		tbb::concurrent_queue<std::pair<std::string, unsigned long>> m_IncomingTimes;
	};

	//! Scoped timer
	class Profiler
	{
	public:
		Profiler(const std::string& label);
		~Profiler();

	private:
		tbb::tick_count m_Start;
		std::string m_Label;
	};

}

#endif
