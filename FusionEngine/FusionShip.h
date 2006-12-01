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

#ifndef Header_FusionEngine_FusionShip
#define Header_FusionEngine_FusionShip

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionShipState.h"
#include "FusionInputData.h"
//#include "FusionShipDrawable.h"

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
	 * An instance of this object holds the state data for each ship in the game.
	 * <br>
	 * ClientEnvironment and ServerEnvironment have friend access to
	 * this class, for efficiant updating. This may, however, be removed at some point.
	 * <br>
	 * 2006/12/01: Rather than having engines fall off when the player is damaged,
	 * it would be cool if you could use them as a sort of kamakaze attack - they
	 * could be jettasoned at will to home on the nearest ship, causing insta-gib on
	 * impact. :)
	 */
	class FusionShip
	{
		friend class ServerEnvironment;
		friend class ClientEnvironment;
	public:
		//! Don't use this
		FusionShip();
		//! Constructor. W/O inputs.
		FusionShip(ShipState initState, FusionPhysicsBody *body, FusionNode *node);
		//! Constructor. 
		FusionShip(ShipState initState, ShipInput initInput, FusionPhysicsBody *body, FusionNode *node);
		//! Destructor
		~FusionShip();

	public:
		//! Set Vel
		/*!
		 * \param[in] physics If this is true, the velocity of the PhysicsBody
		 * associated with this Ship will be set also. This defaults to false,
		 * as a PhysicsBody is usually the /source/ of this information. 
		 */
		void SetVelocity(const CL_Vector2 &velocity, bool physics = false);
		//! Set Pos
		/*!
		 * \param[in] physics If this is true, the position of the PhysicsBody
		 * associated with this Ship will be set also.
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

		//! Scene node
		void SetSceneNode(FusionNode *input);
		//! Self explainatory
		const FusionNode *GetSceneNode() const;
		//! Physics body
		void SetPhysicalBody(FusionPhysicsBody *input);
		//! Self explainatory
		const FusionPhysicsBody *GetPhysicalBody() const;

		//! [depreciated] You know it
		//std::string GetShipResource() const;

		//! Reverts all state data
		void RevertToInitialState();

	protected:
		// Is this needed? - 2006/09/08: no
		//ClientEnvironment *m_Environment;

		//! Main associated node
		FusionNode *m_Node;
		//! Left engine node
		FusionNode *m_LEngNode;
		//! Right engine node
		FusionNode *m_REngNode;
		//! Primary weapon node
		FusionNode *m_PriWepNode;
		//! Secondary weapon node
		FusionNode *m_SecWepNode;
		
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
