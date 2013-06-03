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

#ifndef H_FusionStreamingSystem
#define H_FusionStreamingSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSystemTask.h"

#include "FusionEntityManager.h"
#include "FusionRegionMapLoader.h"

namespace FusionEngine
{

	class StreamingTask : public System::TaskBase
	{
	public:
		StreamingTask(EntityManager* streaming_manager, RegionCellArchivist* archivist)
			: TaskBase(nullptr, "Streaming"),
			m_StreamingManager(streaming_manager),
			m_Archivist(archivist)
		{}
		~StreamingTask() {}

		void Update() override;

		System::SystemType GetTaskType() const { return System::Simulation; }

		PerformanceHint GetPerformanceHint() const { return TaskBase::LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		EntityManager* m_StreamingManager;
		RegionCellArchivist* m_Archivist;
	};

	class StreamingTaskB : public System::TaskBase
	{
	public:
		StreamingTaskB(EntityManager* streaming_manager)
			: TaskBase(nullptr, "Streaming-CamerasOnly"),
			m_StreamingManager(streaming_manager)
		{}
		~StreamingTaskB() {}

		void Update() override;

		System::SystemType GetTaskType() const { return System::Rendering; }

		PerformanceHint GetPerformanceHint() const { return TaskBase::LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		EntityManager* m_StreamingManager;
	};
	
	inline void StreamingTask::Update()
	{
		m_Archivist->BeginTransaction();
		try
		{
			m_StreamingManager->UpdateActiveRegions();
			m_StreamingManager->ProcessActiveEntities(DeltaTime::GetDeltaTime());
		}
		catch (...)
		{
			m_Archivist->EndTransaction();
			throw;
		}
		m_Archivist->EndTransaction();
	}
	
	inline void StreamingTaskB::Update()
	{
		m_StreamingManager->UpdateActiveRegions();
	}

}

#endif
