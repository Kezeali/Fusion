
#include "FusionConsoleStdOutWriter.h"

#include <boost/bind.hpp>

namespace FusionEngine
{

	ConsoleStdOutWriter::ConsoleStdOutWriter()
		: m_Active(false)
	{
		// Use the singleton
		m_ConsoleOnNewLineSlot =
			Console::getSingleton().OnNewLine.connect(boost::bind(&ConsoleStdOutWriter::onConsoleNewline, this, _1));
	}

	ConsoleStdOutWriter::ConsoleStdOutWriter(Console* console)
		: m_Active(false)
	{
		// Use the given Console
		m_ConsoleOnNewLineSlot =
			console->OnNewLine.connect(boost::bind(&ConsoleStdOutWriter::onConsoleNewline, this, _1));
	}

	ConsoleStdOutWriter::~ConsoleStdOutWriter()
	{
		m_ConsoleOnNewLineSlot.disconnect();
	}

	void ConsoleStdOutWriter::Enable()
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

	void ConsoleStdOutWriter::onConsoleNewline(const std::string &message)
	{
		if (!m_Active)
			return;
		//std::string e_marker = Console::getSingleton().GetExceptionMarker();
		//std::string w_marker = Console::getSingleton().GetWarningMarker();

		std::string linemessage = cl_text("-- ") + CL_StringHelp::local8_to_text(message.c_str());
		CL_Console::write_line( linemessage.c_str() );
		//std::cout << "--  " << message << std::endl;
	}

}