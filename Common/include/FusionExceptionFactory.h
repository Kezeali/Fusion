/*
*  Copyright (c) 2007-2011 Fusion Project Team
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

#ifndef H_FusionExceptionFactory
#define H_FusionExceptionFactory

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionException.h"

#include <ClanLib/core.h>

namespace FusionEngine
{

	//! Exception codes [legacy, do not use these]
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

	//! Template trickery
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
		//! Instanciates an exception of the given class
		template <class ExType>
		static ExType Create(const std::string& origin, const std::string& message, const char* file, long line)
		{
			static_assert(std::is_base_of<Exception, ExType>::value, "Requires Exception-derrived type");
			return ExType(message, origin,
#ifdef _DEBUG
				file,
#else
				clan::PathHelp::get_filename(file).c_str(),
#endif
				line);
		}

		//! Tricky template tomfoolery for backwards compatibillity with ExCode
		template <int ExID>
		static typename ExceptionClass<ExID>::type Create(const std::string& origin, const std::string& message, const char* file, long line)
		{
#ifdef _DEBUG
			std::string relativeFile;
			std::string basePath = clan::PathHelp::get_basepath( clan::PathHelp::normalize( __FILE__ ) );
			relativeFile = clan::PathHelp::make_relative(basePath, file);
#endif
			return Create(ExceptionClass<ExID>(), origin, message,
#ifdef _DEBUG
				relativeFile.c_str(),
#else
				clan::PathHelp::get_filename(file).c_str(),
#endif
				line);
		}

		//! Creates a FileSystemException
		static FileSystemException Create(
			FusionEngine::ExceptionClass<ExCode::IO>,
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return FileSystemException(message, origin, file, line);
		}

		//! Creates a FileTypeExcepton
		static FileTypeException Create(
			ExceptionClass<ExCode::FileType>, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return FileTypeException(message, origin, file, line);
		}

		//! Creates a FileNotFoundException
		static FileNotFoundException Create(
			ExceptionClass<ExCode::FileNotFound>, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return FileNotFoundException(message, origin, file, line);
		}

		//! Creates a ResourceNotLoadedException
		static NotImplementedException Create(
			ExceptionClass<ExCode::NotImplemented>, 
			const std::string& origin, const std::string& message, const char* file, long line)
		{
			return NotImplementedException(message, origin, file, line);
		}

		//! Creates a InvalidArgumentException
		static InvalidArgumentException Create(
			ExceptionClass<ExCode::InvalidArgument>,
			const std::string &origin, const std::string &message, const char *file, long line)
		{
			return InvalidArgumentException(message, origin, file, line);
		}
	};

#ifndef __FUNCTION__
#define __FUNCTION__ "(couldn't get function name)"
#endif

#ifndef FSN_EXCEPT
#define FSN_EXCEPT(type, desc) throw FusionEngine::ExceptionFactory::Create<type>( \
	__FUNCTION__, desc, __FILE__, __LINE__ )
#endif
	
	// Unicode version
#ifndef FSN_WEXCEPT
#define FSN_WEXCEPT(type, desc) throw FusionEngine::ExceptionFactory::Create<type>( \
	fe_narrow(__FUNCTION__), fe_narrow(desc), __FILE__, __LINE__ )
#endif

	// Custom Source version
#ifndef FSN_EXCEPT_CS
#define FSN_EXCEPT_CS(type, src, desc) throw FusionEngine::ExceptionFactory::Create<type>( \
	src, desc, __FILE__, __LINE__ )
#endif

}

#endif