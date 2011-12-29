/*
*  Copyright (c) 2006-2010 Fusion Project Team
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

#include "FusionGUI.h"
#include <Rocket/Core.h>
#include <Rocket/Controls.h>
#include <Rocket/Debugger.h>

#include <Rocket/AngelScript/Core/ras_Core.h>
#include <Rocket/AngelScript/Controls/ras_Controls.h>

#include "FusionConsole.h"
#include "FusionLogger.h"
#include "FusionMessageBox.h"
#include "FusionScriptManager.h"
#include "FusionScriptModule.h"

#include "FusionScriptTypeRegistrationUtils.h"
#include "scriptstdstring.h"

#include "FusionElementSelectableDataGrid.h"

using namespace std::placeholders;

namespace FusionEngine
{

	const std::string s_GuiSystemName = "GUI";
	const float GUI::s_ClickPausePeriod = 10*0.001f;

	//! Adds an expand button element when a row has sub-rows to display
	class ExpandButtonFormatter : public Rocket::Controls::DataFormatter
	{
	public:
		ExpandButtonFormatter();

		//! DataFormatter impl.
		void FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data);
	};

	ExpandButtonFormatter::ExpandButtonFormatter()
		: Rocket::Controls::DataFormatter("expand_button")
	{}

	void ExpandButtonFormatter::FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data)
	{
		// Data format:
		// raw_data[0] is the number of children that this row has, a button is created if this is more than zero.

		//int num_children = 0;
		//Rocket::Core::TypeConverter<Rocket::Core::String, int>::Convert(raw_data[0], num_children);

		if (raw_data[0] != "0")
		{
			formatted_data = "<datagridexpand />";
		}
		else
		{
			formatted_data = "";
		}
	}


	struct ScriptStringConverter
	{
		typedef std::string string_type;

		string_type operator() (const Rocket::Core::String& from) const
		{
			string_type obj = std::string(from.CString());
			return obj;
		}

		Rocket::Core::String operator() (const string_type& from) const
		{
			Rocket::Core::String to(from.c_str());
			//from->Release();
			return to;
		}
	};

	Rocket::Core::String stringToEString(std::string *obj)
	{
		return Rocket::Core::String(obj->c_str());
	}

	static void stdstringCtor_FromEMPString(const Rocket::Core::String &copy, std::string* ptr)
	{
		new(ptr) std::string(copy.CString());
	}

	std::string &stdstringAssignEMPString(const Rocket::Core::String &value, std::string &obj)
	{
		obj = value.CString();
		return obj;
	}

	std::string &stdstringAddAssignEMPString(const Rocket::Core::String &value, std::string &obj)
	{
		obj += value.CString();
		return obj;
	}

	GUI::GUI()
		: m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000),
		m_ClickPause(0),
		m_DebuggerInitialized(false),
		m_Initialised(false),
		m_ConsoleDocument(nullptr)
	{
		initScripting(ScriptManager::getSingletonPtr());
	}

	GUI::GUI(CL_DisplayWindow window)
		: m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000),
		m_ClickPause(0),
		m_Display(window),
		m_DebuggerInitialized(false),
		m_Initialised(false),
		m_ConsoleDocument(nullptr)
	{
		initScripting(ScriptManager::getSingletonPtr());
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

		ElementSelectableDataGrid::RegisterElement();
		m_DataFormatters.push_back(std::shared_ptr<Rocket::Controls::DataFormatter>(new ExpandButtonFormatter()));

		CL_GraphicContext gc = m_Display.get_gc();

		m_Context = Rocket::Core::CreateContext("default", Rocket::Core::Vector2i(gc.get_width(), gc.get_height()));

		LoadFonts("core/gui/fonts/");
		m_Context->LoadMouseCursor("core/gui/cursor.rml");

		new MessageBoxMaker(m_Context);

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
		Rocket::Core::String font_names[4];
		font_names[0] = "Delicious-Roman.otf";
		font_names[1] = "Delicious-Italic.otf";
		font_names[2] = "Delicious-Bold.otf";
		font_names[3] = "Delicious-BoldItalic.otf";

		for (int i = 0; i < sizeof(font_names) / sizeof(Rocket::Core::String); i++)
		{
			Rocket::Core::FontDatabase::LoadFontFace(Rocket::Core::String(directory) + font_names[i]);
		}
	}

	void GUI::CleanUp()
	{
		if (m_Initialised)
		{
			m_DataFormatters.clear();

			if (m_ConsoleDocument != nullptr)
			{
				m_ConsoleDocument->Close();
				//m_Context->UnloadDocument(m_ConsoleDocument);
				m_ConsoleDocument->RemoveReference();
				m_ConsoleDocument = nullptr;
				m_Context->Update(); // Make sure the console ui gets freed right now (before script GC is forced below)
				ScriptManager::getSingleton().GetEnginePtr()->GarbageCollect();
			}

			delete MessageBoxMaker::getSingletonPtr();

			m_Context->RemoveReference();
			ScriptManager::getSingleton().GetEnginePtr()->GarbageCollect();
			Rocket::Core::Shutdown();

			delete m_RocketFileSys;
			delete m_RocketSystem;
			delete m_RocketRenderer;

			m_RocketFileSys = nullptr;
			m_RocketSystem = nullptr;
			m_RocketRenderer = nullptr;
			m_Context = nullptr;

			m_Display.show_cursor();

			m_Initialised = false;
		}
	}

	void GUI::Update(float split)
	{
		m_Context->Update();

		if (m_ClickPause > 0)
			m_ClickPause -= split;

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
		//for (int i = 0; i < m_Context->GetNumDocuments(); ++i)
		//{
		//	Rocket::Core::ElementDocument *doc = m_Context->GetDocument(i);
		//	CL_Rectf outline;
		//	outline.top = doc->GetAbsoluteTop();
		//	outline.left = doc->GetAbsoluteLeft(); 
		//	outline.set_width(doc->GetOffsetWidth());
		//	outline.set_height(doc->GetOffsetHeight());
		//	if (doc->IsClassSet("focus"))
		//		outline.expand(16.0f);
		//	else
		//		outline.expand(8.0f);
		//	CL_Draw::fill(m_Display.get_gc(), outline, CL_Colorf(0.0f, 0.0f, 0.0f, 0.75f));
		//}
	}

	Rocket::Core::Context *GUI::GetContext() const
	{
		return m_Context;
	}

	Rocket::Core::ElementDocument *GUI::GetConsoleWindow() const
	{
		return m_ConsoleDocument;
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

	Rocket::Core::Context* GUI_GetContextRef(GUI* obj)
	{
		Rocket::Core::Context* context = obj->GetContext();
		context->AddReference();
		return context;
	}

	void GUI_ShowConsole(GUI* obj)
	{
		Rocket::Core::ElementDocument *consoleWindow = obj->GetConsoleWindow();
		if (consoleWindow != nullptr && consoleWindow->GetContext()->GetFocusElement() != consoleWindow)
			consoleWindow->Show();
	}

	void GUI_HideConsole(GUI* obj)
	{
		Rocket::Core::ElementDocument *consoleWindow = obj->GetConsoleWindow();
		if (consoleWindow != nullptr)
			consoleWindow->Hide();
	}

	void GUI::Register(ScriptManager *mgr)
	{
		asIScriptEngine *engine = mgr->GetEnginePtr();
		int r;

		try
		{
			Rocket::AngelScript::RegisterCore(engine);
			Rocket::AngelScript::Controls::RegisterControls(engine);
			Rocket::AngelScript::StringConversion<ScriptStringConverter>::Register(engine, "string", true);

			r = engine->RegisterObjectBehaviour("string",
				asBEHAVE_CONSTRUCT,
				"void f(const string &in)",
				asFUNCTION(stdstringCtor_FromEMPString),
				asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod("string",
				"string& opAssign(const rString&in)",
				asFUNCTION(stdstringAssignEMPString),
				asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod("string",
				"string& opAddAssign(const rString&in)",
				asFUNCTION(stdstringAddAssignEMPString),
				asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		}
		catch (Rocket::AngelScript::Exception &ex)
		{
			SendToConsole("Failed to register Rocket/AngelScript script classes. " + ex.m_Message);
			return;
		}

		RegisterSingletonType<GUI>("GUI", engine);

		r = engine->RegisterObjectMethod(
			"GUI", "void setMouseShowPeriod(uint)",
			asMETHOD(GUI, SetMouseShowPeriod), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod(
			"GUI", "uint getMouseShowPeriod() const",
			asMETHOD(GUI, GetMouseShowPeriod), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void showMouse()",
			asMETHOD(GUI, ShowMouse), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void enableDebugger()",
			asMETHOD(GUI, InitializeDebugger), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void showDebugger()",
			asMETHOD(GUI, ShowDebugger), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void hideDebugger()",
			asMETHOD(GUI, HideDebugger), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "bool debuggerIsVisible() const",
			asMETHOD(GUI, DebuggerIsVisible), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "Context@ getContext() const",
			asFUNCTION(GUI_GetContextRef), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void showConsole()",
			asFUNCTION(GUI_ShowConsole), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void hideConsole()",
			asFUNCTION(GUI_HideConsole), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void setMouseCursorPosition(int x, int y)",
			asMETHOD(GUI, SetMouseCursorPosition_default), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void setMouseCursorPosition(int x, int y, int modifiers)",
			asMETHOD(GUI, SetMouseCursorPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

	void GUI::SetModule(ModulePtr module)
	{
		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( std::bind(&GUI::onModuleBuild, this, _1) );
	}

	void GUI::initScripting(FusionEngine::ScriptManager *manager)
	{
		manager->RegisterGlobalObject("GUI gui", this);
	}

	void GUI::onModuleBuild(BuildModuleEvent& event)
	{
		if (event.type == BuildModuleEvent::PreBuild)
		{
			Rocket::AngelScript::InitialiseModule(event.manager->GetEnginePtr(), event.module_name);

			event.manager->AddFile("core/gui/console.as", "main");
		}
		else if (event.type == BuildModuleEvent::PostBuild)
		{
			ModulePtr module = event.manager->GetModule(event.module_name);
			try
			{
				// Create the Console window (a window where console commands can be entered)
				module->GetCaller("void InitialiseConsole()")(); // register the element type
				m_ConsoleDocument = GetContext()->LoadDocument("core/gui/console.rml");
			}
			catch (ScriptUtils::Exception &ex)
			{
				AddLogEntry(g_LogGeneral, "Failed to initialise the console window: " + ex.m_Message, LOG_NORMAL);
			}
		}
	}

	inline int getRktModifierFlags(const CL_InputEvent &ev)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;
		return modifier;
	}

	void GUI::onMouseDown(const CL_InputEvent &ev, const CL_InputState &state)
	{
		m_ClickPause = s_ClickPausePeriod;

		int modifier = getRktModifierFlags(ev);
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
			m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, modifier);
			break;
		case CL_MOUSE_WHEEL_DOWN:
			m_Context->ProcessMouseWheel(1, modifier);
			m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, modifier);
			break;
		}
	}

	void GUI::onMouseUp(const CL_InputEvent &ev, const CL_InputState &state)
	{
		m_ClickPause = s_ClickPausePeriod;

		int modifier = getRktModifierFlags(ev);
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
		if (m_ClickPause <= 0)
			m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, getRktModifierFlags(ev));

		if (m_ShowMouseTimer <= 0 && m_MouseShowPeriod > 0)
		{
			m_ShowMouseTimer = m_MouseShowPeriod;
			m_Context->ShowMouseCursor(true);
		}
	}

	inline bool isNonDisplayKey(int id)
	{
		return id != CL_KEY_BACKSPACE && id != CL_KEY_ESCAPE && id != CL_KEY_DELETE;
	}

	void GUI::onKeyDown(const CL_InputEvent &ev, const CL_InputState &state)
	{
		// Grab characters
		if (!ev.alt && !ev.ctrl && !ev.str.empty() && isNonDisplayKey(ev.id))
		{
			Rocket::Core::String text = ev.str.c_str();
			m_Context->ProcessTextInput(text);
		}

		m_Context->ProcessKeyDown(CLKeyToRocketKeyIdent(ev.id), getRktModifierFlags(ev));

		//SendToConsole("Key Down");
	}

	void GUI::onKeyUp(const CL_InputEvent &ev, const CL_InputState &state)
	{
		m_Context->ProcessKeyUp(CLKeyToRocketKeyIdent(ev.id), getRktModifierFlags(ev));

		//SendToConsole("Key Up");
	}

	void GUI::onResize(int x, int y)
	{
		m_Context->SetDimensions(Rocket::Core::Vector2i(x, y));
	}

}
