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

#ifndef H_FusionCLRenderTask
#define H_FusionCLRenderTask

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSystemTask.h"
#include "FusionRenderAction.h"
#include "FusionViewport.h"

#include <ClanLib/display.h>
#include <tbb/concurrent_queue.h>
#include <map>
#include <vector>

namespace FusionEngine
{
	
	class CLRenderWorld;
	class B2DebugDraw;

	//! ClanLib render task
	class CLRenderTask : public SystemTaskBase
	{
	public:
		CLRenderTask(CLRenderWorld* sysworld, Renderer* const renderer);
		~CLRenderTask();

		SystemType GetTaskType() const { return SystemType::Rendering; }

		PerformanceHint GetPerformanceHint() const { return LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return true;
		}

		void SetRenderAction(const RenderAction& action);
		void RemoveRenderAction(const RenderAction& action);
		
		void Update() override;

		void Draw();

	private:
		CLRenderWorld* m_RenderWorld;
		Renderer* const m_Renderer;

		float m_Accumulator;

		std::unique_ptr<B2DebugDraw> m_PhysDebugDraw;

		clan::Font m_DebugFont;
		clan::Font m_DebugFont2;

		struct ViewportRenderAction
		{
			ViewportPtr viewport;
			RenderAction action;
		};

		typedef std::map<std::string, RenderAction> RenderActionsMap;
		typedef std::map<std::string, ViewportRenderAction> ViewportRenderActionsMap;

		RenderActionsMap m_RenderActions;
		ViewportRenderActionsMap m_SingleViewportRenderActions;
		RenderActionsMap m_WorldRenderActions;
	};

	class CLRenderGUITask : public SystemTaskBase
	{
	public:
		CLRenderGUITask(CLRenderWorld* sysworld, const clan::Canvas& canvas, Renderer* const renderer);
		~CLRenderGUITask();

		void Update() override;

		SystemType GetTaskType() const { return SystemType::Simulation; }

		PerformanceHint GetPerformanceHint() const { return Short; }

		bool IsPrimaryThreadOnly() const
		{
			return true;
		}

		void onMouseMove(const clan::InputEvent &ev);

	private:
		CLRenderWorld* m_RenderWorld;
		Renderer* const m_Renderer;

		clan::Canvas m_Canvas;

		clan::Slot m_MouseMoveSlot;
		std::deque<clan::InputEvent> m_BufferedEvents;
		//Vector2i m_LastPosition;
	};

}

#endif
