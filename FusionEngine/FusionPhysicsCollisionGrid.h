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

#ifndef Header_FusionEngine_FusionPhysicsCollisionGrid
#define Header_FusionEngine_FusionPhysicsCollisionGrid

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"
#include <deque>

namespace FusionEngine
{
	//! Compartmentises collision checking to keep redundant checks to a minimum.
	class FusionPhysicsCollisionGrid
	{
	public:

		//! Constructor
		FusionPhysicsCollisionGrid();
		//! Destructor
		~FusionPhysicsCollisionGrid();

	public:
		//** Typedefs **
		typedef std::vector<FusionPhysicsBody *> BodyList;
		typedef std::vector<BodyList> BodyListCollection;

		//! Adds an already existing body to the grid.
		void AddBody(FusionPhysicsBody *body);
		//! Removes the given body from the grid.
		void RemoveBody(FusionPhysicsBody *body);
		/*!
		 * Places all bodies at their correct positions within the grid.
		 *
		 * This action is not immediate, the actual sort takes place next
		 * FusionPhysicsCollisionGrid#Resort() is called.
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
		 * Sets the grid scale.
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
		 * Insures a specific body will be updated when #Resort is called.
		 *
		 * Used internally by FusionPhysicsWorld when it moves a body.
		 */
		void _updateThis(FusionPhysicsBody *body);
		/*!
		 * Insures a specific body will be updated when #Resort is called.
		 *
		 * Used internally by FusionPhysicsWorld when it moves a body.
		 */
		void _updateThis(int cgind);
		/*!
		 * Finds the correct position withing the collision grid for a
		 * specific body.
		 */
		int _getGridPosition(FusionPhysicsBody *body);

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