/*
  Copyright (c) 2007 Fusion Project Team

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

#ifndef Header_FusionEngine_FileSystemException
#define Header_FusionEngine_FileSystemException

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionException.h"

namespace FusionEngine
{

	//! File system exception class
	class FileSystemException : public Exception
	{
		static std::string s_Message;
	public:
		//! Constructor
		FileSystemException() : Exception() {}
		//! Constructor
		FileSystemException(const std::string& origin)
			: Exception(origin)
		{
		}
		//! Constructor
		FileSystemException(const std::string& origin, const std::string& message)
			: Exception(origin, message)
		{
		}

		//! Constructor (full)
		FileSystemException(const std::string& origin, const std::string& message, const char* file, long line)
			: Exception(origin, message, file, line)
		{
		}

	public:
		//! Returns the name of this exception class
		virtual std::string GetName() const;

	};

}

#endif
