/*
  Copyright (c) 2006 Fusion Project Team

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

#ifndef Header_FusionEngine_Exception
#define Header_FusionEngine_Exception

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	//! Fusion exception class.
	/*!
	 * Generic exception class
	 * <br>
	 * On loading exception strings form XML:<br>
	 * An exception constructor would look as follows
	 * NOTE: this doesn't work, as Exception (the superclass
	 * constructor) must be called in the initialisation list
	 * so we'll need to fiture out another way
	 * <code>
	 * MyException(std::string variable1, std::string variable2)
	 * {
	 *	std::string format = Theme::getSingleton().getResource<std::string>("exceptions/myexception/format");
	 *	std::string message = CL_String::format(format, variable1, variable2);
	 *	Exception(Exception::TRIVIAL, message);
	 * }
	 * </code>
	 */
	class Exception
	{
	public:
		//! Types of Exceptions
		enum ExceptionType
		{
			//! Trivial
			TRIVIAL = 0,
			//! A generic exception
			INTERNAL_ERROR,
			//! Disconnected for some (unusual) reason
			NETWORK,
			//! You can't connect to this server
			BANNED,
			//! A package couldn't be sync'ed
			PACKSYNC,
			//! A package is damaged
			PACKVERIFY,
			//! A file couldn't be found
			FILE_NOT_FOUND,
			//! Logger error (obviously these shouldn't be logged!)
			LOGFILE_ERROR,
			//! Generic error within a loading stage or the loader itself
			LOADING,
			//! The method being called is not implemented
			NOT_IMPLEMENTED
		};

	public:
		//! Basic constructor
		Exception();
		//! Constructor +type +message
		/*!
		 * There is no constructor sans-'type'... Which is to say 
		 * 'type' must be specified here; if you are going to use the
		 * Generic-Exception class (slacker), you can at least specify
		 * a type!
		 */
		Exception(ExceptionType type, const std::string &message, bool critical = false);

	public:
		//! Retrieves the type
		ExceptionType GetType() const;
		//! Retrieves the human readable message.
		/*!
		 * \remarks
		 * MCS - I can't call this GetMessage because of a stupid windows macro 
		 * which redefines that as GetMessageW!
		 */
		const std::string& GetError() const;
		//! Returns true if the error type of this Exception object is severe
		bool IsCritical() const;

	protected:
		//! Stores the error type
		ExceptionType m_Type;
		//! Stores the message
		std::string m_Message;
		bool m_Critical;

	};

}

#endif
