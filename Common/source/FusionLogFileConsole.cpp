/*
*  Copyright (c) 2009-2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "PrecompiledHeaders.h"

#include "FusionLogFileConsole.h"

#include "FusionConsole.h"


namespace FusionEngine
{

	ConsoleLogFile::ConsoleLogFile()
		: m_Console(Console::getSingletonPtr())
	{
	}

	ConsoleLogFile::ConsoleLogFile(Console* console)
		: m_Console(console)
	{
	}

	void ConsoleLogFile::Open(const std::string& filename)
	{
		m_Filename = filename;
		if (m_Console != NULL)
		{
			std::stringstream message;
			message << "Writing [" << filename.c_str() << "] entries to the console.";
			m_Console->Add(message.str());
		}
	}

	void ConsoleLogFile::Close()
	{
		if (m_Console != NULL)
		{
			std::stringstream message;
			message << "No longer writing [" << m_Filename.c_str() << "] entries to the console.";
			m_Console->Add(message.str());
		}
	}

	void ConsoleLogFile::Write(const std::string& entry)
	{
		if (m_Console != NULL)
		{
			std::string noNlEntry = entry;
			// Remove trailing newline
			if (noNlEntry[noNlEntry.length()-1] == '\n')
				noNlEntry = noNlEntry.substr(0, noNlEntry.length()-1);

			m_Console->Add(m_Filename, noNlEntry);
		}
	}

	void ConsoleLogFile::Flush()
	{
		// Nothing to do
	}

}
