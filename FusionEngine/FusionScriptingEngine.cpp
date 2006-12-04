
#include "FusionScriptingEngine.h"

namespace FusionEngine
{

	asIScriptEngine *ScriptingEngine::GetEnginePtr() const
	{
		return m_asEngine;
	}

	int ScriptingEngine::ExecuteScript(Script *script)
	{
	}

	int ScriptingEngine::ExecuteString(const std::string &script, char *module, int *context, int timeout, int keep_context)
	{
	}

	int ScriptingEngine::ReExecuteString(int context, char *module)
	{
	}

	void ScriptingEngine::_lineCallback(asIScriptContext *ctx, int *timeOut)
	{
		// If the time out is reached, abort the script
		if( *timeOut < CL_System::get_time() )
			ctx->Abort();
	}

}
