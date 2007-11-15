/*
  Copyright (c) 2006-2007 Fusion Project Team

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

#ifndef Header_FusionEngine_ResourcePointer
#define Header_FusionEngine_ResourcePointer

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include "FusionPhysicsShape.h"
#include "FusionPhysicsBody.h"
#include "FusionPhysicsWorld.h"

namespace FusionEngine
{

	static RegisterPhysicsTypes(asIScriptEngine* engine)
	{
		PhysicsBody::registerType(engine);
		Shape::registerType(engine);
		CircleShape::re
		PhysicsWorld::registerType(engine);

		registerPhysBodyMethods(engine);
	}

	static registerPhysBodyMethods(asIScriptEngine* engine)
	{
	}

	static registerPhysShapeMethods(asIScriptEngine* engine)
	{
	}

	static registerPhysWorldMethods(asIScriptEngine* engine)
	{
	}

}

#endif