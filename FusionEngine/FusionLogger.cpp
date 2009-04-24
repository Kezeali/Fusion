/*
  Copyright (c) 2006-2007 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.

		
	File Author(s):

		Elliot Hayward
*/

#include "FusionLogger.h"

/// Fusion
#include "FusionException.h"
#include "FusionConsole.h"
#include "FusionPaths.h"

#include "FusionLogPhysFS.h"

#include <time.h>

#if _MSC_VER > 1000
# pragma warning(push)
# pragma warning(disable:4996) // unsafe (depreciated) function warnings
#endif

namespace FusionEngine
{

	Logger::Logger()
		: m_ConsoleLogging(false),
		m_UseDating(true),
		m_Ext(g_LogDefaultExt)
	{
	}

	Logger::~Logger()
	{
		// Remove all logs
		m_Logs.clear();
		// Make sure we are disconnected from the console
		DisableConsoleLogging();
	}

	void Logger::ActivateConsoleLogging()
	{
		m_ConsoleOnNewLineSlot =
			Console::getSingleton().OnNewLine.connect(this, &Logger::onConsoleNewLine);
		m_ConsoleLogging = true;

		Console::getSingletonPtr()->Add(L"Console Logging enabled");
	}

	void Logger::DisableConsoleLogging()
	{
		if (m_ConsoleLogging)
		{
			m_ConsoleLogging = false;
			m_ConsoleOnNewLineSlot.destroy();

			Console::getSingletonPtr()->Add(L"Console Logging disabled");
		}
	}

	void Logger::SetExt(const std::string &ext)
	{
		m_Ext = ext;
	}

	const std::string& Logger::GetExt() const
	{
		return m_Ext;
	}

	void Logger::SetUseDating(bool useDating)
	{
		m_UseDating = useDating;
	}

	void Logger::SetDefaultThreshold(LogSeverity threshold)
	{
		m_DefaultThreshold = threshold;
	}


	LogPtr Logger::OpenLog(const std::string& tag)
	{
		LogPtr log;

		try
		{
			log = openLog(tag);
			log->SetThreshold(m_DefaultThreshold);
		}
		catch (FileSystemException &e)
		{
			// We don't want to get stuck in a loop, so disable console logging
			DisableConsoleLogging();
			Console::getSingletonPtr()->PrintLn(e.ToString());
		}

		return log;
	}

	LogPtr Logger::OpenLog(const std::string& tag, LogSeverity threshold)
	{
		LogPtr log;

		try
		{
			log = openLog(tag);
			log->SetThreshold(threshold);
		}
		catch (FileSystemException &e)
		{
			// We don't want to get stuck in a loop, so disable console logging
			DisableConsoleLogging();
			Console::getSingletonPtr()->PrintLn(e.ToString());
		}

		return log;
	}



	void Logger::RemoveLog(const std::string& tag)
	{
		m_Logs.erase(tag);
	}


	void Logger::RemoveLog(LogPtr log)
	{
		LogList::iterator it = m_Logs.find(log->GetTag());
		if (it != m_Logs.end())
		{
			m_Logs.erase(it);
		}
	}

	LogPtr Logger::GetLog(const std::string& tag)
	{
		LogList::iterator it = m_Logs.find(tag);
		if (it != m_Logs.end())
			return it->second;
		else
			return LogPtr();
	}

	void Logger::Add(const std::string& message, const std::string& tag, LogSeverity severity)
	{
		try
		{
			openLog(tag)->AddEntry(message, severity);
		}
		catch (FileSystemException& e)
		{
			DisableConsoleLogging();
			SendToConsole(e);
		}
	}

	void Logger::Add(const Exception& error, const std::string& tag, LogSeverity severity)
	{
		std::wstring message = cl_format("Error: %1", error.ToString().c_str());

		Add(std::string(message.begin(), message.end()), tag, severity);
	}

	void Logger::onConsoleNewLine(const std::wstring &message)
	{
		if (m_ConsoleLogging)
			Add(fe_narrow(message), g_LogConsole);
	}


	LogPtr Logger::openLog(const std::string& tag)
	{
		LogList::iterator it = m_Logs.find(tag);
		if (it != m_Logs.end())
			return it->second;
		
		else
		{
			LogPtr log( new Log(tag, filename(tag)) );

			// Create a log file
			Log::LogFilePtr logFile( new PhysFSLogFile() );
			log->AttachLogFile(logFile);

			// Add this log to the list
			m_Logs[tag] = log;

			return log;
		}
	}

	std::string Logger::filename(const std::string& tag) const
	{
		std::stringstream filename;
		
		// Add the path to where logfiles are to be stored
		filename << LogfilePath;

		if (m_UseDating)
		{
			struct tm *pTime;
			time_t ctTime; time(&ctTime);
			pTime = localtime( &ctTime );

			// Build a dated filename in the format:
			//  <tag>-<year><month><day>.<m_Ext>
			filename << tag << "-"
				<< std::setw(2) << std::setfill('0') << pTime->tm_year +1900
				<< std::setw(2) << std::setfill('0') << pTime->tm_mon +1
				<< std::setw(2) << std::setfill('0') << pTime->tm_mday 
				<< "." << m_Ext;

			//filename.flush();

			// The unsafe way to do it
			//char temp[1024];
			//sprintf(temp,
			//	"%s-%04d%02d%02d.%s",
			//	tag, pTime->tm_year, pTime->tm_mon +1, pTime->tm_mday, m_Ext);
			//filename += temp;

			//filename += name.str();
		}
		else
		{
			filename << tag << "." << m_Ext;
		}

		filename.flush();
		return filename.str();
	}

}

#if _MSC_VER > 1000
# pragma warning(pop)
#endif
