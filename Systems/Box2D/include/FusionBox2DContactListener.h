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

#ifndef H_FusionBox2DContactListener
#define H_FusionBox2DContactListener

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

class b2Contact;
struct b2Manifold;
struct b2ContactImpulse;

namespace FusionEngine
{

	class Box2DContactListener
	{
	public:
		virtual ~Box2DContactListener() {}

		virtual void BeginContact(b2Contact* contact)
		{ /* handle begin event */ }
		virtual void EndContact(b2Contact* contact)
		{ /* handle end event */ }
		virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
		{ /* handle pre-solve event */ }
		virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
		{ /* handle post-solve event */ }
	};

}

#endif