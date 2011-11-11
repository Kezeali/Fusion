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

#include "FusionCellCache.h"

#include <boost/dynamic_bitset.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/array.hpp>

namespace FusionEngine
{

	struct RegionFile;
	struct SmartArrayDevice
	{
		std::unique_ptr<std::vector<char>> data;
		size_t position;

		SmartArrayDevice()
			: position(0)
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

		std::streamsize read(char_type* s, std::streamsize n)
		{
			using namespace std;
			streamsize amt = static_cast<streamsize>(data->size() - position);
			streamsize result = (min)(n, amt);
			if (result != 0)
			{
				std::copy( data->begin() + position, 
					data->begin() + position + result, 
					s );
				position += result;
				return result;
			}
			else
			{
				return -1; // EOF
			}
		}
		std::streamsize write(const char_type* s, std::streamsize n)
		{
			using namespace std;
			streamsize result = 0;
			if (position != data->size())
			{
				streamsize amt = 
					static_cast<streamsize>(data->size() - position);
				result = (min)(n, amt);
				std::copy(s, s + result, data->begin() + position);
				position += result;
			}
			if (result < n)
			{
				data->insert(data->end(), s, s + n);
				position = data->size();
			}
			return n;
		}
		boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
		{
			using namespace std;
			using namespace boost::iostreams;
			// Determine new value of pos_
			stream_offset next;
			if (way == ios_base::beg)
				next = off;
			else if (way == ios_base::cur)
				next = position + off;
			else if (way == ios_base::end)
				next = data->size() + off - 1;
			else
				throw ios_base::failure("bad seek direction");

			// Check for errors
			if (next < 0 || next >= data->size())
				throw ios_base::failure("bad seek offset");

			position = next;
			return position;
		}

	private:
		SmartArrayDevice(const SmartArrayDevice&)
		{}
	};

	struct CellBuffer : public SmartArrayDevice
	{
		CellBuffer(RegionFile* p)
			: parent(p),
			cellIndex(0, 0)
		{}
		CellBuffer(CellBuffer&& other)
			: SmartArrayDevice(std::move(other)),
			parent(other.parent),
			cellIndex(other.cellIndex)
		{
			other.parent = nullptr;
			other.cellIndex.first = 0;
			other.cellIndex.second = 0;
		}
		~CellBuffer();

		std::pair<int32_t, int32_t> cellIndex;

		RegionFile* parent;
	};

	typedef boost::iostreams::filtering_istream ArchiveIStream;
	typedef boost::iostreams::filtering_ostream ArchiveOStream;

	struct RegionFile
	{
		explicit RegionFile(std::string&& filename);

		struct DataLocation
		{
			uint32_t startingSector : 24;
			uint32_t sectorsAllocated : 8;

			bool is_valid() const { return end() != 0; }
			uint32_t end() const { return startingSector + sectorsAllocated; }
		};

		std::string filename;
		std::iostream file;
		// This could be an array (use "cell_x & m_RegionWidth-1" to convert from global coords to region-local coords):
		std::vector<DataLocation> cellDataLocations;
		boost::dynamic_bitset<> free_sectors;

		size_t region_width; // Number of cells in each direction that comprise this region

		std::unique_ptr<ArchiveIStream> getInputCellData(int32_t x, int32_t y);
		std::unique_ptr<ArchiveOStream> getOutputCellData(int32_t x, int32_t y);

		void write(const std::pair<int32_t, int32_t>& i, std::vector<char>& data);
		void write(size_t first_sector, const std::vector<char>& data);

		void setCellDataLocation(const std::pair<int32_t, int32_t>& i, uint32_t startSector, uint32_t sectorsUsed);
		const DataLocation& getCellDataLocation(const std::pair<int32_t, int32_t>& i);
	};

	class RegionCellCache : public CellCache
	{
	public:
		typedef Vector2T<uint32_t> CellCoord_t;

		//! Region-file based cell cache
		RegionCellCache(const std::string& cache_path = "/cache");

		RegionFile& CacheRegionFile(CellCoord_t& coord);

		std::unique_ptr<ArchiveIStream> GetCellStreamForReading(uint32_t cell_x, uint32_t cell_y);
		std::unique_ptr<ArchiveOStream> GetCellStreamForWriting(uint32_t cell_x, uint32_t cell_y);

	private:
		std::map<CellCoord_t, RegionFile> m_Cache;

		std::string m_CachePath;

	};

}

#endif
