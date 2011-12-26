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

#ifndef H_FusionSharedPropertyDispatcher
#define H_FusionSharedPropertyDispatcher

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <functional>
#include <memory>

#include <tbb/concurrent_queue.h>

namespace FusionEngine
{

	class ICallQueue
	{
	public:
		virtual void Call() = 0;
	};

	template <class C>
	class CallQueue : public ICallQueue
	{
	public:
		CallQueue(std::weak_ptr<C> object)
			: m_Object(object)
		{
		}

		void Enqueue(std::function<void (std::shared_ptr<C>)> method)
		{
			m_Calls.push(method);
		}

private:
		void Call()
		{
			auto strongobj = m_Object.lock();
			if (strongobj)
			{
				std::function<void (void)> method;
				while (m_Calls.try_pop(method))
				{
					method(strongobj);
				}
			} 
		}

		std::weak_ptr<C> m_Object;
		tbb::concurrent_queue<std::function<void (std::shared_ptr<C>)>> m_Calls;
	};

}

#endif
