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

#ifndef Header_FusionEngine_FileNotFoundException
#define Header_FusionEngine_FileNotFoundException

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionFileSystemException.h"

namespace FusionEngine
{

	//! FileNotFoundException class (self explainatory, I would hope ;) )
	class FileNotFoundException : public FileSystemException
	{
		static std::string s_Message;
		static std::string s_Message_filename;
	public:
		//! Constructor
		FileNotFoundException() : FileSystemException() {}
		//! Constructor
		FileNotFoundException(const std::string& origin)
			: FileSystemException(origin, s_Message)
		{
		}
		//! Constructor
		FileNotFoundException(const std::string& origin, const std::string& filename)
			: FileSystemException(origin, CL_String::format(s_Message_filename, filename))
		{
		}

	public:
		//! Returns the name of this exception class
		virtual std::string GetName() const;

	};

}

#endif
