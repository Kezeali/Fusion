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

#ifndef H_FusionRegionFile
#define H_FusionRegionFile

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionVector2.h"
#include "FusionHashable.h"
#include "FusionResourcePointer.h"

#include "FusionCellCache.h"

#include <array>
#include <functional>
#include <memory>
#include <unordered_map>

#include <ClanLib/Core/Math/rect.h>

#include <boost/dynamic_bitset.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>

namespace FusionEngine
{

	struct SmartArrayDevice
	{
		typedef std::vector<char> DataArray_t;
		std::shared_ptr<DataArray_t> data;
		std::streamsize position;

		SmartArrayDevice()
			: position(0),
			data(new DataArray_t())
		{}
		SmartArrayDevice(const SmartArrayDevice& other)
			: position(other.position),
			data(other.data)
		{}
		SmartArrayDevice(SmartArrayDevice&& other)
			: data(std::move(other.data)),
			position(other.position)
		{
			other.position = 0;
		}
		virtual ~SmartArrayDevice() {}

		typedef char char_type;
		typedef boost::iostreams::seekable_device_tag category;

		std::streamsize read(char_type* s, std::streamsize n);
		std::streamsize write(const char_type* s, std::streamsize n);
		boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);

		//struct impl
		//{
		//	DataArray_t data;
		//};

		//std::shared_ptr<impl> pimpl;
	};

	class RegionFile
	{
	public:
		RegionFile()
			: region_width(0)
		{}

		explicit RegionFile(const std::string& filename, size_t width);
		explicit RegionFile(std::unique_ptr<std::streambuf>&& file, size_t width);

		~RegionFile();

		void init();

		RegionFile(RegionFile&& other)
			: filename(std::move(other.filename)),
			filebuf(std::move(other.filebuf)),
			filedesc(std::move(other.filedesc)),
			file(std::move(other.file)),
			cellDataLocations(std::move(other.cellDataLocations)),
			free_sectors(std::move(other.free_sectors)),
			region_width(other.region_width)
		{
		}

		RegionFile& operator=(RegionFile&& other)
		{
			filename = std::move(other.filename);
			filebuf = std::move(other.filebuf);
			filedesc = std::move(other.filedesc);
			file = std::move(other.file);
			cellDataLocations = std::move(other.cellDataLocations);
			free_sectors = std::move(other.free_sectors);
			region_width = other.region_width;
			return *this;
		}

		struct DataLocation
		{
			uint32_t startingSector : 24;
			uint32_t sectorsAllocated : 8;

			DataLocation()
				: startingSector(0),
				sectorsAllocated(0)
			{}

			bool is_valid() const { return end() != 0; }
			uint32_t end() const { return startingSector + sectorsAllocated; }
		};

		static const size_t s_CellHeaderSize = sizeof(size_t) + sizeof(uint8_t); // length & version number
		static const size_t s_MaxSectors = 1024;
		static const size_t s_SectorSize = 4096;

		std::string filename;
		std::unique_ptr<std::streambuf> filebuf;
		std::unique_ptr<boost::iostreams::file_descriptor> filedesc;
		std::unique_ptr<std::iostream> file;
		std::array<DataLocation, s_MaxSectors> cellDataLocations;
		boost::dynamic_bitset<> free_sectors;

		size_t region_width; // Number of cells in each direction that comprise this region

		std::unique_ptr<ArchiveIStream> getInputCellData(int32_t x, int32_t y, bool inflate = true);
		std::unique_ptr<ArchiveOStream> getOutputCellData(int32_t x, int32_t y);

		void write(const std::pair<int32_t, int32_t>& i, std::vector<char>& data);
		void write(size_t first_sector, const std::vector<char>& data);

		void setCellDataLocation(const std::pair<int32_t, int32_t>& i, uint32_t startSector, uint32_t sectorsUsed);
		const DataLocation& getCellDataLocation(const std::pair<int32_t, int32_t>& i);
		
	private:
		RegionFile(const RegionFile&) {}
		RegionFile& operator=(const RegionFile&) {}
	};

	namespace RegionMap
	{
		//! Region resource loader callback
		void LoadMapRegionResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data);
		//! Region resource unloader callback - writes data
		void UnloadMapRegionResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data);
	}

}

#endif