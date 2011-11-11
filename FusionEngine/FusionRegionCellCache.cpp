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

#include "FusionStableHeaders.h"

#include "FusionRegionCellCache.h"

#include "FusionBinaryStream.h"
#include "FusionLogger.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIOStream.h"

namespace FusionEngine
{

	static const size_t s_CellHeaderSize = sizeof(size_t) + sizeof(uint8_t); // length & version number
	static const size_t s_MaxSectors = 1024;
	static const size_t s_SectorSize = 4096;
	static const uint8_t s_CellDataVersion = 1;

	static std::array<char, s_SectorSize> s_EmptySectorData;
	
	CellBuffer::~CellBuffer()
	{
		FSN_ASSERT(parent);
		if (data)
			parent->write(cellIndex, *data);
	}

	static inline size_t fileLength(std::iostream& file)
	{
		file.seekg(0, std::ios::end);
		auto len = file.tellg();
		file.seekg(0);
		return len;
	}

	RegionFile::RegionFile(std::string&& filename)
		: filename(std::move(filename)),
		file(IO::PhysFSStream(filename, IO::Read)),
		cellDataLocations(s_MaxSectors) // Hmm
	{
		try
		{
			IO::Streams::CellStreamWriter writer(&file);

			bool new_file = false;
			auto length = fileLength(file);

			if (length < s_SectorSize)
			{
				// New file: write the chunk offset table
				DataLocation nothing; nothing.startingSector = 0; nothing.sectorsAllocated = 0;
				for (int i = 0; i < s_MaxSectors; ++i)
					writer.Write(nothing);

				//sizeDelta += s_SectorSize;

				new_file = true;
			}

			if ((length & 0xFFF) != 0)
			{
				// The file size is not a multiple of 4KB, grow it
				for (int i = 0; i < (length & 0xfff); ++i)
					file.put(0);

				length = fileLength(file);
				FSN_ASSERT(length & 0xFFF == 0);
			}

			// Set up the available-sector map
			auto numSectors = length / s_SectorSize;
			free_sectors.resize(numSectors, true);

			free_sectors.set(0, false); // chunk offset table

			if (!new_file)
			{
				IO::Streams::CellStreamReader reader(&file);

				file.seekg(0);
				for (size_t i = 0; i < s_MaxSectors; ++i)
				{
					auto location = reader.ReadValue<DataLocation>();
					cellDataLocations[i] = location;
					if (location.is_valid() && location.end() <= free_sectors.size())
					{
						const auto endSector = location.end();
						for (size_t sectorNum = location.startingSector; sectorNum < endSector; ++sectorNum)
						{
							free_sectors.set(sectorNum, false);
						}
					}
				}
			}
		}
		catch (std::ios::failure& e)
		{
			AddLogEntry(e.what(), LOG_CRITICAL);
		}
	}

	std::unique_ptr<ArchiveIStream> RegionFile::getInputCellData(int32_t x, int32_t y)
	{
		namespace io = boost::iostreams;
		// TODO: sanity checks

		std::pair<size_t, size_t> location(x, y);

		const auto& locationData = getCellDataLocation(location);

		const auto firstSector = locationData.startingSector;
		const auto numSectors = locationData.sectorsAllocated;

		const auto dataBegin = firstSector * s_SectorSize;

		file.seekg(dataBegin);

		// Read the header
		IO::Streams::CellStreamReader reader(&file);
		const auto dataLength = reader.ReadValue<size_t>() - sizeof(uint8_t); // - sizeof(version number)

		if (dataLength > s_SectorSize * numSectors)
		{
			std::stringstream str; str << location.first << ", " << location.second;
			AddLogEntry("Region file is corrupt: data length for cell [" + str.str() + "] is inconsistent with sectors used", LOG_CRITICAL);
			return std::unique_ptr<ArchiveIStream>();
		}

		const auto dataVersion = reader.ReadValue<uint8_t>();

		SmartArrayDevice device;

		device.data->resize(dataLength);
		file.read(device.data->data(), dataLength);

		auto stream = std::unique_ptr<ArchiveIStream>(new io::filtering_istream());
		stream->push(io::zlib_decompressor());
		stream->push(boost::ref(device));
		return stream;
	}

	std::unique_ptr<ArchiveOStream> RegionFile::getOutputCellData(int32_t x, int32_t y)
	{
		namespace io = boost::iostreams;
		// TODO: sanity checks

		std::pair<size_t, size_t> location(x, y);

		const auto& locationData = getCellDataLocation(location);

		const auto firstSector = locationData.startingSector;
		const auto numSectors = locationData.sectorsAllocated;

		IO::Streams::CellStreamReader reader(&file);
		const auto dataLength = reader.ReadValue<size_t>();

		CellBuffer device(this);

		device.cellIndex = location;
		device.data->reserve(dataLength);

		auto stream = std::unique_ptr<ArchiveOStream>(new io::filtering_ostream());
		stream->push(io::zlib_compressor());
		stream->push(boost::ref(device));
		return stream;
	}

	void RegionFile::write(const std::pair<int32_t, int32_t>& i, std::vector<char>& data)
	{
		const auto length = data.size();

		const auto& dataLocation = getCellDataLocation(i);
		auto sectorNumber = dataLocation.startingSector;
		auto sectorsAllocated = dataLocation.sectorsAllocated;
		auto sectorsNeeded = (length + s_CellHeaderSize) / s_SectorSize + 1;

		if (sectorsNeeded > 256)
		{
			std::stringstream str; str << i.first << ", " << i.second;
			FSN_EXCEPT(FileSystemException, "Too much data for cell [" + str.str() + "] in region " + filename);
		}

		if (sectorNumber != 0 && sectorsAllocated == sectorsNeeded)
		{
			write(sectorNumber, data);
		}
		else // Allocate new sectors
		{
			for (size_t i = 0; i < sectorsAllocated; ++i)
				free_sectors.set(sectorNumber + i, true);

			// Scan for a run of free sectors that will fit the cell data
			auto runStart = free_sectors.find_first();
			size_t runLength = 0;
			if (runStart != boost::dynamic_bitset<>::npos)
			{
				for (auto i = runStart; i < free_sectors.size(); ++i)
				{
					if (runLength != 0)
					{
						if (free_sectors.test(i))
							++runLength;
						else
							runLength = 0;
					}
					else if (free_sectors.test(i))
					{
						runStart = i;
						runLength = 1;
					}
					if (runLength >= sectorsNeeded)
						break;
				}
			}

			if (runLength >= sectorsNeeded) // Free space found
			{
				sectorNumber = runStart;
				setCellDataLocation(i, sectorNumber, sectorsNeeded);
				// Mark these sectors used
				for (size_t i = 0; i < sectorsAllocated; ++i)
					free_sectors.set(sectorNumber + i, false);
				
				write(sectorNumber, data);
			}
			else // No free space found, grow the file
			{
				sectorNumber = free_sectors.size();
				free_sectors.resize(free_sectors.size() + sectorsNeeded, false);

				file.seekp(0, std::ios::end);
				for (size_t i = 0; i < sectorsNeeded; ++i)
				{
					file.write(s_EmptySectorData.data(), s_SectorSize);
				}
				
				write(sectorNumber, data);
				setCellDataLocation(i, sectorNumber, sectorsNeeded);
			}
		}
	}

	void RegionFile::write(size_t sector_number, const std::vector<char>& data)
	{
		IO::Streams::CellStreamWriter writer(&file);

		std::streampos streamPos(sector_number * s_SectorSize);
		file.seekp(streamPos);

		writer.WriteAs<size_t>(data.size() + sizeof(uint8_t)); // The length written includes the version number (written below)
		writer.WriteAs<uint8_t>(s_CellDataVersion);
		file.write(data.data(), data.size());
	}

	void RegionFile::setCellDataLocation(const std::pair<int32_t, int32_t>& i, uint32_t startSector, uint32_t sectorsUsed)
	{
		// Make sure the values given fit within the space available in the bitfield
		FSN_ASSERT(startSector < (1 << 24));
		FSN_ASSERT(sectorsUsed < 256);

		const int32_t x = i.first & 31, y = i.second & 31;

		auto& data = cellDataLocations[x + y * region_width];
		data.startingSector = startSector;
		data.sectorsAllocated = sectorsUsed;

		IO::Streams::CellStreamWriter writer(&file);

		file.seekp(i.first + i.second * region_width * sizeof(DataLocation));
		writer.Write(data);
	}

	const RegionFile::DataLocation& RegionFile::getCellDataLocation(const std::pair<int32_t, int32_t>& i)
	{
		const size_t x = i.first & 31, y = i.second & 31;

		FSN_ASSERT(x + y * region_width < cellDataLocations.size());
		return cellDataLocations[x + y * region_width];
	}

	RegionCellCache::RegionCellCache(const std::string& cache_path)
		: m_CachePath(cache_path)
	{
	}

}
