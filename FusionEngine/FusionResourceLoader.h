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

#include "FusionInputSource_PhysFS.h"
#include "FusionInputSourceProvider_PhysFS.h"

namespace FusionEngine
{

	//! Allows the ResourceManager to select a resource loader by typename
	template<class T>
	static inline std::string GetResourceType()
	{
		return "INVALID";
	}

	//! Generates a specialization for the given resource data-type
	#define REGISTER_RESOURCE_TYPE(c, t) \
	template<> static inline std::string GetResourceType<c>() { return t; }

	/*!
	 * \brief
	 * Interface for resource loaders used by ResourceManager
	 *
	 * \sa
	 * ImageLoader | Resource | ResourceManager
	 */
	class ResourceLoader
	{
	public:
		//! Returns the type identifier for resources this loader can deal with
		virtual const std::string &GetType() const = 0;

		//! Loads the resource reffered to by the given text
		virtual ResourceContainer* LoadResource(const std::string& tag, const std::string &path, CL_InputSourceProvider* provider) = 0;

		//! Reloads the given resource (which has been cleaned up by garbage collection)
		virtual void ReloadResource(ResourceContainer * resource, CL_InputSourceProvider* provider) = 0;

		//! Cleans up resource data (for garbage collection)
		virtual void UnloadResource(ResourceContainer * resource) = 0;

	};

}

#endif
