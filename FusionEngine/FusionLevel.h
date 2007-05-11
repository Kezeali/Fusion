#ifndef Header_FusionEngine_FusionPhysicsTerrain
#define Header_FusionEngine_FusionPhysicsTerrain

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "../micropather/micropather.h"

/// Fusion
#include "FusionPhysicsBody.h"

namespace FusionEngine
{
	//! Ease of storage for holes.
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
	 * Level is to terrain what FusionShip is to ships.
	 *
	 * Has functions for removing sections of the bitmask (to be used in
	 * collision callbacks of the bodies that can destroy it.)
	 *
	 * \sa
	 * LevelResource | FusionShip
	 */
	class Level : public micropather::Graph
	{
	public:
		//! Hole queue
		typedef std::deque<Hole> HoleQueue;
	public:
		//! Constructor.
		Level(FusionPhysicsBody* body, FusionNode *node);
		//! Virtual destructor.
		virtual ~Level() {}

	public:
		//! Marks a point with a hole, so the Environment will create one there
		void MakeHole(int x, int y, int radius);

		//! Returns the next unmade hole
		Hole PopNextHole();

		//! This is called by the Environment on hole creation.
		/*!
		 * When the Environment has validated the new hole, it calls this to make sure
		 * the bitmask is updated.
		 */
		void _madeHole(const CL_Surface *surface);

		//! Run region scrips on the given ship
		void RunRegionScripts(const FusionShip *ship);

		//! Implementation of micropather#Graph#LeastCostEstimate()
		float LeastCostEstimate(void* stateStart, void* stateEnd);

	protected:
		//! List of unmade holes.
		HoleQueue m_Holes;

		int m_BitmaskRes;
		//! The physical body associated with this object
		FusionPhysicsBody *m_TerrainBody;

		//! Scene node
		FusionNode* m_Node;

		//! Scene drawable
		LevelDrawable* m_TerrainDrawable;

	};

}

#endif