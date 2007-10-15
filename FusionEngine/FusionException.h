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
*/

#ifndef Header_FusionEngine_Exception
#define Header_FusionEngine_Exception

#if _MSC_VER > 1000
#pragma once
#endif

#include "Common.h"

namespace FusionEngine
{

	//! Fusion exception class.
	class Exception
	{
	public:
		//! [depreciated] Types of Exceptions
		/*!
		 * Do not use these!
		 */
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
		Exception(ExceptionType type, const std::string &message, bool critical);

		//! Constructor +origin
		Exception(const std::string &origin);
		//! Constructor +origin +message
		Exception(const std::string &origin, const std::string &message);
		//! Constructor +origin +message +file +line
		Exception(const std::string &origin, const std::string &message, const char* file, long line);

	public:
		//! [depreciated] Retrieves the type
		ExceptionType GetType() const;
		//! Retrieves the exception class name
		virtual std::string GetName() const;
		//! Sets the Origin property
		virtual void SetOrigin(const std::string& origin);
		//! Retrieves the origin
		virtual const std::string& GetOrigin() const;
		//! Retrieves the human readable message.
		/*!
		 * \remarks
		 * Me - I can't call this GetMessage because of a windows macro 
		 * which redefines that as GetMessageW! :\
		 */
		virtual std::string GetDescription() const;

		//! [depreciated] Returns true if the error type of this Exception object is severe
		bool IsCritical() const;

		//! Returns a string representing the exception object
		/*!
		 * A string in the following format will be returned: <br>
		 * "<Name> in <Origin>: <Description/Message>"
		 */
		virtual std::string ToString() const;

	protected:
		std::string m_File;
		long m_Line;
		std::string m_Name;
		std::string m_Origin;
		//! Stores the message (usually used as the Description property)
		std::string m_Message;

		// [depreciated]
		ExceptionType m_Type;
		bool m_Critical;

	};

}

#endif
