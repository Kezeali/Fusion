/*
  Copyright (c) 2006 FusionTeam

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

#ifndef Header_CL_PhysFS
#define Header_CL_PhysFS

#if _MSC_VER > 1000
#pragma once
#endif

#include <ClanLib/core.h>

#include "PhysFS.h"


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


#include "FusionInputSourceProvider_PhysFS.h"
#include "FusionInputSource_PhysFS.h"

//namespace FusionEngine
//{

	class SetupPhysFS
	{
	public:
		//! Constructor
		SetupPhysFS(const char *argv0 = 0);

		//! Destructor
		~SetupPhysFS();

	public:

		//! Initialises PhysFS for use in InputSource_PhysFS, etc.
		/*!
		 * <p>
		 * Must be called before any instances of InputSourceProvider_PhysFS or
		 * InputSource_PhysFS are created.
		 * </p>
		 *
		 * \param argv0
		 * Pass argv[0] or NULL here.
		 *
		 * \returns
		 * True if initialisation was successful.
		 */
		static bool init(const char *argv0);

		//! Configures PhysFS.
		/*!
		 * <p>
		 * This will call PHYSFS_setSaneConfig(organisation
		 *                                     appName
		 *                                     archiveExt
		 * <p>
		 * The write directory will be created at [user]/.organisation/appName
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
		 * \remarks
		 * PhysFS preffers you to hand it argv[0] so it can try to find the base
		 * directory from it. It's pretty damn annoying, since ClanLib could just
		 * /tell/ it where to look (with CL_System::get_exe_path()) *sigh*.
		 *
		 * \returns
		 * True if configuration was successful.
		 */
		static bool configure(const std::string &organisation,
										const std::string &appName,
										const std::string &archiveExt,
										bool includeCdRoms = false,
										bool archivesFirst = true);

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
																	bool archivesFirst = false,
																	const std::string &archiveExt = "");

		//! Calls PHYSFS_deinit();
		static void deinit();
	};

//} // namespace FusionEngine

#endif
