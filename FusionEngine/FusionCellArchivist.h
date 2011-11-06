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

#ifndef H_FusionCellArchivist
#define H_FusionCellArchivist

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionStreamingManager.h"

#include <array>
#include <boost/thread.hpp>
#include <ClanLib/Core/System/event.h>
#include <ClanLib/Core/IOData/iodevice.h>
#include <unordered_set>
#include <tbb/concurrent_queue.h>

#include "FusionGameMapLoader.h"
#include "FusionPhysFSIOStream.h"
#include "FusionEntitySerialisationUtils.h"

#include <boost/integer_traits.hpp>
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
			cellIndex(-1)
		{}
		CellBuffer(CellBuffer&& other)
			: SmartArrayDevice(std::move(other)),
			parent(other.parent),
			cellIndex(other.cellIndex)
		{
			other.parent = nullptr;
			other.cellIndex = -1;
		}
		~CellBuffer();

		size_t cellIndex;

		RegionFile* parent;
	};

	typedef boost::iostreams::filtering_istream ArchiveIStream;
	typedef boost::iostreams::filtering_ostream ArchiveOStream;

	struct RegionFile
	{
		std::string filename;
		std::iostream file;
		std::map<size_t, std::pair<size_t, size_t>> cellOffsets;
		std::unique_ptr<ArchiveIStream> getInputCellData(size_t i);
		std::unique_ptr<ArchiveOStream> getOutputCellData(size_t i);

		void write(std::vector<char>& data, size_t i);
	};

	//! CellArchiver implementaion
	class CachingCellArchiver : public CellArchiver
	{
	public:
		CachingCellArchiver(bool edit_mode);
		~CachingCellArchiver();

		InstancingSynchroniser* m_Instantiator;
		void SetSynchroniser(InstancingSynchroniser* instantiator);

		std::shared_ptr<GameMap> m_Map;
		void SetMap(const std::shared_ptr<GameMap>& map);

		void Update(ObjectID id, unsigned char* continuous, size_t con_length, unsigned char* occasional, size_t occ_length)
		{
			Update(id, std::vector<unsigned char>(continuous, continuous + con_length), std::vector<unsigned char>(occasional, occasional + occ_length));
		}

		void Update(ObjectID id, std::vector<unsigned char>&& continuous, std::vector<unsigned char>&& occasional);

		void Store(Cell* cell, size_t i);

		bool Retrieve(Cell* cell, size_t i);

		boost::thread m_Thread;

		void Start();

		void Stop();

		CL_IODevice GetFile(size_t cell_index, bool write) const;

		std::unique_ptr<ArchiveIStream> GetCellStreamForReading(size_t cell_index) const;
		std::unique_ptr<ArchiveOStream> GetCellStreamForWriting(size_t cell_index) const;

		CL_IODevice GetCellData(size_t index) const;

		size_t GetDataBegin() const;
		size_t GetDataEnd() const;

		EntityPtr Load(ICellStream& file, bool includes_id);

		size_t LoadEntitiesFromCellData(size_t index, Cell* cell, ICellStream& file, bool data_includes_ids);

		void Run();

		bool m_EditMode;
		bool m_Running;

		size_t m_BeginIndex;
		size_t m_EndIndex;

		// Loaded cache files
		std::map<size_t, RegionFile> m_Cache;

		typedef Vector2T<size_t> CellCoord_t;

		std::array<CellCoord_t, boost::integer_traits<PlayerID>::const_max> m_EntityLocations;

		std::unordered_set<size_t> m_SynchLoaded;

		//boost::mutex m_WriteQueueMutex;
		//boost::mutex m_ReadQueueMutex;/*std::queue*/
		tbb::concurrent_queue<std::tuple<Cell*, size_t>> m_WriteQueue;
		tbb::concurrent_queue<std::tuple<Cell*, size_t>> m_ReadQueue;

		tbb::concurrent_queue<std::tuple<ObjectID, std::vector<unsigned char>, std::vector<unsigned char>>> m_ObjectUpdateQueue;

		CL_Event m_NewData;
		CL_Event m_Quit;
	};

}

#endif
