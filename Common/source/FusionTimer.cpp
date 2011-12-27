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

#include "FusionTimer.h"

#include "FusionAssert.h"

#include <boost/asio/deadline_timer.hpp>

namespace FusionEngine
{

	class TimerImpl
	{
	public:
		TimerImpl(float interval)
			: m_IntervalSeconds(interval),
#ifndef FSN_TIMER_USE_MICROSECOND
			m_IntervalInteger((int64_t)(interval * 1000)),
#else
			m_IntervalInteger((int64_t)(interval * 1000000)),
#endif
			m_Timer(io_service, m_IntervalInteger)
		{
		}

		TimerImpl(const TimerImpl& copy)
			: m_IntervalSeconds(copy.m_IntervalSeconds),
			m_IntervalInteger(copy.m_IntervalInteger),
			m_Timer(io_service, m_IntervalInteger)
		{}

		void Wait()
		{
			m_Timer.wait();
			m_Timer.expires_from_now(m_IntervalInteger);
		}

	private:
		boost::asio::io_service io_service;
		boost::asio::deadline_timer m_Timer;
		float m_IntervalSeconds;
#ifndef FSN_TIMER_USE_MICROSECOND
		boost::posix_time::milliseconds m_IntervalInteger;
#else
		boost::posix_time::microseconds m_IntervalInteger;
#endif
	};

	Timer::Timer()
	{
	}

	Timer::Timer(float interval)
	{
		m_Impl.reset(new TimerImpl(interval));
	}

	Timer::Timer(const Timer& copy)
	{
		m_Impl.reset(new TimerImpl(*copy.m_Impl));
	}

	Timer& Timer::operator= (const Timer& copy)
	{
		m_Impl.reset(new TimerImpl(*copy.m_Impl));
		return *this;
	}

	Timer::~Timer()
	{
	}

	void Timer::SetInterval(float seconds)
	{
		m_Impl.reset(new TimerImpl(seconds));
	}

	void Timer::Wait()
	{
		FSN_ASSERT(m_Impl);
		m_Impl->Wait();
	}

}
