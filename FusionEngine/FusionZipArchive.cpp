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

#include "FusionStableHeaders.h"

#include "FusionZipArchive.h"

#include <zlib.h>
#include "minizip/zip.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

namespace bfs = boost::filesystem;
namespace bio = boost::iostreams;

namespace FusionEngine { namespace IO
{

	ZipArchive::ZipArchive(const boost::filesystem::path& file_path)
		: m_File(nullptr)
	{
		m_File = zipOpen(file_path.generic_string().c_str(), APPEND_STATUS_CREATE);
	}

	ZipArchive::~ZipArchive()
	{
		int r = ZIP_OK;
		if (m_File)
			r = zipClose(m_File, ":)");
		m_File = nullptr;
		m_Info = ArchiveInfo();
	}

	static bfs::path make_relative(const bfs::path& base, const bfs::path& path)
	{
		bfs::path relativePath;
		bool diverge = false;
		for (auto it1 = base.begin(), end1 = base.end(), it2 = path.begin(), end2 = path.end();
			it1 != end1 && it2 != end2; ++it1, ++it2)
		{
			if (!diverge && it1 != it2)
				diverge = true;
			if (diverge)
				relativePath /= *it2;
		}
	}

	void ZipArchive::AddPath(const bfs::path& path, bfs::path archive_path)
	{
		if (archive_path.empty())
		{
			archive_path = path;
		}

		if (bfs::is_regular_file(path))
			AddFile(path, archive_path);
		else if (bfs::is_directory(path))
			AddFolder(path, archive_path);
		else
		{
			FSN_EXCEPT(FileSystemException, "Failed to build ZipArchive: unknown filesystem item type: " + path.string());
		}
	}

	void ZipArchive::AddFolder(const bfs::path& path, const bfs::path& archive_path)
	{
		++m_Info.folderCount;

		zip_fileinfo fileinfo;
		fileinfo.internal_fa = 0;
#ifdef _WIN32
		fileinfo.external_fa = ::GetFileAttributesW(path.native().c_str());
#else
		{
			struct stat path_stat;
			if (::stat(path.native().c_str(), &path_stat) == 0)
			{
				fileinfo.external_fa = path_stat.st_mode;
			}
		}
#endif
		fileinfo.dosDate = 0;
		// Read the time from the filesystem and convert it
		auto fsTime = bfs::last_write_time(path);
		auto posixTime = boost::posix_time::from_time_t(fsTime);
		auto tm = boost::posix_time::to_tm(posixTime);

		/* TODO: this is how to get the time for a physfs file
		boost::posix_time::ptime posixTime;
		auto milisPastEpoc = PHYSFS_getLastModTime(path.generic_string().c_str());
		if (milisPastEpoc >= 0)
			time = boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1), boost::posix_time::milliseconds(milisPastEpoc));
		else
			time = boost::posix_time::second_clock::local_time();
		*/

		fileinfo.tmz_date.tm_hour = tm.tm_hour;
		fileinfo.tmz_date.tm_min = tm.tm_min;
		fileinfo.tmz_date.tm_sec = tm.tm_sec;
		fileinfo.tmz_date.tm_year = tm.tm_year;
		fileinfo.tmz_date.tm_mon = tm.tm_mon;
		fileinfo.tmz_date.tm_mday = tm.tm_mday;

		auto r = zipOpenNewFileInZip(m_File, (archive_path.generic_string() + "/").c_str(),
			&fileinfo,
			nullptr, 0,
			nullptr, 0,
			nullptr,
			Z_DEFLATED,
			Z_BEST_SPEED);

		zipCloseFileInZip(m_File);

		for (bfs::directory_iterator it(path), end = bfs::directory_iterator(); it != end; ++it)
		{
			AddPath(it->path(), archive_path / it->path().filename());
		}
	}

	void ZipArchive::AddFile(const bfs::path& path, const bfs::path& archive_path)
	{
		zip_fileinfo fileinfo;
		fileinfo.internal_fa = 0;
#ifdef _WIN32
		fileinfo.external_fa = ::GetFileAttributesW(path.native().c_str());
#else
		{
			struct stat path_stat;
			if (::stat(path.native().c_str(), &path_stat) == 0)
			{
				fileinfo.external_fa = path_stat.st_mode;
			}
		}
#endif
		// Read the time from the filesystem and convert it
		auto fsTime = boost::filesystem::last_write_time(path);
		auto posixTime = boost::posix_time::from_time_t(fsTime);
		auto tm = boost::posix_time::to_tm(posixTime);

		fileinfo.tmz_date.tm_hour = tm.tm_hour;
		fileinfo.tmz_date.tm_min = tm.tm_min;
		fileinfo.tmz_date.tm_sec = tm.tm_sec;
		fileinfo.tmz_date.tm_year = tm.tm_year;
		fileinfo.tmz_date.tm_mon = tm.tm_mon;
		fileinfo.tmz_date.tm_mday = tm.tm_mday;

		bio::stream<bio::file_descriptor_source> input(path, std::ios::in | std::ios::binary);
		if (!input.is_open())
		{
			FSN_EXCEPT(FileSystemException, "Couldn't open input file to compress: " + path.string());
		}

		auto r = zipOpenNewFileInZip(m_File, archive_path.generic_string().c_str(),
			&fileinfo,
			nullptr, 0,
			nullptr, 0,
			nullptr,
			Z_DEFLATED,
			Z_BEST_SPEED);

		if (r == ZIP_OK)
		{
			++m_Info.fileCount;

			std::array<char, 4096> buffer;
			unsigned int bytesRead = 0, fileSize = 0;
			while (r == ZIP_OK && input.good())
			{
				input.read(buffer.data(), buffer.size());
				auto gcount = input.gcount();
				bytesRead = static_cast<unsigned int>(gcount);
				fileSize += bytesRead;

				if (gcount > 0)
					r = zipWriteInFileInZip(m_File, buffer.data(), bytesRead);
				else
					break;
			}

			m_Info.uncompressedSize += fileSize;
		}

		zipCloseFileInZip(m_File);

		if (r != ZIP_OK)
		{
			FSN_EXCEPT(FileSystemException, "Error while writing file to zip");
		}
	}

} }
