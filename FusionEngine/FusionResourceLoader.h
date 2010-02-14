/*
  Copyright (c) 2007 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward
*/

#ifndef Header_FusionEngine_ResourceLoader
#define Header_FusionEngine_ResourceLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionResource.h"

#include <ClanLib/Display/Render/graphic_context.h>
#include <ClanLib/Core/IOData/virtual_directory.h>

namespace FusionEngine
{

	//! Pointer to function for loading resources
	typedef void (*resource_load)(ResourceContainer* res, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData);
	//! Pointer to function for unloading resources
	typedef void (*resource_unload)(ResourceContainer* res, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData);
	//! Struct containing resource loader callbacks
	struct ResourceLoader
	{
		resource_load load;
		resource_unload unload;
		resource_unload unloadQLData;
		void *userData;
		std::string type;

		ResourceLoader()
			: load(NULL),
			unload(NULL),
			userData(NULL)
		{
		}

		ResourceLoader(std::string _type, resource_load loadFn, resource_unload unloadFn)
			: type(_type),
			load(loadFn),
			unload(unloadFn),
			userData(NULL)
		{
		}

		ResourceLoader(std::string _type, resource_load loadFn, resource_unload unloadFn, void *_userData)
			: type(_type),
			load(loadFn),
			unload(unloadFn),
			userData(_userData)
		{
		}

		ResourceLoader(std::string _type, resource_load loadFn, resource_unload unloadFn, resource_unload unloadQLDataFn)
			: type(_type),
			load(loadFn),
			unload(unloadFn),
			unloadQLData(unloadQLDataFn)
		{
		}

		ResourceLoader(std::string _type, resource_load loadFn, resource_unload unloadFn, resource_unload unloadQLDataFn, void *_userData)
			: type(_type),
			load(loadFn),
			unload(unloadFn),
			unloadQLData(unloadQLDataFn),
			userData(_userData)
		{
		}
	};

}

#endif
