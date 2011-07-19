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

#ifndef H_FusionTaskScheduler
#define H_FusionTaskScheduler

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentSystem.h"
#include "FusionTaskManager.h"

#include "FusionTimer.h"

#include <tbb/tick_count.h>

namespace FusionEngine
{

	class TaskScheduler
	{
	public:
		TaskScheduler(TaskManager* task_manager);

		~TaskScheduler();

		void SetOntology(const std::vector<ISystemWorld*>& ontology);

		void Execute();

		void SetFramerateLimiter(bool enabled) { m_FramerateLimiterEnabled = enabled; }
		bool GetFramerateLimiter() const { return m_FramerateLimiterEnabled; }

		void SetUnlimited(bool unlimited) { m_Unlimited = unlimited; }

	private:
		tbb::tick_count m_LastTick;
		unsigned int m_LastTime;
		unsigned int m_Accumulator;

		float m_DeltaTime;
		unsigned int m_DeltaTimeMS;

		bool m_FramerateLimiterEnabled;
		bool m_Unlimited; // Update all tasks at an unlimited rate

		bool m_ThreadingEnabled;

		bool m_ResortTasks;

		Timer m_Timer;
		
		std::vector<ISystemWorld*> m_ComponentWorlds;
		std::vector<ISystemTask*> m_SortedTasks; // All tasks, including render tasks
		std::vector<ISystemTask*> m_SortedRenderTasks;

		TaskManager* m_TaskManager;

	};

}

#endif
