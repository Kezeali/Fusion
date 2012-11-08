/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#include "FusionRegionCellCache.h"

#include "FusionBinaryStream.h"
#include "FusionLogger.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIOStream.h"
#include "FusionResourceManager.h"

#include "FusionResource.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/signals2/connection.hpp>

namespace io = boost::iostreams;

namespace FusionEngine
{
	static const uint8_t s_CellDataVersion = 1;

	static std::array<char, RegionFile::s_SectorSize> s_EmptySectorData;

	struct CellBuffer : public SmartArrayDevice
	{
		CellBuffer(RegionFile* p)
			: pimpl(new impl(p, data))
		{}
		CellBuffer(const CellBuffer& other)
			: SmartArrayDevice(other),
			pimpl(other.pimpl)
		{}
		CellBuffer(CellBuffer&& other)
			: SmartArrayDevice(std::move(other)),
			pimpl(std::move(other.pimpl))
		{}

		struct impl
		{
			std::pair<int32_t, int32_t> cellIndex;
			RegionFile* parent;
			std::shared_ptr<DataArray_t> data;
			impl(RegionFile* p, std::shared_ptr<DataArray_t> d)
				: parent(p),
				data(d),
				cellIndex(0, 0)
			{}
			~impl();
		private:
			impl(const impl& other) {}
		};

		std::shared_ptr<impl> pimpl;
	};

	// CellBuffer employed by RegionCellCache to support scheduled asynchronous writing
	struct BureaucraticCellBuffer : public SmartArrayDevice
	{
		BureaucraticCellBuffer(RegionCellCache* cellCache)
			: pimpl(new impl(cellCache, data))
		{}
		BureaucraticCellBuffer(const BureaucraticCellBuffer& other)
			: SmartArrayDevice(other),
			pimpl(other.pimpl)
		{}
		BureaucraticCellBuffer(BureaucraticCellBuffer&& other)
			: SmartArrayDevice(std::move(other)),
			pimpl(std::move(other.pimpl))
		{}

		struct impl
		{
			std::pair<int32_t, int32_t> cellCoords;
			RegionCellCache* parent;
			std::shared_ptr<DataArray_t> data;
			impl(RegionCellCache* p, std::shared_ptr<DataArray_t> d)
				: parent(p),
				data(d),
				cellCoords(0, 0)
			{}
			~impl();
		private:
			impl(const impl& other) {}
		};

		std::shared_ptr<impl> pimpl;
	};

	std::streamsize SmartArrayDevice::read(char_type* s, std::streamsize n)
	{
		using namespace std;
		streamsize amt = static_cast<streamsize>(data->size() - position);
		streamsize result = (min)(n, amt);
		if (result != 0)
		{
			std::copy( data->begin() + /*DataArray_t::difference_type*/(position), 
				data->begin() + /*DataArray_t::difference_type*/(position + result), 
				s );
			position += result;
			return result;
		}
		else
		{
			return -1; // EOF
		}
	}
	std::streamsize SmartArrayDevice::write(const char_type* s, std::streamsize n)
	{
		using namespace std;
		streamsize result = 0;
		if (position != data->size())
		{
			streamsize amt = 
				static_cast<streamsize>(data->size() - position);
			result = (min)(n, amt);
			std::copy(s, s + result, data->begin() + /*DataArray_t::difference_type*/(position));
			position += result;
		}
		if (result < n)
		{
			data->insert(data->end(), s, s + n);
			position = data->size();
		}
		return n;
	}
	boost::iostreams::stream_offset SmartArrayDevice::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
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
			next = data->size() + off;
		else
			throw ios_base::failure("bad seek direction");

		// Check for errors
		if (next < 0 || next > data->size())
			throw ios_base::failure("bad seek offset");

		position = next;
		return position;
	}

	CellBuffer::impl::~impl()
	{
		FSN_ASSERT(parent);
		if (data)
		{
			parent->write(cellIndex, *data);
		}
	}

	BureaucraticCellBuffer::impl::~impl()
	{
		FSN_ASSERT(parent);
		if (data)
		{
			parent->WriteCellData(this->cellCoords, this->data);
		}
	}

	static inline size_t fileLength(std::istream& file)
	{
		if (file.eof())
			file.clear();
		file.seekg(0, std::ios::end);
		auto len = file.tellg();
		file.seekg(0, std::ios::beg);
		if (len >= 0)
			return (size_t)len;
		else
			return 0;
	}

	RegionFile::RegionFile(const std::string& filename, size_t width)
		: filename(filename),
		region_width(width)
	{
		std::unique_ptr<boost::iostreams::file_descriptor> filedesc;

		if (!boost::filesystem::exists(filename))
		{
			// Create the file (creating an in | out stream for a file that doesn't exist will fail)
			filedesc.reset(new io::file_descriptor(filename, std::ios::out | std::ios::binary));
			file.reset(new io::stream<io::file_descriptor>(*filedesc, 0));
			file.reset();
			filedesc.reset();
		}
		filedesc.reset(new io::file_descriptor(filename, std::ios::in | std::ios::out | std::ios::binary));
		file.reset(new io::stream<io::file_descriptor>(*filedesc, 0));

		loadRegionData(std::move(file));

		init();
	}

	RegionFile::RegionFile(std::unique_ptr<std::istream>&& read_only_file, size_t width)
		: region_width(width)
	{
		loadRegionData(std::move(read_only_file));

		init();
	}

	RegionFile::~RegionFile()
	{
		// Write the region to disk (if it wasn't loaded read-only)
		if (!filename.empty())
		{
			std::unique_ptr<boost::iostreams::file_descriptor> filedesc(new io::file_descriptor(filename, std::ios::out | std::ios::binary));
			std::unique_ptr<std::iostream> outFile;
			outFile.reset(new io::stream<io::file_descriptor>(*filedesc, 0));
			outFile->write(regionData->data(), regionData->size());
			outFile.reset();
			filedesc.reset();
		}
	}

	void RegionFile::loadRegionData(std::unique_ptr<std::istream> source)
	{
		const auto length = fileLength(*source);

		// Generate the container into which the region file will be loaded
		regionData = std::make_shared<SmartArrayDevice::DataArray_t>(length);
		regionData->reserve(RegionFile::s_SectorSize * RegionFile::s_MaxSectors);

		// Create the device to access the container as if it is a file
		SmartArrayDevice device(regionData);

		// Read the data from the source into the container
		source->read(regionData->data(), length);

		// Generate the stream so that the device can be read/written as an std::iostream
		auto stream = new io::filtering_stream<SmartArrayDevice::category>();
		stream->push(device);

		FSN_ASSERT(fileLength(*stream) == length);

		// Pass the stream to the file smart pointer
		file = std::unique_ptr<std::iostream>(std::move(stream));
	}

	void RegionFile::init()
	{
		try
		{
			if (!(*file))
				FSN_EXCEPT(FileSystemException, "Failed to open region file: " + filename);

			IO::Streams::CellStreamWriter writer(file.get());

			bool new_file = false;
			auto length = fileLength(*file);
			file->clear();

			file->exceptions(std::ios::failbit);

			if (length < s_SectorSize)
			{
				// New file: write the chunk offset table
				DataLocation nothing; nothing.startingSector = 0; nothing.sectorsAllocated = 0;
				for (int i = 0; i < s_MaxSectors; ++i)
					writer.Write(nothing);

				file->flush();

				//sizeDelta += s_SectorSize;

				length = fileLength(*file);

				new_file = true;
			}

			if ((length & 0xFFF) != 0)
			{
				// The file size is not a multiple of 4KB, grow it
				for (size_t i = 0; i < (length & 0xfff); ++i)
					file->put(0);

				length = fileLength(*file);
				FSN_ASSERT((length & 0xFFF) == 0);
			}

			// Set up the available-sector map
			auto numSectors = std::max<size_t>(length / s_SectorSize, 1);
			free_sectors.resize(numSectors, true);

			free_sectors.set(0, false); // sector zero is the chunk offset table (thus not free from the beginning)

			if (!new_file)
			{
				IO::Streams::CellStreamReader reader(file.get());

				file->seekg(0);
				for (size_t i = 0; i < s_MaxSectors; ++i)
				{
					DataLocation location;
					if (reader.Read(location))
					{
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
					else
					{
						FSN_EXCEPT(FileTypeException, "Failed to read sector data from region file header");
					}
				}
			}
		}
		catch (std::ios::failure& e)
		{
			AddLogEntry(e.what(), LOG_CRITICAL);
			FSN_EXCEPT(FileSystemException, "Failed to load region file");
		}
	}

	std::unique_ptr<ArchiveIStream> RegionFile::getInputCellData(int32_t x, int32_t y, bool inflate)
	{
		namespace io = boost::iostreams;
		// TODO: sanity checks

		auto location = std::make_pair(x, y);

		const auto& locationData = getCellDataLocation(location);

		const auto firstSector = locationData.startingSector;
		const auto numSectors = locationData.sectorsAllocated;

		if (numSectors == 0)
		{
			//std::stringstream str; str << location.first << ", " << location.second;
			//AddLogEntry("There was no cell data for [" + str.str() + "] in the cache", LOG_INFO);
			return std::unique_ptr<ArchiveIStream>();
		}

		FSN_ASSERT(locationData.is_valid());

		const auto dataBegin = firstSector * s_SectorSize;

		file->seekg(dataBegin);

		// Read the header
		IO::Streams::CellStreamReader reader(file.get());
		const auto dataLength = reader.ReadValue<size_t>() - sizeof(uint8_t); // - sizeof(version number)

		if (dataLength > s_SectorSize * numSectors)
		{
			std::stringstream str; str << location.first << ", " << location.second;
			AddLogEntry("Region file is corrupt: data length for cell [" + str.str() + "] is inconsistent with sectors used", LOG_CRITICAL);
			return std::unique_ptr<ArchiveIStream>();
		}

		const auto dataVersion = reader.ReadValue<uint8_t>();

		// When there is a changes to the cell data, add conversion for old data here
		if (dataVersion != s_CellDataVersion)
			FSN_EXCEPT(FileTypeException, "Incorrect cell data type");

		// Decompress the data
		//if (inflate)
		//{
		//	io::filtering_istream decompressor;
		//	decompressor.push(io::zlib_decompressor());
		//	decompressor.push(*file);
		//}

		SmartArrayDevice device;

		device.data->resize(dataLength);
		file->read(device.data->data(), dataLength);

		auto stream = std::unique_ptr<ArchiveIStream>(new io::filtering_istream());
		if (inflate)
			stream->push(io::zlib_decompressor());
		stream->push(device);
		return stream;
	}

	std::unique_ptr<ArchiveOStream> RegionFile::getOutputCellData(int32_t x, int32_t y)
	{
		namespace io = boost::iostreams;
		// TODO: sanity checks

		const auto location = std::make_pair(x, y);

		const auto& locationData = getCellDataLocation(location);

		// The data will probably be about as long as it was last time:
		const auto predictedDataLength = locationData.sectorsAllocated * s_SectorSize;

		CellBuffer device(this); // This is unsafe. TODO: inherit form enable_shared_from_this and pass a smart ptr here (not urgent because this method isn't used ATM of writing)

		device.pimpl->cellIndex = location;
		device.data->reserve(predictedDataLength);

		auto stream = std::unique_ptr<ArchiveOStream>(new io::filtering_ostream());
		stream->push(io::zlib_compressor());
		stream->push(device);
		return stream;
	}

	void RegionFile::write(const std::pair<int32_t, int32_t>& cell_index, std::vector<char>& data)
	{
		const auto length = data.size();

		const auto& dataLocation = getCellDataLocation(cell_index);
		auto sectorNumber = dataLocation.startingSector;
		auto sectorsAllocated = dataLocation.sectorsAllocated;
		auto sectorsNeeded = (length + s_CellHeaderSize) / s_SectorSize + 1;

		if (sectorsNeeded > 256)
		{
			std::stringstream str; str << cell_index.first << ", " << cell_index.second;
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
				setCellDataLocation(cell_index, sectorNumber, sectorsNeeded);
				// Mark these sectors used
				for (size_t i = 0; i < sectorsAllocated; ++i)
					free_sectors.set(sectorNumber + i, false);
				
				write(sectorNumber, data);
			}
			else // No free space found, grow the file
			{
				sectorNumber = free_sectors.size();
				free_sectors.resize(free_sectors.size() + sectorsNeeded, false);

				file->seekp(0, std::ios::end);
				for (size_t i = 0; i < sectorsNeeded; ++i)
				{
					file->write(s_EmptySectorData.data(), s_SectorSize);
				}
				
				write(sectorNumber, data);
				setCellDataLocation(cell_index, sectorNumber, sectorsNeeded);
			}
		}
	}

	void RegionFile::write(size_t sector_number, const std::vector<char>& data)
	{
		IO::Streams::CellStreamWriter writer(file.get());

		std::streampos streamPos(sector_number * s_SectorSize);
		file->seekp(streamPos);

		writer.WriteAs<size_t>(data.size() + sizeof(uint8_t)); // The length written includes the version number (written below)
		writer.WriteAs<uint8_t>(s_CellDataVersion);
		file->write(data.data(), data.size());
	}

	void RegionFile::setCellDataLocation(const std::pair<int32_t, int32_t>& cell_index, uint32_t startSector, uint32_t sectorsUsed)
	{
		// Make sure the values given fit within the space available to them in the bitfield
		FSN_ASSERT(startSector < (1 << 24));
		FSN_ASSERT(sectorsUsed < 256);

		const auto x = cell_index.first;
		const auto y = cell_index.second;

		FSN_ASSERT(x >= 0 && y >= 0);

		FSN_ASSERT(x < (int32_t)region_width);
		FSN_ASSERT(y < (int32_t)region_width);
		FSN_ASSERT(x + y * region_width < cellDataLocations.size());

		auto& data = cellDataLocations[x + y * region_width];
		data.startingSector = startSector;
		data.sectorsAllocated = sectorsUsed;

		//auto output = data;

		IO::Streams::CellStreamWriter writer(file.get());

		std::streampos pos((x + y * region_width) * sizeof(DataLocation));
		//file->seekp(x + y * region_width * sizeof(DataLocation), std::ios::beg);
		if (file->seekp(pos))
			writer.Write(data);
		const auto p = file->tellp();
		const auto expectedp = pos + std::streamoff(sizeof(data));
		FSN_ASSERT(file->tellp() == (pos + std::streamoff(sizeof(data))));
	}

	const RegionFile::DataLocation& RegionFile::getCellDataLocation(const std::pair<int32_t, int32_t>& cell_coords)
	{
		const auto x = cell_coords.first;
		const auto y = cell_coords.second;

		FSN_ASSERT(x < (int32_t)region_width);
		FSN_ASSERT(y < (int32_t)region_width);

		FSN_ASSERT(x + y * region_width < cellDataLocations.size());
		return cellDataLocations[x + y * region_width];
	}

	RegionCellCache::RegionCellCache(const std::string& cache_path, int32_t region_size)
		: m_CachePath(cache_path),
		m_RegionSize(region_size),
		m_MaxLoadedFiles(10),
		m_EditMode(false)
	{
		FSN_ASSERT(region_size > 0);

		FSN_ASSERT(region_size * region_size < RegionFile::s_MaxSectors);

		ResourceManager::getSingleton().AddResourceLoader(
			ResourceLoader("MapRegion" + m_CachePath, &RegionMap::LoadMapRegionResource, RegionMap::UnloadMapRegionResource, region_size));
	}

	void RegionCellCache::DropCache()
	{
		//CacheMutex_t::scoped_lock lock(m_CacheMutex);

		m_Cache.clear();
		m_CacheImportance.clear();
	}

	void RegionCellCache::SetupEditMode(bool enable, CL_Rect bounds)
	{
		m_EditMode = enable;
		m_Bounds = bounds;
	}

	RegionCellCache::RegionCoord_t RegionCellCache::cellToRegionCoord(int32_t* cell_x, int32_t* cell_y) const
	{
		RegionCoord_t regionCoord((int32_t)std::floor(*cell_x / (float)m_RegionSize), (int32_t)std::floor(*cell_y / (float)m_RegionSize));
		// Figure out the location of the cell relative to the origin (0,0) point of the region
		*cell_x = *cell_x - regionCoord.x * m_RegionSize, *cell_y = *cell_y - regionCoord.y * m_RegionSize;
		return regionCoord;
	}
	
	void RegionCellCache::ReloadRegionFile(const RegionLoadedCallback& loadedCallback, const RegionCellCache::RegionCoord_t& coord)
	{
		//CacheMutex_t::scoped_lock lock(m_CacheMutex);

		//m_Cache.erase(coord);
		m_Cache[coord].Release();
		GetRegionFile(loadedCallback, coord, true);
	}

	class RegionFileLoadedCallbackHandle
	{
	public:
		RegionFileLoadedCallbackHandle()
		{}
		RegionFileLoadedCallbackHandle(const boost::signals2::connection& connection)
			: m_Connection(connection)
		{}

		~RegionFileLoadedCallbackHandle()
		{
			m_Connection.disconnect();
		}
		boost::signals2::connection m_Connection;

		std::list<RegionCellCache::RegionLoadedCallback> m_OtherCallbacks;
	};

	void RegionCellCache::GetRegionFile(const RegionLoadedCallback& loadedCallback, const RegionCellCache::RegionCoord_t& coord, bool load_if_uncached)
	{
		auto entry = m_Cache.find(coord);
		if (entry == m_Cache.end())
		{
			if (load_if_uncached)
			{
				// Add a new cache entry
				std::stringstream str; str << coord.x << "." << coord.y;
				std::string filename = str.str();
				std::string filePath = m_CachePath + filename + ".celldata";

				CacheMutex_t::scoped_lock lock(m_CacheMutex);

				// Request the region file resource
				using namespace std::placeholders;
				CallbackHandles_t::accessor accessor;
				if (m_CallbackHandles.insert(accessor, coord))
				{
					AddLogEntry("cells_loaded", "** Requested " + filePath);
					FSN_ASSERT_MSG(!accessor->second.m_Connection.connected(), "What??!");
					accessor->second.m_Connection =
						ResourceManager::getSingleton().GetResource("MapRegion" + m_CachePath,
						filePath,
						std::bind(&RegionCellCache::OnRegionFileLoaded, this, _1, coord),
						-1000);
				}
				// Add the given callback to the end of the list for this region
				accessor->second.m_OtherCallbacks.push_back(loadedCallback);

				m_CacheImportance.push_back(coord);

				FSN_ASSERT(m_MaxLoadedFiles > 0);
				if (m_CallbackHandles.size() > m_MaxLoadedFiles)
				{
					AddLogEntry("cells_loaded", "** Dropped " + filePath);
					// Remove the least recently accessed file
					//m_Cache.erase(m_CacheImportance.front());
					m_Cache[m_CacheImportance.front()].Release();
					m_CallbackHandles.erase(m_CacheImportance.front());
					m_CacheImportance.pop_front();
				}
			}
			else
				loadedCallback(nullptr);
		}
		else
		{
			CacheMutex_t::scoped_lock lock(m_CacheMutex);
			// Make the existing entry more important
			m_CacheImportance.remove(coord);
			m_CacheImportance.push_back(coord);

			return loadedCallback(entry->second.Get());
		}
	}

	void RegionCellCache::OnRegionFileLoaded(ResourceDataPtr& resource, const RegionCoord_t& coord)
	{
		//CacheMutex_t::scoped_lock lock(m_CacheMutex);

		auto resourcePointer = m_Cache[coord] = ResourcePointer<RegionFile>(resource);
		RegionFile* regionFile = resourcePointer.Get();

		std::stringstream str; str << coord.x << "," << coord.y;
		AddLogEntry("cells_loaded", "<RegionFileLoaded [" + str.str() + "]>");

		// Fulfill all the requests for this region file
		CallbackHandles_t::const_accessor accessor;
		if (m_CallbackHandles.find(accessor, coord))
		{
			auto& callbackProxy = accessor->second;
			for (auto it = callbackProxy.m_OtherCallbacks.begin(); it != callbackProxy.m_OtherCallbacks.end(); ++it)
			{
				(*it)(regionFile);
			}
		}

		AddLogEntry("cells_loaded", "</RegionFileLoaded[" + str.str() + "]>");
	}

	void RegionCellCache::GetRawCellStreamForReading(const GotCellForReadingCallback& callback, int32_t cell_x, int32_t cell_y)
	{
		//CacheMutex_t::scoped_lock lock(m_CacheMutex);

		const auto regionCoord = cellToRegionCoord(&cell_x, &cell_y);

		GetRegionFile([callback, cell_x, cell_y](RegionFile* regionFile)
		{
			if (regionFile)
			{
				callback(regionFile->getInputCellData(cell_x, cell_y, false));
			}
		}, regionCoord, false);
	}

	void RegionCellCache::GetCellStreamForReading(const GotCellForReadingCallback& callback, int32_t cell_x, int32_t cell_y)
	{
		//CacheMutex_t::scoped_lock lock(m_CacheMutex);

		const auto regionCoord = cellToRegionCoord(&cell_x, &cell_y);

		GetRegionFile([callback, cell_x, cell_y](RegionFile* regionFile)
		{
			if (regionFile)
			{
				callback(regionFile->getInputCellData(cell_x, cell_y));
			}
		}, regionCoord, true);
	}

	std::unique_ptr<ArchiveOStream> RegionCellCache::GetCellStreamForWriting(int32_t cell_x, int32_t cell_y)
	{
		// Expand the bounds (edit mode)
		if (m_EditMode)
		{
			m_Bounds.left = std::min(m_Bounds.left, cell_x);
			m_Bounds.right = std::max(m_Bounds.right, cell_x);
			m_Bounds.top = std::min(m_Bounds.top, cell_y);
			m_Bounds.bottom = std::max(m_Bounds.bottom, cell_y);
		}

		const size_t predictedDataLength = RegionFile::s_SectorSize;

		BureaucraticCellBuffer device(this);

		// Note that this is the world-relative cell_x and y coords, not the region relative
		//  ones (that would be obtained using cellToRegionCoord()) because WriteCellData
		//  needs to use this info to retrieve the region file to write.
		device.pimpl->cellCoords = std::make_pair(cell_x, cell_y);
		device.data->reserve(predictedDataLength);

		auto stream = std::unique_ptr<ArchiveOStream>(new io::filtering_ostream());
		stream->push(io::zlib_compressor());
		stream->push(device);
		return stream;
	}

	void RegionCellCache::WriteCellData(std::pair<int32_t, int32_t> cellIndex, std::shared_ptr<SmartArrayDevice::DataArray_t> data)
	{
		//CacheMutex_t::scoped_lock lock(m_CacheMutex);

		const auto regionCoord = cellToRegionCoord(&cellIndex.first, &cellIndex.second);

		GetRegionFile([cellIndex, data](RegionFile* regionFile)
		{
			FSN_ASSERT(regionFile);
			regionFile->write(cellIndex, *data);
		}, regionCoord, true);
	}

	void RegionCellCache::Sustain()
	{
		ResourceManager::getSingleton().PauseUnload("MapRegion" + m_CachePath);
	}

	void RegionCellCache::EndSustain()
	{
		ResourceManager::getSingleton().ResumeUnload("MapRegion" + m_CachePath);
	}

	namespace RegionMap
	{
		void LoadMapRegionResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
		{
			if (resource->IsLoaded())
			{
				delete static_cast<RegionFile*>(resource->GetDataPtr());
			}

			try
			{
				auto regionSize = boost::any_cast<int32_t>(&user_data);
				if (regionSize)
				{
					RegionFile* regionFile = new RegionFile(resource->GetPath(), (size_t)(*regionSize));
					resource->SetDataPtr(regionFile);
					resource->setLoaded(true);
				}
			}
			catch (boost::filesystem::filesystem_error& ex)
			{
				FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded: " + std::string(ex.what()));
			}
			catch (Exception&)
			{
				throw;
			}
		}

		void UnloadMapRegionResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
		{
			if (resource->IsLoaded())
			{
				resource->setLoaded(false);
				delete static_cast<RegionFile*>(resource->GetDataPtr());
			}
			resource->SetDataPtr(nullptr);
		}
	}

}
