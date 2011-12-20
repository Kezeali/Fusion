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

#include "FusionStableHeaders.h"

#include "FusionScriptDebuggerGUI.h"

#include "FusionBoostSignals2.h"
#include "FusionScriptManager.h"
#include "FusionGUI.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>


namespace FusionEngine
{

	ScriptDebuggerGui::ScriptDebuggerGui(FusionEngine::ScriptManager *manager)
		: m_Manager(manager)
	{
	}

	ScriptDebuggerGui::~ScriptDebuggerGui()
	{
	}

	bool ScriptDebuggerGui::Initialize()
	{
		Rocket::Core::Context *guiCtx = GUI::getSingletonPtr()->GetContext();
		m_Document = guiCtx->LoadDocument("gui/debugger.rml");

		if (m_Manager != NULL)
		{
			m_Manager->SubscribeToDebugEvents( std::bind(&ScriptDebuggerGui::OnDebugEvent, this, std::placeholders::_1) );
		}

		return true;
	}

	void ScriptDebuggerGui::CleanUp()
	{
		m_Document->Close();
	}

	void ScriptDebuggerGui::Update(float split)
	{
	}

	void ScriptDebuggerGui::Draw()
	{
	}

	void ScriptDebuggerGui::ProcessEvent(Rocket::Core::Event &ev)
	{
		using namespace Rocket::Controls;
		if (ev.GetTargetElement()->GetId() == "allow_breakpoints")
		{
			ElementFormControlInput *control =
				dynamic_cast<ElementFormControlInput*>( ev.GetTargetElement() );
			
			SetAllowBreakpoints( control->GetAttribute("checked", false) );
		}

		else if (ev.GetTargetElement()->GetId() == "pause")
		{			
			SetEnableStepthrough( true );
		}

		else if (ev.GetTargetElement()->GetId() == "resume")
		{			
			SetEnableStepthrough( false );
			Resume();
		}

		else if (ev.GetTargetElement()->GetId() == "step_into")
			TakeStep();
	}

	void ScriptDebuggerGui::OnDebugEvent(DebugEvent &ev)
	{
		if (ev.type == DebugEvent::Breakpoint && BreakpointsAllowed())
		{
			this->PushMessage(SystemMessage(SystemMessage::PAUSE, "Entities"));

			SetContext(ev.context);
		}
		else if (ev.type == DebugEvent::Step && StepthroughEnabled())
		{
			this->PushMessage(SystemMessage(SystemMessage::PAUSE, "Entities"));

			SetContext(ev.context);
		}
		else if (ev.type == DebugEvent::Exception && BreakpointsAllowed())
		{
			this->PushMessage(SystemMessage(SystemMessage::PAUSE, "Entities"));

			SetContext(ev.context);
		}

		else
		{
			// This debugger isn't set to pause on the given event type.
			ev.Resume();
		}
	}

	void ScriptDebuggerGui::SetAllowBreakpoints(bool allowed)
	{
		m_AllowBreakpoints = allowed;
	}
	bool ScriptDebuggerGui::BreakpointsAllowed() const
	{
		return m_AllowBreakpoints;
	}

	void ScriptDebuggerGui::SetEnableStepthrough(bool enabled)
	{
		m_EnableStepthrough = enabled;
	}
	bool ScriptDebuggerGui::StepthroughEnabled() const
	{
		return m_EnableStepthrough;
	}

	void ScriptDebuggerGui::Resume()
	{
		if (m_CurrentContext != NULL && (m_CurrentContext->GetState() & asEXECUTION_SUSPENDED) == asEXECUTION_SUSPENDED)
			m_CurrentContext->Execute();
	}

	void ScriptDebuggerGui::TakeStep()
	{
		this->PushMessage(SystemMessage(SystemMessage::STEP, "Entities"));
		Resume();
	}

	void ScriptDebuggerGui::SetContext(asIScriptContext *context)
	{
		if (m_CurrentContext != NULL)
			m_CurrentContext->Release();

		if (context != NULL)
			context->AddRef();

		m_CurrentContext = context;

		m_ContextChanged = true;
	}

	asIScriptContext *ScriptDebuggerGui::GetContext() const
	{
		return m_CurrentContext;
	}

}
