#ifndef Header_FusionEngine_FusionShipEngine
#define Header_FusionEngine_FusionShipEngine

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// inherits
#include "FusionDrawable.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Represents an engine (thrustor) visually.
	 */
	class FusionShipEngine : public FusionDrawable
	{
	public:
		//! Sets the image this drawable should draw
		void SetImage(CL_Surface *image);
		//void SetResource(const std::string &resid);

		//! Draws the damn thing
		void Draw();

	protected:
		//! The image to draw
		CL_Surface *m_Image;

		//! [depreciated] for simplicity's sake. Allows the engine drawable to access the relavant ShipResource
		//ClientEnvironment *m_Env;

		//! Resource (sent by FusionShip)
		//std::string m_ResourceID;
	};

}

#endif
