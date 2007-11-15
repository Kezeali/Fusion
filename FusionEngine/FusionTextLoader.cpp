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

#include "FusionTextLoader.h"

#include <boost/shared_ptr.hpp>

namespace FusionEngine
{

	const std::string &TextLoader::GetType() const
	{
		static std::string strType("TEXT");
		return strType;
	}

	ResourceContainer* TextLoader::LoadResource(const std::string& tag, const std::string &path, CL_InputSourceProvider* provider)
	{
		std::string* dp = load(path);
		ResourceContainer* rsc = new ResourceContainer(this->GetType(), tag, path, dp);
		return rsc;
	}

	void TextLoader::ReloadResource(ResourceContainer* resource, CL_InputSourceProvider* provider)
	{
		if (resource->IsValid())
		{
			delete resource->GetDataPtr();
		}

		std::string* dp = load(resource->GetPath());

		resource->SetDataPtr(dp);
		resource->_setValid(true);
	}

	void TextLoader::UnloadResource(ResourceContainer* resource)
	{
		if (resource->IsValid())
			delete resource->GetDataPtr();
		resource->SetDataPtr(NULL);

		resource->_setValid(false);
	}

	std::string* TextLoader::load(const std::string &path)
	{
		std::string* data = new std::string;
		try
		{
			InputSourceProvider_PhysFS provider("");
			boost::shared_ptr<CL_InputSource> in(provider.open_source(path));

			// Read the file
			int len = in->size();
			data->resize(len);

			in->read(&(*data)[0], len);
		}
		catch (CL_Error&)
		{
			delete data;
			FSN_EXCEPT(ExCode::IO, "ScriptLoader::loadDocument", "'" + path + "' could not be loaded");
		}

		return data;
	}

};
