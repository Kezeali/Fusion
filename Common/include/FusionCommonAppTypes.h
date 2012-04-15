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

#ifndef H_FusionCommonAppTypes
#define H_FusionCommonAppTypes

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionAppType.h"

#include <boost/mpl/end.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/list_c.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/string.hpp>
#include <cstdint>

#include "FusionVectorTypes.h"

#include <angelscript.h>
#include <ClanLib/Display/2D/color.h>
#include <ClanLib/Core/Math/rect.h>

namespace FusionEngine
{

	namespace Scripting
	{

		typedef boost::mpl::list<bool, std::int8_t, std::int16_t, std::int32_t, std::int64_t, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t> IntTypes;
		typedef boost::mpl::list<float, double> FloatTypes;
		typedef boost::mpl::list<std::string, Vector2, CL_Colorf, CL_Rectf> AppTypes;

		typedef boost::mpl::insert_range<IntTypes,
			boost::mpl::end<IntTypes>::type,
			FloatTypes>::type FundimentalTypes;

		typedef boost::mpl::insert_range<FundimentalTypes,
			boost::mpl::end<FundimentalTypes>::type,
			AppTypes>::type CommonAppTypes;

		void RegisterCommonAppTypes(asIScriptEngine* engine);

	}

}

#endif
