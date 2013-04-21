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

#include "PrecompiledHeaders.h"

#include "FusionGraphicalProfilerTask.h"

#include "FusionCLRenderSystem.h"

#include "FusionDeltaTime.h"

#include "FusionProfiling.h"
#include "FusionNetworkManager.h"
#include "FusionRakNetwork.h"
#include <RakNet/RakNetStatistics.h>

#include <tbb/parallel_sort.h>
#include <tbb/parallel_for.h>

namespace FusionEngine
{

	GraphicalProfilerTask::GraphicalProfilerTask(CLRenderWorld* sysworld, clan::Canvas canvas)
		: ISystemTask(sysworld),
		m_RenderWorld(sysworld)
	{
		m_DebugFont = clan::Font(canvas, "Lucida Console", 14);
		m_DebugFont2 = clan::Font(canvas, "Lucida Console", 10);
	}

	GraphicalProfilerTask::~GraphicalProfilerTask()
	{
	}

	namespace {
		template <class Iter1, class Iter2>
		bool sequenceEqual(Iter1 begin1, Iter1 end1, Iter2 begin2, Iter2 end2)
		{
			for (; begin1 != end1 && begin2 != end2; ++begin1, ++begin2)
			{
				if (*begin1 != *begin2)
					return false;
			}
			return true;
		}
	}

	void GraphicalProfilerTask::Update(const float delta)
	{
		const std::vector<std::string> selectedPath;
		const int depth = selectedPath.size();

		m_HistoryTree.history.push_back(FrameStats());

		for (auto entry : Profiling::getSingleton().GetTimes())
		{
			const auto& key = entry.first;
			const auto& value = entry.second;

			const auto path = fe_splitstring(key, "/");
			if (path.size() == depth + 1 && path[depth][0] != '@' && path[depth][0] != '!' && path[depth][0] != '~')
			{
				const auto& statKey = path[depth];

				auto& node = m_HistoryTree;

				auto& stats = node.history.back();
				stats[statKey] = value;
			}
		}

		if (m_HistoryTree.history.size() > 399)
			m_HistoryTree.history.pop_front();
		auto selectedHistory = m_HistoryTree.history;

		const clan::Sizef graphSize(600.f, 600.f);

		std::map<std::string, std::vector<clan::Vec2f>> lines;
		float x = 0;
		for (auto entry : selectedHistory)
		{
			std::pair<std::string, double> maxStat("", -1.0);
			for (auto stat : entry)
			{
				if (stat.second > maxStat.second)
					maxStat = stat;
			}
			for (auto stat : entry)
			{
				float y = float(stat.second / maxStat.second * graphSize.height);
				lines[stat.first].push_back(clan::Vec2f(x, y));
			}

			x += graphSize.width / selectedHistory.size();
		}

		auto font = m_DebugFont;

		m_RenderWorld->EnqueueViewportRenderAction([graphSize, lines, font](clan::Canvas canvas, Vector2 cam_pos)
		{
			clan::Rectf box(graphSize);
			clan::Colorf colour = clan::Colorf::lightyellow;
			canvas.draw_box(box, colour);

			for (auto it = lines.begin(); it != lines.end(); ++it)
			{
				const auto fraction = std::distance(it, lines.end()) / (float)lines.size();
				clan::ColorHSVf lineColour(fraction * 360.f, 0.8f, 0.8f, 1.0f);

				const auto& line = *it;

				canvas.draw_line_strip(line.second.data(), line.second.size(), lineColour);
				lineColour.h += 0.01f;
			}

			auto fontCopy = font; // draw_text isn't const, ugg.

			const float textX = box.right;

			for (auto it = lines.begin(); it != lines.end(); ++it)
			{
				const auto fraction = std::distance(it, lines.end()) / (float)lines.size();
				clan::ColorHSVf lineColour(fraction * 360.f, 0.8f, 0.8f, 1.0f);

				const auto& line = *it;

				fontCopy.draw_text(canvas, clan::Pointf(textX, line.second.back().y), line.first, lineColour);
			}
		});
	}

}
