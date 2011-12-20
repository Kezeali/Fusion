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

#ifndef Header_FusionEngine_FusionPhysicsProperties
#define Header_FusionEngine_FusionPhysicsProperties
#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * [depreciated] Overcomplication - only the response needs to be polymorphic.
	 * Holds data about a physical object.
	 *
	 * \remarks
	 * For FusionShips this should be initialisd by the ShipFactory when it creates
	 * a new ship. It should remain indipendant of the ClientEnvironment after that
	 * point - all modification to it can be done manually, rather than requiring it
	 * to know of ShipResource.
	 * 
	 * \see
	 * FusionPhysicsBody
	 */
	class FusionPhysicsProperties
	{
	public:
		//@{
		//! Guess.
		float m_Mass;
		Vector2 m_AppliedForce;
		Vector2 m_Acceleration;
		Vector2 m_Velocity;
		Vector2 m_Position;

		float m_Rotation;
		/*! 
		 * Current velocity of rotation.
		 *
		 * \remarks
		 * (ShipResourceBundle has RotationVelocity [no 'al'], that being the <i>maximum</i>
		 * velocity of rotation.)
		 */
		float m_RotationalVelocity;
		//@}

		//! Each body can have a derived class for different collison responses :D.
		virtual void CollisionResponse() {};

	};

}

#endif