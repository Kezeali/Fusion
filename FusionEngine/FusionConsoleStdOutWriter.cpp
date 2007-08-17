
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
		std::string e_marker = Console::getSingleton().GetExceptionMarker();
		std::string w_marker = Console::getSingleton().GetWarningMarker();

		if (message.substr(0, e_marker.length()) == e_marker)
		{
			std::cout << "** " << message.substr(e_marker.length()) << std::endl;
		}
		else if (message.substr(0, w_marker.length()) == w_marker)
		{
			std::cout << "++ " << message.substr(w_marker.length()) << std::endl;
		}
		else
		{
			std::cout << "--  " << message << std::endl;
		}
	}

}