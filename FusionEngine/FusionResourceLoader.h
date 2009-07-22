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

#include "FusionCommon.h"

#include "FusionResource.h"



namespace FusionEngine
{

	/*!
	 * \brief
	 * Interface for resource loaders used by ResourceManager
	 *
	 * \sa
	 * ImageLoader | Resource | ResourceManager
	 */
	//class ResourceLoader
	//{
	//protected:
	//	CL_VirtualDirectory m_Directory;
	//public:
	//	//! Constructor - resource manager must pass a VirtualDirectory object for the filesystem in use
	//	ResourceLoader(CL_VirtualDirectory vdir)
	//		: m_Directory(vdir)
	//	{
	//	}

	//	//! Returns the type identifier for resources this loader can deal with
	//	virtual const std::string &GetType() const = 0;

	//	//! Loads the resource reffered to by the given text
	//	virtual ResourceContainer* LoadResource(const std::wstring& tag, const std::wstring &path) = 0;

	//	//! Reloads the given resource
	//	virtual void ReloadResource(ResourceContainer * resource) = 0;

	//	//! Cleans up resource data
	//	virtual void UnloadResource(ResourceContainer * resource) = 0;

	//};

	//! Factory method template
	/*!
	 * Factory method template for ResourceLoader factory methods,
	 * which are used by ResourceManager to create ResourceLoader
	 * objects at runtime
	 */
	//template<class T>
	//static ResourceLoader* ResourceLoader_Factory(CL_VirtualDirectory vdir)
	//{
	//	return new T(vdir);
	//}

	/*! Type for factory function pointer
	 *
	 * Factory function pointers are stored in ResourceManager
	 * at runtime, indexed by the Type string for the
	 * resource loader they will create.
	 */
	//typedef ResourceLoader* (*resourceLoader_creator)(CL_VirtualDirectory vdir);


	typedef void (*resource_load)(ResourceContainer* res, CL_VirtualDirectory vdir, void* userData);
	typedef void (*resource_unload)(ResourceContainer* res, CL_VirtualDirectory vdir, void* userData);
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
