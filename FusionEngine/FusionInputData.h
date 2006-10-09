/*
  Copyright (c) 2006 Elliot Hayward

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

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
