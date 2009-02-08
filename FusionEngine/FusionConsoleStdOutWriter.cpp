
#include "FusionConsoleStdOutWriter.h"

namespace FusionEngine
{

	ConsoleStdOutWriter::ConsoleStdOutWriter()
		: m_Active(false)
	{
		// Use the singleton
		m_ConsoleOnNewLineSlot =
			Console::getSingleton().OnNewLine.connect(this, &ConsoleStdOutWriter::onConsoleNewline);
	}

	ConsoleStdOutWriter::ConsoleStdOutWriter(Console* console)
		: m_Active(false)
	{
		// Use the given Console
		m_ConsoleOnNewLineSlot =
			console->OnNewLine.connect(this, &ConsoleStdOutWriter::onConsoleNewline);
	}

	ConsoleStdOutWriter::~ConsoleStdOutWriter()
	{
	}

	void ConsoleStdOutWriter::Activate()
	{
		//m_ConsoleOnNewLineSlot.enable();

		m_Active = true;
	}

	void ConsoleStdOutWriter::Disable()
	{
		if (m_Active)
		{
			m_Active = false;
			//m_ConsoleOnNewLineSlot.disable();
		}
	}

	void ConsoleStdOutWriter::onConsoleNewline(const std::wstring &message)
	{
		//std::wstring e_marker = Console::getSingleton().GetExceptionMarker();
		//std::wstring w_marker = Console::getSingleton().GetWarningMarker();

		std::wstring e_marker = L"**";
		std::wstring w_marker = L"##";

		if (message.substr(0, e_marker.length()) == e_marker)
		{
			CL_Console::write_line( L"** " + message.substr(e_marker.length()) );
			//std::cout << "** " << message.substr(e_marker.length()) << std::endl;
		}
		else if (message.substr(0, w_marker.length()) == w_marker)
		{
			CL_Console::write_line( L"++ " + message.substr(w_marker.length()) );
			//std::cout << "++ " << message.substr(w_marker.length()) << std::endl;
		}
		else
		{
			CL_Console::write_line( L"-- " + message );
			//std::cout << "--  " << message << std::endl;
		}
	}

}