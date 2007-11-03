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

#include "Common.h"

#include "FusionException.h"

namespace FusionEngine
{

	//! Exception codes
	struct ExCode
	{
		enum ExceptionCodes
		{
			Base,
			IO,
			FileNotFound,
			FileType,
			ResourceNotLoaded
		};
	};

	//! A simple way to differentiate the various factory overloads
	template <int T>
	struct ExceptionClass
	{
		enum { code = T };
	};

	//! [WIP] String manipulation functions
	/*!
	 * Does stuff with strings - Reduces dependance on the ClanLib API
	 * by wrapping CL_String; Adds additional functionality.
	 *
	 * \todo StringStream
	 */
	namespace StrUtil
	{
		//! Formats a string
		template <typename T1>
		static std::string Format(const std::string& format, const T1 &p1)
		{
			return CL_String::format<T1>(format, p1);
		}
		//! Formats a string
		template <typename T1, typename T2>
		static std::string Format(const std::string& format, const T1& p1, const T2& p2)
		{
			return CL_String::format<T1, T2>(format, p1, p2);
		}
	}

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
		static ResourceNotLoadedException Create(
			ExceptionClass<ExCode::ResourceNotLoaded> type, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return ResourceNotLoadedException(origin, message, file, line);
		}
	};

#ifndef FSN_EXCEPT
#define FSN_EXCEPT(num, src, desc) throw FusionEngine::ExceptionFactory::Create( \
	FusionEngine::ExceptionClass<num>(), src, desc, __FILE__, __LINE__ )
#endif

}

#endif