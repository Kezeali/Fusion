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


	File Author(s):

		Elliot Hayward
*/

#include "FusionScriptingEngine.h"

#include "FusionScriptingFunctions.h"
#include "as_scriptstring.h"
#include "FusionScriptVector.h"

namespace FusionEngine
{
	ScriptingEngine::ScriptingEngine()
	{
		m_asEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		registerGlobals();
	}


	asIScriptEngine *ScriptingEngine::GetEnginePtr() const
	{
		return m_asEngine;
	}

	void ScriptingEngine::RegisterScript(Script *script, const char *module)
	{
	}

	int ScriptingEngine::ExecuteScript(Script *script, const char *function)
	{
		return 0;
	}

	int ScriptingEngine::ExecuteString(const std::string &script, const char *module, int *context, int timeout, bool keep_context)
	{
		return 0;
	}

	int ScriptingEngine::ReExecuteString(int context, const char *module)
	{
		return 0;
	}

	void ScriptingEngine::_lineCallback(asIScriptContext *ctx, int *timeOut)
	{
		// If the time out is reached, abort the script
		if( *timeOut < CL_System::get_time() )
			ctx->Abort();
	}


	void ScriptingEngine::registerGlobals()
	{
		// Register types
		RegisterScriptString(m_asEngine);
		RegisterScriptVector(m_asEngine);

		// Register functions
		registerWeaponMethods();
		registerConsoleMethods();
	}

	
	void ScriptingEngine::registerWeaponMethods()
	{
		int r;

		r = m_asEngine->BeginConfigGroup(g_ASConfigWeapon.c_str()); cl_assert( r >= 0 );
		{
			if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
			{
				r = m_asEngine->RegisterGlobalFunction("void DetonateProjectile(uint16)", asFUNCTION(SCR_DetonateProjectile), asCALL_CDECL); cl_assert( r >= 0 );

				r = m_asEngine->RegisterGlobalFunction("void ApplyEngineForce(uint16)", asFUNCTION(SCR_ApplyEngineForce), asCALL_CDECL); cl_assert( r >= 0 );
				r = m_asEngine->RegisterGlobalFunction("void ApplyForce(uint16, Vector)", asFUNCTION(SCR_ApplyForce), asCALL_CDECL); cl_assert( r >= 0 );
			}
			else
			{
				r = m_asEngine->RegisterGlobalFunction("void DetonateProjectile(uint16)", asFUNCTION(SCR_DetonateProjectileG), asCALL_GENERIC); cl_assert( r >= 0 );

				r = m_asEngine->RegisterGlobalFunction("void ApplyEngineForce(uint16)", asFUNCTION(SCR_ApplyEngineForceG), asCALL_GENERIC); cl_assert( r >= 0 );
				r = m_asEngine->RegisterGlobalFunction("void ApplyForce(uint16, Vector)", asFUNCTION(SCR_ApplyForceG), asCALL_GENERIC); cl_assert( r >= 0 );
			}
		}
		r = m_asEngine->EndConfigGroup(); cl_assert( r >= 0 );
	}


	void ScriptingEngine::registerConsoleMethods()
	{
		int r;

		r = m_asEngine->BeginConfigGroup(g_ASConfigConsole.c_str()); cl_assert( r >= 0 );
		{
			if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
			{
				r = m_asEngine->RegisterGlobalFunction("GetProjectileList(uint16)", asFUNCTION(CON_ListProjectiles), asCALL_CDECL); cl_assert( r >= 0 );

				r = m_asEngine->RegisterGlobalFunction("DetonateProjectile(uint16)", asFUNCTION(SCR_DetonateProjectile), asCALL_CDECL); cl_assert( r >= 0 );

				r = m_asEngine->RegisterGlobalFunction("ApplyEngineForce(uint16)", asFUNCTION(SCR_ApplyEngineForce), asCALL_CDECL); cl_assert( r >= 0 );
				r = m_asEngine->RegisterGlobalFunction("ApplyForce(uint16, Vector)", asFUNCTION(SCR_ApplyForce), asCALL_CDECL); cl_assert( r >= 0 );
			}
			else
			{
				r = m_asEngine->RegisterGlobalFunction("GetProjectileList(uint16)", asFUNCTION(CON_ListProjectilesG), asCALL_CDECL); cl_assert( r >= 0 );

				r = m_asEngine->RegisterGlobalFunction("DetonateProjectile(uint16)", asFUNCTION(SCR_DetonateProjectileG), asCALL_GENERIC); cl_assert( r >= 0 );
				
				r = m_asEngine->RegisterGlobalFunction("ApplyEngineForce(uint16)", asFUNCTION(SCR_ApplyEngineForceG), asCALL_GENERIC); cl_assert( r >= 0 );
				r = m_asEngine->RegisterGlobalFunction("ApplyForce(uint16, Vector)", asFUNCTION(SCR_ApplyForceG), asCALL_GENERIC); cl_assert( r >= 0 );
			}
		}
		r = m_asEngine->EndConfigGroup(); cl_assert( r >= 0 );
	}

}
