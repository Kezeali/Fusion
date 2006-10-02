#ifndef Header_FusionEngine_ShipResource
#define Header_FusionEngine_ShipResource

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{
	class DestructableImage;

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
	class ShipResource
	{
	public:
		// Note that the struct names here don't matter,
		// as these structs are only instanciated here.
		struct
		{
			std::string Name;
			std::string Tag;
			std::string Description;
		}
		General;

		struct Positioning
		{
			CL_Point LeftEngine;
			CL_Point RightEngine;
			CL_Point PrimaryWeapon;
			CL_Point SecondaryWeapon;
		}
		Positions;

		struct ImageInfo
		{
			CL_Surface *Body;
			CL_Surface *LeftEngine;
			CL_Surface *RightEngine;
			CL_Surface *PrimaryWeapon;
			CL_Surface *SecondaryWeapon;
		}
		Images;

		struct Physical
		{
			float Mass;
			float EngineForce;
			/*! 
		   * Maximum velocity of rotation.
		   *
		   * \remarks
		   * FusionPhysicsBody has RotationalVelocity [note 'al'], that being the
		   * <i>current</i> velocity of rotation. Yeah, just don't question my naming.
			 * It seemed logical at the time damnit!
		   */
			float RotationVelocity;
		}
		Physics;

	};

}

#endif
