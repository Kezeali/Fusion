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

	Log::Log(const std::string& tag, const std::string& filename, bool keep_open)
		: m_Filename(filename),
		m_KeepOpen(keep_open),
		m_Tag(tag),
		m_Verbosity(VBO_HIGH)
	{
		open();
	}

	Log::~Log()
	{
		close();
	}

	void Log::SetKeepOpen(bool keepOpen)
	{
		// Since m_KeepOpen should still be false, Log::open() will open the file.
		//  If m_KeepOpen is true, the file doesn't need to be opened anyway, so
		//  nothing will happen here.
		//
		//  That is to say, this will:
		//  o Open m_Filename if m_KeepOpen is false (it shouldn't already be open)
		//  o Check whether m_Filename is open if m_KeepOpen (it should already be open)
		if (keepOpen)
			reOpen();

		else
			close();

		m_KeepOpen = keepOpen;
	}
			

	void Log::LogVerbatim(const std::string& text)
	{
		reOpen();
		
		m_Logfile << text << std::endl;

		reClose();
	}

	void Log::LogMessage(const std::string& message, LogSeverity severity)
	{
		reOpen();

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
			//m_Logfile << pTime->tm_hour
			//	<< ":"  << pTime->tm_min
			//	<< ":"  << pTime->tm_sec 
			//	<< ": " << message << std::endl;
		}

		reClose();
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

	void Log::reOpen()
	{
		// Try to open the file if it should be closed
		if (!m_KeepOpen)
		{
			open();
		}

		// ... otherwise the file should be open already
		else if (!m_Logfile.is_open())
		{
			throw LogfileException(
				CL_String::format("Logfile '%1' should have been open, but wasn't", m_Filename)
				);
		}
	}

	void Log::close()
	{
		m_Logfile.flush();
		m_Logfile.close();
	}

	void Log::reClose()
	{
		if (!m_KeepOpen)
		{
			close();
		}
	}

}