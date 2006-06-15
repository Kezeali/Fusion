#ifndef Header_FusionEngine_FusionShipFrame
#define Header_FusionEngine_FusionShipFrame

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionShip.h"
#include "FusionInputData.h"
#include "FusionMessage.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Data container for transferring player state information between client and server.
	 */
	class FusionShipFrame : public FusionMessage
	{
	public:
		//! Miliseconds since game started.
		int time;
		//! Current input from user when the frame was created.
		ShipInput input;
		/*!
		 * State (position, velocity, ammo, damage, etc) of the ship when the frame 
		 * was created.
		 */
		ShipState state;

	private:
		//--- FusionMessage / serilizeable implimentation
		template <class Archive>
		void serialize(Archive &ar, const unsigned int ver);
	};

}

#endif