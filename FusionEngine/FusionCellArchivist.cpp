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

#include "FusionCellArchivist.h"

#include "FusionGameMapLoader.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionInstanceSynchroniser.h"
#include "FusionVirtualFileSource_PhysFS.h"

namespace FusionEngine
{

	
	CellBuffer::~CellBuffer()
	{
		FSN_ASSERT(parent);
		if (data)
			parent->write(*data, cellIndex);
	}

	std::unique_ptr<IStreamDevice> RegionFile::getInputCellData(size_t i)
	{
		namespace io = boost::iostreams;

		auto offsetEntry = cellOffsets.find(i);
		FSN_ASSERT(offsetEntry != cellOffsets.end());

		const auto& cellOffsetData = offsetEntry->second;
		const auto cellBegin = cellOffsetData.first;
		const auto cellLength = cellOffsetData.second;

		SmartArrayDevice device;

		device.data->resize(cellLength);
		file.get(device.data->data(), cellLength);

		auto stream = std::unique_ptr<IStreamDevice>(new io::filtering_istream());
		stream->push(io::zlib_decompressor());
		stream->push(boost::ref(device));
		return stream;
	}

	std::unique_ptr<OStreamDevice> RegionFile::getOutputCellData(size_t i)
	{
		namespace io = boost::iostreams;

		auto offsetEntry = cellOffsets.find(i);
		FSN_ASSERT(offsetEntry != cellOffsets.end());

		const auto& cellOffsetData = offsetEntry->second;
		const auto cellBegin = cellOffsetData.first;
		const auto cellLength = cellOffsetData.second;

		CellBuffer device(this);

		device.cellIndex = i;
		device.data->reserve(cellLength);

		auto stream = std::unique_ptr<OStreamDevice>(new io::filtering_ostream());
		stream->push(io::zlib_compressor());
		stream->push(boost::ref(device));
		return stream;
	}

	void RegionFile::write(std::vector<char>& data, size_t i)
	{
		auto offsetEntry = cellOffsets.find(i);
		FSN_ASSERT(offsetEntry != cellOffsets.end());

		const auto& cellOffsetData = offsetEntry->second;
		const auto cellBegin = cellOffsetData.first;
		const auto cellLength = cellOffsetData.second;
	}

	CachingCellArchiver::CachingCellArchiver(bool edit_mode)
		: m_NewData(false),
		m_Instantiator(nullptr),
		m_Running(false),
		m_EditMode(edit_mode),
		m_BeginIndex(std::numeric_limits<size_t>::max()),
		m_EndIndex(0)
	{
	}

	CachingCellArchiver::~CachingCellArchiver()
	{
		Stop();
	}

	void CachingCellArchiver::SetSynchroniser(InstancingSynchroniser* instantiator)
	{
		m_Instantiator = instantiator;
	}

	void CachingCellArchiver::SetMap(const std::shared_ptr<GameMap>& map)
	{
		m_Map = map;
	}

	void CachingCellArchiver::Update(ObjectID id, std::vector<unsigned char>&& continuous, std::vector<unsigned char>&& occasional)
	{
		m_ObjectUpdateQueue.push(std::make_tuple(id, std::move(continuous), std::move(occasional)));
		m_NewData.set();
	}

	void CachingCellArchiver::Store(Cell* cell, size_t i)
	{
		if (cell->waiting.fetch_and_store(Cell::Store) != Cell::Store && cell->loaded)
		{
			cell->AddHist("Enqueued Out");
			m_WriteQueue.push(std::make_tuple(cell, i));
			m_NewData.set();
		}
	}

	bool CachingCellArchiver::Retrieve(Cell* cell, size_t i)
	{
		if (cell->waiting.fetch_and_store(Cell::Retrieve) != Cell::Retrieve && !cell->loaded)
		{
			cell->AddHist("Enqueued In");
			m_ReadQueue.push(std::make_tuple(cell, i));
			m_NewData.set();
			return true;
		}
		return false;
	}

	void CachingCellArchiver::Start()
	{
		m_Running = true;

		m_Quit.reset();
		m_Thread = boost::thread(&CachingCellArchiver::Run, this);
#ifdef _WIN32
		SetThreadPriority(m_Thread.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
#endif
	}

	void CachingCellArchiver::Stop()
	{
		m_Quit.set();
		m_Thread.join();

		m_Running = false;
	}

	CL_IODevice CachingCellArchiver::GetFile(size_t cell_index, bool write) const
	{
		try
		{
			std::stringstream str; str << cell_index;
			std::string filename = "cache/" + str.str();
			if (write || PHYSFS_exists(filename.c_str()))
			{
				CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
				auto file = vdir.open_file(filename, write ? CL_File::create_always : CL_File::open_existing, write ? CL_File::access_write : CL_File::access_read);
				return file;
			}
			else return CL_IODevice();
		}
		catch (CL_Exception& ex)
		{
			SendToConsole(ex.what());
			return CL_IODevice();
		}
	}

	CL_IODevice CachingCellArchiver::GetCellData(size_t index) const
	{
		if (m_EditMode && !m_Running)
			return GetFile(index, false);
		else
			FSN_EXCEPT(InvalidArgumentException, "Can't access cell data while running");
	}

	size_t CachingCellArchiver::GetDataBegin() const
	{
		if (m_EditMode)
			return m_BeginIndex;
		else
			FSN_EXCEPT(InvalidArgumentException, "This function is only available in edit mode");
	}

	size_t CachingCellArchiver::GetDataEnd() const
	{
		if (m_EditMode)
			return m_EndIndex;
		else
			FSN_EXCEPT(InvalidArgumentException, "This function is only available in edit mode");
	}

	EntityPtr CachingCellArchiver::Load(CL_IODevice& file, bool includes_id)
	{
		return EntitySerialisationUtils::LoadEntity(file, includes_id, m_Instantiator->m_Factory, m_Instantiator->m_EntityManager, m_Instantiator);
	}

	void CachingCellArchiver::Run()
	{
		using namespace EntitySerialisationUtils;

		bool retrying = false;
		// TODO: make m_NewData not auto-reset
		while (CL_Event::wait(m_Quit, m_NewData, retrying ? 100 : -1) != 0)
		{
			std::list<std::tuple<Cell*, size_t>> writesToRetry;
			std::list<std::tuple<Cell*, size_t>> readsToRetry;

			// Read / write cell data
			{
				std::tuple<Cell*, size_t> toWrite;
				while (m_WriteQueue.try_pop(toWrite))
				{
					Cell*& cell = std::get<0>(toWrite);
					size_t& i = std::get<1>(toWrite);
					Cell::mutex_t::scoped_try_lock lock(cell->mutex);
					if (lock)
					{
						// Check active_entries since the Store request may be stale
						if ((m_EditMode || cell->active_entries == 0) && cell->waiting == Cell::Store)
						{
							try
							{
								FSN_ASSERT(cell->loaded == true); // Don't want to create an inaccurate cache (without entities from the map file)

								size_t numSynched = 0;
								size_t numPseudo = 0;
								std::for_each(cell->objects.begin(), cell->objects.end(), [&](const Cell::CellEntryMap::value_type& obj)
								{
									if (!obj.first->IsSyncedEntity())
										++numPseudo;
									else
										++numSynched;
								});

								auto write = [](CL_IODevice& file, const Cell* cell, size_t numEntries, const bool synched)
								{
									using namespace EntitySerialisationUtils;
									file.write(&numEntries, sizeof(size_t));
									for (auto it = cell->objects.cbegin(), end = cell->objects.cend(); it != end; ++it)
									{
										if (it->first->IsSyncedEntity() == synched)
										{
											SaveEntity(file, it->first, synched);
											FSN_ASSERT(numEntries-- > 0);
										}
									}
								};

								if (m_EditMode)
								{
									if (numSynched > 0 || numPseudo > 0)
									{
										auto file = GetFile(i, true);
										if (!file.is_null())
										{
											m_BeginIndex = std::min(i, m_BeginIndex);
											m_EndIndex = std::max(i, m_EndIndex);
											write(file, cell, numPseudo, false);
											write(file, cell, numSynched, true);
										}
										else
											FSN_EXCEPT(FileSystemException, "Failed to open file to dump edit-mode cache");
									}
								}
								else if (numSynched > 0)
								{
									auto file = GetFile(i, true);
									file.write(&numSynched, sizeof(size_t));
									for (auto it = cell->objects.cbegin(), end = cell->objects.cend(); it != end; ++it)
									{
										if (it->first->IsSyncedEntity())
										{
											SaveEntity(file, it->first, true);
											FSN_ASSERT(numSynched-- > 0);
										}
									}
								}

								cell->AddHist("Written and cleared", numSynched);

								//std::stringstream str; str << i;
								//SendToConsole("Cell " + str.str() + " streamed out");

								// TEMP
								if (!m_EditMode || cell->active_entries == 0)
								{
									cell->objects.clear();
									cell->loaded = false;
								}
							}
							catch (...)
							{
								std::stringstream str; str << i;
								SendToConsole("Exception streaming out cell " + str.str());
							}
						}
						else
						{
							//std::stringstream str; str << i;
							//SendToConsole("Cell still in use " + str.str());
							//writesToRetry.push_back(toWrite);
						}
						cell->waiting = Cell::Ready;
						//cell->mutex.unlock();
					}
					else
					{
						cell->AddHist("Cell locked (will retry write later)");
						std::stringstream str; str << i;
						SendToConsole("Retrying write on cell " + str.str());
						writesToRetry.push_back(toWrite);
					}
				}
			}
			{
				std::tuple<Cell*, size_t> toRead;
				while (m_ReadQueue.try_pop(toRead))
				{
					Cell*& cell = std::get<0>(toRead);
					size_t& i = std::get<1>(toRead);//, y = std::get<1>(toRead);
					Cell::mutex_t::scoped_try_lock lock(cell->mutex);
					if (lock)
					{
						// Make sure this cell hasn't been re-activated:
						if (cell->active_entries == 0 && cell->waiting == Cell::Retrieve)
						{
							try
							{
								auto file = GetFile(i, false);

								// Last param makes the method load synched entities from the map if the cache file isn't available:
								if (m_Map)
								{
									bool uncached = m_SynchLoaded.insert(i).second;
									m_Map->LoadCell(cell, i, uncached, m_Instantiator->m_Factory, m_Instantiator->m_EntityManager, m_Instantiator);
								}

								if (!file.is_null() && file.get_size() > 0)
								{
									auto load = [&](const bool synched)->size_t
									{
										size_t numEntries;
										file.read(&numEntries, sizeof(size_t));
										for (size_t n = 0; n < numEntries; ++n)
										{
											//auto& archivedEntity = *it;
											auto archivedEntity = Load(file, synched);

											Vector2 pos = archivedEntity->GetPosition();
											// TODO: Cell::Add(entity, CellEntry = def) rather than this bullshit
											CellEntry entry;
											entry.x = ToGameUnits(pos.x); entry.y = ToGameUnits(pos.y);

											archivedEntity->SetStreamingCellIndex(i);

											cell->objects.push_back(std::make_pair(archivedEntity, std::move(entry)));
										}
										return numEntries;
									};
									if (m_EditMode)
										load(false);
									size_t num = load(true);

									//std::stringstream str; str << i;
									//SendToConsole("Cell " + str.str() + " streamed in");

									cell->AddHist("Loaded", num);
									cell->loaded = true;
								}
								else
								{
									cell->loaded = true; // No data to load
									cell->AddHist("Loaded (no data)");
								}
							}
							catch (...)
							{
								//std::stringstream str; str << i;
								//SendToConsole("Exception streaming in cell " + str.str());
								//readsToRetry.push_back(toRead);
							}
						}
						cell->waiting = Cell::Ready;
						//cell->mutex.unlock();
					}
					else
					{
						cell->AddHist("Cell locked (will retry read later)");
						readsToRetry.push_back(toRead);
					}
				}
			}
			retrying = false;
			if (!writesToRetry.empty())
			{
				retrying = true;
				for (auto it = writesToRetry.begin(), end = writesToRetry.end(); it != end; ++it)
					m_WriteQueue.push(*it);
				writesToRetry.clear();
			}
			if (!readsToRetry.empty())
			{
				retrying = true;
				for (auto it = readsToRetry.begin(), end = readsToRetry.end(); it != end; ++it)
					m_ReadQueue.push(*it);
				readsToRetry.clear();
			}

			using namespace IO;

			{
				std::tuple<ObjectID, std::vector<unsigned char>, std::vector<unsigned char>> objectUpdateData;
				while (m_ObjectUpdateQueue.try_pop(objectUpdateData))
				{
					const ObjectID id = std::get<0>(objectUpdateData);
					auto& incommingConData = std::get<1>(objectUpdateData);
					auto& incommingOccData = std::get<2>(objectUpdateData);

					const auto loc = m_EntityLocations[id];

					//auto& inData = GetCellOutputStream(loc.x, loc.y);
					//auto& outData = GetCellInputStream(loc.x, loc.y);
					std::ifstream inData;
					std::ofstream outData;

					RakNet::BitStream iConDataStream(incommingConData.data(), incommingConData.size(), false);
					RakNet::BitStream iOccDataStream(incommingOccData.data(), incommingOccData.size(), false);

					MergeEntityData(inData, outData, iConDataStream, iOccDataStream);
				}
			}

			/*std::stringstream str;
			str << m_Archived.size();
			SendToConsole(str.str() + " cells archived");*/
		}
	}

}
