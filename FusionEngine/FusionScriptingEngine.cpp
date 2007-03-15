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
	}

	int ScriptingEngine::ExecuteString(const std::string &script, const char *module, int *context, int timeout, int keep_context)
	{
	}

	int ScriptingEngine::ReExecuteString(int context, const char *module)
	{
	}

	void ScriptingEngine::_lineCallback(asIScriptContext *ctx, int *timeOut)
	{
		// If the time out is reached, abort the script
		if( *timeOut < CL_System::get_time() )
			ctx->Abort();
	}


	void ScriptingEngine::registerGlobals()
	{
		int r;

		// Register types
		RegisterScriptString(m_asEngine);
		RegisterScriptVector(m_asEngine);

		// Register functions
		RegisterWeaponMethods();
		RegisterConsoleMethods();
	}

}
