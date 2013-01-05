/*
*  Copyright (c) 2013 Fusion Project Team
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

#ifndef H_FusionTEvent
#define H_FusionTEvent

#include "FusionPrerequisites.h"

#include <cstdint>
#include <condition_variable>
#include <mutex>

namespace FusionEngine { namespace Threading
{

	//! Use this to wait for something to happen
	class Event
	{
	public:
		Event(bool auto_reset = true);

		int Wait(std::int64_t timeout);

		void Notify(int id);

	private:
		int notified;
		bool autoReset;
		std::condition_variable conditionVariable;
		std::mutex mutex;
	};

	Event::Event(bool auto_reset)
		: autoReset(auto_reset)
	{
	}

	int Event::Wait(std::int64_t timeout)
	{
		std::unique_lock<std::mutex> lock(mutex);
		conditionVariable.wait_for(lock, std::chrono::seconds(timeout), [this]() { return notified >= 0; });

		auto whatWasNotified = notified;
		if (autoReset)
			notified = -1;
		return whatWasNotified;
	}

	void Event::Notify(int id)
	{
		notified = id;
		conditionVariable.notify_all();
	}

} }

#endif