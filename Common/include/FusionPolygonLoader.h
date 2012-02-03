/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionPolygonLoader
#define H_FusionPolygonLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionResource.h"
#include "FusionVector2.h"

#include <iostream>
#include <string>

#include <Box2D/Box2D.h>

namespace FusionEngine
{

	//! b2PolygonShape loader utility
	class PolygonResource
	{
	public:
		static std::unique_ptr<b2PolygonShape> Load(CL_IODevice dev);
		static void Save(CL_IODevice dev, const b2PolygonShape& shape);
	};

	//! Polygon resource loader callback
	void LoadPolygonResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData);
	//! SpriteAnimation resource unloader callback
	void UnloadPolygonResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData);
	
}

#endif
