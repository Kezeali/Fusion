/*
*  Copyright (c) 2006-2012 Fusion Project Team
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

#include <Rocket/AngelScript/Core/Core.h>
#include <Rocket/AngelScript/Controls/Controls.h>

#include "FusionConsole.h"
#include "FusionLogger.h"
#include "FusionMessageBox.h"
#include "FusionScriptManager.h"
#include "FusionScriptModule.h"

#include "FusionScriptTypeRegistrationUtils.h"
#include "scriptstdstring.h"

#include "FusionElementSelectableDataGrid.h"

#include "FusionFilesystemDataSource.h"

using namespace std::placeholders;

namespace FusionEngine
{

	const std::string s_GuiSystemName = "GUI";
	const float GUI::s_ClickPausePeriod = 10 * 0.001f;
	const float GUIContext::s_ClickPausePeriod = 10 * 0.001f;

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

	namespace
	{
		Rocket::Core::String stringToEString(std::string *obj)
		{
			return Rocket::Core::String(obj->c_str());
		}

		void stdstringCtor_FromEMPString(const Rocket::Core::String &copy, std::string* ptr)
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
	}

	GUIContext::GUIContext()
		: m_Context(nullptr),
		m_MouseShowPeriod(4.f),
		m_ShowMouseTimer(m_MouseShowPeriod),
		m_ClickPause(0)
	{
	}

	GUIContext::GUIContext(Rocket::Core::Context* context, clan::InputContext ic, bool enable_mouse)
		: m_Context(context),
		m_MouseShowPeriod(4.f),
		m_ShowMouseTimer(m_MouseShowPeriod),
		m_ClickPause(0)
	{
		if (enable_mouse)
		{
			// Mouse Events
			m_Slots.connect(ic.get_mouse().sig_key_down(), this, &GUIContext::onMouseDown);
			m_Slots.connect(ic.get_mouse().sig_key_up(), this, &GUIContext::onMouseUp);
			m_Slots.connect(ic.get_mouse().sig_pointer_move(), this, &GUIContext::onMouseMove);
		}
		// KBD events
		m_Slots.connect(ic.get_keyboard().sig_key_down(), this, &GUIContext::onKeyDown);
		m_Slots.connect(ic.get_keyboard().sig_key_up(), this, &GUIContext::onKeyUp);
	}

	GUIContext::GUIContext(const std::string& name, clan::InputContext ic, const Vector2i& size, bool enable_mouse)
		: m_Context(nullptr),
		m_MouseShowPeriod(4.f),
		m_ShowMouseTimer(m_MouseShowPeriod),
		m_ClickPause(0)
	{
		m_Context = Rocket::Core::CreateContext(Rocket::Core::String(&name[0], &name[0] + name.size()), Rocket::Core::Vector2i(size.x, size.y));

		if (enable_mouse)
		{
			// Mouse Events
			m_Slots.connect(ic.get_mouse().sig_key_down(), this, &GUIContext::onMouseDown);
			m_Slots.connect(ic.get_mouse().sig_key_up(), this, &GUIContext::onMouseUp);
			m_Slots.connect(ic.get_mouse().sig_pointer_move(), this, &GUIContext::onMouseMove);
		}
		// KBD events
		m_Slots.connect(ic.get_keyboard().sig_key_down(), this, &GUIContext::onKeyDown);
		m_Slots.connect(ic.get_keyboard().sig_key_up(), this, &GUIContext::onKeyUp);
	}

	GUIContext::~GUIContext()
	{
		if (m_Context)
			m_Context->RemoveReference();
		m_Slots.disconnect_all();
	}

	void GUIContext::Update(float dt)
	{
		if (m_Context)
		{
			m_Context->Update();

			if (m_ClickPause > 0)
				m_ClickPause -= dt;

			// Hide the cursor if the timeout has been reached
			if (m_ShowMouseTimer <= 0)
			{
				m_Context->ShowMouseCursor(false);
			}
			else
			{
				m_ShowMouseTimer -= dt;
			}
		}
	}

	void GUIContext::Draw()
	{
		if (m_Context)
			m_Context->Render();
	}

	void GUIContext::SetDimensions(const Vector2i &size)
	{
		if (m_Context)
			m_Context->SetDimensions(Rocket::Core::Vector2i(size.x, size.y));
	}

	void GUIContext::SetMouseShowPeriod(float period)
	{
		m_MouseShowPeriod = period;
	}

	float GUIContext::GetMouseShowPeriod() const
	{
		return m_MouseShowPeriod;
	}

	void GUIContext::ShowMouse()
	{
		m_ShowMouseTimer = m_MouseShowPeriod;
		m_Context->ShowMouseCursor(true);
	}

	void GUIContext::SetMouseCursorPosition(int x, int y, int modifier)
	{
		m_Context->ProcessMouseMove(x, y, modifier);
	}

	inline int getRktModifierFlags(const clan::InputEvent &ev)
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

	void GUIContext::onMouseDown(const clan::InputEvent &ev)
	{
		m_ClickPause = s_ClickPausePeriod;

		int modifier = getRktModifierFlags(ev);
		switch(ev.id)
		{
		case clan::mouse_left:
			m_Context->ProcessMouseButtonDown(0, modifier);
			break;
		case clan::mouse_right:
			m_Context->ProcessMouseButtonDown(1, modifier);
			break;
		case clan::mouse_middle:
			m_Context->ProcessMouseButtonDown(2, modifier);
			break;
		case clan::mouse_xbutton1:
			m_Context->ProcessMouseButtonDown(3, modifier);
			break;
		case clan::mouse_xbutton2:
			m_Context->ProcessMouseButtonDown(4, modifier);
			break;
		case clan::mouse_wheel_up:
			m_Context->ProcessMouseWheel(-1, modifier);
			m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, modifier);
			break;
		case clan::mouse_wheel_down:
			m_Context->ProcessMouseWheel(1, modifier);
			m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, modifier);
			break;
		}
	}

	void GUIContext::onMouseUp(const clan::InputEvent &ev)
	{
		m_ClickPause = s_ClickPausePeriod;

		int modifier = getRktModifierFlags(ev);
		switch(ev.id)
		{
		case clan::mouse_left:
			m_Context->ProcessMouseButtonUp(0, modifier);
			break;
		case clan::mouse_right:
			m_Context->ProcessMouseButtonUp(1, modifier);
			break;
		case clan::mouse_middle:
			m_Context->ProcessMouseButtonUp(2, modifier);
			break;
		case clan::mouse_xbutton1:
			m_Context->ProcessMouseButtonUp(3, modifier);
			break;
		case clan::mouse_xbutton2:
			m_Context->ProcessMouseButtonUp(4, modifier);
			break;
		case clan::mouse_wheel_up:
			m_Context->ProcessMouseWheel(0, modifier);
			break;
		case clan::mouse_wheel_down:
			m_Context->ProcessMouseWheel(0, modifier);
			break;
		}
	}

	void GUIContext::onMouseMove(const clan::InputEvent &ev)
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
		return id != clan::keycode_backspace && id != clan::keycode_escape && id != clan::keycode_delete;
	}

	void GUIContext::onKeyDown(const clan::InputEvent &ev)
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

	void GUIContext::onKeyUp(const clan::InputEvent &ev)
	{
		m_Context->ProcessKeyUp(CLKeyToRocketKeyIdent(ev.id), getRktModifierFlags(ev));

		//SendToConsole("Key Up");
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

	GUI::GUI(clan::Canvas canvas)
		: m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000),
		m_ClickPause(0),
		m_Canvas(canvas),
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

		m_RocketFileSys.reset(new RocketFileSystem());
		m_RocketRenderer.reset(new RocketRenderer(m_Canvas));
		m_RocketSystem.reset(new RocketSystem());
		
		Rocket::Core::SetFileInterface(m_RocketFileSys.get());
		Rocket::Core::SetRenderInterface(m_RocketRenderer.get());
		Rocket::Core::SetSystemInterface(m_RocketSystem.get());
		Rocket::Core::Initialise();
		Rocket::Controls::Initialise();

		LoadFonts("Data/core/gui/fonts/");

		ElementSelectableDataGrid::RegisterElement();
		m_DataFormatters.push_back(std::make_shared<ExpandButtonFormatter>());

		m_DataSources.push_back(std::make_shared<FilesystemDataSource>());
		m_DataFormatters.push_back(std::make_shared<FilesystemDataFormatter>());

		clan::GraphicContext gc = m_Canvas.get_window().get_gc();
		clan::InputContext ic =  m_Canvas.get_window().get_ic();

		// Create the world context, no input
		m_Contexts["world"] = std::make_shared<GUIContext>("world", ic, Vector2i(gc.get_width(), gc.get_height()), false);
		auto& screenCtx = m_Contexts["screen"] = std::make_shared<GUIContext>("screen", ic, Vector2i(gc.get_width(), gc.get_height()));

		if (auto c = screenCtx->m_Context->LoadMouseCursor("Data/core/gui/cursor.rml"))
			c->RemoveReference();
		if (auto c = screenCtx->m_Context->LoadMouseCursor("Data/core/gui/cursor-move.rml"))
			c->RemoveReference();

		m_MessageBoxMaker.reset(new MessageBoxMaker(screenCtx->m_Context));

		m_Slots.connect(m_Canvas.get_window().sig_resize(), this, &GUI::onResize);

		//m_Canvas.get_window().hide_cursor();

		m_Initialised = true;

		return true;
	}

	void GUI::InitialiseConsole(ScriptManager* script_manager)
	{
		// Generate the gui_base module used by the window template
		{
			auto guiModule = script_manager->GetModule("gui_base");
			Rocket::AngelScript::InitialiseModule(script_manager->GetEnginePtr(), "gui_base");
			script_manager->AddFile("Data/core/gui/gui_base.as", "gui_base");
			guiModule->Build();
		}

		auto consoleModule = script_manager->GetModule("console");
		Rocket::AngelScript::InitialiseModule(script_manager->GetEnginePtr(), "console");
		script_manager->AddFile("Data/core/gui/console.as", "console");
		consoleModule->Build();

		ScriptUtils::Calling::Caller registerConsole = ScriptUtils::Calling::Caller::Create(consoleModule->GetASModule(), "void RegisterConsole()");
		if (registerConsole)
		{
			registerConsole();
			m_ConsoleDocument = GetContext()->LoadDocument("Data/core/gui/console.rml");
		}
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
				m_Contexts["screen"]->m_Context->Update(); // Make sure the console ui gets freed right now (before script GC is forced below)
				ScriptManager::getSingleton().GetEnginePtr()->GarbageCollect();
			}

			//delete MessageBoxMaker::getSingletonPtr();
			m_MessageBoxMaker.reset();

			m_Contexts.clear();
			ScriptManager::getSingleton().GetEnginePtr()->GarbageCollect();
			Rocket::Core::Shutdown();

			//delete m_RocketFileSys;
			//delete m_RocketSystem;
			//delete m_RocketRenderer;
			m_RocketFileSys.reset();
			m_RocketSystem.reset();
			m_RocketRenderer.reset();


			//m_RocketFileSys = nullptr;
			//m_RocketSystem = nullptr;
			//m_RocketRenderer = nullptr;
			//m_Context = nullptr;
			m_Contexts.clear();

			m_Canvas.get_window().show_cursor();

			m_Initialised = false;
		}
	}

	void GUI::Update(float split)
	{
		for (auto it = m_Contexts.begin(), end = m_Contexts.end(); it != end; ++it)
			it->second->Update(split);

		//m_Context->Update();

		//if (m_ClickPause > 0)
		//	m_ClickPause -= split;

		//// Hide the cursor if the timeout has been reached
		//if ( m_ShowMouseTimer <= 0 )
		//{
		//	m_Context->ShowMouseCursor(false);
		//}
		//else
		//{
		//	m_ShowMouseTimer -= (int)split;
		//}
	}

	void GUI::Draw(const clan::Canvas& draw_to)
	{
		//m_Context->Render();
		m_Canvas.flush();
	}

	const std::shared_ptr<GUIContext>& GUI::CreateContext(const std::string& name, Vector2i size)
	{
		if (size.x == 0 && size.y == 0)
			size.set(m_Canvas.get_gc().get_width(), m_Canvas.get_gc().get_height());
		return m_Contexts[name] = std::make_shared<GUIContext>(name, m_Canvas.get_window().get_ic(), size);
	}

	Rocket::Core::Context *GUI::GetContext(const std::string& name) const
	{
		return m_Contexts.at(name)->m_Context;
	}

	Rocket::Core::ElementDocument *GUI::GetConsoleWindow() const
	{
		return m_ConsoleDocument;
	}

	void GUI::InitializeDebugger(const std::string& context)
	{
		auto entry = m_Contexts.find(context);
		if (entry != m_Contexts.end())
			m_DebuggerInitialized = Rocket::Debugger::Initialise(entry->second->m_Context);
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

	//void GUI::SetMouseShowPeriod(unsigned int period)
	//{
	//	m_MouseShowPeriod = period;
	//}

	//unsigned int GUI::GetMouseShowPeriod() const
	//{
	//	return m_MouseShowPeriod;
	//}

	//void GUI::ShowMouse()
	//{
	//	m_ShowMouseTimer = m_MouseShowPeriod;
	//	m_Context->ShowMouseCursor(true);
	//}

	//void GUI::SetMouseCursorPosition(int x, int y, int modifier)
	//{
	//	m_Context->ProcessMouseMove(x, y, modifier);
	//}

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
			Rocket::AngelScript::Initialise(engine);
			Rocket::AngelScript::Controls::Initialise(engine);

			Rocket::AngelScript::StringConversion<ScriptStringConverter>::Register(engine, "::string", true);

			r = engine->RegisterObjectBehaviour("string",
				asBEHAVE_CONSTRUCT,
				"void f(const Rocket::String &in)",
				asFUNCTION(stdstringCtor_FromEMPString),
				asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod("string",
				"string& opAssign(const Rocket::String &in)",
				asFUNCTION(stdstringAssignEMPString),
				asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

			r = engine->RegisterObjectMethod("string",
				"string& opAddAssign(const Rocket::String &in)",
				asFUNCTION(stdstringAddAssignEMPString),
				asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		}
		catch (Rocket::AngelScript::Exception &ex)
		{
			SendToConsole("Failed to register Rocket/AngelScript script classes. " + ex.m_Message);
			return;
		}

		RegisterSingletonType<FilesystemDataSource>("FilesystemDataSource", engine);

		r = engine->RegisterObjectMethod(
			"FilesystemDataSource", "bool isDirectory(const string &in, int row)",
			asMETHOD(FilesystemDataSource, IsDirectory), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"FilesystemDataSource", "string filename(const string &in, int row)",
			asMETHOD(FilesystemDataSource, GetFilename), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"FilesystemDataSource", "string path(const string &in, int row, bool include_filesystem = false)",
			asMETHOD(FilesystemDataSource, GetPath), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"FilesystemDataSource", "string preprocessPath(const string &in)",
			asMETHOD(FilesystemDataSource, PreproPath), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"FilesystemDataSource", "void refresh()",
			asMETHOD(FilesystemDataSource, Refresh), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"FilesystemDataSource", "void clearCache()",
			asMETHOD(FilesystemDataSource, ClearCache), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		RegisterSingletonType<GUI>("GUI", engine);

		//r = engine->RegisterObjectMethod(
		//	"GUI", "void setMouseShowPeriod(uint)",
		//	asMETHOD(GUI, SetMouseShowPeriod), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		//r = engine->RegisterObjectMethod(
		//	"GUI", "uint getMouseShowPeriod() const",
		//	asMETHOD(GUI, GetMouseShowPeriod), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		//r = engine->RegisterObjectMethod(
		//	"GUI", "void showMouse()",
		//	asMETHOD(GUI, ShowMouse), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void enableDebugger(const string &in)",
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
			"GUI", "Rocket::Context@ getContext() const",
			asFUNCTION(GUI_GetContextRef), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void showConsole()",
			asFUNCTION(GUI_ShowConsole), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(
			"GUI", "void hideConsole()",
			asFUNCTION(GUI_HideConsole), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		//r = engine->RegisterObjectMethod(
		//	"GUI", "void setMouseCursorPosition(int x, int y, int modifiers)",
		//	asMETHOD(GUI, SetMouseCursorPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
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

			event.manager->AddFile("Data/core/gui/console.as", event.module_name);
		}
		else if (event.type == BuildModuleEvent::PostBuild)
		{
			ModulePtr module = event.manager->GetModule(event.module_name);
			try
			{
				// Create the Console window (a window where console commands can be entered)
				module->GetCaller("void InitialiseConsole()")(); // register the element type
				m_ConsoleDocument = GetContext()->LoadDocument("Data/core/gui/console.rml");
			}
			catch (ScriptUtils::Exception &ex)
			{
				AddLogEntry(g_LogGeneral, "Failed to initialise the console window: " + ex.m_Message, LOG_NORMAL);
			}
		}
	}

	void GUI::onResize(int x, int y)
	{
		for (auto it = m_Contexts.begin(), end = m_Contexts.end(); it != end; ++it)
			it->second->SetDimensions(Vector2i(x, y));
	}

}
