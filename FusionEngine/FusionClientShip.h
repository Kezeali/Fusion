/*
  Copyright (c) 2006 FusionTeam

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

#ifndef Header_FusionEngine_FusionClientShip
#define Header_FusionEngine_FusionClientShip

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionShipState.h"
#include "FusionInputData.h"
#include "FusionShipDrawable.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Holds all ship state information for each ship.
	 *
	 * State information such as current buttons pressed and health is stored in
	 * this class. Other classes that need such information can gain access to it
	 * via the public members.
	 *
	 * \remarks
	 * DON'T LET THE NAME FOOL YOU: due to poor foresight, this class was left with
	 * a misnomer and is infact used by both ClientEnvironment and ServerEnvironment
	 * :P <br>
	 * An instance of this object holds the state data for each ship in the game.
	 * <br>
	 * ClientEnvironment and ServerEnvironment have friend access to
	 * this class, for efficiant updating.
	 */
	class FusionClientShip
	{
		friend class ServerEnvironment;
		friend class ClientEnvironment;
	public:
		FusionClientShip();
		FusionClientShip(ShipState initState);
		~FusionClientShip();

	public:
		//! Set Vel
		/*!
		 * \param physics If this is true, the velocity of the PhysicsBody
		 * associated with this Ship will be set too. This defaults to false,
		 * as a PhysicsBody is usually the /source/ of this information. 
		 */
		void SetVelocity(const CL_Vector2 &velocity, bool physics = false);
		//! Set Pos
		/*!
		 * \param physics If this is true, the position of the PhysicsBody
		 * associated with this Ship will be set too.
		 */
		void SetPosition(const CL_Vector2 &position, bool physics = false);

		//! Ship state
		void SetShipState(ShipState input);
		//! Input state
		void SetInputState(ShipInput input);

		//! Guess
		const ShipState &GetShipState() const;
		//! Self explainatory
		const ShipInput &GetInputState() const;

		//! [depreciated] You know it
		//std::string GetShipResource() const;

		//! Reverts all state data
		void RevertToInitialState();

	protected:
		// Is this needed?
		//ClientEnvironment *m_Environment;

		//! Associated node
		FusionNode *m_Node;
		//! [depreciated] FusionNode lists all attached drawables. Associated drawable
		//FusionShipDrawable *m_Drawable;
		
		//! \see FusionPhysicsBody
		FusionPhysicsBody *m_PhysicalBody;

		//! [depreciated] Allows this object to find its resource in the client env
		//std::string m_ResourceID;

		//! Input state
		ShipInput m_Input;
		//! Current spacial attributes
		ShipState m_CurrentState;
		//! Default (initial) spacial attributes
		ShipState m_InitialState;
	};

}

#endif
