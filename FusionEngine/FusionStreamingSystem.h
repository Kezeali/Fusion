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

#include "FusionComponentSystem.h"

#include "FusionEntityManager.h"
#include "FusionRegionMapLoader.h"

namespace FusionEngine
{

	class StreamingTask : public ISystemTask
	{
	public:
		StreamingTask(EntityManager* streaming_manager, RegionMapLoader* archivist)
			: ISystemTask(nullptr), m_StreamingManager(streaming_manager), m_Archivist(archivist),
			newSlowness(0u)
		{}
		~StreamingTask() {}

		void Update(const float delta);

		std::string GetName() const { return "Streaming"; }

		SystemType GetTaskType() const { return SystemType::Simulation; }

		PerformanceHint GetPerformanceHint() const { return ISystemTask::LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		EntityManager* m_StreamingManager;
		RegionMapLoader* m_Archivist;

		uint32_t newSlowness;
	};

	class StreamingTaskB : public ISystemTask
	{
	public:
		StreamingTaskB(EntityManager* streaming_manager)
			: ISystemTask(nullptr), m_StreamingManager(streaming_manager)
		{}
		~StreamingTaskB() {}

		void Update(const float delta);

		SystemType GetTaskType() const { return SystemType::Rendering; }

		PerformanceHint GetPerformanceHint() const { return ISystemTask::LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		EntityManager* m_StreamingManager;
	};
	
	inline void StreamingTask::Update(const float delta)
	{
		m_Archivist->BeginTransaction();
		try
		{
			m_StreamingManager->UpdateActiveRegions();
			m_StreamingManager->ProcessActiveEntities(delta);
		}
		catch (...)
		{
			m_Archivist->EndTransaction();
			throw;
		}
		m_Archivist->EndTransaction();
	}
	
	inline void StreamingTaskB::Update(const float delta)
	{
		m_StreamingManager->UpdateActiveRegions();
	}

}

#endif
