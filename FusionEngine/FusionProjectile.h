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

#ifndef Header_FusionEngine_FusionProjectile
#define Header_FusionEngine_FusionProjectile

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Fusion
#include "FusionProjectileState.h"

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
	class FusionProjectile
	{
	public:
		//! Basic Constructor
		FusionProjectile();
		//! Constructor +state
		FusionProjectile(ProjectileState initialState);
		//! Destructor
		~FusionProjectile();

	public:
		//! Sets the state
		void SetState(ProjectileState state);

		//! Guess
		const ProjectileState &GetState() const;

		//! Callback
		void CollisionResponse(FusionPhysicsBody *other, const CL_Vector2 &collision_point);

		//! Calls GenericEnvioronment#Detonate(this).
		/*!
		 * Script function.
		 */
		void Detonate();

	protected:
		//! The current state
		ProjectileState m_CurrentState;

		//! The environment
		GenericEnvironment *m_Environment;

	};

}

#endif
