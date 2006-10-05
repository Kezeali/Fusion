#ifndef Header_FusionEngine_FusionPhysicsTerrain
#define Header_FusionEngine_FusionPhysicsTerrain

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Fusion
#include "FusionPhysicsStatic.h"

namespace FusionEngine
{
	//! Ease of storage for holes. Not used ATM.
	struct Hole
	{
		//! Data
		int x, y, radius;
	};

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
		//! Marks a point with a hole, so the Environment will create one there
		void MakeHole(int x, int y, int radius);

		//! Returns the next unmade hole
		Hole GetNextHole();

		//! This is called by the Environment on hole creation.
		/*!
		 * When the Environment has finished making a hole in the terrain image,
		 * it calls this so the FusionPhysicsTerrain will update its bitmask.
		 */
		void _madeHole(const CL_Surface *surface);

	protected:
		//! List of unmade holes.
		std::deque<Hole> m_Holes;

	};

}

#endif