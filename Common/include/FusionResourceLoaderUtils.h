/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionResouceLoaderUtils
#define H_FusionResouceLoaderUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionResourceLoader.h"

namespace FusionEngine
{
	struct FileMetadata 
	{
		std::int64_t modTime;
		size_t length;
		std::uint32_t checksum;
	};

	std::uint32_t checksumClanLibDevice(clan::IODevice& device);
	std::uint32_t checksumStream(std::istream& stream);

	FileMetadata CreateFileMetadata(const std::string& path, std::istream& stream);

	//! Returns true when a resource using FileMetadata has changed
	bool FileMetadataResourceHasChanged(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);
}

#endif
