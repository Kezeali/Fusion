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


	File Author(s):

		Elliot Hayward
*/

#include "FusionPhysFS.h"

#include "FusionPaths.h"

#include "PhysFS.h"
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>

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
	return (PHYSFS_init(argv0) ? true : false);
}

void SetupPhysFS::deinit()
{
	PHYSFS_deinit();
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

		return true;
	}
	else
		return false;
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

	// Ripped from PHYSFS_setSaneConfig (and converted somewhat to c++)
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

	// Ripped from PHYSFS_setSaneConfig (and converted somewhat to c++)
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
						PHYSFS_mount(arc, mount_point.c_str(), (archivesFirst ? 0 : 1));
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
