/*
  Copyright (c) 2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
		
		
	File Author(s):

		Elliot Hayward

*/

#ifndef H_FusionEngine_ScriptDebuggerGUI
#define H_FusionEngine_ScriptDebuggerGUI

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionState.h"

#include "FusionScriptManager.h"

#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/ElementDocument.h>


namespace FusionEngine
{

	class ScriptDebuggerGui : public System, public Rocket::Core::EventListener
	{
	public:
		ScriptDebuggerGui(ScriptManager *manager);
		~ScriptDebuggerGui();

	public:
		bool Initialize();

		void CleanUp();

		void Update(float split);

		void Draw();

		//! Process GUI element events
		void ProcessEvent(Rocket::Core::Event& ev);

		void OnDebugEvent(DebugEvent& ev);

		void SetAllowBreakpoints(bool allowed);
		bool BreakpointsAllowed() const;

		void SetEnableStepthrough(bool enabled);
		bool StepthroughEnabled() const;

		void Resume();
		void TakeStep();

		void SetContext(asIScriptContext *context);
		asIScriptContext *GetContext() const;

	protected:
		ScriptManager *m_Manager;

		Rocket::Core::ElementDocument *m_Document;

		bool m_AllowBreakpoints;
		bool m_EnableStepthrough;

		bool m_ContextChanged;

		asIScriptContext *m_CurrentContext;
	};

}

#endif
