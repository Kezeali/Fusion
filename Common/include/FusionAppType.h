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

#ifndef H_FusionAppType
#define H_FusionAppType

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <string>

namespace FusionEngine
{

	namespace Scripting
	{

		template <typename T>
		struct AppType
		{
			static int type_id;
			static std::string name;
		};

		template <typename T>
		int AppType<T>::type_id = 0;
		template <typename T>
		std::string AppType<T>::name = "";

		template <typename T>
		void DefineAppType(int type_id, const std::string& name)
		{
			AppType<T>::type_id = type_id;
			AppType<T>::name = name;
			
			typedef typename std::add_pointer<T>::type pointer_type;
			AppType<pointer_type>::type_id = asTYPEID_OBJHANDLE | type_id;
			AppType<pointer_type>::name = name + "@";
		}

	}

}

#endif