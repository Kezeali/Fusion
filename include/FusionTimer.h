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

#ifndef H_FusionTimer
#define H_FusionTimer

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

// If this is defined the Timer interval will be represented in
//  microseconds, rather than milliseconds, internally
//#define FSN_TIMER_USE_MICROSECOND

namespace FusionEngine
{

	class TimerImpl;

	//! Provides a mechanism to block if a certain time has not been reached
	class Timer
	{
	public:
		Timer();
		Timer(float interval);
		Timer(const Timer& copy);

		Timer& operator= (const Timer& copy);

		// This must be explicitly defined /after/ TimerImpl (i.e. in FusionTimer.cpp); if an
		//  inline / compiler-generated destructor is used unique_ptr's default deleter will
		//  be compliled against the incomplete TimerImpl (resulting in a compile error)
		~Timer();

		void SetInterval(float seconds);

		void Wait();

	private:
		std::unique_ptr<TimerImpl> m_Impl;
	};

}

#endif
