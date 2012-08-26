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
#include "FusionStreamingManager.h"

#include "FusionTimer.h"

#include <tbb/tick_count.h>

namespace FusionEngine
{

	class RegionCellArchivist;

	class TaskScheduler
	{
	public:
		TaskScheduler(TaskManager* task_manager, EntityManager* entity_manager, RegionCellArchivist* archivist);

		~TaskScheduler();

		void SetUniverse(const std::vector<std::shared_ptr<ISystemWorld>>& universe);

		void SetMaxFrameskip(unsigned int frameskip) { m_MaxFrameskip = frameskip; }

		void SetDT(float dt);
		float GetDT() const;

		void SetFramerateLimiter(bool enabled) { m_FramerateLimiterEnabled = enabled; }
		bool GetFramerateLimiter() const { return m_FramerateLimiterEnabled; }

		void SetUnlimited(bool unlimited) { m_Unlimited = unlimited; }

		uint8_t Execute(uint8_t what = 0xFF);

	private:
		tbb::tick_count m_LastTick;
		unsigned int m_LastTime;
		unsigned int m_Accumulator;
		unsigned int m_FramesSkipped;

		unsigned int m_MaxFrameskip;

		float m_DeltaTime;
		unsigned int m_DeltaTimeMS;

		bool m_FramerateLimiterEnabled;
		bool m_Unlimited; // Update all tasks at an unlimited rate

		bool m_ThreadingEnabled;

		bool m_ResortTasks;

		Timer m_Timer;
		
		std::vector<std::shared_ptr<ISystemWorld>> m_ComponentWorlds;
		std::vector<ISystemTask*> m_SortedTasks; // All tasks (simulation and render tasks)
		std::vector<ISystemTask*> m_SortedSimulationTasks;
		std::vector<ISystemTask*> m_SortedRenderTasks;

		std::vector<std::unique_ptr<ISystemTask>> m_ProxyTasks; // This is to make sure they are deleted
		std::unique_ptr<ISystemTask> m_StreamingTask;

		TaskManager* m_TaskManager;
		EntityManager* m_EntityManager;
		RegionCellArchivist* m_Archivist;

		void SortTasks();

	};

}

#endif
