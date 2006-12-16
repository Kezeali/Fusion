
#include "FusionPhysFS.h"

#include "PhysFS.h"

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

bool SetupPhysFS::configure(const std::string &organisation,
														const std::string &appName,
														const std::string &archiveExt,
														bool includeCdRoms,
														bool archivesFirst)
{
	return (

		PHYSFS_setSaneConfig(
		organisation.c_str(),
		appName.c_str(),
		archiveExt.c_str(),
		(int)includeCdRoms,
		(int)archivesFirst)

		? true : false);
}

bool SetupPhysFS::add_subdirectory(const std::string &path,
																	 bool archivesFirst,
																	 const std::string &archiveExt)
{
	// Get the absoulte path to the given directory (or archive)
	std::string full_path = std::string( PHYSFS_getRealDir(path.c_str()) ) + path;

	// Add the directory to the end of the search path
	bool added = (PHYSFS_addToSearchPath(full_path.c_str(), 1) ? true : false);


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


	return added;
}

void SetupPhysFS::deinit()
{
	PHYSFS_deinit();
}
