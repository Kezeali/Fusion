/*
  Copyright (c) 2006 Fusion Project Team

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

#include "FusionCommon.h"

/// Inherited
#include "FusionPhysicsCallback.h"

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
	 * via the public methods.
	 *
	 * \remarks
	 * An instance of this object holds the state data for each ship in the game.
	 * <br>
	 * ClientEnvironment and ServerEnvironment have friend access to
	 * this class, for efficiant updating. This may, however, be removed at some point.
	 * <br>
	 * 2006/12/01: Rather than having engines fall off when the player is damaged,
	 * it would be cool if you could use them as a sort of kamakaze attack - they
	 * could be jettasoned at will to home on the nearest ship, causing instant death
	 * on impact. :)
	 */
	class FusionShip : public CollisionHandler
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
		void SetVelocity(const Vector2 &velocity, bool physics = false);
		//! Set Pos
		/*!
		 * \param[in] physics If this is true, the position of the PhysicsBody
		 * associated with this Ship will be set also.
		 */
		void SetPosition(const Vector2 &position, bool physics = false);
		//! Set rv
		/*!
		 * \param[in] physics If this is true, the rot. velocity of the PhysicsBody
		 * associated with this Ship will be set also.
		 */
		void SetRotationalVelocity(float velocity, bool physics);
		//! Set rot
		/*!
		 * \param[in] physics If this is true, the rot. of the PhysicsBody
		 * associated with this Ship will be set also.
		 */
		void SetRotation(float rotation, bool physics);

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

		//! Returns true if the input have changed recently.
		/*!
		 * Returns true if the input data stored here has been changed since the last
		 * call to _inputSynced().
		 */
		bool InputHasChanged() const;
		//! Returns true if the state have changed recently.
		/*!
		 * Returns true if the state stored here has been changed since the last
		 * call to _stateSynced().
		 */
		bool StateHasChanged() const;

		//! Makes InputHasChanged return false.
		/*!
		 * This should be called after input data has been read from this 'ship'
		 * (that is to say, this 'player data storeage location') so that no
		 * unnecessary packets will be sent.
		 *
		 * \sa ClientEnvironment#send() | ServerEnvironment#send()
		 */
		bool _inputSynced() const;
		//! Makes StateHasChanged return false.
		/*!
		 * This should be called after state data has been read from this 'ship'
		 * (that is to say, this 'player data storeage location') so that no
		 * unnecessary packets will be sent.
		 */
		bool _stateSynced() const;

		//! [depreciated] You know it
		//std::string GetShipResource() const;

		//! Reverts all state data
		void RevertToInitialState();

		bool CanCollideWith(const FusionPhysicsBody *other);

		//! What to do if an absolute position for a collision is given
		void CollisionWith(const FusionPhysicsBody *other, const Vector2 &collision_point);


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

		//! True if the input state has been modified
		bool m_InputChanged;
		//! True if the ShipState has been modified
		bool m_StateChanged;
	};

}

#endif
