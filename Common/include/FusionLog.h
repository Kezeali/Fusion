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

#ifndef H_FusionEngine_Log
#define H_FusionEngine_Log

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionException.h"

//#define VBO_THRESHOLD 4

namespace FusionEngine
{

//! Level of severity for log messages
	enum LogSeverity : int
	{
		LOG_DEFAULT = -1,
		LOG_INFO = 0,
		LOG_TRIVIAL = 1,
		LOG_NORMAL = 2,
		LOG_CRITICAL = 3,
		LOG_MAX
	};

	typedef FileSystemException LogfileException;

	//! LogFile (log target) interface
	class ILogFile
	{
	public:
		//! Returns a string identifing the type
		/*!
		 * \remarks
		 * This is used to make sure Logs only have one LogFile of any given type.
		 */
		virtual std::string GetType() const =0;
	public:
		//! \todo This should be OnAttach() (Calling it 'open' is inaccurate)
		virtual void Open(const std::string& filename) =0;
		//! \todo This should be OnDetach() (calling it 'close' is inaccurate - the LogFile should close automatically on destruction)
		virtual void Close() =0;
		virtual void Write(const std::string& entry) =0;
		virtual void Flush() =0;
	};

	class LogTask;

	//! Represents a Log
	/*!
	* Call Logger#OpenLog() to create a new Log object (Log's constructor is protected.)
	*
	* \sa FusionEngine#Logger
	*/
	class Log
	{
		friend class Logger;
		friend class LogTask;
	public:
		typedef std::shared_ptr<ILogFile> LogFilePtr;
		typedef std::map<std::string, LogFilePtr> LogFileList;

	private:
		//! Constructor +tag +filename +safe
		Log(const std::string& tag, const std::string& filename);

	public:
		//! Destructor
		~Log();

		//! Returns the tag given to this log
		inline const std::string& GetTag() const { return m_Tag; }
		const std::string &GetFilename() const { return m_Filename; }

		//! Sets the verbosity level
		void SetThreshold(LogSeverity verbosity);
		//! Returns the verbosity level
		LogSeverity GetThreshold() const;

		//! Attaches a new target
		void AttachLogFile(LogFilePtr log_file);
		void DetachLogFile(LogFilePtr log_file);
		//! Detaches the target with the given type
		void DetachLogFile(const std::string& type);
		//! Returns true if a logfile of the given type is attached
		bool HasLogFileType(const std::string& type);

		//! Adds the given string to the log, as is
		void AddVerbatim(const std::string& text);

		//! Adds an entry to the log
		/*!
		 * \param message
		 * The message to add
		 *
		 * \param severity
		 * The message level (defines whether this is worthy of logging,
		 * and style of the actual entry in the file)
		 */
		void AddEntry(const std::string& message, LogSeverity severity);

		//! Force file write
		void Flush();

	private:
		void addHeader(LogFilePtr log_file);
		void addFooter(LogFilePtr log_file);

		//void addHeaderToAll();
		void addFooterToAll();

		std::string m_Tag;
		std::string m_Filename;
		LogSeverity m_Threshold;

		clan::Mutex m_LogFilesMutex;
		LogFileList m_LogFiles;
		
	};

}

#endif
