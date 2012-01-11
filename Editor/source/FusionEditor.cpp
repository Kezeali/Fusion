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

#include "PrecompiledHeaders.h"

#include "FusionEditor.h"

#include "FusionCamera.h"
#include "FusionGUI.h"

#include "FusionAngelScriptSystem.h"

namespace FusionEngine
{

	Editor::Editor(const std::vector<CL_String>& args)
		: m_Rebuild(false)
	{
		auto& context = GUI::getSingleton().CreateContext("editor");
		context->SetMouseShowPeriod(500);
	}

	Editor::~Editor()
	{
	}

	void Editor::SetDisplay(const CL_DisplayWindow& display)
	{
		m_DisplayWindow = display;
		m_KeyUpSlot = m_DisplayWindow.get_ic().get_keyboard().sig_key_up().connect(this, &Editor::onKeyUp);
	}

	void Editor::OnWorldCreated(const std::shared_ptr<ISystemWorld>& world)
	{
		if (auto asw = std::dynamic_pointer_cast<AngelScriptWorld>(world))
		{
			SetAngelScriptWorld(asw);
		}
	}

	void Editor::Update(float time, float dt)
	{
		if (m_EditCam)
		{
			auto camPos = m_EditCam->GetPosition();
			if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_UP))
				camPos.y -= 400 * dt;
			if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_DOWN))
				camPos.y += 400 * dt;
			if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_LEFT))
				camPos.x -= 400 * dt;
			if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_RIGHT))
				camPos.x += 400 * dt;

			m_EditCam->SetPosition(camPos.x, camPos.y);
		}

		if (m_Rebuild && m_AngelScriptWorld)
		{
			m_Rebuild = false;
			m_AngelScriptWorld->BuildScripts();
		}
	}

	std::vector<std::shared_ptr<RendererExtension>> Editor::MakeRendererExtensions() const
	{
		return std::vector<std::shared_ptr<RendererExtension>>();
	}

	void Editor::onKeyUp(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (ev.id == CL_KEY_F7)
		{
			m_Rebuild = true;
		}
	}

}
