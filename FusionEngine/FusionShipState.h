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

#ifndef Header_FusionEngine_FusionShipState
#define Header_FusionEngine_FusionShipState

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>

namespace FusionEngine
{

	//! Engine flag settings
	enum ActiveEngines{ LEFT, RIGHT };
	//! Weapon flag settings
	enum ActiveWeapons{ PRIMARY, SECONDARY };
	/*!
	 * \brief
	 * Ship state syncronisation structure. Supports serialization.
	 */
	struct ShipState
	{
		//[depreciated] friend class boost::serialization::access;

		//! The unique identifier of the ship this state reffers to
		PlayerInd PID;

		//@{
		//! Position and velocity vars.
		CL_Vector2 Velocity;
		CL_Vector2 Position;
		float Rotation;
		float RotationalVelocity;
		//@}

		//! Health
		int health;

		//@{
		//! Selected weapon (Resource ID's).
		int current_primary;
		int current_secondary;
		int current_bomb;
		//@}
		/*!
		 * \remarks
		 * The valid values for engines and weapons are as follows:
		 * <table>
		 *   <tr> <td>Bin</td> <td>Dec</td> <td>Active component(s)</td> </tr>
		 *   <tr>
		 *        <td>00</td>  <td>0</td>   <td>none</td>
		 *   </tr>
		 *   <tr>
		 *        <td>01</td>  <td>1</td>   <td>right only</td>
		 *   </tr>
		 *   <tr>
		 *        <td>10</td>  <td>2</td>   <td>left only</td>
		 *   </tr>
		 *   <tr>
		 *        <td>11</td>  <td>3</td>   <td>both</td>
		 *   </tr>
		 * </table>
		 *
		 *  For weapons 'right' = 'primary' and 'left' = 'secondary'.
		 */
		cl_uint8 engines;
		//! See ShipState#engines for info.
		cl_uint8 weapons;

	//private:
		//template <class Archive>
		//void serialize(Archive &ar, unsigned int ver);
	};

}

#endif
