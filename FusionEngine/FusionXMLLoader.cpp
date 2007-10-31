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

#include "FusionXMLLoader.h"

#include <boost/shared_ptr.hpp>

namespace FusionEngine
{

	const std::string &XMLLoader::GetType() const
	{
		static std::string strType("XML");
		return strType;
	}

	ResourceContainer* XMLLoader::LoadResource(const std::string& tag, const std::string &path, CL_InputSourceProvider* provider)
	{
		TiXmlDocument* dp = loadDocument(path);
		ResourceContainer* rsc = new ResourceContainer(this->GetType(), tag, path, dp);
		return rsc;
	}

	void XMLLoader::ReloadResource(ResourceContainer* resource, CL_InputSourceProvider* provider)
	{
		if (resource->IsValid())
		{
			delete resource->GetDataPtr();
		}

		//! \todo ???Set inputsourceprovider for ResourceLoaders on construction
		TiXmlDocument* dp = loadDocument(resource->GetPath());

		resource->SetDataPtr(dp);
		resource->_setValid(true);
	}

	void XMLLoader::UnloadResource(ResourceContainer* resource)
	{
		if (resource->IsValid())
			delete resource->GetDataPtr();
		resource->SetDataPtr(NULL);

		resource->_setValid(false);
	}

	TiXmlDocument* XMLLoader::loadDocument(const std::string &path)
	{
		TiXmlDocument* doc = new TiXmlDocument;
		try
		{
			InputSourceProvider_PhysFS provider("");
			boost::shared_ptr<CL_InputSource> in(provider.open_source(path));

			char filedata[2084];
			in->read(&filedata, in->size());

			doc->Parse((const char*)filedata, 0, TIXML_ENCODING_UTF8);
		}
		catch (CL_Error&)
		{
			delete doc;
			FSN_EXCEPT(ExCode::FileSystem, "XMLLoader::loadDocument", "'" + path + "' could not be loaded");
		}

		return doc;
	}

};
