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
#ifndef Header_FusionEngine_Log
#define Header_FusionEngine_Log

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionError.h"

//#define VBO_THRESHOLD 4

namespace FusionEngine
{

	//! Level of severity for log messages
	enum LogSeverity
	{
		LOG_TRIVIAL = 1,
		LOG_NORMAL = 2,
		LOG_CRITICAL = 3,
		LOG_MAX
	};

	//! Exception type for FusionEngine#Log and FusionEngine#Logger
	class LogfileException : public Error
	{
	public:
		LogfileException(const std::string& message)
			: Error(Error::LOGFILE_ERROR, message)
		{}

	};

	//! Represents a logfile (usually within FusionEngine#Logger)
	/*!
	 * \remarks
	 * !(keep open) desn't seem to work. For now, everything in
	 * FusionEngine#Logger defaults to (keep open) to subdue this bug.
	 */
	class Log
	{
	public:
		//! Determines how sever log messages must be before they are actually logged
		enum LogVerbosity
		{
			//! = LOG_<minimum>
			VBO_LOW = 1,
			//! = LOG_MAX / 2
			VBO_MEDIUM = 2,
			//! = LOG_MAX - 1
			VBO_HIGH = 3,
			//! = LOG_MAX
			VBO_THRESHOLD = 4
		};

	public:
		//! Constructor +tag +filename +keep_open
		Log(const std::string& tag, const std::string& filename, bool keep_open);
		//! Destructor
		~Log();

	public:
		//! Returns the tag given to this log
		inline const std::string& GetTag() const { return m_Tag; }

		//! Activates / deactivates keep open
		/*!
		 * Depending on the setting, the this method will automatically open
		 * or close the file (as well as setting the value of m_KeepOpen)
		 */
		void SetKeepOpen(bool keepOpen);
		//! Returns the value of m_KeepOpen
		inline bool GetKeepOpen() const { return m_KeepOpen; }

		//! Adds the given string to the logfile, as is
		void LogVerbatim(const std::string& text);

		//! Adds a message to the logfile
		/*!
		 * \param message
		 * The string to add (verbatim)
		 *
		 * \param severity
		 * The message level (defines whether this is worthy of logging,
		 * and style of the actual entry in the file)
		 */
		void LogMessage(const std::string& message, LogSeverity severity);

		//! Force file write
		/*!
		 * Useful for logs set to 'keep open' only.
		 */
		void Flush();

	protected:
		bool m_KeepOpen;
		std::string m_Tag;
		std::string m_Filename;
		std::ofstream m_Logfile;
		LogVerbosity m_Verbosity;

	protected:
		//! Just opens the file
		void open();
		//! Opens the file if and only if m_KeepOpen is false
		void reOpen();

		//! Closes the file
		void close();
		//! Closes the file if and only if m_KeepOpen is false
		void reClose();
	};

}

#endif