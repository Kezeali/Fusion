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

#ifndef H_FusionMapLoader
#define H_FusionMapLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionVector2.h"
#include "FusionHashable.h"

#include "FusionCellCache.h"

#include <boost/dynamic_bitset.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/array.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>

namespace FusionEngine
{

	class RegionFile;
	struct SmartArrayDevice
	{
		std::shared_ptr<std::vector<char>> data;
		std::streamsize position;

		SmartArrayDevice()
			: position(0),
			data(new std::vector<char>())
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
		//	std::vector<char> data;
		//};

		//std::shared_ptr<impl> pimpl;
	};

	struct CellBuffer : public SmartArrayDevice
	{
		CellBuffer(RegionFile* p)
			: pimpl(new cell_impl(p, data))
		{}
		CellBuffer(const CellBuffer& other)
			: SmartArrayDevice(other),
			pimpl(other.pimpl)
		{}
		CellBuffer(CellBuffer&& other)
			: SmartArrayDevice(std::move(other)),
			pimpl(std::move(other.pimpl))
		{}

		struct cell_impl// : public SmartArrayDevice::impl
		{
			std::pair<int32_t, int32_t> cellIndex;
			RegionFile* parent;
			std::shared_ptr<std::vector<char>> data;
			cell_impl(RegionFile* p, std::shared_ptr<std::vector<char>> d)
				: parent(p),
				data(d),
				cellIndex(0, 0)
			{}
			~cell_impl();
		private:
			cell_impl(const cell_impl& other) {}
		};

		std::shared_ptr<cell_impl> pimpl;
	};

	class RegionFile
	{
	public:
		RegionFile()
			: region_width(0)
		{}

		explicit RegionFile(const std::string& filename, size_t width = 16);
		explicit RegionFile(std::unique_ptr<std::streambuf>&& file, size_t width = 16);

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

			bool is_valid() const { return end() != 0; }
			uint32_t end() const { return startingSector + sectorsAllocated; }
		};

		std::string filename;
		std::unique_ptr<std::streambuf> filebuf;
		std::unique_ptr<boost::iostreams::file_descriptor> filedesc;
		std::unique_ptr<std::iostream> file;
		// This could be a std::array (its a static array in the minecraft impl)
		std::vector<DataLocation> cellDataLocations;
		boost::dynamic_bitset<> free_sectors;

		size_t region_width; // Number of cells in each direction that comprise this region

		std::unique_ptr<ArchiveIStream> getInputCellData(int32_t x, int32_t y);
		std::unique_ptr<ArchiveOStream> getOutputCellData(int32_t x, int32_t y);

		void write(const std::pair<int32_t, int32_t>& i, std::vector<char>& data);
		void write(size_t first_sector, const std::vector<char>& data);

		void setCellDataLocation(const std::pair<int32_t, int32_t>& i, uint32_t startSector, uint32_t sectorsUsed);
		const DataLocation& getCellDataLocation(const std::pair<int32_t, int32_t>& i);
		
	private:
		RegionFile(const RegionFile&) {}
		RegionFile& operator=(const RegionFile&) {}
	};

	class RegionCellCache : public CellCache
	{
	public:
		typedef Vector2T<uint32_t> CellCoord_t;

		//! Region-file based cell cache
		RegionCellCache(const std::string& cache_path);

		RegionFile& CreateRegionFile(CellCoord_t& coord);
		RegionFile* GetRegionFile(CellCoord_t& coord, bool create);

		std::unique_ptr<ArchiveIStream> GetCellStreamForReading(int32_t cell_x, int32_t cell_y);
		std::unique_ptr<ArchiveOStream> GetCellStreamForWriting(int32_t cell_x, int32_t cell_y);

	private:
		std::unordered_map<CellCoord_t, RegionFile, boost::hash<CellCoord_t>> m_Cache;
		std::list<CellCoord_t> m_CacheImportance;
		size_t m_MaxLoadedFiles;

		std::string m_CachePath;

		size_t m_RegionSize;

	};

}

#endif
