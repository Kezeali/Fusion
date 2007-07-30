/*
  Copyright (c) 2007 Fusion Project Team

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

#ifndef Header_FusionEngine_ProjectileResourceBundle
#define Header_FusionEngine_ProjectileResourceBundle

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * The resource data for the projectile sections of packages
	 *
	 * \todo Allow weapons to override the hard-coded drawing method with
	 *  a scripted method. Scripts should draw using Effects and basic drawing
	 *  functions such as draw_lines (polygon tool) and others built into ClanLib.
	 *
	 * \todo Physics functions for weapon scripts (to allow for grappling-hook
	 *  functionality, etc.)
	 *
	 * \sa WeaponResourceBundle | ShipResourceBundle
	 */
	class ProjectileResourceBundle : public ResourceBundle
	{
	public:
		struct
		{
			float Damage;
			float BlastRadius;
			bool DamageTerrain;
		}
		Payload;

	};

}

#endif
