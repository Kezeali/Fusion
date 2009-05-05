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

#include "FusionException.h"

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

	typedef FileSystemException LogfileException;

	class ILogFile
	{
	public:
		virtual void Open(const std::string& filename) =0;
		virtual void Close() =0;
		virtual void Write(const std::string& entry) =0;
		virtual void Flush() =0;
	};

	//! Represents a logfile
	/*!
	 * \sa FusionEngine#Logger
	 */
	class Log
	{
	public:
		typedef std::tr1::shared_ptr<ILogFile> LogFilePtr;
		typedef std::vector<LogFilePtr> LogFileList;

	public:
		//! Constructor +tag +filename +safe
		Log(const std::string& tag, const std::string& filename);
		//! Constructor +tag +filename +safe +verbosity
		Log(const std::string& tag, const std::string& filename, LogSeverity threshold);
		//! Destructor
		~Log();

	public:
		//! Returns the tag given to this log
		inline const std::string& GetTag() const { return m_Tag; }
		const std::string &GetFilename() const { return m_Filename; }

		//! Sets the verbosity level
		void SetThreshold(LogSeverity verbosity);
		//! Returns the verbosity level
		LogSeverity GetThreshold() const;

		void AttachLogFile(LogFilePtr log_file);
		void DetachLogFile(LogFilePtr log_file);

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

	protected:
		void addHeader();
		void addFooter();

	protected:
		std::string m_Tag;
		std::string m_Filename;
		LogSeverity m_Threshold;

		LogFileList m_LogFiles;
		
	};

}

#endif