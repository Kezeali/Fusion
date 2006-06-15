#ifndef Header_FusionEngine_FusionShipDrawable
#define Header_FusionEngine_FusionShipDrawable

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Represents a ship visually.
	 *
	 * \remarks
	 * An instance of this object looks after each ship in the scene.
	 */
	class FusionShipDrawable : public FusionDrawable
	{
	public:
		//! The state container for the ship that this draws.
		FusionShip Ship;

		FusionShipDrawable();
		~FusionShipDrawable();

		/*!
		 * \brief
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
		void UpdateNode();

		virtual void Draw();
	};

}

#endif