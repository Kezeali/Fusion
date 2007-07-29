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
#include "FusionError.h"
#include "FusionConsole.h"
#include "FusionPaths.h"

#include <time.h>

namespace FusionEngine
{

	Logger::Logger()
		: m_ConsoleLogging(false),
		m_UseDating(true),
		m_Ext(g_LogDefaultExt)
	{
	}

	Logger::Logger(bool console_logging)
		: m_ConsoleLogging(console_logging),
		m_UseDating(true),
		m_Ext(g_LogDefaultExt)
	{
		if (m_ConsoleLogging)
			ActivateConsoleLogging();
	}

	Logger::~Logger()
	{
		// Destroy all logs
		LogList::iterator i;
		for (i = m_Logs.begin(); i != m_Logs.end(); ++i)
		{
			if (!i->second->IsEnded()) {
				EndLog(i->second);
			}
			delete i->second;
		}
		// Make sure we are disconnected from the console
		DisableConsoleLogging();
	}

	void Logger::ActivateConsoleLogging()
	{
		m_ConsoleOnNewLineSlot =
			Console::getSingleton().OnNewLine.connect(this, &Logger::onConsoleNewLine);
		m_ConsoleLogging = true;

		Console::getSingletonPtr()->Add("Console Logging enabled");
	}

	void Logger::DisableConsoleLogging()
	{
		if (m_ConsoleLogging)
		{
			m_ConsoleLogging = false;
			Console::getSingleton().OnNewLine.disconnect(m_ConsoleOnNewLineSlot);

			Console::getSingletonPtr()->Add("Console Logging disabled");
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

	// [inlined]
	//void Logger::GetUseDating()
	//{
	//	return m_Dating;
	//}

	void Logger::TagLink(const std::string& tag, const std::string& alias)
	{
		m_Logs.insert( LogList::value_type(alias, GetLog(tag)) );
	}

	Log* Logger::BeginLog(const std::string& tag, bool safe)
	{
		Log* log = NULL;

		try
		{
			log = openHeadlessLog(tag, safe);

			std::stringstream header;

			// Get the date/time
			time_t t = time(NULL);
			std::string tstr = asctime(localtime(&t));
			tstr[tstr.length() - 1] = 0;

			header << "-------------------- Log began on " << tstr << " --------------------";

			log->LogVerbatim(header.str());

			log->_setIsEnded(false);
		}
		catch (LogfileException e)
		{
			// We don't want to get stuck in a loop, so disable console logging
			DisableConsoleLogging();
			Console::getSingletonPtr()->Add(e.GetError());
		}

		return log;
	}

	void Logger::BeginLog(Log* log)
	{
		try
		{
			std::stringstream header;

			// Get the date/time
			time_t t = time(NULL);
			std::string tstr = asctime(localtime(&t));
			tstr[tstr.length() - 1] = 0;

			header << "-------------------- Log began on " << tstr << " --------------------";

			log->LogVerbatim(header.str());

			log->_setIsEnded(false);
		}
		catch (LogfileException e)
		{
			// We don't want to get stuck in a loop, so disable console logging
			DisableConsoleLogging();
			Console::getSingletonPtr()->Add(e.GetError());
		}
	}

	void Logger::EndLog(FusionEngine::Log* log)
	{
		try
		{
			std::stringstream header;

			// Get the date/time
			time_t t = time(NULL);
			std::string tstr = asctime(localtime(&t));
			tstr[tstr.length() - 1] = 0;

			header << "-------------------- Log ended on " << tstr << " --------------------";

			log->LogVerbatim(header.str());

			log->_setIsEnded(true);
		}
		catch (LogfileException e)
		{
			// We don't want to get stuck in a loop, so we disable console logging first
			DisableConsoleLogging();
			SendToConsole(&e);
		}
	}

	void Logger::EndLog(const std::string& tag)
	{
		Log* log = GetLog(tag);

		if (log == NULL) return;

		EndLog(log);
	}

	void Logger::RemoveAndDestroyLog(const std::string& tag)
	{
		LogList::iterator it = m_Logs.find(tag);
		if (it != m_Logs.end())
		{
			Log* log = it->second;
			m_Logs.erase(it);
			delete log;
		}
	}


	void Logger::RemoveAndDestroyLog(Log *log)
	{
		LogList::iterator it = m_Logs.find(log->GetTag());
		if (it != m_Logs.end())
		{
			m_Logs.erase(it);
		}
		delete log;
	}

	Log* Logger::GetLog(const std::string& tag)
	{
		LogList::iterator it = m_Logs.find(tag);
		if (it != m_Logs.end())
			return it->second;
		else
			return NULL;
	}

	void Logger::Add(const std::string& message, const std::string& tag, LogSeverity severity)
	{
		try
		{
			Log* log = openLog(tag);
			log->LogMessage(message, severity);
		}
		catch (LogfileException e)
		{
			DisableConsoleLogging();
			SendToConsole(&e);
		}
	}

	void Logger::Add(const Error* error, const std::string& tag, LogSeverity severity)
	{
		std::string message =
			CL_String::format("Error [Type %1]: %2", error->GetType(), error->GetError());

		Add(message, tag, severity);
	}

	void Logger::onConsoleNewLine(const std::string &message)
	{
		Add(message, g_LogConsole);
	}


	Log* Logger::openLog(const std::string& tag, bool safe)
	{
		LogList::iterator it = m_Logs.find(tag);
		if (it != m_Logs.end())
			return it->second;
		
		else
		{
			Log* log = new Log(tag, filename(tag), safe);
			// Add this log to the list
			m_Logs.insert( LogList::value_type(tag, log) );

			BeginLog(log);

			return log;
		}
	}

	Log* Logger::openHeadlessLog(const std::string& tag, bool keepopen)
	{
		LogList::iterator it = m_Logs.find(tag);
		if (it != m_Logs.end())
			return it->second;

		else
		{
			Log* log = new Log(tag, filename(tag), keepopen);
			// Add this log to the list
			m_Logs.insert( LogList::value_type(tag, log) );

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