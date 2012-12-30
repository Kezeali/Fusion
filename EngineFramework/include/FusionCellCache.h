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

#ifndef H_FusionCellCache
#define H_FusionCellCache

#include "FusionPrerequisites.h"

#include <boost/iostreams/filtering_stream.hpp>

#include <memory>

#include <ClanLib/Core/Math/rect.h>

namespace FusionEngine
{
	typedef boost::iostreams::filtering_istream ArchiveIStream;
	typedef boost::iostreams::filtering_ostream ArchiveOStream;

	//! Interface to retrieve raw cell data
	class CellDataSource
	{
	public:
		virtual ~CellDataSource() {}

		typedef std::function<void (std::shared_ptr<ArchiveIStream>)> GotCellForReadingCallback;
		//typedef std::function<void (std::shared_ptr<ArchiveOStream>)> GotCellForWritingCallback;

		virtual void GetCellStreamForReading(const GotCellForReadingCallback& callback, int32_t cell_x, int32_t cell_y) = 0;
		virtual std::unique_ptr<ArchiveOStream> GetCellStreamForWriting(int32_t cell_x, int32_t cell_y) = 0;

		virtual void GetRawCellStreamForReading(const GotCellForReadingCallback& callback, int32_t cell_x, int32_t cell_y) = 0;
	};

}

#endif
