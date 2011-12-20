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

#ifndef Header_FusionEngine_AIModule
#define Header_FusionEngine_AIModule

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Fusion
#include "FusionScript.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Runs AI Scripts passing them a relavant ship ID
	 *
	 * \todo Implement AI supplement system - AI scripts can be augmented by other specific scripts, for example, a basic
	 *  "aggressive script could be used by any ship on any level, but a CTF level requires specific goals to be
	 *  accomplished thus a goal-priority system will also have to be implemented.
	 *  <code>if (getGoalPriority(20) >= getTopPriority()) // do something</code> PERHAPS
	 *
	 * \todo Shared goals for team AI (etc.)
	 *
	 * \sa
	 * Singleton
	 */
	class AIModule
	{
	public:
		//! Creates an AI module for the given script and ship.
		AIModule(Script* scr, ObjectID ship);

	public:
		//! Executes the ai script, passing the ship id
		bool Run(unsigned int split);

		void AddGoal(int id, ObjectID data, std::string tag, GoalPriority priority = GoalPriority::MINIMUM);
		void AddDecision(std::string tag, bool stay);

	protected:
		ObjectID m_Target;

	};

}

#endif
