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
		void SetImage(ResourcePointer<CL_Surface> *image);
		//void SetResource(const std::string &resid);

		//! Draws the damn thing
		void Draw();

	protected:
		//! The image to draw
		ResourcePointer<CL_Surface> m_Image;
	};

}

#endif
