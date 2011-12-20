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

#include "FusionInputSourceProvider_PhysFS.h"

#include "FusionInputSource_PhysFS.h"
#include "FusionPhysFS.h"

InputSourceProvider_PhysFS::InputSourceProvider_PhysFS(const std::string &path)
{
	// --Add the path to the PhysFS search, if necessary--
	if (path.empty() || path == ".")
		return; // Don't add this path

	const char *path_cstr = path.c_str();

	m_PathAdded = false;

	// PHYSFS_addToSearchPath will return true even if the path was already in the list
	//  as long as there were no explicit errors, so we have to check manually beforehand
	//  to make sure it is actually this instance of InputSourceProvider_PhysFS that
	//  needs to add the path
	char **it;
	for (it = PHYSFS_getSearchPath(); *it != NULL; it++)
	{
		if (strcmp((*it), path_cstr) == 0)
			m_PathAdded = true;
	}

	// If it wasn't in the list...
	if (m_PathAdded)
	{
		// Try to add the path
		m_PathAdded = (PHYSFS_addToSearchPath(path_cstr, 0) ? true : false);
	}


	// --Store the path for later--
	m_Provider_path = path;

}

InputSourceProvider_PhysFS::~InputSourceProvider_PhysFS()
{
	if (m_PathAdded)
		PHYSFS_removeFromSearchPath(m_Provider_path.c_str());
}

CL_InputSource *InputSourceProvider_PhysFS::open_source(const std::string &filename)
{
	return new InputSource_PhysFS(filename.c_str());
}

std::string InputSourceProvider_PhysFS::get_pathname(const std::string &filename)
{        
	//std::string filepath;

	//if (PHYSFS_exists(filename.c_str()))
	//	filepath = filename;
	//else
	//	filepath = m_Provider_path + filename;

	return filename;
}

CL_InputSourceProvider *InputSourceProvider_PhysFS::create_relative(const std::string &path)
{
	return new InputSourceProvider_PhysFS(m_Provider_path + path);
}

CL_InputSourceProvider *InputSourceProvider_PhysFS::clone()
{
	return new InputSourceProvider_PhysFS(m_Provider_path);
}
