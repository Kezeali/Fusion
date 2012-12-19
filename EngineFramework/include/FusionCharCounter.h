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

#ifndef H_FusionCharCounter
#define H_FusionCharCounter

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionCellStreamTypes.h"

#include <boost/iostreams/categories.hpp>
#include <memory>
#include <iostream>

namespace FusionEngine
{

	//! boost::iostreams filter that counts bytes written
	struct CharCounter
	{
		typedef char char_type;
		typedef boost::iostreams::multichar_output_filter_tag category;

		std::shared_ptr<std::streamsize> totalWritten;

		CharCounter()
			: totalWritten(new std::streamsize(0))
		{}

		std::streamsize count() const { return *totalWritten; }

		template <typename Sink>
		std::streamsize write(Sink& snk, const char_type* s, std::streamsize n)
		{
			*totalWritten += n;
			bio::write(snk, s, n);
			return n;
		}
	};

}

#endif
