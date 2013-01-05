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

#include "PrecompiledHeaders.h"

#include "FusionResourceLoaderUtils.h"

#include <boost/crc.hpp>
#include <boost/filesystem.hpp>
#include <physfs.h>

namespace FusionEngine
{

	std::uint32_t checksumClanLibDevice(clan::IODevice& device)
	{
		boost::crc_32_type crc;
		std::array<char, 1 << 15> buffer;
		int count = 0; 
		while ((count = device.read(buffer.data(), buffer.size())) > 0)
		{
			crc.process_bytes(buffer.data(), count);
		}

		return crc.checksum();
	}

	std::uint32_t checksumStream(std::istream& stream)
	{
		stream.seekg(0);

		boost::crc_32_type crc;
		std::array<char, 1 << 15> buffer;
		while (!stream.eof() && !stream.bad())
		{
			stream.read(buffer.data(), buffer.size());
			const auto count = stream.gcount();
			if (count > 0)
				crc.process_bytes(buffer.data(), (size_t)count);
		}

		return crc.checksum();
	}

	FileMetadata CreateFileMetadata(const std::string& path, std::istream& stream)
	{
		FileMetadata metadata;
		if (PHYSFS_exists(path.c_str()))
			metadata.modTime = PHYSFS_getLastModTime(path.c_str());
		else
			metadata.modTime = boost::filesystem::last_write_time(path);
		metadata.checksum = checksumStream(stream);
		stream.seekg(0, std::ios::end);
		metadata.length = (size_t)stream.tellg();
		return metadata;
	}

	bool ResourceModTimeHasChanged(ResourceContainer* resource, clan::VirtualDirectory vdir, const FileMetadata& metadata)
	{
		const auto path = resource->GetPath();

		const auto currentModTime = PHYSFS_exists(path.c_str()) ? PHYSFS_getLastModTime(path.c_str()) : boost::filesystem::last_write_time(path);

		return currentModTime != metadata.modTime;
	}

	bool ResourceContentHasChanged(ResourceContainer* resource, clan::VirtualDirectory vdir, const FileMetadata& metadata)
	{
		clan::IODevice device = vdir.open_file_read(resource->GetPath());

		// Compare length
		if (device.get_size() != metadata.length)
			return true;

		// Compare sum
		const auto newChecksum = checksumClanLibDevice(device);

		return newChecksum != metadata.checksum;
	}

	bool FileMetadataResourceHasChanged(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		FileMetadata metadata;
		if (resource->TryGetMetadata(metadata))
		{
			return ResourceModTimeHasChanged(resource, vdir, metadata) || ResourceContentHasChanged(resource, vdir, metadata);
		}
		else
			return false;
	}

};
