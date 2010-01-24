/*
  Copyright (c) 2006-2009 Fusion Project Team

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
#include "FusionCommon.h"

#include "FusionGUI.h"

#include "FusionConsole.h"
#include "FusionLogger.h"
#include "FusionScriptingEngine.h"
#include "FusionScriptModule.h"

#include "FusionElementSelectableDataGrid.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>
#include <Rocket/Debugger.h>

#include <Rocket/AngelScript/Core/ras_Core.h>
#include <Rocket/AngelScript/Controls/ras_Controls.h>

#include "scriptstring.h"


namespace FusionEngine
{

	const std::string s_GuiSystemName = "GUI";

	struct ScriptStringConverter
	{
		typedef CScriptString* string_type;

		string_type operator() (const EMP::Core::String& from) const
		{
			string_type obj = new CScriptString(from.CString());
			return obj;
		}

		EMP::Core::String operator() (const string_type& from) const
		{
			EMP::Core::String to(from->buffer.c_str());
			//from->Release();
			return to;
		}
	};

	EMP::Core::String stringToEString(CScriptString *obj)
	{
		return EMP::Core::String(obj->buffer.c_str());
	}

	CScriptString *CScriptStringFactory_FromEMPString(const EMP::Core::String &copy)
	{
		return new CScriptString(copy.CString());
	}

	CScriptString &CScriptStringAssignEMPString(const EMP::Core::String &value, CScriptString *obj)
	{
		obj->buffer = value.CString();
		return *obj;
	}

	CScriptString &CScriptStringAddAssignEMPString(const EMP::Core::String &value, CScriptString *obj)
	{
		obj->buffer += value.CString();
		return *obj;
	}

	GUI::GUI()
		: m_Modifiers(NOMOD),
		m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000),
		m_DebuggerInitialized(false),
		m_Initialised(false)
	{
		initScripting(ScriptingEngine::getSingletonPtr());
	}

	GUI::GUI(CL_DisplayWindow window)
		: m_Modifiers(NOMOD),
		m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000),
		m_Display(window),
		m_DebuggerInitialized(false),
		m_Initialised(false)
	{
		initScripting(ScriptingEngine::getSingletonPtr());
	}

	GUI::~GUI()
	{
		CleanUp();
	}

	const std::string &GUI::GetName() const
	{
		return s_GuiSystemName;
	}

	void GUI::Configure(const std::string& fname)
	{
	}

	bool GUI::Initialise()
	{
		using namespace Rocket;

		if (m_Initialised) // Don't allow repeated initialisation
			return false;

		m_DebuggerInitialized = false;

		m_RocketFileSys = new RocketFileSystem();
		m_RocketRenderer = new RocketRenderer(m_Display.get_gc());
		m_RocketSystem = new RocketSystem();
		
		Rocket::Core::SetFileInterface(m_RocketFileSys);
		Rocket::Core::SetRenderInterface(m_RocketRenderer);
		Rocket::Core::SetSystemInterface(m_RocketSystem);
		Rocket::Core::Initialise();
		Rocket::Controls::Initialise();

		//asIScriptEngine *iengine = ScriptingEngine::getSingleton().GetEnginePtr();
		//try
		//{
		//	Rocket::AngelScript::RegisterCore(iengine);
		//	Rocket::AngelScript::Controls::RegisterControls(iengine);
		//}
		//catch (Rocket::AngelScript::Exception &ex)
		//{
		//	SendToConsole("Failed to register Rocket/AngelScript script classes. " + ex.m_Message);
		//	return false;
		//}

		ElementSelectableDataGrid::RegisterElement();

		CL_GraphicContext gc = m_Display.get_gc();

		m_Context = Rocket::Core::CreateContext("default", EMP::Core::Vector2i(gc.get_width(), gc.get_width()));
		
		LoadFonts("core/gui/fonts/");
		m_Context->LoadMouseCursor("core/gui/cursor.rml");

		CL_InputContext ic = m_Display.get_ic();

		// Mouse Events
		m_Slots.connect(ic.get_mouse().sig_key_down(), this, &GUI::onMouseDown);
		m_Slots.connect(ic.get_mouse().sig_key_up(), this, &GUI::onMouseUp);
		m_Slots.connect(ic.get_mouse().sig_pointer_move(), this, &GUI::onMouseMove);
		// KBD events
		m_Slots.connect(ic.get_keyboard().sig_key_down(), this, &GUI::onKeyDown);
		m_Slots.connect(ic.get_keyboard().sig_key_up(), this, &GUI::onKeyUp);

		m_Slots.connect(m_Display.sig_resize(), this, &GUI::onResize);

		m_Display.hide_cursor();

		m_Initialised = true;

		return true;
	}

	void GUI::LoadFonts(const char* directory)
	{
		EMP::Core::String font_names[4];
		font_names[0] = "Delicious-Roman.otf";
		font_names[1] = "Delicious-Italic.otf";
		font_names[2] = "Delicious-Bold.otf";
		font_names[3] = "Delicious-BoldItalic.otf";

		for (int i = 0; i < sizeof(font_names) / sizeof(EMP::Core::String); i++)
		{
			Rocket::Core::FontDatabase::LoadFontFace(EMP::Core::String(directory) + font_names[i]);
		}
	}

	void GUI::Update(float split)
	{
		m_Context->Update();

		// Hide the cursor if the timeout has been reached
		if ( m_ShowMouseTimer <= 0 )
		{
			m_Context->ShowMouseCursor(false);
		}
		else
		{
			m_ShowMouseTimer -= (int)split;
		}
	}

	void GUI::Draw()
	{
		m_Context->Render();
	}

	void GUI::CleanUp()
	{
		if (m_Initialised)
		{
			m_Context->RemoveReference();
			ScriptingEngine::getSingleton().GetEnginePtr()->GarbageCollect();
			Rocket::Core::Shutdown();

			delete m_RocketFileSys;
			delete m_RocketSystem;
			delete m_RocketRenderer;

			m_RocketFileSys = NULL;
			m_RocketSystem = NULL;
			m_RocketRenderer = NULL;
			m_Context = NULL;

			m_Initialised = false;
		}

		m_Display.show_cursor();
	}

	Rocket::Core::Context *GUI::GetContext() const
	{
		return m_Context;
	}

	void GUI::InitializeDebugger()
	{
		if (!m_DebuggerInitialized)
			m_DebuggerInitialized = Rocket::Debugger::Initialise(m_Context);
	}

	void GUI::ShowDebugger()
	{
		Rocket::Debugger::SetVisible(true);
	}

	void GUI::HideDebugger()
	{
		Rocket::Debugger::SetVisible(false);
	}

	bool GUI::DebuggerIsVisible() const
	{
		return Rocket::Debugger::IsVisible();
	}

	void GUI::SetMouseShowPeriod(unsigned int period)
	{
		m_MouseShowPeriod = period;
	}

	unsigned int GUI::GetMouseShowPeriod() const
	{
		return m_MouseShowPeriod;
	}

	void GUI::ShowMouse()
	{
		m_ShowMouseTimer = m_MouseShowPeriod;
		m_Context->ShowMouseCursor(true);
	}

	void GUI::SetMouseCursorPosition(int x, int y, int modifier)
	{
		m_Context->ProcessMouseMove(x, y, modifier);
	}
	void GUI::SetMouseCursorPosition_default(int x, int y)
	{
		m_Context->ProcessMouseMove(x, y, 0);
	}

	void GUI::Register(ScriptingEngine *engine)
	{
		asIScriptEngine *iengine = engine->GetEnginePtr();
		int r;

		try
		{
			Rocket::AngelScript::RegisterCore(iengine);
			Rocket::AngelScript::Controls::RegisterControls(iengine);
			Rocket::AngelScript::StringConversion<ScriptStringConverter>::Register(iengine, "string", true);

			r = iengine->RegisterObjectBehaviour("string",
				asBEHAVE_FACTORY,
				"string@ f(const e_String&in)",
				asFUNCTION(CScriptStringFactory_FromEMPString),
				asCALL_CDECL); FSN_ASSERT(r >= 0);

			r = iengine->RegisterObjectMethod("string",
				"string& opAssign(const e_String&in)",
				asFUNCTION(CScriptStringAssignEMPString),
				asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = iengine->RegisterObjectMethod("string",
				"string& opAddAssign(const e_String&in)",
				asFUNCTION(CScriptStringAddAssignEMPString),
				asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		}
		catch (Rocket::AngelScript::Exception &ex)
		{
			SendToConsole("Failed to register Rocket/AngelScript script classes. " + ex.m_Message);
			return;
		}

		RegisterSingletonType<GUI>("GUI", iengine);

		r = iengine->RegisterObjectMethod(
			"GUI", "void setMouseShowPeriod(uint)",
			asMETHOD(GUI, SetMouseShowPeriod), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = iengine->RegisterObjectMethod(
			"GUI", "uint getMouseShowPeriod() const",
			asMETHOD(GUI, GetMouseShowPeriod), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = iengine->RegisterObjectMethod(
			"GUI", "void showMouse()",
			asMETHOD(GUI, ShowMouse), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = iengine->RegisterObjectMethod(
			"GUI", "void enableDebugger()",
			asMETHOD(GUI, InitializeDebugger), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = iengine->RegisterObjectMethod(
			"GUI", "void showDebugger()",
			asMETHOD(GUI, ShowDebugger), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = iengine->RegisterObjectMethod(
			"GUI", "void hideDebugger()",
			asMETHOD(GUI, HideDebugger), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = iengine->RegisterObjectMethod(
			"GUI", "bool debuggerIsVisible() const",
			asMETHOD(GUI, DebuggerIsVisible), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = iengine->RegisterObjectMethod(
			"GUI", "Context& getContext()",
			asMETHOD(GUI, GetContext), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = iengine->RegisterObjectMethod(
			"GUI", "void setMouseCursorPosition(int x, int y)",
			asMETHOD(GUI, SetMouseCursorPosition_default), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = iengine->RegisterObjectMethod(
			"GUI", "void setMouseCursorPosition(int x, int y, int modifiers)",
			asMETHOD(GUI, SetMouseCursorPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

	void GUI::SetModule(ModulePtr module)
	{
		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( boost::bind(&GUI::onModuleBuild, this, _1) );
	}

	void GUI::initScripting(FusionEngine::ScriptingEngine *manager)
	{
		manager->RegisterGlobalObject("GUI gui", this);
	}

	void GUI::onModuleBuild(BuildModuleEvent& event)
	{
		if (event.type == BuildModuleEvent::PreBuild)
		{
			Rocket::AngelScript::InitialiseModule(event.manager->GetEnginePtr(), event.module_name);
		}
	}

	void GUI::onMouseDown(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		switch(ev.id)
		{
		case CL_MOUSE_LEFT:
			m_Context->ProcessMouseButtonDown(0, modifier);
			break;
		case CL_MOUSE_RIGHT:
			m_Context->ProcessMouseButtonDown(1, modifier);
			break;
		case CL_MOUSE_MIDDLE:
			m_Context->ProcessMouseButtonDown(2, modifier);
			break;
		case CL_MOUSE_XBUTTON1:
			m_Context->ProcessMouseButtonDown(3, modifier);
			break;
		case CL_MOUSE_XBUTTON2:
			m_Context->ProcessMouseButtonDown(4, modifier);
			break;
		case CL_MOUSE_WHEEL_UP:
			m_Context->ProcessMouseWheel(-1, modifier);
			break;
		case CL_MOUSE_WHEEL_DOWN:
			m_Context->ProcessMouseWheel(1, modifier);
			break;
		}

	}

	void GUI::onMouseUp(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		switch(ev.id)
		{
		case CL_MOUSE_LEFT:
			m_Context->ProcessMouseButtonUp(0, modifier);
			break;
		case CL_MOUSE_RIGHT:
			m_Context->ProcessMouseButtonUp(1, modifier);
			break;
		case CL_MOUSE_MIDDLE:
			m_Context->ProcessMouseButtonUp(2, modifier);
			break;
		case CL_MOUSE_XBUTTON1:
			m_Context->ProcessMouseButtonUp(3, modifier);
			break;
		case CL_MOUSE_XBUTTON2:
			m_Context->ProcessMouseButtonUp(4, modifier);
			break;
		case CL_MOUSE_WHEEL_UP:
			m_Context->ProcessMouseWheel(0, modifier);
			break;
		case CL_MOUSE_WHEEL_DOWN:
			m_Context->ProcessMouseWheel(0, modifier);
			break;
		}

	}

	void GUI::onMouseMove(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, modifier);

		if (m_ShowMouseTimer <= 0 && m_MouseShowPeriod > 0)
		{
			m_ShowMouseTimer = m_MouseShowPeriod;
			m_Context->ShowMouseCursor(true);
		}
	}

	void GUI::onKeyDown(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		// Grab characters
		if (!ev.alt && !ev.ctrl && !ev.str.empty())
		{
			if (ev.id != CL_KEY_BACKSPACE && ev.id != CL_KEY_DELETE)
			{
				Rocket::Core::String text = (EMP::Core::word*)ev.str.c_str();
				m_Context->ProcessTextInput(text);
			}
			//const wchar_t* c_str = ev.str.c_str();
			// Inject all the characters given
			//for (int c = 0; c < ev.str.length(); c++)
			//	m_Context->ProcessTextInput( EMP::Core::word(c_str[c]) );
		}

		m_Context->ProcessKeyDown(CLKeyToRocketKeyIdent(ev.id), modifier);
	}

	void GUI::onKeyUp(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		m_Context->ProcessKeyUp(CLKeyToRocketKeyIdent(ev.id), modifier);
	}

	void GUI::onResize(int x, int y)
	{
		m_Context->SetDimensions(EMP::Core::Vector2i(x, y));
	}

}
