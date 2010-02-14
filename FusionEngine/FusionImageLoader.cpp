/*
  Copyright (c) 2007-2009 Fusion Project Team

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

#include "Common.h"

#include "FusionImageLoader.h"

#include <ClanLib/Core/IOData/path_help.h>

#include "FusionExceptionFactory.h"
#include "FusionSpriteDefinition.h"

namespace FusionEngine
{

	void LoadImageResource(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->IsLoaded())
		{
			delete resource->GetDataPtr();
		}

		CL_String ext = CL_PathHelp::get_extension(resource->GetPath());
		CL_PixelBuffer sp;
		try
		{
			sp = CL_ImageProviderFactory::load(resource->GetPath(), vdir, ext);
		}
		catch (CL_Exception&)
		{
			FSN_EXCEPT(ExCode::IO, "LoadImageResource", "'" + resource->GetPath() + "' could not be loaded");
		}

		CL_PixelBuffer *data = new CL_PixelBuffer(sp);
		resource->SetDataPtr(data);

		resource->_setValid(true);
	}

	void UnloadImageResouce(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->IsLoaded())
		{
			resource->_setValid(false);
			delete resource->GetDataPtr();
		}
		resource->SetDataPtr(NULL);
	}

	void LoadSpriteResource(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->IsLoaded())
		{
			delete resource->GetDataPtr();
		}

		if (!resource->HasQuickLoadData())
		{
			SpriteDefinition *def = new SpriteDefinition();
			try
			{
				LoadSpriteDefinition(*def, resource->GetPath(), vdir);
				resource->SetQuickLoadDataPtr(def);
				resource->_setHasQuickLoadData(true);
			}
			catch (FileSystemException& ex)
			{
				delete def;
				FSN_EXCEPT(ExCode::IO, "LoadSpriteResource", "Definition data for '" + resource->GetPath() + "' could not be loaded: " + ex.GetDescription());
			}
		}

		CL_Sprite *sprite = NULL;
		try
		{
			SpriteDefinition *def = static_cast<SpriteDefinition*>( resource->GetQuickLoadDataPtr() );
			sprite = def->CreateSprite(gc, vdir);
		}
		catch (CL_Exception&)
		{
			FSN_EXCEPT(ExCode::IO, "LoadSpriteResource", "'" + resource->GetPath() + "' could not be loaded");
		}

		resource->SetDataPtr(sprite);
		resource->_setValid(sprite != NULL);
	}

	void UnloadSpriteResource(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->IsLoaded())
		{
			resource->_setValid(false);

			delete resource->GetDataPtr();

			if (resource->HasQuickLoadData())
			{
				SpriteDefinition *def = static_cast<SpriteDefinition*>( resource->GetQuickLoadDataPtr() );
				def->SpriteReleased();
			}
		}

		resource->SetDataPtr(NULL);
	}

	void UnloadSpriteQuickLoadData(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->HasQuickLoadData())
			delete resource->GetQuickLoadDataPtr();

		resource->SetQuickLoadDataPtr(NULL);
		resource->_setHasQuickLoadData(false);
	}
	
};
