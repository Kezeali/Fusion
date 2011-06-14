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

#ifndef H_FusionSharedPropertyTypes
#define H_FusionSharedPropertyTypes

#if _MSC_VER > 1000
#pragma once
#endif

#include <boost/mpl/vector.hpp>
#include <boost/mpl/index_of.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/variant.hpp>
#include <ClanLib/Display/2D/color.h>
#include "FusionVector2.h"

#define PROPERTY_TYPES_SEQ \
	(bool)\
	(int8_t)(int16_t)(int32_t)(int64_t)\
	(uint8_t)(uint16_t)(uint32_t)(uint64_t)\
	(float)(double)\
	(std::string)(FusionEngine::Vector2)(CL_Colorf)

namespace FusionEngine
{

	typedef boost::mpl::vector<BOOST_PP_SEQ_ENUM(PROPERTY_TYPES_SEQ)> PropertyTypes;
	typedef boost::make_variant_over<PropertyTypes>::type PropertyVariant;

	namespace Properties
	{
		template <typename T>
		static int GetTypeIndex()
		{
			return boost::mpl::index_of<PropertyTypes, T>::type();
		}
	}

}

#endif
