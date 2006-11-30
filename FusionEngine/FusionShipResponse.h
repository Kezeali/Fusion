/*
  Copyright (c) 2006 FusionTeam

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

#ifndef Header_FusionEngine_FusionShipResponse
#define Header_FusionEngine_FusionShipResponse

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionPhysicsResponse.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Collision Response for Ships
	 *
	 * \see
	 * FusionPhysicsResponse
	 */
	class FusionShipResponse : public FusionPhysicsResponse
	{
	public:
		FusionShipResponse();
		virtual ~FusionShipResponse();

	public:
		void CollisionResponse(void);
		void CollisionResponse(const CL_Vector2 &collision_point);
	};

}

#endif