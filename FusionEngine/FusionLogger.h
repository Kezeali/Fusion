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

#ifndef Header_FusionEngine_Logger
#define Header_FusionEngine_Logger

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

#include "FusionLog.h"

namespace FusionEngine
{

	//! Data entered into the console
	const std::string g_LogConsole = "console";
	//! Exceptions not specific to client or server
	const std::string g_LogException = "error";
	//! Exceptions in client
	const std::string g_LogExceptionClient = g_LogException + "_client";
	//! Exceptions in server
	const std::string g_LogExceptionServer = g_LogException + "_server";

	//! Default extension for logfiles (excluding the dot).
	static const std::string g_LogDefaultExt = "log";

	//! Provides logfile access to all FusionEngine objects
	/*!
	 * Manages logfiles.
	 *
	 * \todo Allow mapping of log tags to other tags - i.e. if someone
	 *  calls Logger::Add("Arrrrg", "mylogfile"); and one previously called
	 *  Logger::MapTag("mylogfile", "betterlogfilename"); then "Arrrrg" would
	 *  be added to the file "betterlogfilename-<date>.<logext>"
	 */
	class Logger : public Singleton<Logger>
	{
	public:
		//! Maps tags to LogFiles
		typedef std::map<std::string, Log*> LogList;

	public:
		//! Constructor
		Logger();
		//! Constructor +console_logging
		Logger(bool console_logging);

		//! Destructor
		~Logger();

	public:
		//! Console logging prints all console messages to a log.
		/*!
		 * This will connect to the Console#OnNewLine signal to
		 * capture messages.
		 */
		void ActivateConsoleLogging();
		//! Disconnects from the console
		void DisableConsoleLogging();

		//! Sets the extension given to logfiles
		void SetExt(const std::string& ext);
		//! Gets the extension currently given to logfiles
		const std::string& GetExt() const;

		//! Activates or disables the addition of dates to log file names
		void SetUseDating(bool useDating);

		//! Returns true if dating is active
		inline bool GetUseDating() { return m_UseDating; }

		//! Maps the given alias tag to an existing tag
		/*!
		 * \param[in] alias
		 * The new tag which will redirect to the other given tag
		 *
		 * \param[in] tag
		 * The existing tag to which 'alias' will redirect
		 */
		void TagLink(const std::string& tag, const std::string& alias);

		//! Opens or creates the logfile corresponding to the given tag
		/*!
		 * Opens/creates the specified file and adds the opening line (date, etc.). If
		 * the file already exists, the opening line will be appended to the end.
		 *
		 * \param[in] tag
		 * Tag to open
		 *
		 * \param[in] safe
		 * If true, the file will be flushed after every write.
		 */
		Log* BeginLog(const std::string& tag, bool safe = true);

		//! Adds a header to an already open log
		/*!
		 * Opens the specified file and adds the opening line (date, etc.). If
		 * the file already exists, the opening line will be appended to the end.
		 *
		 * \param[in] log
		 * Existing logfile to open
		 */
		void BeginLog(Log *log);

		//! Gets the log corresponding to the given tag.
		/*!
		 * \todo Perhaps this should throw an INVALID_PARAMETERS exception
		 *  if the tag isn't found.
		 *
		 * \param[in] tag
		 * Tag to find
		 *
		 * \returns Log*
		 * If the tag exists
		 */
		Log* GetLog(const std::string& tag);

		//! Prints the footer to the given log
		void EndLog(Log *log);

		//! Finds the given log and calls Log#EndLog(Log*)
		void EndLog(const std::string& tag);

		//! Closes a logfile and removes it from the list
		/*!
		 * \param log
		 * The logfile to close
		 */
		void RemoveAndDestroyLog(const std::string& tag);

		//! Closes a logfile and removes it from the list
		/*!
		 * \param log
		 * The logfile to close
		 */
		void RemoveAndDestroyLog(Log* log);

		//! Adds the given message to the given log
		/*!
		 * The given tag will be appended with a date and .log extension, unless
		 * otherwise specified, to create the log-file name. If the resulting
		 * filename does not exist, it will be created.
		 */
		void Add(const std::string &message, const std::string &tag = g_LogException, LogSeverity severity = LOG_NORMAL);

		//! Adds the given Error to the given log
		/*!
		 * Formats the given error to a string, then calls the normal Add()
		 */
		void Add(const Exception* error, const std::string &tag = g_LogException, LogSeverity severity = LOG_CRITICAL);

		//! Called by the OnNewLine signal from the console
		void onConsoleNewLine(const std::string &message);

	protected:
		//! True if console logging is active
		bool m_ConsoleLogging;
		//! True if dating is active
		bool m_UseDating;
		std::string m_Ext;

		LogList m_Logs;

		CL_Slot m_ConsoleOnNewLineSlot;

	protected:
		//! Opens a logfile (creates it if it doesn't exist)
		/*!
		 * Always returns a FusionEngine#Log. Throws an exception otherwise.
		 * <br>
		 * If the log has to be created, a header will be added by calling Log#BeginLog()
		 *
		 * \param tag
		 * The tag to look for and create a file for if necessary
		 *
		 * \param safe
		 * If the log is must be created, this will be its 'safe' setting
		 */
		Log* openLog(const std::string& tag, bool safe = true);

		//! Opens the given log. Will not add header.
		/*!
		 * Even if the log has to be created, no header will be added. i.e. this method
		 * will never call Log#BeginLog()
		 *
		 * \param tag
		 * The tag to look for and create a file for if necessary
		 *
		 * \param safe
		 * If the log is must be created, this will be its 'safe' setting
		 */
		Log* openHeadlessLog(const std::string& tag, bool safe = true);


		//! Makes a filename for the given tag
		/*!
		 * If dating is active, the filename format will be: <br>
		 * <code> [tag]-[year][month][day].[m_Ext] </code>
		 */
		std::string filename(const std::string& tag) const;

	};

}

#endif
