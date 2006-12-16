#ifndef Header_FusionEngine_FusionShipDrawable
#define Header_FusionEngine_FusionShipDrawable

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionDrawable.h"

/// Fusion
#include "FusionShip.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Represents a ship visually.
	 *
	 * \todo Perhaps Drawables could be simply used to
	 * store resID's, and drawing could be done completely by the clientenv
	 * (or a graphics mangaer) reading scrips... either that or I could
	 * just leave it as it is AND ACTUALLY FINISH THIS DAMN THING!! I don't
	 * need your so-called object-orentation and morals!... On a lighter note,
	 * this todo is kinda long, I wonder if Doxygen will still use it...
	 *
	 * \remarks
	 * An instance of this object looks after each ship in the scene.
	 */
	class FusionShipDrawable : public FusionDrawable
	{
	public:
		//! Constructor
		FusionShipDrawable();
		//! Destructor
		~FusionShipDrawable();

	public:
		/*!
		 * \brief
		 * [depreciated] This is now done by FusionShip#SetPosition
		 *
		 * Moves the ship and all its attached accessories.
		 *
		 * This function is only used on client-side, as it is used to ensure
		 * weapons / engines update their absolute positions for drawing. The server doesn't
		 * care about drawing weapons / engines - only their relative positions for fireing,
		 * and whether they are still attached, which are stored in ShipResource and ShipState
		 * respectively.
		 *
		 * \sa
		 * FusionScene | FusionNode | FusionShip | ShipState | ShipResource
		 */
		//void UpdateNode();

		//! Sets the image this drawable should draw
		void SetImage(CL_Surface *image);

		////! Allows the FusionShip to set the resourceid for this drawable
		//void SetResource(const std::string &resid);

		virtual void Draw();

	protected:
		/*! \brief The image to draw.
		 * This is my "Occam's razor" fix for the over-reliance-on-clientEnv problem :P
		 */
		CL_Surface *m_Image;

		//! The ship associated with this drawable
		FusionShip *m_Ship;

		//! Allows the ship drawable to access the relavant ShipResource
		//ClientEnvironment *m_Env;

		//! Resource (sent by FusionShip)
		//std::string m_ResourceID;
	};

}

#endif
