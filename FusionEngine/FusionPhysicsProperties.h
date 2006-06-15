#ifndef Header_FusionEngine_FusionPhysicsBody
#define Header_FusionEngine_FusionPhysicsBody
#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Holds data about a physical object.
	 *
	 * \remarks
	 * For FusionShips this should be initialisd by the ShipFactory when it creates
	 * a new ship. It should remain indipendant of the ClientEnvironment after that
	 * point - all modification to it can be done manually, rather than requiring it
	 * to know of ShipResource.
	 * 
	 * \see
	 * FusionPhysicsBody
	 */
	class FusionPhysicsProperties
	{
	public:
		float Mass;
		CL_Vector2 AppliedForce;
		Cl_Vector2 Acceleration;
		Cl_Vector2 Velocity;
		Cl_Vector2 Position;

		//! Each body can have a derived class for different collison responses :D.
		virtual void CollisionResponse() {};

	};

}

#endif