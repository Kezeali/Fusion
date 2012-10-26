/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionCellArchivistSystem
#define H_FusionCellArchivistSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentSystem.h"

#include "FusionRegionMapLoader.h"

namespace FusionEngine
{

	class CellArchivistSystem : public ISystemTask
	{
	public:
		CellArchivistSystem(RegionCellArchivist* archivist)
			: ISystemTask(nullptr),
			m_Archivist(archivist)
		{}
		~CellArchivistSystem() {}

		void Update(const float delta);

		std::string GetName() const { return "CellArchivist"; }

		SystemType GetTaskType() const { return SystemType::Simulation; }

		PerformanceHint GetPerformanceHint() const { return ISystemTask::LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		RegionCellArchivist* m_Archivist;
	};

}

#endif
