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

#include "Common.h"

#include "FusionEditor.h"

#include "FusionBoostSignals2.h"
#include "FusionRenderer.h"
#include "FusionEntityManager.h"
#include "FusionScriptingEngine.h"
#include "FusionGUI.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>


namespace FusionEngine
{

	Editor::Editor(InputManager *input, Renderer *renderer, StreamingManager *streaming_manager, EntityManager *ent_manager)
		: m_Input(input),
		m_Renderer(renderer),
		m_Streamer(streaming_manager),
		m_EntityManager(ent_manager)
	{
	}

	Editor::~Editor()
	{
	}

	const std::string s_EditorSystemName = "Editor";

	const std::string &Editor::GetName() const
	{
		return s_EditorSystemName;
	}

	bool Editor::Initialise()
	{
		Enable(false);

		m_Viewport.reset(new Viewport());
		m_Camera.reset( new Camera(ScriptingEngine::getSingleton().GetEnginePtr()) );
		m_Viewport->SetCamera(m_Camera);

		m_RawInputConnection = m_Input->SignalRawInput.connect( boost::bind(&Editor::OnRawInput, this, _1) );

		return true;
	}

	void Editor::CleanUp()
	{
	}

	void Editor::Update(float split)
	{
		if (m_Enabled)
		{
			const CL_Vec2f &currentPos = m_Camera->GetPosition();
			m_Camera->SetPosition(currentPos.x + m_CamVelocity.x, currentPos.y + m_CamVelocity.y);
			m_Camera->Update(split);

			m_Renderer->Update(split);
		}
	}

	void Editor::Draw()
	{
		m_Renderer->Draw(m_Viewport);
	}

	void Editor::Enable(bool enable)
	{
		if (enable)
		{
			//this->PushMessage(new SystemMessage(SystemMessage::PAUSE, "Entities"));
			this->PushMessage(new SystemMessage(SystemMessage::HIDE, "Entities"));

			m_EntityManager->SetDomainState(GAME_DOMAIN, DS_STREAMING);

			this->PushMessage(new SystemMessage(SystemMessage::RESUME));
			this->PushMessage(new SystemMessage(SystemMessage::SHOW));
		}
		else
		{
			//this->PushMessage(new SystemMessage(SystemMessage::RESUME, "Entities"));
			this->PushMessage(new SystemMessage(SystemMessage::SHOW, "Entities"));

			m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL);

			this->PushMessage(new SystemMessage(SystemMessage::PAUSE));
			this->PushMessage(new SystemMessage(SystemMessage::HIDE));
		}
		m_Enabled = enable;
	}

	void Editor::OnRawInput(const RawInput &ev)
	{
		if (ev.ButtonPressed == true)
		{
			if (ev.Code == CL_KEY_LEFT)
			{
				m_CamVelocity.x = -10;
			}
			if (ev.Code == CL_KEY_RIGHT)
			{
				m_CamVelocity.x = 10;
			}
			if (ev.Code == CL_KEY_UP)
			{
				m_CamVelocity.y = -10;
			}
			if (ev.Code == CL_KEY_DOWN)
			{
				m_CamVelocity.y = 10;
			}
		}
		else if (ev.ButtonPressed == false)
		{
			if (ev.Code == CL_KEY_LEFT || ev.Code == CL_KEY_RIGHT)
			{
				m_CamVelocity.x = 0;
			}
			if (ev.Code == CL_KEY_UP || ev.Code == CL_KEY_DOWN)
			{
				m_CamVelocity.y = 0;
			}
		}
	}

	void Editor::StartEditor()
	{
		Enable();
	}

	void Editor::StopEditor()
	{
		Enable(false);
	}

	void Editor::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSingletonType<Editor>("Editor", engine);
		r = engine->RegisterObjectMethod("Editor",
			"void enable()",
			asMETHOD(Editor, Enable), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void startEditor()",
			asMETHOD(Editor, StartEditor), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void stopEditor()",
			asMETHOD(Editor, StopEditor), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

}
