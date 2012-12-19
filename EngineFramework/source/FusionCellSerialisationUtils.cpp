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

#include "FusionCellSerialisationUtils.h"

#include "FusionBinaryStream.h"
#include "FusionCharCounter.h"
#include "FusionEntitySerialisationUtils.h"

#include <boost/iostreams/filtering_stream.hpp>

namespace bio = boost::iostreams;

namespace FusionEngine
{
	namespace CellSerialisationUtils
	{

		std::vector<std::tuple<ObjectID, std::streamoff, std::streamsize>> WriteCellData(std::ostream& file_param, const Cell::CellEntryMap& entities, const bool synched, const EntitySerialisationUtils::SerialisedDataStyle data_style)
		{
			using namespace EntitySerialisationUtils;

			size_t expectedNumEntries = 0;
			std::for_each(entities.begin(), entities.end(), [&](const Cell::CellEntryMap::value_type& obj)
			{
				if (obj.first->IsSyncedEntity() == synched)
					++expectedNumEntries;
			});

			CharCounter counter;
			bio::filtering_ostream file;
			file.push(counter, 0);
			file.push(file_param);

			IO::Streams::CellStreamWriter writer(&file);

			writer.Write(expectedNumEntries);

			std::vector<std::tuple<ObjectID, std::streamoff, std::streamsize>> dataPositions;

			//std::streamoff headerPos = sizeof(expectedNumEntries);
			if (synched)
			{
				// Leave some space for the header data
				//const std::vector<char> headerSpace(expectedNumEntries * (sizeof(ObjectID) /*+ sizeof(std::streamoff)*/));
				//file.write(headerSpace.data(), headerSpace.size());

				for (auto it = entities.cbegin(), end = entities.cend(); it != end; ++it)
				{
					const bool entSynched = it->first->IsSyncedEntity();
					if (entSynched)
						writer.Write(it->first->GetID());
				}

				dataPositions.reserve(expectedNumEntries);
			}

			std::streamoff beginPos;
			for (auto it = entities.cbegin(), end = entities.cend(); it != end; ++it)
			{
				const bool entSynched = it->first->IsSyncedEntity();
				if (entSynched == synched)
				{
					if (entSynched)
					{
						beginPos = counter.count();
					}

					SaveEntity(file, it->first, false, data_style);

					if (entSynched)
					{
						auto endPos = counter.count();
						std::streamsize length = endPos - beginPos;
						dataPositions.push_back(std::make_tuple(it->first->GetID(), beginPos, length));
					}

					FSN_ASSERT(expectedNumEntries-- > 0); // Confirm the number of synched / unsynched entities expected
				}
			}

			return dataPositions;
		}

		std::pair<size_t, std::vector<ObjectID>> ReadCellIntro(const CellCoord_t& coord, ICellStream& file, bool data_includes_ids, const EntitySerialisationUtils::SerialisedDataStyle)
		{
			size_t numEntries;
			file.read(reinterpret_cast<char*>(&numEntries), sizeof(size_t));

			FSN_ASSERT_MSG(numEntries < 65535, "Probably invalid data: entry count is implausible");

			std::vector<ObjectID> ids;

			if (data_includes_ids)
			{
				IO::Streams::CellStreamReader reader(&file);
				for (size_t i = 0; i < numEntries; ++i)
				{
					ObjectID id;
					reader.Read(id);
					ids.push_back(id);
				}
			}

			return std::make_pair(numEntries, ids);
		}

	}
}
