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
#include <ClanLib/display.h>

#include "FusionExceptionFactory.h"
#include "FusionResourceLoaderUtils.h"
#include "FusionSpriteDefinition.h"

namespace FusionEngine
{

	void LoadImageResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			delete static_cast<clan::PixelBuffer*>(resource->GetDataPtr());
		}

		std::string ext = clan::PathHelp::get_extension(resource->GetPath());
		clan::PixelBuffer sp;
		try
		{
			clan::IODevice file = vdir.open_file_read(resource->GetPath());
			// Load the image
			sp = clan::ImageProviderFactory::load(file, ext);
			// Create the resource metadata
			file.seek(0);
			FileMetadata metadata;
			metadata.modTime = PHYSFS_getLastModTime(resource->GetPath().c_str());
			metadata.length = file.get_size();
			metadata.checksum = checksumClanLibDevice(file);
			resource->SetMetadata(metadata);

			clan::PixelBuffer *data = new clan::PixelBuffer(sp);
			resource->SetDataPtr(data);

			resource->setLoaded(true);
		}
		catch (clan::Exception& ex)
		{
			resource->setLoaded(false);
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded: " + std::string(ex.what()));
		}
	}

	void UnloadImageResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<clan::PixelBuffer*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

	void LoadTextureResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		LoadImageResource(resource, vdir, user_data);
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			resource->setRequiresGC(true);
		}
	}

	void UnloadTexture2DResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			resource->setRequiresGC(false);
			delete static_cast<clan::Texture2D*>(resource->GetDataPtr());
		}
		else if (resource->RequiresGC())
		{
			resource->setRequiresGC(false);
			delete static_cast<clan::PixelBuffer*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

	void LoadTexture2DResourceIntoGC(ResourceContainer* resource, clan::GraphicContext& gc, boost::any user_data)
	{
		if (!resource->IsLoaded() && resource->RequiresGC())
		{
			clan::PixelBuffer* imageData = static_cast<clan::PixelBuffer*>(resource->GetDataPtr());
			FSN_ASSERT(imageData);

			clan::Texture2D* data = new clan::Texture2D();
			data->set_image(gc, *imageData);

			delete imageData;

			resource->SetDataPtr(data);
			//resource->setRequiresGC(false);
			resource->setLoaded(true);
		}
	}

	void UnloadTexture3DResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			resource->setRequiresGC(false);
			delete static_cast<clan::Texture3D*>(resource->GetDataPtr());
		}
		else if (resource->RequiresGC())
		{
			resource->setRequiresGC(false);
			delete static_cast<clan::PixelBuffer*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

	void LoadTexture3DResourceIntoGC(ResourceContainer* resource, clan::GraphicContext& gc, boost::any user_data)
	{
		if (!resource->IsLoaded() && resource->RequiresGC())
		{
			clan::PixelBuffer* imageData = static_cast<clan::PixelBuffer*>(resource->GetDataPtr());
			FSN_ASSERT(imageData);

			clan::Texture3D* data = new clan::Texture3D();
			data->set_image(gc, *imageData, 0);

			delete imageData;

			resource->SetDataPtr(data);
			//resource->setRequiresGC(false);
			resource->setLoaded(true);
		}
	}

	void LoadLegacySpriteResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
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
		catch (clan::Exception&)
		{
			delete def;
			resource->SetDataPtr(nullptr);
			resource->setLoaded(false);
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded");
		}
		
		resource->SetDataPtr(def);
		resource->setLoaded(true);
	}

	void UnloadLegacySpriteResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);

			delete static_cast<LegacySpriteDefinition*>(resource->GetDataPtr());
		}

		resource->SetDataPtr(nullptr);
	}
	
};
