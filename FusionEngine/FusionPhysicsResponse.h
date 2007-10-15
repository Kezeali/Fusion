/*
  Copyright (c) 2006 Fusion Project Team

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
*/

#ifndef Header_FusionEngine_FusionPhysicsResponse
#define Header_FusionEngine_FusionPhysicsResponse

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * [depreciated] See ICollisionHandler. (Aparently I forgot that I implemented 
	 * this so I created ICollisionHandler. Now I've used ICollisionHandler so 
	 * I'll stick with that)
	 *
	 * Each body can have a derived class for different collison responses :D.
	 *
	 * \see
	 * FusionPhysicsBody
	 */
	class FusionPhysicsResponse
	{
	public:
		//! Constructor
		FusionPhysicsResponse();
		//! Virtual Destructor
		virtual ~FusionPhysicsResponse();

	public:
		//! What to do if a collision is detected
		virtual void CollisionResponse(const FusionPhysicsBody *other) =0;
		//! What to do if an absolute position for a collision is given
		virtual void CollisionResponse(const FusionPhysicsBody *other, const Vector2 &collision_point) =0;

	protected:
		FusionPhysicsBody *m_Owner;

	};

}

#endif
