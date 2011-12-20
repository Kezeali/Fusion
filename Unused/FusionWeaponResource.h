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

#ifndef Header_FusionEngine_WeaponResourceBundle
#define Header_FusionEngine_WeaponResourceBundle

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionResourceBundle.h"
#include "FusionProjectileResourceBundle.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Stores information loaded from a weapon package.
	 */
	class WeaponResourceBundle : public ResourceBundle
	{
	public:
		// Note that struct typenames aren't used,
		// as these structs are only instanciated here.

		struct
		{
			ResourcePointer<Script> script;

			ScriptFuncSig onfire;
			// player pressed fire while the weapon was unloaded - only for
			//  ReloadTime = -1 weapons
			ScriptFuncSig ontrigger; 
			ScriptFuncSig creation;
		}
		//! Weapon script
		Scripts;

		struct
		{
			// -1 means the weapon only reloads when the primary projectile detonates
			int ReloadTime;
		}
		Behaviour;


		struct
		{
			float Mass;
			float EngineForce;
			/*!
		   * Maximum velocity of rotation.
		   *
		   * \remarks
		   * FusionPhysicsBody has RotationalVelocity [note 'al'], that being the
		   * <i>current</i> velocity of rotation. Yeah, just don't question my naming;
			 * It seemed logical at the time damnit!
		   */
			float RotationVelocity;
		}
		//! Physical properties
		Physics;

	};

}

#endif
