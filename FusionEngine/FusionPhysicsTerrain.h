#ifndef Header_FusionEngine_FusionPhysicsTerrain
#define Header_FusionEngine_FusionPhysicsTerrain

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Inherited
#include "FusionPhysicsStatic.h"

namespace FusionEngine
{
	//! Ease of storage for holes. Not used ATM.
	struct Hole
	{
		//! x loc
		int x;
		//! y loc
		int y;
		//! rad
		int radius;
	};

	/*!
	 * \brief
	 * To terrain what FusionShip is to ships.
	 *
	 * Has functions for removing sections of the bitmask (to be used in
	 * collision callbacks of the bodies that can destroy it.)
	 *
	 * \sa
	 * TerrainResource
	 */
	class Terrain
	{
	public:
		/*!
		 * \brief
		 * Constructor.
		 */
		Terrain();
		//! Virtual destructor.
		virtual ~Terrain();

	public:
		void 
		//! Marks a point with a hole, so the Environment will create one there
		void MakeHole(int x, int y, int radius);

		//! Returns the next unmade hole
		Hole PopNextHole();

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