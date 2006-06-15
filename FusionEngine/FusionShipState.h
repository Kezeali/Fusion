#ifndef Header_FusionEngine_FusionStateData
#define Header_FusionEngine_FusionStateData

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace FusionEngine
{

	/*!
	 * \brief
	 * Ship state syncronisation structure. Supports serialization.
	 */
	struct ShipState
	{
		friend class boost::serialization::access;

		//@{
		//! Position and velocity vars.
		CL_Vector Velocity;
		CL_Vector Position;
		float Rotation;
		float RotationalVelocity;
		//@}

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

	private:
		template <class Archive>
		void serialize(Archive &ar, unsigned int ver);
	};

}

#endif