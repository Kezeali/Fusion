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

#include "PrecompiledHeaders.h"

#include "FusionAnyFS.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>

#include <physfs.h>

namespace bfs = boost::filesystem;
namespace bio = boost::iostreams;

namespace FusionEngine { namespace IO { namespace AnyFS
{

	PathType is_folder(const bfs::path& path)
	{
		if (PHYSFS_isDirectory(path.generic_string().c_str()))
			return PathType::PhysFS;
		else if (bfs::is_directory(path))
			return PathType::Native;
		else
			return PathType::NonExistent;
	}

	PathType is_file(const bfs::path& path)
	{
		auto genericString = path.generic_string();
		if (PHYSFS_exists(genericString.c_str()) || !PHYSFS_isDirectory(genericString.c_str()))
			return PathType::PhysFS;
		else if (bfs::is_regular_file(path))
			return PathType::Native;
		else
			return PathType::NonExistent;
	}

} } }
