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

#include "FusionLog.h"

#include <iomanip>
//#include <time.h>
#include <boost/date_time.hpp>

#include "FusionException.h"

namespace FusionEngine
{

	Log::Log(const std::string& tag, const std::string& filename)
		: m_Filename(filename),
		m_Tag(tag),
		m_Threshold(LOG_TRIVIAL)
	{
	}

	Log::~Log()
	{
		addFooterToAll();
	}

	//void Log::addHeaderToAll()
	//{
	//	for (LogFileList::iterator it = m_LogFiles.begin(), end = m_LogFiles.end(); it != end; ++it)
	//	{
	//		addHeader(it->second);
	//	}
	//}

	void Log::addFooterToAll()
	{
		clan::MutexSection lock(&m_LogFilesMutex);
		for (auto it = m_LogFiles.begin(), end = m_LogFiles.end(); it != end; ++it)
		{
			addFooter(it->second);
		}
	}

	void Log::addHeader(LogFilePtr log_file)
	{
		std::stringstream header;

		// Get the date/time
		auto now = boost::posix_time::second_clock::local_time();
		std::string tstr = boost::posix_time::to_simple_string(now);

		header << "------------------ Log began on " << tstr << " ------------------" << std::endl;

		log_file->Write(header.str());
	}

	void Log::addFooter(LogFilePtr log_file)
	{
		std::stringstream header;

		// Get the date/time
		auto now = boost::posix_time::second_clock::local_time();
		std::string tstr = boost::posix_time::to_simple_string(now);

		header << "------------------ Log ended on " << tstr << " ------------------" << std::endl;

		log_file->Write(header.str());
		log_file->Flush();
	}

	void Log::SetThreshold(LogSeverity threshold)
	{
		m_Threshold = threshold;
	}

	LogSeverity Log::GetThreshold() const
	{
		return m_Threshold;
	}

	void Log::AttachLogFile(LogFilePtr log_file)
	{
		// Open the file
		log_file->Open(m_Filename);
		addHeader(log_file);

		{
			clan::MutexSection lock(&m_LogFilesMutex);
			m_LogFiles[log_file->GetType()] = log_file;
		}
	}

	void Log::DetachLogFile(LogFilePtr log_file)
	{
		DetachLogFile(log_file->GetType());
	}

	void Log::DetachLogFile(const std::string& type)
	{
		clan::MutexSection lock(&m_LogFilesMutex);
		LogFileList::iterator _where = m_LogFiles.find(type);
		if (_where != m_LogFiles.end())
		{
			auto logFile = _where->second;

			m_LogFiles.erase(_where);

			addFooter(logFile);

			logFile->Close();
		}
	}

	bool Log::HasLogFileType(const std::string &type)
	{
		clan::MutexSection lock(&m_LogFilesMutex);
		LogFileList::iterator _where = m_LogFiles.find(type);
		return _where != m_LogFiles.end();
	}

	void Log::AddVerbatim(const std::string& text)
	{
		clan::MutexSection lock(&m_LogFilesMutex);
		for (LogFileList::iterator it = m_LogFiles.begin(), end = m_LogFiles.end(); it != end; ++it)
		{
			it->second->Write(text);
			it->second->Flush();
		}
	}

	void Log::AddEntry(const std::string& message, LogSeverity severity)
	{
		if (severity >= m_Threshold)
		{
			std::stringstream tempStream;

			auto now = boost::posix_time::second_clock::local_time();
			auto pTime = boost::posix_time::to_tm(now);

			tempStream
				<< "[" << std::setw(2) << std::setfill('0') << pTime.tm_hour
				<< ":" << std::setw(2) << std::setfill('0') << pTime.tm_min
				<< ":" << std::setw(2) << std::setfill('0') << pTime.tm_sec
				<< "]  " << message;

			// Add a new-line at the end if neccessary, otherwise just flush
			if (message.empty() || message[message.length()-1] != '\n')
				tempStream << std::endl;

			const std::string entry = tempStream.str();

			clan::MutexSection lock(&m_LogFilesMutex);
			for (LogFileList::iterator it = m_LogFiles.begin(), end = m_LogFiles.end(); it != end; ++it)
			{
				it->second->Write(entry);
				it->second->Flush();
			}
		}
	}

	void Log::Flush()
	{
		clan::MutexSection lock(&m_LogFilesMutex);
		for (LogFileList::iterator it = m_LogFiles.begin(), end = m_LogFiles.end(); it != end; ++it)
			it->second->Flush();
	}

}
