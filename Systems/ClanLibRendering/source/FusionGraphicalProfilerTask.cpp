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

#include <boost/lexical_cast.hpp>

namespace FusionEngine
{

	GraphicalProfilerTask::GraphicalProfilerTask(CLRenderWorld* sysworld, clan::Canvas canvas)
		: ISystemTask(sysworld),
		m_RenderWorld(sysworld),
		m_PollingInterval(0.05f),
		m_LastPolledTime(0.0f),
		m_HistoryCapacity(300),
		m_NextColour(0.0f, 0.8f, 0.8f, 1.0f),
		m_SelectedPath("/")
	{
		m_DebugFont = clan::Font(canvas, "Lucida Console", 14);
		m_DebugFont2 = clan::Font(canvas, "Lucida Console", 10);

		Console::getSingleton().BindCommand("prog_history_size", [this](const StringVector& args)->std::string
		{
			if (args.size() >= 2)
			{
				const auto value = boost::lexical_cast<size_t>(args[1]);
				m_HistoryCapacity = value;
				this->m_History.set_capacity(value);
			}
			return "";
		});

		m_History.set_capacity(m_HistoryCapacity);
	}

	GraphicalProfilerTask::~GraphicalProfilerTask()
	{
		Console::getSingleton().UnbindCommand("prog_history_size");
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
		m_LastPolledTime += delta;
		if (m_LastPolledTime < m_PollingInterval)
		{
			// scrub
		}
		else
		{
			const clan::Sizef graphSize(400.f, 400.f);

			auto newDisplayData = std::make_pair(std::string(), HistogramLine());
			newDisplayData.second.colour = m_NextColour;

			if (m_NewSelectedPath != m_SelectedPath)
			{
				m_SelectedPath = m_NewSelectedPath;

				m_SelectedNode = &m_NavigationRoot;

				auto path = fe_splitstring(m_SelectedPath, "/");
				if (!path.empty())
				{
					for (auto it = path.begin(); it != path.end() - 1; ++it)
					{
						m_SelectedNode = &m_SelectedNode->children[*it];
					}
				}
			}

			{
				clan::Vec2f vert;
				const float xInterval = graphSize.width / m_History.size();

				const auto times = Profiling::getSingleton().GetTimes();
				for (const auto& entry : times)
				{
					const auto& key = entry.first;
					const auto& value = entry.second;

					const auto pathLength = key.rfind("/");
					if (pathLength != std::string::npos)
					{
						const auto path = key.substr(0, pathLength);
						const auto statKey = key.substr(pathLength);
						//if (path[depth][0] != '@' && path[depth][0] != '!' && path[depth][0] != '~' && path[depth][0] != '#')
						if (statKey[0] != '@' && statKey[0] != '!' && statKey[0] != '~' && statKey[0] != '#')
						{
							vert.y = float(value / DeltaTime::GetActualDeltaTime() * graphSize.height);

							auto r = m_DisplayData.insert(newDisplayData);
							if (r.second)
							{
								newDisplayData.second.colour = m_NextColour;
								m_NextColour.h += 16.6f;
								if (m_NextColour.h > 360.0f)
									m_NextColour.h -= 360.0f;
							}
							auto& displayData = r.first->second;
							displayData.verts.push_back(vert);
							displayData.lastValue = value;
							displayData.label = statKey;
						}
					}
				}
			}

			std::vector<std::vector<clan::Vec2f>> lines;
			std::vector<clan::Colorf> colours;
			std::vector<std::string> labels;

			if (m_SelectedNode)
			{
				size_t i = 0;
				for (const auto& node : m_SelectedNode->children)
				{
					const auto& stat = m_DisplayData[node.first];
					{
						lines[i].assign(stat.verts.begin(), stat.verts.end());
						colours[i] = stat.colour;
						labels[i] = stat.label + " " + boost::lexical_cast<std::string>(stat.lastValue);
					}
					++i;
				}
			}

			auto font = m_DebugFont;

			m_RenderWorld->EnqueueViewportRenderAction([graphSize, lines, colours, labels, font](clan::Canvas canvas, Vector2 cam_pos)
			{
				clan::Rectf box(graphSize);
				clan::Colorf colour = clan::Colorf::lightyellow;
				canvas.draw_box(box, colour);

				for (size_t i = 0; i < lines.size(); ++i)
				{
					canvas.draw_line_strip(lines[i].data(), lines[i].size(), colours[i]);
				}

				auto fontCopy = font; // draw_text isn't const, ugg.

				const auto textLineHeight = fontCopy.get_text_size(canvas.get_gc(), "X").height;

				const float textX = box.right;
				const float textTop = box.bottom - textLineHeight * lines.size();

				for (size_t i = 0; i < lines.size(); ++i)
				{
					const auto& text = labels[i];
					const auto& lineColour = colours[i];

					fontCopy.draw_text(canvas, clan::Pointf(textX, textTop + textLineHeight * i), text, lineColour);
				}
			});
		}
	}

}
