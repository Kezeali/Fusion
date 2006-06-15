#ifndef Header_FusionEngine_FusionClientShip
#define Header_FusionEngine_FusionClientShip

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Holds all client side ship state information for each ship.
	 *
	 * State information such as current buttons pressed and health is stored in
	 * this class. Other classes that need such information can gain access to it
	 * via the public members. Unlike the server version, this also holds information
	 * about ascociated drawables and nodes.
	 *
	 * \remarks
	 * An instance of this object holds the state data for each ship in the game.
	 * This object should remain general enough to be used by client and server.
	 */
	class FusionClientShip
	{
	public:

		void SetVelocity(const CL_Vector2 &position);
		void SetPosition(const CL_Vector2 &position);

		const ShipState &GetShipState() const;
		const InputState &GetInputState() const;

		const ShipResource &GetShipResource() const;

		void RevertToInitialState() const;

		FusionClientShip();
		FusionClientShip(ShipState initState);
		~FusionClientShip();

	protected:
		ClientEnvironment *m_Environment;

		FusionNode *m_Node;
		FusionShipDrawable *m_Drawable;
		
		FusionPhysicsElipse *m_PhysicalBody;

		std::string m_ResourceID;
		ShipInput m_Input;
		ShipState m_CurrentState;
		ShipState m_InitialState;
	};

}

#endif
