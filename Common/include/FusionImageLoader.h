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

#ifndef H_FusionImageLoader
#define H_FusionImageLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionResourceLoader.h"

namespace FusionEngine
{
	//! Image (pixel-buffer) resource loader callback
	void LoadImageResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);
	//! Image resource unloader callback
	void UnloadImageResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);

	//! Texture resource loader
	/*
	*  Just loads the image data into memory, them marks it as requiring GC load
	*/
	void LoadTextureResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);
	//! Texture resource unloader
	void UnloadTexture2DResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);
	//! Loads a texture resource (creates the, possibly GFX memory resident, texture object)
	void LoadTexture2DResourceIntoGC(ResourceContainer* resource, clan::GraphicContext& gc, boost::any user_data);

	//! 3D Texture resource unloader
	void UnloadTexture3DResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);
	//! 3D Texture resource loader callback
	void LoadTexture3DResourceIntoGC(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);

}

#endif
