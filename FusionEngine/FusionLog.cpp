/*
  Copyright (c) 2006-2009 Fusion Project Team

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

#include "FusionLog.h"

/// Fusion
#include "FusionException.h"

#include <time.h>

namespace FusionEngine
{

	Log::Log(const std::string& tag, const std::string& filename)
		: m_Filename(filename),
		m_Tag(tag),
		m_Threshold(LOG_TRIVIAL)
	{
		addHeader();
	}

	Log::~Log()
	{
		addFooter();
	}

	void Log::addHeader()
	{
		std::stringstream header;

		// Get the date/time
		time_t t = time(NULL);
		std::string tstr = asctime(localtime(&t));
		tstr[tstr.length() - 1] = 0;

		header << "-------------------- Log began on " << tstr << " --------------------";

		this->AddVerbatim(header.str());
	}

	void Log::addFooter()
	{
		std::stringstream header;

		// Get the date/time
		time_t t = time(NULL);
		std::string tstr = asctime(localtime(&t));
		tstr[tstr.length() - 1] = 0;

		header << "-------------------- Log ended on " << tstr << " --------------------";

		this->AddVerbatim(header.str());
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
		m_LogFiles.push_back(log_file);
	}

	void Log::DetachLogFile(LogFilePtr log_file)
	{
		for (LogFileList::iterator it = m_LogFiles.begin(), end = m_LogFiles.end(); it != end; ++it)
		{
			if ((*it) == log_file)
			{
				m_LogFiles.erase(it);
				break;
			}
		}
	}

	void Log::AddVerbatim(const std::string& text)
	{
		for (LogFileList::iterator it = m_LogFiles.begin(), end = m_LogFiles.end(); it != end; ++it)
		{
			(*it)->Write(text + "\n");
			(*it)->Flush();
		}
	}

	void Log::AddEntry(const std::string& message, LogSeverity severity)
	{
		if (severity >= m_Threshold)
		{
			std::stringstream tempStream;

			struct tm *pTime;
			time_t ctTime; time(&ctTime);
			pTime = localtime( &ctTime );

			tempStream
				<< "[" << std::setw(2) << std::setfill('0') << pTime->tm_hour
				<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_min
				<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_sec
				<< "]  " << message << std::endl;

			for (LogFileList::iterator it = m_LogFiles.begin(), end = m_LogFiles.end(); it != end; ++it)
			{
				(*it)->Write(tempStream.str());
				(*it)->Flush();
			}
		}
	}

	void Log::Flush()
	{
		for (LogFileList::iterator it = m_LogFiles.begin(), end = m_LogFiles.end(); it != end; ++it)
			(*it)->Flush();
	}

}