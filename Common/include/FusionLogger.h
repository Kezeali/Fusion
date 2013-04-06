/*
*  Copyright (c) 2006-2013 Fusion Project Team
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

#ifndef H_FusionEngine_Logger
#define H_FusionEngine_Logger

#if _MSC_VER > 1000
# pragma once
#endif

#include "FusionPrerequisites.h"

#include <boost/signals2.hpp>

#include "FusionSingleton.h"

#include "FusionException.h"
#include "FusionLog.h"

#include <tbb/concurrent_hash_map.h>

namespace FusionEngine
{

	//! Data entered into the console
	const std::string g_LogConsole = "console";
	//! The main log file
	const std::string g_LogGeneral = "general";

	//! Default extension for logfiles (excluding the dot).
	static const std::string g_LogDefaultExt = "log";


	//! Provides logfile access
	/*!
	* Manages logfiles.
	*
	* \remarks
	* Many of the methods in the class are basically for convenience, since
	* Log objects can be interacted with directly. OpenLog is, probably,
	* the only Logger method you *need* to use.
	*/
	class Logger : public Singleton<Logger>
	{
	public:
		//! Constructor
		Logger();

		//! Destructor
		~Logger();

		//! Console logging prints all console messages to a log.
		/*!
		* This writes all messages added to the console to a log, as
		* opposed to SetLogingToConsole() which makes a log push
		* all messages written to it onto the console.
		*
		* This will connect to the Console#OnNewLine signal to
		* capture messages.
		*/
		void ActivateConsoleLogging();
		//! Disconnects from the console
		void DisableConsoleLogging();

		//! Enables or disables the Console target on the given log
		void SetLogingToConsole(const std::string& tag, bool activate = true);
		//! Enables or disables the given targets on the given log
		void SetTargets(const std::string& tag, bool file, bool console);
		//! Returns true if the given Log has a Console target
		bool IsLoggingToConsole(const std::string& tag);
		//! Returns true if the given Log has a file target
		bool IsLoggingToFile(const std::string& tag);

		//! Sets the targets that will be attached to new Logs
		void SetDefaultTargets(bool file, bool console);

		//! Sets the extension given to logfiles
		void SetExt(const std::string& ext);
		//! Gets the extension currently given to logfiles
		const std::string& GetExt() const;

		//! Activates or disables the addition of dates to log file names
		void SetUseDating(bool useDating);

		//! Returns true if dating is active
		inline bool GetUseDating() { return m_UseDating; }

		//! Sets the default threshold for new logs
		void SetDefaultThreshold(LogSeverity threshold);
		//! Gets the default threshold
		inline LogSeverity GetDefaultThreshold() const { return m_DefaultThreshold; }

		//! Opens or creates a logfile
		LogPtr OpenLog(const std::string& tag);

		//! Opens or creates the logfile corresponding to the given tag
		/*!
		* Opens/creates the specified file and adds the opening line (date, etc.). If
		* the file already exists, the opening line will be appended to the end.
		*
		* \param[in] tag
		* Tag to open
		*/
		LogPtr OpenLog(const std::string& tag, LogSeverity threshold);

		enum CreationMode { ReturnNullIfNotExist, CreateIfNotExist, ReplaceIfExist };

		//! Gets the log corresponding to the given tag.
		/*!
		* \param[in] tag
		* Tag to find
		*
		* \returns Log*
		* If the tag exists
		*/
		LogPtr GetLog(const std::string& tag, CreationMode create = CreateIfNotExist, LogSeverity threshold_if_new = LOG_DEFAULT);

		//! Removes a log from the list
		/*!
		* \param log
		* The log to close
		*/
		void RemoveLog(const std::string& tag);

		//! Removes a log from the list
		/*!
		* \param log
		* The log to close
		*/
		void RemoveLog(LogPtr log);

		//! Adds the given message to the given log
		/*!
		* The given tag will be appended with a date and .log extension, unless
		* otherwise specified, to create the log-file name. If the resulting
		* filename does not exist, it will be created.
		*/
		void Add(const std::string &message, const std::string &tag = g_LogGeneral, LogSeverity severity = LOG_NORMAL);

		//! Adds the given error to the given log
		/*!
		* \deprecated
		* Formats the given error to a string, then calls the normal Add()
		*/
		void Add(const Exception& error, const std::string &tag = g_LogGeneral, LogSeverity severity = LOG_CRITICAL);

		//! Called by the OnNewLine signal from the console
		void onConsoleNewLine(const std::string &message);

	protected:
		//! True if console logging is active
		bool m_ConsoleLogging;
		//! True if dating is active
		bool m_UseDating;
		std::string m_Ext;
		LogSeverity m_DefaultThreshold;
		bool m_DefaultTarget_File;
		bool m_DefaultTarget_Console;

		//! Maps tags to LogFiles
		typedef tbb::concurrent_hash_map<std::string, LogPtr> LogList;

		LogList m_Logs;

		boost::signals2::connection m_ConsoleNewLine;

		//! Opens a logfile (creates it if it doesn't exist)
		/*!
		* Always returns a FusionEngine#Log. Throws an exception otherwise.
		*
		* \param tag
		* The tag to look for and create a file for if necessary
		*
		* \param threshold
		* The threshold setting for the log
		*/
		LogPtr openLog(const std::string& tag);


		//! Makes a filename for the given tag
		/*!
		* If dating is active, the filename format will be: <br>
		* <code> [tag]-[year][month][day].[m_Ext] </code>
		*/
		std::string filename(const std::string& tag) const;

	};

	//! Add a log entry
	void AddLogEntry(const std::string& file_tag, const std::string& entry, LogSeverity severity = LOG_NORMAL);

	//! Add a log entry to the default log file
	void AddLogEntry(const std::string& entry, LogSeverity severity);

	//! Add a log entry to the default log file
	void AddLogEntry(const std::string& entry);

	//! An object that allows the use of the stream operator to create a log entry
	class MakeLog
	{
	public:
		MakeLog(const std::string& tag, LogSeverity severity = LOG_NORMAL)
			: severity(severity)
		{
			if (auto logger = Logger::getSingletonPtr())
			{
				log = logger->GetLog(tag);
			}
		}

		~MakeLog()
		{
			if (log)
				log->AddEntry(stream.str(), severity);
		}

		//! Stream input operator
		template<class T>
		MakeLog& operator<<(T const& other) { stream << other; return *this; }

		MakeLog(MakeLog&& other)
			: log(std::move(other.log)),
			severity(other.severity)
		{
		}

	private:
		LogPtr log;
		LogSeverity severity;

		std::stringstream stream;

		//! noncopyable
		MakeLog(const MakeLog& other) {}
	};

	//! Make a MakeLog object
	MakeLog Log(const std::string& file_tag, LogSeverity severity);

	//! Make a MakeLog object with the default severity
	MakeLog Log(const std::string& file_tag);

	//! Make a MakeLog object for the default log file
	MakeLog Log(LogSeverity severity);

	//! Make a MakeLog object with the default severity for the default log file
	MakeLog Log();

}

#endif
