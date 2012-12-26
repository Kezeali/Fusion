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

#include "FusionAudioLoader.h"

#include "FusionCLStream.h"
#include "FusionExceptionFactory.h"
#include "FusionResourceLoaderUtils.h"

#include <ClanLib/Core/System/exception.h>
#include <ClanLib/sound.h>
#include <ClanLib/Sound/SoundProviders/soundprovider_factory.h>

namespace FusionEngine
{

	void LoadAudio(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any)
	{
		if (resource->IsLoaded())
			delete static_cast<CL_SoundBuffer*>(resource->GetDataPtr());

		//std::string& ext = CL_String::get_extension(resource->GetPath());
		CL_SoundProvider* sp;
		try
		{
			CL_IODevice file = vdir.open_file_read(resource->GetPath());
			sp = CL_SoundProviderFactory::load(file, false, CL_PathHelp::get_extension(resource->GetPath()));

			resource->SetMetadata(CreateFileMetadata(resource->GetPath(), IO::CLStream(file)));
		}
		catch (CL_Exception&)
		{
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded");
		}
		
		CL_SoundBuffer* p = new CL_SoundBuffer( sp );
		resource->SetDataPtr(p);

		resource->setLoaded(true);
	}

	void UnloadAudio(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<CL_SoundBuffer*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(NULL);
	}

	void LoadAudioStream(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any)
	{
		if (resource->IsLoaded())
			delete static_cast<CL_SoundBuffer*>(resource->GetDataPtr());

		//std::string& ext = CL_String::get_extension(resource->GetPath());
		CL_SoundProvider* sp;
		try
		{
			CL_IODevice file = vdir.open_file_read(resource->GetPath());
			sp = CL_SoundProviderFactory::load(file, true, CL_PathHelp::get_extension(resource->GetPath()));

			resource->SetMetadata(CreateFileMetadata(resource->GetPath(), IO::CLStream(file)));
		}
		catch (CL_Exception&)
		{
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded");
		}
		
		CL_SoundBuffer* p = new CL_SoundBuffer( sp );
		resource->SetDataPtr(p);

		resource->setLoaded(true);
	}

	void UnloadAudioStream(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<CL_SoundBuffer*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(NULL);
	}

};