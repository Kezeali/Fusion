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

	//! Exception codes [do not use these]
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

		typedef Exception type;
	};

	template <>
	struct ExceptionClass<ExCode::IO> {
		enum { code = ExCode::IO }; typedef FileSystemException type;
	};
	template <>
	struct ExceptionClass<ExCode::FileNotFound> {
		enum { code = ExCode::FileNotFound }; typedef FileNotFoundException type;
	};
	template <>
	struct ExceptionClass<ExCode::FileType> {
		enum { code = ExCode::FileType }; typedef FileTypeException type;
	};
	template <>
	struct ExceptionClass<ExCode::NotImplemented> {
		enum { code = ExCode::NotImplemented }; typedef NotImplementedException type;
	};
	template <>
	struct ExceptionClass<ExCode::InvalidArgument> {
		enum { code = ExCode::InvalidArgument }; typedef InvalidArgumentException type;
	};

	//! Creates exceptions
	class ExceptionFactory
	{
	public:

		template <class ExType>
		static ExType Create(const std::string& origin, const std::string& message, const char* file, long line)
		{
			static_assert (std::is_base_of<Exception, ExType>::value);
			return ExType(origin, message, file, line);
		}

		template <int ExID>
		static typename ExceptionClass<ExID>::type Create(const std::string& origin, const std::string& message, const char* file, long line)
		{
#ifdef _DEBUG
			std::string relativeFile;
			std::string basePath = CL_PathHelp::get_basepath( CL_PathHelp::normalize( __FILE__ ) );
			relativeFile = CL_PathHelp::make_relative(basePath, file);
#endif
			return Create(ExceptionClass<ExID>(), origin, message,
#ifdef _DEBUG
				relativeFile.c_str(),
#else
				CL_PathHelp::get_filename(file).c_str(),
#endif
				line);
		}

		//! Creates a FileSystemException
		static FileSystemException Create(
			FusionEngine::ExceptionClass<ExCode::IO> type,
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return FileSystemException(message, origin, file, line);
		}

		//! Creates a FileTypeExcepton
		static FileTypeException Create(
			ExceptionClass<ExCode::FileType> type, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return FileTypeException(message, origin, file, line);
		}

		//! Creates a FileNotFoundException
		static FileNotFoundException Create(
			ExceptionClass<ExCode::FileNotFound> type, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return FileNotFoundException(message, origin, file, line);
		}

		//! Creates a ResourceNotLoadedException
		static NotImplementedException Create(
			ExceptionClass<ExCode::NotImplemented> type, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return NotImplementedException(message, origin, file, line);
		}

		//! Creates a InvalidArgumentException
		static InvalidArgumentException Create(
			ExceptionClass<ExCode::InvalidArgument> type,
			const std::string &origin, const std::string &message, const char *file, long line)
		{
			return InvalidArgumentException(message, origin, file, line);
		}
	};

#ifndef FSN_EXCEPT
#define FSN_EXCEPT(type, src, desc) throw FusionEngine::ExceptionFactory::Create<type>( \
	src, desc, __FILE__, __LINE__ )
#endif
	// Unicode version
#ifndef FSN_WEXCEPT
#define FSN_WEXCEPT(type, src, desc) throw FusionEngine::ExceptionFactory::Create<type>( \
	fe_narrow(src), fe_narrow(desc), __FILE__, __LINE__ )
#endif


}

#endif