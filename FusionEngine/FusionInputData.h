#ifndef Header_FusionEngine_FusionInputData
#define Header_FusionEngine_FusionInputData

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Input state structure for each ship.
	 *
	 * This structure has two purposes: 1. as part of FusionShipFrame to syncronise input 
	 * between client and server, and 2. returned by FusionInput to tell the
	 * ClientEnvironment what inputs are currently active (on local hardware) for a specific 
	 * ship.
	 */
	struct ShipInput
	{
		bool thrust;
		bool reverse;
		bool left;
		bool right;
		bool primary;
		bool secondary;
		bool bomb;
	};

	//! Input state structure for the ClientEnvironment.
	struct GlobalInput
	{
		bool menu;
		bool console;
	};

}

#endif