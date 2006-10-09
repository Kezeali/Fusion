/*
  Copyright (c) 2006 Elliot Hayward

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

#ifndef Header_FusionEngine_FusionProjectileState
#define Header_FusionEngine_FusionProjectileState

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>

namespace FusionEngine
{
	/*!
	 * \brief
	 * Holds all state information for each projectile.
	 *
	 * An instance of this object holds the state data for each projectile in the game.
	 * This object should remain general enough to be used by client and server.
	 *
	 * \remarks
	 * When a weapon fires, and instance of this class is produced.
	 */
	struct ProjectileState
	{
		//[depreciated] friend class boost::serialization::access;
		//! The player who owns this projectile
		PlayerInd PID;
		//! The unique ID for this projectile (globally unique, generated by GetUID();)
		ObjectID OID;

		//@{
		//! Position and velocity vars.
		CL_Vector2 Velocity;
		CL_Vector2 Position;
		float Rotation;
		float RotationalVelocity;
		//@}

	private:
		//template <class Archive>
		//void serialize(Archive &ar, unsigned int ver);
	};

}

#endif