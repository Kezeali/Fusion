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

namespace FusionEngine
{

	class TaskScheduler
	{
	public:
		TaskScheduler(TaskManager* task_manager);

		~TaskScheduler();

		void SetOntology(const std::vector<ISystemWorld*>& ontology);

		void Execute();

	private:
		unsigned int m_LastTime;

		bool m_ThreadingEnabled;

		bool m_ResortTasks;
		
		std::vector<ISystemWorld*> m_ComponentWorlds;
		std::vector<ISystemTask*> m_SortedTasks;

		TaskManager* m_TaskManager;

	};

}

#endif
