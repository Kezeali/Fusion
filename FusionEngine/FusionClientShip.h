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

#ifndef Header_FusionEngine_FusionClientShip
#define Header_FusionEngine_FusionClientShip

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"
#include "FusionShipState.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Holds all ship state information for each ship.
	 *
	 * State information such as current buttons pressed and health is stored in
	 * this class. Other classes that need such information can gain access to it
	 * via the public members. Unlike the server version, this also holds information
	 * about ascociated drawables and nodes.
	 *
	 * \remarks
	 * An instance of this object holds the state data for each ship in the game.
	 * This object should remain general enough to be used by client and server.
	 * <br>
	 * FusionClientEnvironment and FusionServerEnvironment have friend access to
	 * this class, for efficiant updating.
	 */
	class FusionClientShip
	{
		friend class FusionServerEnvironment;
		friend class FusionClientEnvironment;
	public:
		FusionClientShip();
		FusionClientShip(ShipState initState);
		~FusionClientShip();

	public:
		//! Set Vel
		void SetVelocity(const CL_Vector2 &position);
		//! Set Pos
		void SetPosition(const CL_Vector2 &position);

		//! Guess
		ShipState *GetShipState() const;
		//! Self explainatory
		ShipInput *GetInputState() const;

		//! You know it
		std::string GetShipResource() const;

		//! Reverts all state data
		void RevertToInitialState() const;

	protected:
		// Is this needed?
		//ClientEnvironment *m_Environment;

		FusionNode *m_Node;
		FusionShipDrawable *m_Drawable;
		
		FusionPhysicsBody *m_PhysicalBody;

		std::string m_ResourceID;
		ShipInput m_Input;
		ShipState m_CurrentState;
		ShipState m_InitialState;
	};

}

#endif
