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

	void LoadImageResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		if (resource->IsValid())
		{
			delete resource->GetDataPtr();
		}

		CL_String ext = CL_PathHelp::get_extension(path);
		CL_PixelBuffer sp;
		try
		{
			sp = CL_ImageProviderFactory::load(resource->GetPath, ext, m_Directory);
		}
		catch (CL_Exception&)
		{
			FSN_WEXCEPT(ExCode::IO, L"ImageLoader::loadSurface", L"'" + path + L"' could not be loaded");
		}

		CL_PixelBuffer *data = new CL_PixelBuffer(sp);
		resource->SetDataPtr(data);

		resource->_setValid(true);
	}

	void UnloadImageResouce(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		if (resource->IsValid())
			delete resource->GetDataPtr();
		resource->SetDataPtr(NULL);

		resource->_setValid(false);
	}

};