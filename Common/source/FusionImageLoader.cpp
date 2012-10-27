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

#include "PrecompiledHeaders.h"

#include "FusionImageLoader.h"

#include <ClanLib/Core/IOData/path_help.h>

#include "FusionExceptionFactory.h"
#include "FusionSpriteDefinition.h"

namespace FusionEngine
{

	void LoadImageResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			delete static_cast<CL_PixelBuffer*>(resource->GetDataPtr());
		}

		CL_String ext = CL_PathHelp::get_extension(resource->GetPath());
		CL_PixelBuffer sp;
		try
		{
			sp = CL_ImageProviderFactory::load(resource->GetPath(), vdir, ext);
		}
		catch (CL_Exception& ex)
		{
			resource->setLoaded(false);
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded: " + std::string(ex.what()));
		}

		CL_PixelBuffer *data = new CL_PixelBuffer(sp);
		resource->SetDataPtr(data);

		resource->setLoaded(true);
	}

	void UnloadImageResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<CL_PixelBuffer*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

	void LoadTextureResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		LoadImageResource(resource, vdir, user_data);
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			resource->setRequiresGC(true);
		}
	}

	void UnloadTextureResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			resource->setRequiresGC(false);
			delete static_cast<CL_Texture*>(resource->GetDataPtr());
		}
		else if (resource->RequiresGC())
		{
			resource->setRequiresGC(false);
			delete static_cast<CL_PixelBuffer*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

	void LoadTextureResourceIntoGC(ResourceContainer* resource, CL_GraphicContext& gc, boost::any user_data)
	{
		if (!resource->IsLoaded() && resource->RequiresGC())
		{
			CL_PixelBuffer* pre_gc_data = static_cast<CL_PixelBuffer*>(resource->GetDataPtr());
			FSN_ASSERT(pre_gc_data);
			CL_Texture* data = new CL_Texture(gc, pre_gc_data->get_width(), pre_gc_data->get_height(), pre_gc_data->get_format());
			data->set_image(*pre_gc_data);
			delete pre_gc_data;
			resource->SetDataPtr(data);
			//resource->setRequiresGC(false);
			resource->setLoaded(true);
		}
	}

	void LoadLegacySpriteResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			delete static_cast<LegacySpriteDefinition*>(resource->GetDataPtr());
		}

		LegacySpriteDefinition* def = new LegacySpriteDefinition();
		try
		{
			LoadSpriteDefinition(*def, resource->GetPath(), vdir);
		}
		catch (CL_Exception&)
		{
			delete def;
			resource->SetDataPtr(nullptr);
			resource->setLoaded(false);
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded");
		}
		
		resource->SetDataPtr(def);
		resource->setLoaded(true);
	}

	void UnloadLegacySpriteResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);

			delete static_cast<LegacySpriteDefinition*>(resource->GetDataPtr());
		}

		resource->SetDataPtr(nullptr);
	}
	
};
