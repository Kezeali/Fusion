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

#include "FusionImageLoader.h"

namespace FusionEngine
{

	const std::string &ImageLoader::GetType() const
	{
		static std::string strType("IMAGE");
		return strType;
	}

	ResourceContainer* ImageLoader::LoadResource(const std::string& tag, const std::string &path, CL_InputSourceProvider* provider)
	{
		CL_Surface* sur = loadSurface(path, provider);
		ResourceContainer* rsc = new ResourceContainer(this->GetType(), tag, path, sur);
		return rsc;
	}

	void ImageLoader::ReloadResource(ResourceContainer* resource, CL_InputSourceProvider* provider)
	{
		if (resource->IsValid())
		{
			delete resource->GetDataPtr();
		}

		CL_Surface* sur = loadSurface(resource->GetPath(), provider);

		resource->SetDataPtr(sur);

		resource->_setValid(true);
	}

	void ImageLoader::UnloadResource(ResourceContainer* resource)
	{
		if (resource->IsValid())
			delete resource->GetDataPtr();
		resource->SetDataPtr(NULL);

		resource->_setValid(false);
	}

	CL_Surface* ImageLoader::loadSurface(const std::string &path, CL_InputSourceProvider* notUsed)
	{
		InputSourceProvider_PhysFS provider("");

		std::string& ext = CL_String::get_extension(path);
		CL_PixelBuffer sp;
		try
		{
			sp = CL_ProviderFactory::load(path, ext, &provider);
		}
		catch (CL_Error&)
		{
			FSN_EXCEPT(ExCode::IO, "ImageLoader::loadSurface", "'" + path + "' could not be loaded");
		}
		
		CL_Surface* sur = new CL_Surface( sp );

		return sur;
	}

};