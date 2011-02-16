/*
*  Copyright (c) 2006-2010 Fusion Project Team
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
*/

#ifndef Header_FusionEngine_Exception
#define Header_FusionEngine_Exception

#if _MSC_VER > 1000
#pragma once
#endif

#include <exception>
#include <string>

namespace FusionEngine
{

	//! Fusion exception class.
	class Exception : public std::exception
	{
	public:
		//! Basic constructor
		Exception();

		//! Constructor +message
		Exception(const std::string &message);
		//! Constructor +message +origin
		Exception(const std::string &message, const std::string &function);
		//! Constructor +message +origin +file +line
		Exception(const std::string &message, const std::string &function, const char* file, long line);

		virtual ~Exception();

	public:
		//! Retrieves the exception class name
		virtual const std::string& GetName() const;
		//! Retrieves the origin
		const std::string& GetOrigin() const;
		//! Retrieves the human readable message.
		const std::string& GetDescription() const;

		//! Returns a string representing the exception object
		/*!
		 * A string in the following format will be returned: <br>
		 * "<Name> in <Origin>(file : line): <Description>"
		 */
		std::string ToString() const;

		//! Implementation of std::exception. Returns the string from GetDescription()
		const char *what() const;

	protected:
		std::string m_File;
		long m_Line;
		std::string m_Origin;
		std::string m_Message;

	};

	//! Filesystem exception class
	class FileSystemException : public Exception
	{
	public:
		//! Constructor
		FileSystemException(const std::string& description, const std::string& origin, const char* file, long line)
			: Exception(description, origin, file, line) {}
	};
	//! FileNotFoundException class
	class FileNotFoundException : public FileSystemException
	{
	public:
		//! Constructor
		FileNotFoundException(const std::string& description, const std::string& origin, const char* file, long line)
			: FileSystemException(description, origin, file, line) {}
	};
	//! FileTypeException class
	class FileTypeException : public FileSystemException
	{
	public:
		//! Constructor
		FileTypeException(const std::string& description, const std::string& origin, const char* file, long line)
			: FileSystemException(description, origin, file, line) {}
	};
	//! NotImplemented exception
	/*!
	 * For use when a method has not been implemented (but exists as a placeholder).
	 */
	class NotImplementedException : public Exception 
	{
	public:
		NotImplementedException(const std::string& description, const std::string& origin, const char* file, long line)
			: Exception(description, origin, file, line) {}
	};
	//! InvalidArgument exception
	class InvalidArgumentException : public Exception
	{
	public:
		InvalidArgumentException(const std::string& description, const std::string& origin, const char* file, long line)
			: Exception(description, origin, file, line) {}
	};

}

#endif
