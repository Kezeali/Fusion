#ifndef Header_FusionEngine_FusionShipWeapon
#define Header_FusionEngine_FusionShipWeapon

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{

	/*!
	 * \brief
	 * A weapon drawable, what it actually draws is defined by a script.
	 *
	 * An example of a weapon could be a spacebeam, yeah. I got the spacebeam. Yeah.
	 */
	class FusionShipWeapon
	{
	public:

		int WeaponID;       /*! not sure how to do this, suppose just a series of
                                constants, ballistic, consta-beam, self propelled
                                etc,
                                Have them loaded from XML, but thats your job elliot :P */
		virtual void Draw();
	};

}

#endif