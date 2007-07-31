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

#include "FusionAIManager.h"

namespace FusionEngine
{

	AIManager::AIManager(ScriptingEngine *engine)
		: m_Engine(engine)
	{
	}

	AIManager::registerAIConfiguration()
	{
		int r;
		asIScriptEngine* asEngine = m_Engine->GetEnginePtr();

		r = asEngine->BeginConfigGroup(g_ASConfigAI); cl_assert( r >= 0 );
		{
			if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
			{
				r = asEngine->RegisterGlobalFunction("AddDecision", asFUNCTION(AI_AddDecision), asCALL_CDECL); cl_assert( r >= 0 );
				r = asEngine->RegisterGlobalFunction("AddGoal", asFUNCTION(AI_AddGoal), asCALL_CDECL); cl_assert( r >= 0 );

				r = asEngine->RegisterGlobalFunction("GetCurrentWeapon", asFUNCTION(AI_GetCurrentWeapon), asCALL_CDECL); cl_assert( r >= 0 );
				r = asEngine->RegisterGlobalFunction("GetCurrentAmmo", asFUNCTION(AI_GetCurrentAmmo), asCALL_CDECL); cl_assert( r >= 0 );
				r = asEngine->RegisterGlobalFunction("GetCurrentHealth", asFUNCTION(AI_GetCurrentHealth), asCALL_CDECL); cl_assert( r >= 0 );
				r = asEngine->RegisterGlobalFunction("GetCurrentFacing", asFUNCTION(AI_GetCurrentFacing), asCALL_CDECL); cl_assert( r >= 0 );

				r = asEngine->RegisterGlobalFunction("GetNearestShip", asFUNCTION(AI_GetNearestShip), asCALL_CDECL); cl_assert( r >= 0 );
			}
			else
			{
				r = m_asEngine->RegisterGlobalFunction("AddDecision", asFUNCTION(AI_AddDecisionG), asCALL_GENERIC); cl_assert( r >= 0 );

				r = m_asEngine->RegisterGlobalFunction("ApplyEngineForce", asFUNCTION(SCR_ApplyEngineForceG), asCALL_GENERIC); cl_assert( r >= 0 );
				r = m_asEngine->RegisterGlobalFunction("ApplyForce", asFUNCTION(SCR_ApplyForceG), asCALL_GENERIC); cl_assert( r >= 0 );
			}
		}
		r = asEngine->EndConfigGroup(); cl_assert( r >= 0 );
	}


	//////////////////
	// Script methods
	void AI_AddGoal(ObjectID ship, int id, ObjectID target)
	{
		AIManager* aiman = AIManager->getSingletonPtr();
		if (aiman == 0)
			throw Exception(Exception::INTERNAL_ERROR, "AIManager not initialised");
		aiman->AddGoal(ObjectID ship, id, ObjectID target);
	}

	void AI_AddDecision(ObjectID ship, const std::string& tag)
	{
		AIManager* aiman = AIManager->getSingletonPtr();
		if (aiman == 0)
			throw Exception(Exception::INTERNAL_ERROR, "AIManager not initialised");
		aiman->AddDecision(ObjectID ship, const std::string& tag);
	}

	ObjectID AI_GetGoal(ObjectID ship, int id);
	{
		AIManager* aiman = AIManager->getSingletonPtr();
		if (aiman == 0)
			throw Exception(Exception::INTERNAL_ERROR, "AIManager not initialised");
		aiman->GetGoal(ObjectID ship, ObjectID target);
	}

	void AI_GetCurrentWeapon(ObjectID ship);
	void AI_GetCurrentAmmo(ObjectID ship);
	void AI_GetCurrentHealth(ObjectID ship);
	void AI_GetCurrentFacing(ObjectID ship);
	void AI_GetNearestShip(ObjectID ship);

}