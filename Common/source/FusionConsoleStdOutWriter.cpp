
#include "PrecompiledHeaders.h"

#include "FusionConsoleStdOutWriter.h"

#include <ClanLib/core.h>

namespace FusionEngine
{

	ConsoleStdOutWriter::ConsoleStdOutWriter()
		: m_Active(false)
	{
		// Use the singleton
		m_ConsoleOnNewLineSlot = Console::getSingleton().OnNewData.connect([this](const std::string& data) {
			if (m_Active)
				clan::Console::write(data);
		});
	}

	ConsoleStdOutWriter::ConsoleStdOutWriter(Console* console)
		: m_Active(false)
	{
		// Use the given Console
		m_ConsoleOnNewLineSlot = console->OnNewData.connect([this](const std::string& data) {
			if (m_Active)
				clan::Console::write( data );
		});
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
	}

}