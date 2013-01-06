/*
*  Copyright (c) 2013 Fusion Project Team
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

#ifndef H_FusionInspectorTypeUtils
#define H_FusionInspectorTypeUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <ClanLib/Core/Math/origin.h>

#include <string>

//! Utils for converting to / from property types for which no GUI controls are available
namespace FusionEngine { namespace Inspectors { namespace TypeUtils
{

	inline clan::Origin StringToOrigin(const std::string str)
	{
		if (str == "Top-Left")
			return clan::origin_top_left;
		else if (str == "Top-Center")
			return clan::origin_top_center;
		else if (str == "Top-Right")
			return clan::origin_top_right;

		else if (str == "Center-Left")
			return clan::origin_center_left;
		else if (str == "Center")
			return clan::origin_center;
		else if (str == "Center-Right")
			return clan::origin_center_right;

		else if (str == "Bottom-Left")
			return clan::origin_bottom_left;
		else if (str == "Bottom-Center")
			return clan::origin_bottom_center;
		else if (str == "Bottom-Right")
			return clan::origin_bottom_right;

		else
			return clan::origin_center;
	}

	inline std::string OriginToString(const clan::Origin origin)
	{
		switch (origin)
		{
		case clan::origin_top_left: return "Top-Left";
		case clan::origin_top_center: return "Top-Center";
		case clan::origin_top_right: return "Top-Right";

		case clan::origin_center_left: return "Center-Left";
		case clan::origin_center: return "Center";
		case clan::origin_center_right: return "Center-Right";

		case clan::origin_bottom_left: return "Bottom-Left";
		case clan::origin_bottom_center: return "Bottom-Center";
		case clan::origin_bottom_right: return "Bottom-Right";
		default: return "Center";
		};
	}

} } }

#endif
