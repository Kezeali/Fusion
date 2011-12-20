#ifndef Header_FusionEngine_FusionShipWeapon
#define Header_FusionEngine_FusionShipWeapon

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionDrawable.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * A weapon drawable, what it actually draws is defined by a script.
	 *
	 * An example of a weapon could be a spacebeam, yeah. I got the spacebeam, yeah.
	 */
	class FusionShipWeapon : public FusionDrawable
	{
	public:

		int WeaponID;
		virtual void Draw();
	};

}

#endif