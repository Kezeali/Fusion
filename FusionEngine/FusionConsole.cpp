
#include "FusionConsole.h"

namespace FusionEngine
{

	void Console::Add(const std::string &message)
	{
		m_Data.push_back(message);

		OnNewLine(message);
	}

	void Console::Add(const FusionEngine::Error *error)
	{
		m_Data.push_back(error->GetError());

		OnNewLine(error->GetError());
	}

	void Console::Clear()
	{
		m_Data.clear();

		OnClear();
	}

	const ColsoleLines &Console::GetHistory() const
	{
		return m_Data;
	}

}