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
	 * An implimentation of PhysicsBody. Handles distance based collisions.
	 * 
	 * \sa
	 * FusionBody | FusionPhysicsWorld.
	 */
	class FusionPhysicsTerrain : public FusionPhysicsBody
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

	};

}

#endif