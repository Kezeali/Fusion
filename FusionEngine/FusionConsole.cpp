
#include "FusionConsole.h"

namespace FusionEngine
{

	void Console::Add(const std::string &message)
	{
		m_Data.push_back(message);
	}

	void Console::Add(const FusionEngine::Error *error)
	{
		m_Data.push_back(error->GetError());
	}

}