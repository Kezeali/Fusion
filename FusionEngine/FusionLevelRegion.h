/*
  Copyright (c) 2006-2007 Fusion Project Team

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


	File Author(s):

		Elliot Hayward
*/

#ifndef Header_FusionEngine_LevelRegion
#define Header_FusionEngine_LevelRegion

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "micropather.h"

/// Fusion
#include "FusionPhysicsBody.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * A Region (script trigger) within a level.
	 *
	 * \sa
	 * Level | LevelResourceBundle
	 */
	class LevelRegion : public micropather::Graph
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

		//! Util for creating the path cost graph
		void InitAStarGraph();

		//! Implementation of micropather#Graph#LeastCostEstimate()
		float LeastCostEstimate(void* stateStart, void* stateEnd);

		//! Implementation of micropather#Graph#AdjacentCost()
		void AdjacentCost(void* state, std::vector<micropather::StateCost> *adjacent);

		//! Implementation of micropather#Graph#PrintStateInfo()
		void PrintStateInfo(void* state);

	protected:
		CL_Rectf m_Location;

		ScriptFuncSig m_Function;

	};

}

#endif