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

#ifndef H_FusionCellCache
#define H_FusionCellCache

#include "FusionPrerequisites.h"

#include <boost/iostreams/filtering_stream.hpp>

namespace FusionEngine
{
	typedef boost::iostreams::filtering_istream ArchiveIStream;
	typedef boost::iostreams::filtering_ostream ArchiveOStream;

	// TODO: rename to CellDataSource
	class CellCache
	{
	public:
		virtual ~CellCache() {}

		virtual std::unique_ptr<ArchiveIStream> GetCellStreamForReading(int32_t cell_x, int32_t cell_y) = 0;
		virtual std::unique_ptr<ArchiveOStream> GetCellStreamForWriting(int32_t cell_x, int32_t cell_y) = 0;

		//! Used when compiling a map from a cell cache
		virtual std::unique_ptr<ArchiveIStream> GetRawCellStreamForReading(int32_t cell_x, int32_t cell_y) = 0;
		virtual CL_Rect GetUsedBounds() const = 0;
	};

}

#endif