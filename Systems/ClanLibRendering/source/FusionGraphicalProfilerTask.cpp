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
#include "FusionCLRenderSystemMessageTypes.h"

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
		: SystemTaskBase(sysworld, "GraphicalProfilerTask"),
		m_RenderWorld(sysworld),
		m_PollingInterval(0.1f),
		m_LastPolledTime(0.0f),
		m_HistoryCapacity(300),
		m_NextColour(0.0f, 0.8f, 0.8f, 1.0f),
		m_ClearDisplayData(false),
		m_NewSelectedPath("/"),
		m_Tick(m_HistoryCapacity)
	{
		m_DebugFont = clan::Font(canvas, "Lucida Console", 14);
		m_DebugFont2 = clan::Font(canvas, "Lucida Console", 10);

		Console::getSingleton().BindCommand("prog_polling_interval", [this](const StringVector& args)->std::string
		{
			if (args.size() >= 2)
			{
				const auto value = boost::lexical_cast<float>(args[1]);
				m_PollingInterval = value;
			}
			return "";
		});

		Console::getSingleton().BindCommand("prog_history_size", [this](const StringVector& args)->std::string
		{
			if (args.size() >= 2)
			{
				const auto value = boost::lexical_cast<size_t>(args[1]);
				this->m_HistoryCapacity = value;
				this->m_ClearDisplayData = true;
			}
			return "";
		});
	}

	GraphicalProfilerTask::~GraphicalProfilerTask()
	{
		Console::getSingleton().UnbindCommand("prog_polling_interval");
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

	void GraphicalProfilerTask::Update()
	{
		m_LastPolledTime += DeltaTime::GetActualDeltaTime();
		if (m_LastPolledTime < m_PollingInterval)
		{
			// scrub
		}
		else
		{
			m_LastPolledTime = 0.0f;
			const clan::Sizef graphSize(400.f, 400.f);

			++m_Tick;

			// TODO: generic task Invoke, much like render actions but for intra-task execution (like clearing the display data)
			if (m_ClearDisplayData)
				m_DisplayData.clear();

			if (m_NewSelectedPath != m_SelectedPath)
			{
				m_SelectedPath = m_NewSelectedPath;

				m_SelectedNode = &m_NavigationRoot;

				auto path = fe_splitstring(m_SelectedPath, "/");
				if (!path.empty())
				{
					for (auto it = path.begin(); it != path.end() - 1; ++it)
					{
						auto newNode = &m_SelectedNode->children[*it];
						newNode->parent = m_SelectedNode;
						m_SelectedNode = newNode;
					}
				}
			}

			{
				auto newDisplayData = std::make_pair(std::string(), HistogramLine());
				newDisplayData.second.colour = m_NextColour;
				newDisplayData.second.percentages.set_capacity(m_HistoryCapacity);

				const auto times = Profiling::getSingleton().GetTimes();
				const auto actualDT = times.at("$ActualDT");
				for (const auto& entry : times)
				{
					const auto& key = entry.first;
					const auto& value = entry.second;

					if (!key.empty() && key[0] == '/')
					{
						const auto pathLength = key.rfind("/");
						if (pathLength != std::string::npos)
						{
							const auto path = key.substr(0, pathLength + 1);
							const auto statKey = key.substr(pathLength + 1);
							if (path == m_SelectedPath)
							{
								// Populate the navigation tree
								if (m_SelectedNode)
								{
									auto& node = m_SelectedNode->children[statKey];
									node.parent = m_SelectedNode;
									node.label = statKey;
								}

								const auto percentage = value / actualDT;

								newDisplayData.first = key;
								auto r = m_DisplayData.insert(newDisplayData);
								if (r.second)
								{
									r.first->second.colour = m_NextColour;
									m_NextColour.h += 41.8f;
									if (m_NextColour.h > 360.0f)
										m_NextColour.h -= 360.0f;
								}
								auto& displayData = r.first->second;
								displayData.percentages.push_back(std::make_pair(m_Tick, percentage));
								displayData.lastValue = value;
								displayData.label = statKey;
							}
						}
					}
				}
			}

			std::vector<std::vector<clan::Vec2f>> lines;
			std::vector<clan::Colorf> colours;
			std::vector<std::string> labels;

			clan::Rectf graphRect(clan::Pointf(), graphSize);

			if (m_SelectedNode)
			{
				const auto numLines = m_SelectedNode->children.size();
				lines.resize(numLines);
				colours.resize(numLines);
				labels.resize(numLines);

				const auto earliestTick = m_Tick - m_HistoryCapacity;

				size_t i = 0;
				for (const auto& node : m_SelectedNode->children)
				{
					const auto& data = m_DisplayData[m_SelectedPath + node.first];
					{
						//clan::Vec2f vert(graphRect.left, 0.0f);
						for (const auto& dataPoint : data.percentages)
						{
							if (dataPoint.first >= earliestTick)
							{
								const float x = float(dataPoint.first - earliestTick) / float(m_HistoryCapacity) * graphSize.width;
								const float y = float(dataPoint.second * graphSize.height);
								const clan::Vec2f vert(graphRect.left + x, graphRect.bottom - y);
								lines[i].push_back(vert);
							}
						}
						colours[i] = data.colour;
						labels[i] = data.label + " " + boost::lexical_cast<std::string>(data.lastValue);
					}
					++i;
				}
			}

			auto font = m_DebugFont;

			m_SystemWorld->PostSystemMessage("CLRenderSystem", char(CLRenderSystemMessageType::SetRenderAction), [graphRect, lines, colours, labels, font](clan::Canvas canvas, Vector2 cam_pos)
			{
				clan::Colorf colour = clan::Colorf::lightyellow;
				canvas.draw_box(graphRect, colour);

				for (size_t i = 0; i < lines.size(); ++i)
				{
					canvas.draw_line_strip(lines[i].data(), lines[i].size(), colours[i]);
				}

				auto fontCopy = font; // draw_text isn't const, ugg.

				const auto textLineHeight = fontCopy.get_text_size(canvas.get_gc(), "X").height;

				const float textX = graphRect.right;
				const float textTop = graphRect.bottom - textLineHeight * lines.size();

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
