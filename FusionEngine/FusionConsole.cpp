
#include "FusionConsole.h"

namespace FusionEngine
{
	Console::Console()
		: m_MaxData(g_ConsoleDefaultMaxData)
	{
		m_Data.resize(m_MaxData);
	}

	const std::string &Console::GetExceptionMarker() const
	{
		static std::string strExMkr("**");
		return strExMkr;
	}

	const std::string &Console::GetWarningMarker() const
	{
		static std::string strWnMkr("##");
		return strWnMkr;
	}

	void Console::Add(const std::string &message)
	{
		m_Data.push_back(message);

		if (m_Data.size() > m_MaxData)
			m_Data.pop_front();

		// Signal
		OnNewLine(message);
	}

	void Console::Add(const std::string& message, MessageType type)
	{
		std::string headedMessage = message;
		switch (type)
		{
		case MTNORMAL:
			// headedMessage = message;
			break;
		case MTWARNING:
			headedMessage = CL_String::format(GetWarningMarker() + " Warning:   %1", message);
			break;
		case MTERROR:
			headedMessage = CL_String::format(GetExceptionMarker() + " Exception: %1", message);
			break;
		}

		// Add the message to the console data
		m_Data.push_back(headedMessage);

		if (m_Data.size() > m_MaxData)
			m_Data.pop_front();

		// Signal
		OnNewLine(headedMessage);
	}

	void Console::Add(const FusionEngine::Exception *ex)
	{
		Add(ex->GetDescription(), ex->IsCritical() ? Console::MTERROR : Console::MTWARNING);
	}

	void Console::Clear()
	{
		if (!m_Data.empty())
		{
			m_Data.clear();

			// Signal
			OnClear();
		}
	}

	const Console::ConsoleLines& Console::GetHistory() const
	{
		return m_Data;
	}

}