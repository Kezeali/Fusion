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

#include "FusionLog.h"

/// Fusion
#include "FusionError.h"

#include <time.h>

namespace FusionEngine
{

	Log::Log(const std::string& tag, const std::string& filename, bool safe)
		: m_Filename(filename),
		m_Safe(safe),
		m_Ended(true),
		m_Tag(tag),
		m_Verbosity(VBO_HIGH)
	{
		open();
	}

	Log::~Log()
	{
		Flush();
	}

	void Log::SetSafe(bool safe)
	{
		// Flush immeadiatly, to be extra safe ;)
		if (safe)
			Flush();

		m_Safe = safe;
	}

	bool Log::IsSafe() const
	{
		return m_Safe;
	}

	void Log::SetVerbosity(Log::LogVerbosity verbosity)
	{
		m_Verbosity = verbosity;
	}

	Log::LogVerbosity Log::GetVerbosity() const
	{
		return m_Verbosity;
	}

	void Log::LogVerbatim(const std::string& text)
	{
		verifyOpen();
		
		m_Logfile << text << std::endl;

		flushForSafety();
	}

	void Log::LogMessage(const std::string& message, LogSeverity severity)
	{
		verifyOpen();

		if ((m_Verbosity + severity) >= VBO_THRESHOLD)
		{
			struct tm *pTime;
			time_t ctTime; time(&ctTime);
			pTime = localtime( &ctTime );

			m_Logfile
				<< "[" << std::setw(2) << std::setfill('0') << pTime->tm_hour
				<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_min
				<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_sec 
				<< "]  " << message << std::endl;
		}

		flushForSafety();
	}

	void Log::Flush()
	{
		m_Logfile.flush();
	}

	void Log::open()
	{
		m_Logfile.open(m_Filename.c_str(), std::ios::out|std::ios::app);

		if (!m_Logfile.is_open())//m_Logfile.fail())
		{
			throw LogfileException(
				CL_String::format("The logfile '%1' could not be opened", m_Filename)
				);
		}
	}

	void Log::verifyOpen()
	{
		if (!m_Logfile.is_open())
		{
			throw LogfileException(
				CL_String::format("Logfile '%1' should have been open, but wasn't", m_Filename)
				);
		}
	}

	void Log::flushForSafety()
	{
		if (m_Safe)
			Flush();
	}

}