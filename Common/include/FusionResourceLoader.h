/*
*  Copyright (c) 2007-2011 Fusion Project Team
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

#ifndef H_FusionEngine_ResourceLoader
#define H_FusionEngine_ResourceLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionResource.h"

#include <ClanLib/Display/Render/graphic_context.h>
#include <ClanLib/Core/IOData/virtual_directory.h>
#include <boost/any.hpp>

namespace FusionEngine
{

	//! Fn. pointer for loading resources
	typedef void (*resource_load)(ResourceContainer* res, CL_VirtualDirectory vdir, boost::any user_data);
	//! Fn. pointer for unloading resources
	typedef void (*resource_unload)(ResourceContainer* res, CL_VirtualDirectory vdir, boost::any user_data);
	//! Fn. pointer for loading stuff into a GC (textures)
	typedef void (*resource_gcload)(ResourceContainer* res, CL_GraphicContext& gc, boost::any user_data);

	//! Fn. pointer for checking for changes
	typedef void (*resource_haschanged)(ResourceContainer* res, CL_VirtualDirectory vdir, boost::any user_data);

	typedef std::vector< std::pair< std::string, std::string > > DepsList;
	//! Fn. pointer - should return a list of resources that this resource needs access to
	/*!
	* \return True if loading of this resource can complete before dependencies are loaded
	*/
	typedef bool (*resource_list_prerequisites)(ResourceContainer* res, DepsList& dependencies, boost::any user_data);

	//! Struct containing resource loader callbacks
	struct ResourceLoader
	{
		resource_load load;
		resource_unload unload;
		resource_gcload gcload;
		resource_list_prerequisites list_prereq;
		boost::any userData;
		std::string type;

		ResourceLoader()
			: load(nullptr),
			unload(nullptr),
			gcload(nullptr),
			list_prereq(nullptr)
		{
		}

		ResourceLoader(std::string _type, resource_load loadFn, resource_unload unloadFn)
			: type(_type),
			load(loadFn),
			unload(unloadFn),
			gcload(nullptr),
			list_prereq(nullptr)
		{
		}

		ResourceLoader(std::string _type, resource_load loadFn, resource_unload unloadFn, boost::any _userData)
			: type(_type),
			load(loadFn),
			unload(unloadFn),
			gcload(nullptr),
			list_prereq(nullptr),
			userData(_userData)
		{
		}

		ResourceLoader(std::string _type, resource_load loadFn, resource_unload unloadFn, resource_gcload gcLoadFn)
			: type(_type),
			load(loadFn),
			unload(unloadFn),
			gcload(gcLoadFn),
			list_prereq(nullptr)
		{
		}

		ResourceLoader(std::string _type, resource_load loadFn, resource_unload unloadFn, resource_gcload gcLoadFn, boost::any _userData)
			: type(_type),
			load(loadFn),
			unload(unloadFn),
			gcload(gcLoadFn),
			list_prereq(nullptr),
			userData(_userData)
		{
		}
	};

}

#endif
