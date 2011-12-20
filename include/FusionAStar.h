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

#ifndef Header_FusionEngine_AStar
#define Header_FusionEngine_AStar

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionLevel.h"

#include "micropather.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Stores all AStarPathways.
	 */
	class AStar
	{
	public:
		typedef std::vector<AStarPathway*> PathList;

	public:
		//! Constructor.
		/*!
		 * Graph will usually be a FusionEngine#Level object.
		 */
		AStar(micropather::Graph* graph, int width);
		//! Constructor
		AStar(Level* graph);

	public:
		//! Finds and stores a path, returning an index that can be used to access the steering pathway
		int Solve(const Vector2& start, const Vector2& end);
		//! Gets a path to be used by steer
		AStarPathway* GetPathway(int index);


	protected:
		PathList m_Paths;

		micropather::Graph* m_Graph;
		micropather::MicroPather* m_Pather;

	};

}

#endif
