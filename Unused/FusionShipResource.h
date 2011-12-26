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

#ifndef Header_FusionEngine_ShipResourceBundle
#define Header_FusionEngine_ShipResourceBundle

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionResourceBundle.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Stores information loaded from a ship package.
	 *
	 * Loaded information includes
	 * - Engine and weapon positions (CL_Point-s)
	 * - Destructable 'Body' image (the main part of the ship)
	 * - CL_Surfaces for engines and weapons, and other accessories.
	 *
	 * \remarks
	 * This resource has slightly varing uses on the client and server (or rather, more
	 * <i>specific</i> uses.) On the client it is used to define the positions of scene
	 * nodes and give FusionDrawableObject-s something to draw. On the server they are
	 * used to initilise FusionServerShip-s.
	 */
	class ShipResourceBundle : public ResourceBundle
	{
	public:
		// Note that the struct names here don't matter,
		// as these structs are only instanciated here.
		struct
		{
			CL_Point LeftEngine;
			CL_Point RightEngine;
			CL_Point PrimaryWeapon;
			CL_Point SecondaryWeapon;
		}
		Positions;

		struct
		{
			CL_Surface *Body;
			CL_Surface *LeftEngine;
			CL_Surface *RightEngine;
			CL_Surface *PrimaryWeapon;
			CL_Surface *SecondaryWeapon;
		}
		Images;

		struct
		{
			float Mass;
			float EngineForce;
			/*!
		   * Maximum velocity of rotation.
		   *
		   * \remarks
		   * FusionPhysicsBody has RotationalVelocity [note 'al'], that being the
		   * <i>current</i> velocity of rotation. Yeah, just don't question the naming;
			 * It seemed logical at the time damnit!
		   */
			float RotationVelocity;
		}
		Physics;

	};

}

#endif