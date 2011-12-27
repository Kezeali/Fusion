/*
*  Copyright (c) 2006-2011 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionPhysFS.h"

#include "FusionPaths.h"
#include "FusionExceptionFactory.h"

#include <stack>
#include <regex>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>

namespace FusionEngine
{

	//! Like strcmp, but case-insensitive. Hopefully cross-platform.
	static int platform_stricmp(const char *x, const char *y)
	{
#if (defined _MSC_VER)
		return (_stricmp(x, y));
#else
		int ux, uy;

		do
		{
			ux = toupper((int) *x);
			uy = toupper((int) *y);
			if (ux != uy)
				return((ux > uy) ? 1 : -1);
			x++;
			y++;
		} while ((ux) && (uy));

		return(0);
#endif
	}

	namespace PhysFSHelp
	{

		void copy_file(const std::string& from, const std::string& to)
		{
			PHYSFS_File *fromFile = PHYSFS_openRead(from.c_str());
			if (fromFile == nullptr)
				FSN_EXCEPT(FusionEngine::FileSystemException, "Failed to open " + from + " for copying (probably doesn't exist).");
			PHYSFS_File *toFile = PHYSFS_openWrite(to.c_str());
			if (toFile == nullptr)
			{
				PHYSFS_close(fromFile);
				FSN_EXCEPT(FusionEngine::FileSystemException, "Failed to create / overwrite target file " + to + ".");
			}
			try
			{
				std::vector<char> buffer(4194304);
				PHYSFS_sint64 readLength = 0;
				do
				{
					readLength = PHYSFS_read(fromFile, buffer.data(), sizeof(char), buffer.size());
					if (readLength < 0)
						FSN_EXCEPT(FusionEngine::FileSystemException, "Failed to read " + from + " for copying.");

					if (PHYSFS_write(toFile, buffer.data(), sizeof(char), (PHYSFS_uint32)readLength) != readLength)
						FSN_EXCEPT(FusionEngine::FileSystemException, "Failed to write copied data to " + to + ".");
				} while (!PHYSFS_eof(fromFile));
			}
			catch (FusionEngine::FileSystemException&)
			{
				PHYSFS_close(toFile);
				PHYSFS_close(fromFile);
				throw;
			}

			PHYSFS_close(toFile);
			PHYSFS_close(fromFile);
		}

		std::vector<std::string> regex_find(const std::string& path, const std::string& expression, bool recursive)
		{
			return regex_find(path, std::regex(expression), recursive);
		}

		std::vector<std::string> regex_find(const std::string& path, const std::regex& expression, bool recursive)
		{
			std::vector<std::string> results;

			auto files = PHYSFS_enumerateFiles(path.c_str());
			for (auto it = files; *it != NULL; ++it)
			{
				std::string filename = *it;
				std::string filePath = path + filename;

				if (std::regex_match(filePath, expression))
				{
					results.push_back(filePath);
				}

				if (recursive && PHYSFS_isDirectory(filePath.c_str()))
				{
					regex_find(filePath, expression, recursive);
				}
			}
			PHYSFS_freeList(files);

			return results;
		}

	}

	SetupPhysFS::SetupPhysFS()
	{
		CL_String8 path = CL_StringHelp::text_to_local8(CL_System::get_exe_path());
		init(path.c_str());
	}

	SetupPhysFS::SetupPhysFS(const char *argv0)
	{
		init(argv0);
	}

	SetupPhysFS::~SetupPhysFS()
	{
		deinit();
	}

	bool SetupPhysFS::init(const char *argv0)
	{
		return PHYSFS_init(argv0) == 1;
	}

	void SetupPhysFS::deinit()
	{
		PHYSFS_deinit();
	}

	bool SetupPhysFS::is_init()
	{
		return PHYSFS_isInit() == 1;
	}

	bool SetupPhysFS::configure(const std::string &organisation,
		const std::string &appName,
		const std::string &archiveExt,
		bool includeCdRoms,
		bool archivesFirst)
	{
		if (
			PHYSFS_setSaneConfig(
			organisation.c_str(),
			appName.c_str(),
			archiveExt.c_str(),
			(int)includeCdRoms,
			(int)archivesFirst))
		{
			PHYSFS_mkdir(FusionEngine::s_PackagesPath.c_str());
			PHYSFS_mkdir(FusionEngine::s_LogfilePath.c_str());
			PHYSFS_mkdir(FusionEngine::s_TempPath.c_str());
			PHYSFS_mkdir(FusionEngine::s_SavePath.c_str());
			PHYSFS_mkdir(FusionEngine::s_EditorPath.c_str());

			PHYSFS_mkdir("ScriptCache");

			return true;
		}
		else
			return false;
	}

	struct DirectoryToDelete
	{
		std::string path;
		char **file_list;
		char **current_file;
	};

	void SetupPhysFS::clear_temp()
	{
		DirectoryToDelete current_directory;
		current_directory.file_list = PHYSFS_enumerateFiles(FusionEngine::s_TempPath.c_str());
		current_directory.current_file = current_directory.file_list;

		std::stack<DirectoryToDelete> directories;

		while (true)
		{
			while (*current_directory.current_file != NULL)
			{
				std::string filename =
					(current_directory.path.empty() ? FusionEngine::s_TempPath : current_directory.path) + *current_directory.current_file;
				if (PHYSFS_isDirectory(filename.c_str()) == 1)
				{
					// Store the current position - will return here when the sub-dir is deleted
					directories.push(current_directory);
					// Switch to the directory that was just found
					current_directory = DirectoryToDelete();
					current_directory.path = filename + "/";
					current_directory.file_list = PHYSFS_enumerateFiles(filename.c_str());
					current_directory.current_file = current_directory.file_list;
				}
				else
				{
					PHYSFS_delete(filename.c_str());
					++current_directory.current_file;
				}
			}

			// All the files in the current directory have been deleted, so now the directory can be deleted
			if (!current_directory.path.empty())
				PHYSFS_delete(current_directory.path.c_str());

			PHYSFS_freeList(current_directory.file_list);

			if (directories.empty())
				break;
			// Jump back up to the parent directory
			current_directory = directories.top();
			++current_directory.current_file; // Go to the next file
			directories.pop();
		}
	}

	bool SetupPhysFS::add_subdirectory(const std::string &path,
		const std::string &archiveExt,
		bool archivesFirst)
	{
		// Get the absoulte path to the given directory (or archive)
		const char* dir = PHYSFS_getRealDir(path.c_str());
		if (dir == NULL)
			return false;

		bool added = false;

		// Try to add the path relative to app
		std::string full_path = std::string( PHYSFS_getBaseDir() ) + path;
		// Add the directory to the end of the search path
		if (PHYSFS_addToSearchPath(full_path.c_str(), 1) > 0)
			added = true;

		// Try to add the path from anywhere we can
		full_path = std::string( dir ) + PHYSFS_getDirSeparator() + path;
		// Add the directory to the end of the search path
		if (PHYSFS_addToSearchPath(full_path.c_str(), 1) > 0)
			added = true;

		if (!added)
			return false;

		// Ripped from PHYSFS_setSaneConfig (and somewhat c++ized)
		if (!archiveExt.empty())
		{
			const char *dirsep = PHYSFS_getDirSeparator();

			char **rc = PHYSFS_enumerateFiles(path.c_str());
			char **i;
			size_t extlen = archiveExt.length();
			char *ext;
			char *arc;

			for (i = rc; *i != NULL; i++)
			{
				size_t l = strlen(*i);
				if ((l > extlen) && ((*i)[l - extlen - 1] == '.'))
				{
					ext = (*i) + (l - extlen);
					if (platform_stricmp(ext, archiveExt.c_str()) == 0)
					{
						const char *d = PHYSFS_getRealDir(*i);
						arc = (char *)malloc(strlen(d) + strlen(dirsep) + l + 1);
						if (arc != NULL)
						{
							sprintf(arc, "%s%s%s", d, dirsep, *i);
							PHYSFS_addToSearchPath(arc, (archivesFirst ? 0 : 1) );
							free(arc);
						} /* if */
					} /* if */
				} /* if */
			} /* for */

			PHYSFS_freeList(rc);
		} // if (!archiveExt.empty())

		// If we got here we assume the addition was successful
		return true;
	}

	bool SetupPhysFS::mount(const std::string &path, const std::string &mount_point, 
		const std::string &archiveExt,
		bool archivesFirst)
	{
		// Get the absoulte path to the given directory (or archive)
		const char* dir = PHYSFS_getRealDir(path.c_str());
		if (dir == NULL)
			return false;

		bool added = false;

		// Try to add the path relative to app
		std::string full_path = std::string( PHYSFS_getBaseDir() ) + path;
		// Add the directory to the end of the search path
		if (PHYSFS_mount(full_path.c_str(), (mount_point.empty() ? NULL : mount_point.c_str()), 1) > 0)
			added = true;

		// Try to add the path from anywhere we can
		full_path = std::string( dir ) + PHYSFS_getDirSeparator() + path;
		// Add the directory to the end of the search path
		if (PHYSFS_mount(full_path.c_str(), (mount_point.empty() ? NULL : mount_point.c_str()), 1) > 0)
			added = true;

		if (!added)
			return false;

		// Ripped from PHYSFS_setSaneConfig (and somewhat c++ized)
		if (!archiveExt.empty())
			added = mount_archives(path, mount_point, archiveExt, archivesFirst);

		return added;
	}

	bool SetupPhysFS::mount_archives(const std::string &path, const std::string &mount_point, const std::string &archive_ext, bool archives_first)
	{
		const char *dirsep = PHYSFS_getDirSeparator();

		char **rc = PHYSFS_enumerateFiles(path.c_str());
		char **i;
		size_t extlen = archive_ext.length();
		char *ext;
		char *arc;

		bool ok = true;

		for (i = rc; *i != NULL; i++)
		{
			size_t l = strlen(*i);
			if ((l > extlen) && ((*i)[l - extlen - 1] == '.'))
			{
				ext = (*i) + (l - extlen);
				if (platform_stricmp(ext, archive_ext.c_str()) == 0)
				{
					const char *d = PHYSFS_getRealDir(*i);
					arc = (char *)malloc(strlen(d) + strlen(dirsep) + l + 1);
					if (arc != NULL)
					{
						sprintf(arc, "%s%s%s", d, dirsep, *i);
						ok = PHYSFS_mount(arc, mount_point.c_str(), (archives_first ? 0 : 1)) != 0;
						free(arc);
						if (!ok)
							break;
					}
				}
			}
		}

		PHYSFS_freeList(rc);

		return ok;
	}

	std::string SetupPhysFS::parse_path(const std::string &working_directory, const std::string &path)
	{
		if (path[0] == '/')
		{
			return path;
		}

		else
		{
			typedef std::vector<std::string> SplitResults;

			SplitResults currentPath;
			boost::split(currentPath, working_directory, boost::is_any_of("/"));

			SplitResults pathTokens;
			boost::split(pathTokens, path, boost::is_any_of("/"));
			for (SplitResults::iterator it = pathTokens.begin(), end = pathTokens.end(); it != end; ++it)
			{
				if (*it == "..")
					currentPath.pop_back();
				else
					currentPath.push_back(*it);
			}

			return boost::join(currentPath, "/");
		}
	}

}
