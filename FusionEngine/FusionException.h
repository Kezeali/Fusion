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
		 * Me - I can't call this GetMessage because of a stupid windows macro 
		 * which redefines that as GetMessageW!
		 */
		const std::string& GetError() const;
		//! Returns true if the error type of this Exception object is severe
		bool IsCritical() const;

		//! Returns a string for an exception message (always, even if the given 'resource' is invalid)
		static std::string GetString(const std::string &origin, const std::string &resource)
		{
			// This doesn't need to be in the header (not a template) but it's probably best for readabilities sake
			return origin + grabReliableString(resource);
		}

		//! Returns a string for an exception message (always)
		/*!
		 * \param[in] p1
		 * Pass a param here if the message requires it
		 */
		template<class Param1>
		static std::string GetString(const std::string &origin, const std::string &resource, const Param1 &p1)
		{
			return origin + CL_String::format<Param1>(grabReliableString(resource, 1), p1);
		}

		//! Returns a string for an exception message (always)
		/*!
		 * \param[in] p[n]
		 * Pass a param here if the message requires it
		 */
		template<class Param1, class Param2>
		static std::string GetString(const std::string &origin, const std::string &resource, const Param1 &p1, const Param1 &p2)
		{
			return origin + CL_String::format<Param1, Param2>(grabReliableString(resource, 2), p1, p2);
		}

		//! Returns a string for an exception message (always)
		/*!
		 * \param[in] p[n]
		 * Pass a param here if the message requires it
		 */
		template<class Param1, class Param2, class Param3>
		static std::string GetString(const std::string &origin, const std::string &resource, const Param1 &p1, const Param2 &p2, const Param3 &p3)
		{
			return origin + CL_String::format<Param1, Param2, Param3>(grabReliableString(resource, 3), p1, p2, p3);
		}

		//! Returns a string for an exception message (always)
		/*!
		 * \param[in] p[n]
		 * Pass a param here if the message requires it
		 */
		template<class Param1, class Param2, class Param3, class Param4>
		static std::string GetString(const std::string &origin, const std::string &resource, const Param1 &p1, const Param2 &p2, const Param3 &p3, const Param4 &p4)
		{
			return origin + CL_String::format<Param1, Param2, Param3, Param4>(grabReliableString(resource, 4), p1, p2, p3, p4);
		}

	protected:
		//! Always returns a string
		/*!
		 * \todo Make this (and GetString()) seperate from Exception (Exception shouldn't depend ResourceManager!)
		 * Gets the string from the given resource, or makes one up if that is impossible
		 */
		static std::string grabReliableString(const std::string& resource, int args = 0);

	protected:
		//! Stores the error type
		ExceptionType m_Type;
		//! Stores the message
		std::string m_Message;
		bool m_Critical;

	};

}

#endif
