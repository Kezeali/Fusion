
#include "FusionConsole.h"

namespace FusionEngine
{

	Console::Console()
	{
	}

	void Console::Add(const std::string &message)
	{
		m_Data.push_back(message);

		// Signal
		OnNewLine(message);
	}

	void Console::Add(const FusionEngine::Error *error)
	{
		m_Data.push_back(error->GetError());

		// Signal
		OnNewLine(error->GetError());
	}

	void Console::Clear()
	{
		m_Data.clear();

		// Signal
		OnClear();
	}

	const Console::ConsoleLines& Console::GetHistory() const
	{
		return m_Data;
	}

}