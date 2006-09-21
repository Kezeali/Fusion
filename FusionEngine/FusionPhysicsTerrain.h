#ifndef Header_FusionEngine_FusionPhysicsTerrain
#define Header_FusionEngine_FusionPhysicsTerrain

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Fusion
#include "FusionBody.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * An implimentation of PhysicsBody. Handles bitmask based collisions.
	 *
	 * Has functions for removing sections of the bitmask.
	 *
	 * \sa
	 * FusionBody | FusionPhysicsWorld.
	 */
	class FusionPhysicsTerrain : public FusionPhysicsStatic
	{
	public:
		/*!
		 * \brief
		 * Constructor.
		 *
		 * \param world
		 * The world in which this body resides.
		 */
		FusionPhysicsTerrain(FusionPhysicsWorld *world);
		//! Virtual destructor.
		virtual ~FusionPhysicsTerrain();

	public:
		void MakeHole(int x, int y, int radius);

	};

}

#endif