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

#ifndef Header_FusionEngine_AIManager
#define Header_FusionEngine_AIManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Fusion
#include "FusionScript.h"
#include "FusionScriptingEngine.h"

namespace FusionEngine
{

	//const g_ASConfigAI = "AIConfig";

	/*!
	 * \brief
	 * AI manager.
	 *
	 * Lists and runs AIModules
	 *
	 * \sa ScriptingEngine | AIModule
	 */
	class AIManager : public Singleton<AIManager>
	{
	public:
		//! Creates an AI manager linked to the given scripting engine.
		AIManager(ScriptingEngine *engine);

	public:
		void AddGoal(ObjectID group, int id, ObjectID target);
		ObjectID GetGoal(ObjectID group, int id);
		
	private:
		//! The scripting engine in use
		ScriptingEngine* m_Engine;

	private:
		//! Registers global methods which scripts can use.
		void registerAIConfiguration();

	};

	//////////////////
	// Script methods
	void AI_AddDecision(ObjectID ship, const std::string& tag);
	void AI_AddGoal(ObjectID ship, int id, ObjectID target);

	void AI_GetGoal(ObjectID ship, int id);

	void AI_GetCurrentWeapon(ObjectID ship);
	void AI_GetCurrentAmmo(ObjectID ship);
	void AI_GetCurrentHealth(ObjectID ship);
	void AI_GetCurrentFacing(ObjectID ship);
	void AI_GetNearestShip(ObjectID ship);

}

#endif
