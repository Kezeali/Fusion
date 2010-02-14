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

#ifndef Header_FusionEngine_ExceptionFactory
#define Header_FusionEngine_ExceptionFactory

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionStableHeaders.h"

#include "FusionException.h"

namespace FusionEngine
{

	//! Namespace containing exception types
	// This is probably a better alternative to the ExCode enum
	//  (since it would work with a single generic throw method -
	//  no need for a factory.) The factory is just allows you to
	//  use error codes to generate exceptions, which is for 
	//  compatibility with legacy code - which ironically doesn't
	//  exist in my case (I didn't fully realize this when I was
	//  following the article this is based on.)
	//namespace ExCode
	//{
	//	typedef Exception Base;
	//	typedef FileSystemException IO;
	//	typedef FileNotFoundException FileNotFound;
	//	typedef FileTypeException FileType;
	//	typedef NotImplementedException NotImplemented;
	//	typedef InvalidArgumentException InvalidArgument;
	//}

	//! Exception codes
	struct ExCode
	{
		enum ExceptionCodes
		{
			Base,
			IO,
			FileNotFound,
			FileType,
			NotImplemented,
			InvalidArgument
		};
	};

	//! A simple way to differentiate the various factory overloads
	template <int T>
	struct ExceptionClass
	{
		enum { code = T };
	};

	//! Creates exceptions
	class ExceptionFactory
	{
	public:

		//! Creates a FileSystemException
		static FileSystemException Create(
			ExceptionClass<ExCode::IO> type, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return FileSystemException(origin, message, file, line);
		}

		//! Creates a FileTypeExcepton
		static FileTypeException Create(
			ExceptionClass<ExCode::FileType> type, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return FileTypeException(origin, message, file, line);
		}

		//! Creates a FileNotFoundException
		static FileNotFoundException Create(
			ExceptionClass<ExCode::FileNotFound> type, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return FileNotFoundException(origin, message, file, line);
		}

		//! Creates a ResourceNotLoadedException
		static NotImplementedException Create(
			ExceptionClass<ExCode::NotImplemented> type, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return NotImplementedException(origin, message, file, line);
		}

		//! Creates a InvalidArgumentException
		static InvalidArgumentException Create(
			ExceptionClass<ExCode::InvalidArgument> type,
			const std::string &origin, const std::string &message, const char *file, long line)
		{
			return InvalidArgumentException(origin, message, file, line);
		}
	};

#ifndef FSN_EXCEPT
#define FSN_EXCEPT(num, src, desc) throw FusionEngine::ExceptionFactory::Create( \
	FusionEngine::ExceptionClass<num>(), src, desc, __FILE__, __LINE__ )
#endif
	// Unicode version
#ifndef FSN_WEXCEPT
#define FSN_WEXCEPT(num, src, desc) throw FusionEngine::ExceptionFactory::Create( \
	FusionEngine::ExceptionClass<num>(), fe_narrow(src), \
	fe_narrow(desc), __FILE__, __LINE__ )
#endif


}

#endif