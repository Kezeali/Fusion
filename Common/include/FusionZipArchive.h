/*
*  Copyright (c) 2011 Fusion Project Team
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
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

// Inspired by 'microzip': http://code.google.com/p/microzip/ (no code was taken directly, since that uses depreciated boost::filesystem functions)
//  which is aparently based on .dan.g.'s "Win32 Wrapper classes for Gilles Volant's Zip/Unzip API"

#ifndef H_FusionZipArchive
#define H_FusionZipArchive

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <boost/filesystem/path.hpp>

#include "minizip/zip.h"

namespace FusionEngine
{

	namespace IO { namespace FileType
	{
		// TODO: implement filesystem helper
		/*
		template <class FSHelper>
		class ZipArchive
		{
		ZipArchive(... path, FSHelper fs_helper = FSHelper())
		{
		...
		}

		... AddPath(... path, ...)
		{
		if (m_FSHelper.IsFile(path))
		AddFile(path);
		else if dir...
		}

		void AddFile(... path, ...)
		{
		...
		auto tm = m_FSHelper.LastWriteTime(path);
		fileinfo.tmz_date.tm_hour = tm.tm_hour;
		...
		auto input = m_FSHelper.OpenFile(path);
		}
		private:
		std::unique_ptr<detail::ZipArchiveImpl> m_Impl;
		*/

		namespace detail
		{
			class ZipArchiveImpl;
		}

		//! Write files to a .zip
		class ZipArchive
		{
		public:
			//! Ctor. Creates a null object
			ZipArchive();
			//! Ctor
			ZipArchive(const boost::filesystem::path& file_path);
			//! DTOR
			~ZipArchive();

			//! Move Ctor
			ZipArchive(ZipArchive&& other);

			ZipArchive& operator= (ZipArchive&& other);

			//! Re-initialise this object by closing the current file (if open) and opening a new one
			void Open(const boost::filesystem::path& file_path);
			//! Close the current file
			void Close();

			//! Add the file or folder at the given path to the archive
			void AddPath(const boost::filesystem::path& path, boost::filesystem::path path_in_archive = boost::filesystem::path());

			//! Create an archive from the given folder
			static ZipArchive Create(const boost::filesystem::path& path, boost::filesystem::path path_in_archive = boost::filesystem::path());

		private:
			typedef detail::ZipArchiveImpl Impl_t;
			std::unique_ptr<Impl_t> m_Impl;
		};

	} }

}

#endif
