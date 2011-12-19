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

	namespace IO
	{
		// TODO: add full copyright info / credits to this file

		// TODO: implement filesystem helper
		/*
		template <class FSHelper>
		class ZipArchive
		{
		ZipArchive(... path, FSHelper fs_helper = FSHelper())
		{
		m_Impl = std::make_shared<...>(path);
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
		std::shared_ptr<detail::ZipArchiveImpl> m_Impl;
		*/

		//! Write files to a .zip
		class ZipArchive
		{
		public:
			//! Ctor
			ZipArchive(const boost::filesystem::path& file_path);
			~ZipArchive();

			// TODO: implement
			ZipArchive(ZipArchive&& other);

			//! Add the file or folder at the given path to the archive
			void AddPath(const boost::filesystem::path& path, boost::filesystem::path path_in_archive = boost::filesystem::path());

			// TODO: implement
			//! Create an archive from the given folder
			static ZipArchive Create(const boost::filesystem::path& path, boost::filesystem::path path_in_archive = boost::filesystem::path());

		private:
			void AddFolder(const boost::filesystem::path& path, const boost::filesystem::path& arc_path);
			void AddFile(const boost::filesystem::path& path, const boost::filesystem::path& arc_path);

			zipFile m_File;
			boost::filesystem::path m_WorkingDir;

			struct ArchiveInfo
			{
				int fileCount;
				int folderCount;
				unsigned long uncompressedSize;

				ArchiveInfo()
				{
					memset(this, 0, sizeof(ArchiveInfo));
				}
			} m_Info;
		};

	}

}

#endif
