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

#ifndef H_FusionCellSerialisationUtils
#define H_FusionCellSerialisationUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntitySerialisationUtils.h"
#include "FusionCell.h"
#include "FusionCellStreamTypes.h"
#include "FusionTypes.h"

namespace FusionEngine
{

	typedef Vector2T<int32_t> CellCoord_t;

	typedef std::vector<EntityPtr> CellEntryContainer;

	namespace CellSerialisationUtils
	{
		std::vector<std::tuple<ObjectID, std::streamoff, std::streamsize>> WriteCellData(std::ostream& file_param, const Cell::CellEntryMap& entities, const bool synched, const EntitySerialisationUtils::SerialisedDataStyle data_style);

		std::pair<size_t, std::vector<ObjectID>> ReadCellIntro(const CellCoord_t& coord, ICellStream& file, const bool data_includes_ids, const EntitySerialisationUtils::SerialisedDataStyle data_style);
	}

}

#endif
