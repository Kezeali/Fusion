/*
  Copyright (c) 2006 Fusion Project Team

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

#ifndef Header_FusionEngine_CollisionGrid
#define Header_FusionEngine_CollisionGrid

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionPhysicsBody.h"

namespace FusionEngine
{
	//! Compartmentises collision checking to keep redundant checks to a minimum.
	class CollisionGrid
	{
	public:

		//! Constructor
		CollisionGrid();
		//! Constructor + initialization
		CollisionGrid(int cell_w, int cell_h, int level_x, int level_y);
		//! Destructor
		~CollisionGrid();

	public:
		//typedef std::vector<FusionPhysicsBody *> BodyList;
		//! Type for a collection of BodyLists
		typedef std::vector<BodyList> BodyListCollection;

	public:

		//! Adds an already existing body to the grid.
		void AddBody(PhysicsBody *body);
		//! Removes the given body from the grid.
		void RemoveBody(PhysicsBody *body);
		/*!
		 * \brief Places all bodies at their correct positions within the grid.
		 *
		 * This action is not immediate, the actual sort takes place the next
		 * time CollisionGrid#Resort() is called.
		 * Call CollisionGrid#ForceResortAll() for an 
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
		 * \brief [depreciated], use SetCellSize().
		 *
		 * Sets the grid scale.
		 * The level dimensions must be provided to work out the new grid
		 * dimensions.
		 *
		 * \param scale The scale of the grid compared to the level.
		 *  From 0.01f to 1.f
		 *
		 * \param level_x Self explanatory.
		 */
		void SetScale(float scale, int level_x, int level_y);
		//! [depreciated] Retreives the length of the scale vector.
		float GetScale() const { assert(0); }

		/*!
		 * \brief Sets the grid scale.
		 *
		 * The level dimensions must be provided to work out the new grid
		 * dimensions.
		 *
		 * \param cell_w
		 * The width of each cell in pixels.
		 *
		 * \param level_x Self explanatory.
		 */
		void SetCellSize(int cell_w, int cell_h, int level_x, int level_y);
		//! Get cell width property
		int GetCellWidth() const;
		//! Get cell height property
		int GetCellHeight() const;
		/*!
		 * \brief Checks for bodies which may collide with the given body.
		 *
		 * This is probably the most important function of the CollisionBody class.
		 *
		 * \returns a STL vector of FusionPhysicsBodys
		 */
		BodyList FindAdjacentBodies(PhysicsBody *body);

		/*!
		 * \brief Finds bodies adjacent to the cell at the given co-ord
		 *
		 * \returns a STL vector of FusionPhysicsBodys
		 */
		BodyList FindAdjacentBodies(float x, float y);

		/*!
		 * \brief
		 * Returns the contents of the cells adjacent to the cell at the given index
		 *
		 * x and y are passed to optimise preformance
		 */
		BodyList _findAdjacentBodies(int x, int y, int cell_index);

		/*!
		 * \brief Ensures a specific body will be updated when #Resort is called.
		 *
		 * Used internally by FusionPhysicsWorld when it moves a body.
		 */
		void _updateThis(PhysicsBody *body);
		/*!
		 * [depreciated] Ensures a specific body will be updated when #Resort is called.
		 *
		 * Used internally by FusionPhysicsWorld when it moves a body.
		 */
		//void _updateThis(int cgind);

		/*!
		 * Finds the correct position (i.e. array index) within the collision grid
		 * for a specific body.
		 */
		unsigned int _getGridPosition(PhysicsBody *body) const;

		/*!
		 * Finds the correct position (i.e. array index) within the collision grid
		 * for a specific x,y co-ord.
		 */
		unsigned int _getGridPosition(float x, float y) const;

		//! Converts the given x ord grid scale
		inline unsigned int _scaleX(float x) const;
		//! Converts the given y ord grid scale
		inline unsigned int _scaleY(float y) const;

		//! Converts the given grid co-ord to an array index
		inline int _getIndex(int gx, int gy) const;

		void DebugDraw() const;

	protected:
		//! Cell width and height
		int m_CellWidth, m_CellHeight;
		//! The ratio of the grid coords to the physics world coords.
		double m_GridXScale, m_GridYScale;
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