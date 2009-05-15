
#include "FusionConsole.h"

#include "FusionScriptingEngine.h"
#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{
	Console::Console()
		: m_MaxData(g_ConsoleDefaultMaxData)
	{
		m_Data.resize(m_MaxData);
	}

	const std::wstring &Console::GetExceptionMarker() const
	{
		static std::wstring strExMkr(L"**");
		return strExMkr;
	}

	const std::wstring &Console::GetWarningMarker() const
	{
		static std::wstring strWnMkr(L"##");
		return strWnMkr;
	}

	void Console::add_raw(const std::wstring& message)
	{
		m_Data.push_back(message);

		if (m_Data.size() > m_MaxData)
			m_Data.pop_front();

		// Signal
		OnNewLine.invoke(message);
	}

	inline std::wstring nHyphens(std::wstring::size_type n)
	{
		// Could do this with a table, but I don't care that much
		std::wstring string;
		for (std::wstring::size_type i=0; i<n; i++)
			string += L"-";
		return string;
	}

	void Console::Add(const std::wstring &message)
	{
		// Close the grouped entries section
		if (!m_LastHeading.empty())
		{
			m_LastHeading = L"";
			// Add a closing line
			add_raw( L"}" );
		}

		add_raw(message);
	}

	void Console::Add(const std::wstring& message, MessageType type)
	{
		std::wstring headedMessage = message;
		switch (type)
		{
		case MTNORMAL:
			// headedMessage = message;
			break;
		case MTWARNING:
			headedMessage = cl_format(GetWarningMarker() + L" Warning:   %1", message);
			break;
		case MTERROR:
			headedMessage = cl_format(GetExceptionMarker() + L" Exception: %1", message);
			break;
		}

		// Add the message to the console data
		Add(headedMessage);
	}

	void Console::Add(const std::wstring& heading, const std::wstring& message)
	{
		if (heading != m_LastHeading)
		{
			// Add new heading (will also close the current heading if there is one)
			Add(L"[" + heading + L"]:");
			add_raw(L"{");
		}

		// Add the message
		add_raw(L" " + message);

		m_LastHeading = heading;
	}

	//void Console::Add(const FusionEngine::Exception *ex)
	//{
	//	Add(ex->GetDescription(), ex->IsCritical() ? Console::MTERROR : Console::MTWARNING);
	//}

	void Console::PrintLn(const std::string &message)
	{
		Add(std::wstring(message.begin(), message.end()));
	}

	void Console::PrintLn_int(int message)
	{
		Add(CL_StringHelp::int_to_text(message));
	}

	void Console::PrintLn_double(double message)
	{
		Add(CL_StringHelp::double_to_text(message));
	}

	void Console::Clear()
	{
		if (!m_Data.empty())
		{
			m_Data.clear();

			// Signal
			OnClear.invoke();
		}
	}

	const Console::ConsoleLines& Console::GetHistory() const
	{
		return m_Data;
	}

	void Console::RegisterScriptElements(ScriptingEngine *manager)
	{
#ifndef FSN_DONT_USE_SCRIPTING
		int r;
		asIScriptEngine* engine = manager->GetEnginePtr();

		RegisterTypeNoHandle<Console>("Console", engine);
		
		engine->RegisterObjectMethod("Console", "void println(string &in)", asMETHOD(Console, PrintLn), asCALL_THISCALL);
		engine->RegisterObjectMethod("Console", "void println(int)", asMETHOD(Console, PrintLn_int), asCALL_THISCALL);
		engine->RegisterObjectMethod("Console", "void println(double)", asMETHOD(Console, PrintLn_double), asCALL_THISCALL);

		engine->RegisterObjectMethod("Console", "void clear()", asMETHOD(Console, Clear), asCALL_THISCALL);

		manager->RegisterGlobalObject("Console console", this);
#endif
	}

}