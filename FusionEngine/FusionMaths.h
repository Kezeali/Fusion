/*
*  Copyright (c) 2010 Fusion Project Team
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

#ifndef Header_FusionMaths
#define Header_FusionMaths

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionAssert.h"

namespace FusionEngine
{
	namespace Maths
	{

		template <typename T>
		inline T Clamp(T value, T lower_bound, T upper_bound)
		{
			FSN_ASSERT(lower_bound < upper_bound);
			if (value < lower_bound)
				return lower_bound;
			else if (value > upper_bound)
				return upper_bound;
			else
				return value;
		}

		//template <typename T>
		//inline T& Clamp(T& value, T& lower_bound, T& upper_bound)
		//{
		//	FSN_ASSERT(lower_bound < upper_bound);
		//	return (value < lower_bound) ? lower_bound : (value > upper_bound) ? upper_bound : value;
		//}

		template <typename T>
		inline void ClampThis(T& value, T lower_bound, T upper_bound)
		{
			FSN_ASSERT(lower_bound < upper_bound);
			value = (value < lower_bound) ? lower_bound : (value > upper_bound) ? upper_bound : value;
		}

	}
}

#endif