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

#ifndef H_FusionGraphicalProfilerTask
#define H_FusionGraphicalProfilerTask

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentSystem.h"
#include "FusionRenderAction.h"
#include "FusionViewport.h"

#include "FusionProfiling.h"

#include <map>
#include <vector>

#include <boost/circular_buffer.hpp>

namespace FusionEngine
{
	
	class CLRenderWorld;

	//! ClanLib render task
	class GraphicalProfilerTask : public ISystemTask
	{
	public:
		GraphicalProfilerTask(CLRenderWorld* sysworld, clan::Canvas canvas);
		~GraphicalProfilerTask();

		SystemType GetTaskType() const { return SystemType::Rendering; }

		PerformanceHint GetPerformanceHint() const { return Short; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}
		
		void Update(const float delta);

	private:
		CLRenderWorld* m_RenderWorld;

		clan::Font m_DebugFont;
		clan::Font m_DebugFont2;
		
		float m_PollingInterval;
		float m_LastPolledTime;

		size_t m_HistoryCapacity;

		struct ProfilingNode
		{
			std::string label;
			std::map<std::string, ProfilingNode> children;
			ProfilingNode* parent;
		};

		ProfilingNode m_NavigationRoot;

		struct HistogramLine
		{
			boost::circular_buffer<clan::Vec2f> verts;
			clan::Colorf colour;
			Profiling::TimeValue_t lastValue;
			std::string label;
		};

		clan::ColorHSVf m_NextColour;

		std::map<std::string, HistogramLine> m_DisplayData;

		boost::circular_buffer<Profiling::Times_t> m_History;

		std::string m_NewSelectedPath;
		std::string m_SelectedPath;
		ProfilingNode* m_SelectedNode;
	};

}

#endif
