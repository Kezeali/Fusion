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

#ifndef Header_FusionEngine_XmlLoader
#define Header_FusionEngine_XmlLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionResourceLoader.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Loads xml documents
	 *
	 * \sa
	 * ResourceLoader
	 */
	class XMLLoader : public ResourceLoader
	{
	public:
		//! Returns the type identifier for resources this loader can deal with
		virtual const std::string &GetType() const;

		//! Loads the resource reffered to by the given text
		virtual ResourceContainer* LoadResource(const std::string& tag, const std::string &text, CL_InputSourceProvider* provider);

		//! Reloads the given resource (which has been cleaned up by garbage collection)
		virtual void ReloadResource(ResourceContainer * resource, CL_InputSourceProvider* provider = NULL);

		//! Cleans up resource data (for garbage collection)
		virtual void UnloadResource(ResourceContainer * resource);

	protected:
		TiXmlDocument* loadDocument(const std::string &path);

	};

}

#endif
