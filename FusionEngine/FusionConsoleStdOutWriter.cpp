
#include "FusionConsoleStdOutWriter.h"

namespace FusionEngine
{

	ConsoleStdOutWriter::ConsoleStdOutWriter()
		: m_Active(false)
	{
	}

	ConsoleStdOutWriter::~ConsoleStdOutWriter()
	{
		// Make sure we are disconnected
		Disable();
	}

	void ConsoleStdOutWriter::Activate(Console* console)
	{
		if (console != NULL)
		{
			// Use the given Console
			m_ConsoleOnNewLineSlot =
				console->OnNewLine.connect(this, &ConsoleStdOutWriter::onConsoleNewline);
		}
		else
		{
			// Use the singleton
			m_ConsoleOnNewLineSlot =
				Console::getSingleton().OnNewLine.connect(this, &ConsoleStdOutWriter::onConsoleNewline);
		}

		m_Active = true;
	}

	void ConsoleStdOutWriter::Disable()
	{
		if (m_Active)
		{
			m_Active = false;
			Console::getSingleton().OnNewLine.disconnect(m_ConsoleOnNewLineSlot);
		}
	}

	void ConsoleStdOutWriter::onConsoleNewline(const std::string &message)
	{
		if (message.substr(0, 2) == "**")
		{
			std::cout << "** " << message.substr(2) << std::endl;
		}
		else if (message.substr(0, 7) == "Warning")
		{
			std::cout << "++ " << message << std::endl;
		}
		else
		{
			std::cout << "--  " << message << std::endl;
		}
	}

}