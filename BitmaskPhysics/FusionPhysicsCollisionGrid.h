/*
  Copyright (c) 2006 FusionTeam

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

#ifndef Header_FusionEngine_FusionPhysicsCollisionGrid
#define Header_FusionEngine_FusionPhysicsCollisionGrid

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionPhysicsBody.h"

namespace FusionEngine
{
	//! Compartmentises collision checking to keep redundant checks to a minimum.
	class FusionPhysicsCollisionGrid
	{
	public:

		//! Constructor
		FusionPhysicsCollisionGrid();
		//! Constructor + initialiser
		FusionPhysicsCollisionGrid(float scale, int level_x, int level_y);
		//! Destructor
		~FusionPhysicsCollisionGrid();

	public:
		//typedef std::vector<FusionPhysicsBody *> BodyList;
		//! Type for a collection of BodyLists
		typedef std::vector<BodyList> BodyListCollection;

	public:

		//! Adds an already existing body to the grid.
		void AddBody(FusionPhysicsBody *body);
		//! Removes the given body from the grid.
		void RemoveBody(FusionPhysicsBody *body);
		/*!
		 * \brief Places all bodies at their correct positions within the grid.
		 *
		 * This action is not immediate, the actual sort takes place the next
		 * time FusionPhysicsCollisionGrid#Resort() is called.
		 * Call FusionPhysicsCollisionGrid#ForceResortAll() for an 
		 * immeadiate resort.
		 */
		void ResortAll();
		//! Warning! This function is very slow; only call if you are sure you need to.
		void ForceResortAll();
		//! Removes all values from the grid.
		void Clear();
		//! Moves bodies which have marked themselves as needing an update.
		void Resort();
		/*!
		 * \brief Sets the grid scale.
		 *
		 * The level dimensions must be provided to work out the new grid
		 * dimensions.
		 *
		 * \param scale The scale of the grid compared to the level.
		 *  From 0.01f to 1.f
		 *
		 * \param level_x Self explanatory.
		 */
		void SetScale(float scale, int level_x, int level_y);
		//! Retreives the scale property.
		float GetScale() const;
		/*!
		 * \brief Checks for bodies which may collide with the given body.
		 *
		 * This is probably the most important function of the CollisionBody class.
		 *
		 * \returns a STL vector of FusionPhysicsBodys
		 */
		BodyList FindAdjacentBodies(FusionPhysicsBody *body);

		/*!
		 * \brief Ensures a specific body will be updated when #Resort is called.
		 *
		 * Used internally by FusionPhysicsWorld when it moves a body.
		 */
		void _updateThis(FusionPhysicsBody *body);
		/*!
		 * [depreciated] Ensures a specific body will be updated when #Resort is called.
		 *
		 * Used internally by FusionPhysicsWorld when it moves a body.
		 */
		//void _updateThis(int cgind);

		/*!
		 * Finds the correct position (i.e. vector index) within the collision grid
		 * for a specific body.
		 */
		unsigned int _getGridPosition(FusionPhysicsBody *body);

	protected:
		//! The ratio of the grid coords to the physics world coords.
		float m_GridScale;
		//! The dimensions of the grid.
		int m_GridWidth, m_GridHeight;
		//! [depreciated] All bodies which have been added are listed here, for resort purposes.
		BodyList m_Bodies;
		//! Guess :P
		BodyListCollection m_Grid;

		//! Self explainatory (nb. this replaces m_Bodies.)
		BodyList m_BodiesToUpdate;

	};

}

#endif