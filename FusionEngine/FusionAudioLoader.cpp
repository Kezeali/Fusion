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

#include "FusionAudioLoader.h"

#include <ClanLib/Sound/SoundProviders/soundprovider_factory.h>

namespace FusionEngine
{

	const std::string &AudioLoader::GetType() const
	{
		static std::string strType("AUDIO");
		return strType;
	}

	ResourceContainer* AudioLoader::LoadResource(const std::string& tag, const std::string &path, CL_InputSourceProvider* provider)
	{
		CL_SoundBuffer* p = loadSound(path, provider);
		ResourceContainer* rsc = new ResourceContainer(this->GetType(), tag, path, p);
		return rsc;
	}

	void AudioLoader::ReloadResource(ResourceContainer* resource, CL_InputSourceProvider* provider)
	{
		if (resource->IsValid())
		{
			delete resource->GetDataPtr();
		}

		//! \todo ???Set inputsourceprovider for ResourceLoaders on construction
		CL_SoundBuffer* p = loadSound(resource->GetPath(), provider);

		resource->SetDataPtr(p);

		resource->_setValid(true);
	}

	void AudioLoader::UnloadResource(ResourceContainer* resource)
	{
		if (resource->IsValid())
			delete resource->GetDataPtr();
		resource->SetDataPtr(NULL);

		resource->_setValid(false);
	}

	CL_SoundBuffer* AudioLoader::loadSound(const std::string &path, CL_InputSourceProvider* notUsed)
	{
		//if (provider == NULL)
		//	provider = CL_InputSourceProvider::create_file_provider(".");
		InputSourceProvider_PhysFS provider("");

		//std::string& ext = CL_String::get_extension(path);
		CL_SoundProvider* sp;
		try
		{
			sp = CL_SoundProviderFactory::load(path, false, "",& provider);
		}
		catch (CL_Error&)
		{
			FSN_EXCEPT(ExCode::IO, "AudioLoader::loadSound", "'" + path + "' could not be loaded");
		}
		
		CL_SoundBuffer* sur = new CL_SoundBuffer( sp );

		return sur;
	}

	//////////
	// AudioStreamLoader
	const std::string &AudioStreamLoader::GetType() const
	{
		static std::string strType("AUDIO/STREAM");
		return strType;
	}

	CL_SoundBuffer* AudioStreamLoader::loadSound(const std::string &path, CL_InputSourceProvider* notUsed)
	{
		//if (provider == NULL)
		//	provider = CL_InputSourceProvider::create_file_provider(".");
		InputSourceProvider_PhysFS provider("");

		//std::string& ext = CL_String::get_extension(path);
		CL_SoundProvider* sp;
		try
		{
			sp = CL_SoundProviderFactory::load(path, true, "", &provider);
		}
		catch (CL_Error&)
		{
			FSN_EXCEPT(ExCode::IO, "AudioStreamLoader::loadSound", "'" + path + "' could not be loaded");
		}
		
		CL_SoundBuffer* sur = new CL_SoundBuffer( sp );

		return sur;
	}

};