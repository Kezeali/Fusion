/*
*  Copyright (c) 2006 Fusion Project Team
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

#ifndef H_FusionPhysFS
#define H_FusionPhysFS

#if _MSC_VER > 1000
#pragma once
#endif

#include <physfs.h>
#include <regex>

namespace FusionEngine
{

	//! Helpful functions for PhysFS
	namespace PhysFSHelp
	{
		//! Copies the given file
		/*!
		* \param[in] from
		* File to copy
		* \param[in] from
		* Target file
		*/
		void copy_file(const std::string& from, const std::string& to);

		//! Returns files & folders matching the given expression
		std::vector<std::string> regex_find(const std::string& path, const std::string& expression, bool recursive = false);

		//! Returns files & folders matching the given expression
		std::vector<std::string> regex_find(const std::string& path, const std::regex& expression, bool recursive = false);

	}

	class SetupPhysFS
	{
	public:
		//! Constructor
		SetupPhysFS();
		//! Constructor + argv0
		SetupPhysFS(const char *argv0);

		//! Destructor
		~SetupPhysFS();

	public:

		//! Initialises PhysFS.
		/*!
		* <p>
		* Must be called before any instances of InputSourceProvider_PhysFS or
		* InputSource_PhysFS are created.
		* </p>
		*
		* \param argv0
		* Pass argv[0], CL_System::get_exe_path() or NULL here.
		*
		* \returns
		* True if initialisation was successful.
		*/
		static bool init(const char *argv0);

		//! Calls PHYSFS_deinit();
		static void deinit();

		//! Returns true if phys-fs has been initialised
		static bool is_init();

		//! Configures PhysFS.
		/*!
		* <p>
		* This will call PHYSFS_setSaneConfig(organisation,
		*                                     appName,
		*                                     archiveExt,
		*                                     includeCdRoms,
		*                                     archivesFirst);
		* </p>
		* <p>
		* The write folder will be created at [user]/.organisation/appName, and
		* will be populated with the basic folders (logs, packages, etc.)
		* </p>
		*
		*
		* \param organisation
		* Where to put the write directory, below the user directory.
		*
		* \param appName
		* Where to put the write directory, below the org. directory.
		*
		* \param archiveExt
		* File extension used by your program to specify an archive.
		*
		* \param includeCdRoms
		* True to include CD-ROMs in the search path. 
		*
		* \param archivesFirst
		* True to search archives first (before directories) when looking for files.
		* Ignored if you don't specify (archiveExt).
		*
		*
		* \returns
		* True if configuration was successful.
		*/
		static bool configure(const std::string &organisation,
			const std::string &appName,
			const std::string &archiveExt,
			bool includeCdRoms = false,
			bool archivesFirst = false);

		//! Deletes all files / folders in the tempoary ('temp') folder
		static void clear_temp();

		//! Adds the given sub-directory to the search path.
		/*!
		* <p>
		* This will add base dir/(path) to the search path.
		* </p>
		* <p>
		* For example, if your base directory was "/game/", and you wanted to add
		* "/game/Levels/" to the search path, you could call:
		* \code
		* SetupPhysFS::add_subdirectory("Levels/");
		* \endcode
		* or, if you wanted to add an archive:
		* \code
		* SetupPhysFS::add_subdirectory("Levels/MyArchive.ZIP");
		* \endcode
		* </p>
		*
		* \param[in] path
		* The path, relative to the base, to add to the search.
		*
		* \param[in] archivesFirst
		* Same effect as archivesFirst param for SetupPhysFS#configure()
		*
		* \param[in] addArchives
		* If this is true, archives found in the given directory will be added.
		*/
		static bool add_subdirectory(const std::string &path,
			const std::string &archiveExt = "",
			bool archivesFirst = false);

		//! Mounts the given (native-fs) path
		static bool mount(const std::string &native_path, const std::string &mount_point,
			const std::string &archive_ext = "",
			bool archives_first = false);

		//! Mounts all archives found below the given path
		static bool mount_archives(const std::string &physfs_path, const std::string &mount_point, const std::string &archive_ext, bool first = false);

		//! Parses the given path relative to the given directory to get an abosulte path
		static std::string parse_path(const std::string &working_directory, const std::string &path);
	};

}

#endif
