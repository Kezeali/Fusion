/*
*  Copyright (c) 2006-2011 Fusion Project Team
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

#include "FusionLogger.h"

#include <iomanip>
#include <time.h>
#include <ClanLib/core.h>

#include "FusionConsole.h"
#include "FusionException.h"
#include "FusionPaths.h"
// Log targets (ILogFile implementations)
#include "FusionLogPhysFS.h"
#include "FusionLogFileConsole.h"

using namespace std::placeholders;

namespace FusionEngine
{

	Logger::Logger()
		: m_ConsoleLogging(false),
		m_UseDating(true),
		m_Ext(g_LogDefaultExt),
		m_DefaultThreshold(LOG_TRIVIAL),
		m_DefaultTarget_File(true),
		m_DefaultTarget_Console(false)
	{
	}

	Logger::~Logger()
	{
		CL_MutexSection lock(&m_LogsMutex);
		// Make sure we are disconnected from the console
		DisableConsoleLogging();
		// Remove all logs
		m_Logs.clear();
	}

	void Logger::ActivateConsoleLogging()
	{
		m_ConsoleNewLine =
			Console::getSingleton().OnNewLine.connect(std::bind(&Logger::onConsoleNewLine, this, _1));
		m_ConsoleLogging = true;

		SendToConsole("Console Logging enabled");
	}

	void Logger::DisableConsoleLogging()
	{
		if (m_ConsoleLogging)
		{
			SendToConsole("Console Logging disabled");

			m_ConsoleLogging = false;
			m_ConsoleNewLine.disconnect();
		}
	}

	void Logger::SetLogingToConsole(const std::string& tag, bool activate)
	{
		if (tag == g_LogConsole) // This would cause a feedback loop
			return;

		LogList::iterator _where = m_Logs.find(tag);
		if (_where != m_Logs.end())
		{
			if (activate)
			{
				Console* currentCnsl = Console::getSingletonPtr();
				if (currentCnsl == NULL)
					return; // Can't add a console-logfile if the console doesn't exist :P

				Log::LogFilePtr logFile(new ConsoleLogFile(currentCnsl));
				_where->second->AttachLogFile(logFile);
			}
			else
			{
				_where->second->DetachLogFile("console");
			}
		}
	}

	void Logger::SetTargets(const std::string& tag, bool file, bool console)
	{
		if (tag == g_LogConsole) 
			return;

		LogList::iterator _where = m_Logs.find(tag);
		if (_where != m_Logs.end())
		{
			if (file)
			{
				Log::LogFilePtr logFile(new PhysFSLogFile());
				_where->second->AttachLogFile(logFile);
			}
			else if (!file)
			{
				_where->second->DetachLogFile("physfs");
			}

			// tag must not be g_LogConsole: Can't make the console a target
			//  for the log that receives mesages from the console (that
			//  would create a feedback loop)
			if (console && tag != g_LogConsole)
			{
				Console* currentCnsl = Console::getSingletonPtr();
				if (currentCnsl == NULL)
					return; // Can't add a console-logfile if the console doesn't exist :P

				Log::LogFilePtr logFile(new ConsoleLogFile(currentCnsl));
				_where->second->AttachLogFile(logFile);
			}
			else if (!console)
			{
				_where->second->DetachLogFile("console");
			}
		}
	}

	bool Logger::IsLoggingToConsole(const std::string &tag)
	{
		LogList::iterator _where = m_Logs.find(tag);
		if (_where != m_Logs.end())
			return _where->second->HasLogFileType("console");
		else
			return false;
	}

	bool Logger::IsLoggingToFile(const std::string &tag)
	{
		LogList::iterator _where = m_Logs.find(tag);
		if (_where != m_Logs.end())
			return _where->second->HasLogFileType("physfs");
		else
			return false;
	}

	void Logger::SetDefaultTargets(bool file, bool console)
	{
		m_DefaultTarget_File = file;
		m_DefaultTarget_Console = console;
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
			log->SetThreshold(threshold != LOG_DEFAULT ? threshold : GetDefaultThreshold());
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
		CL_MutexSection lock(&m_LogsMutex);
		m_Logs.erase(tag);
	}


	void Logger::RemoveLog(LogPtr log)
	{
		CL_MutexSection lock(&m_LogsMutex);
		m_Logs.erase(log->GetTag());
	}

	LogPtr Logger::GetLog(const std::string& tag, CreationMode creation_mode, LogSeverity threshold_if_new)
	{
		CL_MutexSection lock(&m_LogsMutex);
		LogList::iterator it = m_Logs.find(tag);
		if (creation_mode != ReplaceIfExist)
		{
			if (it != m_Logs.end())
				return it->second;
			else if (creation_mode == CreateIfNotExist)
				return OpenLog(tag, threshold_if_new == LOG_DEFAULT ? GetDefaultThreshold() : threshold_if_new);
			else
				return LogPtr();
		}
		else
			return OpenLog(tag, threshold_if_new);
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
		CL_String message = cl_format("Error: %1", error.ToString().c_str());

		Add(CL_StringHelp::text_to_local8(message), tag, severity);
	}

	void Logger::onConsoleNewLine(const std::string &message)
	{
		if (m_ConsoleLogging)
			Add(message, g_LogConsole);
	}

	LogPtr Logger::openLog(const std::string& tag)
	{
		CL_MutexSection lock(&m_LogsMutex);

		LogList::iterator it = m_Logs.find(tag);
		if (it != m_Logs.end())
			return it->second;
		
		else
		{
			LogPtr log( new Log(tag, filename(tag)) );
			log->SetThreshold(m_DefaultThreshold);

			if (m_DefaultTarget_File)
			{
				// Create a log file
				Log::LogFilePtr logFile( new PhysFSLogFile() );
				log->AttachLogFile(logFile);
			}
			if (m_DefaultTarget_Console)
			{
				// Create a console target
				Console* currentCnsl = Console::getSingletonPtr();
				// Can't add a console-logfile if the console doesn't exist :P
				if (currentCnsl == NULL)
				{
					Log::LogFilePtr logFile( new ConsoleLogFile(currentCnsl) );
					log->AttachLogFile(logFile);
				}
			}

			// Add this log to the list
			m_Logs[tag] = log;

			return log;
		}
	}

	std::string Logger::filename(const std::string& tag) const
	{
		std::stringstream filename;
		
		// Add the path to where logfiles are to be stored
		filename << s_LogfilePath;

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
