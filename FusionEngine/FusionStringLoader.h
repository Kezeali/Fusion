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

#ifndef Header_FusionEngine_StringLoader
#define Header_FusionEngine_StringLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionResourceLoader.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * STRING type Resources are just the raw data from the resource file
	 * (literally the 'Text' property of the Resource), so this Loader
	 * just makes the Data (ptr) property point to the Text property ;)
	 *
	 * \sa
	 * ResourceLoader
	 */
	class StringLoader : public ResourceLoader<std::string>
	{
	public:
		//! Basic constructor.
		StringLoader();

	public:
		//! Returns the type identifier for resources this loader can deal with
		const std::string &GetType() const;

		//! Returns a resource
		Resource<std::string>* LoadResource(ResourceTag tag, const std::string &text);
		//! Validates the given resource
		void ReloadResource(Resource<std::string>* resource);
		//! Invalidates the given resource
		void UnloadResource(Resource<std::string>* resource);

	protected:
		const std::string *m_Resource;

	};

}

#endif
