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
	 * \sa
	 * FusionShip | ShipResource
	 */
	class FusionShipDrawable : public FusionDrawable
	{
	public:
		//! Constructor
		FusionShipDrawable(const std::string& resource_id);
		//! Destructor
		~FusionShipDrawable();

	public:
		//! Maybe [depreciated] Sets the image this drawable should draw
		void SetImage(CL_Surface *image);

		//! Sets the resourceid for this drawable
		void SetResource(const std::string &resid);

		//! Draws
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

		//! Resource
		std::string m_ResourceID;
	};

}

#endif
