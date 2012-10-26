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

#ifndef H_FusionEngine_AudioLoader
#define H_FusionEngine_AudioLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionResourceLoader.h"

namespace FusionEngine
{

	//! Fully loads a sound clip into memory
	void LoadAudio(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data);
	//! Unloads an audio resource
	void UnloadAudio(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data);

	//! Loads an audio file for streaming
	/*!
	 * Generally used for music.
	 */
	void LoadAudioStream(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data);
	//! Unloads an audio resource
	//void UnloadAudioStream(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data);

}

#endif
