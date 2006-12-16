/*
  Copyright (c) 2006 FusionTeam

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
*/

#ifndef Header_FusionEngine_Logger
#define Header_FusionEngine_Logger

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionError.h"

namespace FusionEngine
{

	//! Provides logfile access to all FusionEngine objects
	/*!
	 * This manages and
	 */
	class Logger : public Singleton<Console>
	{
	public:
		//! Basic constructor
		Logger();

		//! Constructor +console_logging
		Logger(bool console_logging);

	public:
		//! Console logging prints all console messages to a log.
		/*!
		 * This will connect to the Console#OnNewLine signal to
		 * capture messages.
		 */
		void ActivateConsoleLogging();

		//! Adds the given message to the given log
		/*!
		 * The given tag will be appended with a date and .log extension, unless
		 * otherwise specified, to create the log-file name. If the resulting
		 * filename does not exist, it will be created.
		 */
		void Add(const std::string &message, const std::string &tag);

		//! Adds the given Error to the given log
		/*!
		 * The given tag will be appended with a date and .log extension, unless
		 * otherwise specified, to create the log-file name. If the resulting
		 * filename does not exist, it will be created.
		 */
		void Add(const Error *error, const std::string &tag);

		//! Called by the OnNewLine signal from the console
		void onConsoleNewline(const std::string &message);

	protected:
		//! True if console logging is active
		bool m_ConsoleLogging;

	};

}

#endif
