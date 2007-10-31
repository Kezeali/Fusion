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

//#include "FusionScriptingFunctions.h"
#include "FusionScriptReference.h"
#include "scriptstring.h"
#include "FusionScriptVector.h"

namespace FusionEngine
{

	ScriptingEngine::ScriptingEngine()
	{
		m_asEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		if (m_asEngine != NULL)
		{
			m_asEngine->SetMessageCallback(asMETHOD(COutStream,Callback), &m_Out, asCALL_THISCALL);
			registerGlobals();
		}
	}


	asIScriptEngine *ScriptingEngine::GetEnginePtr() const
	{
		return m_asEngine;
	}

	void ScriptingEngine::RegisterScript(Script *script, const char *module)
	{
		int r;
		r = m_asEngine->AddScriptSection(module, 0, script->GetScript().c_str(), script->GetScript().length());
		if (r >= 0)
		{
			script->_setModule(module);
			script->_notifyRegistration();
		}
	}

	bool ScriptingEngine::AddCode(const std::string& script, const char *module)
	{
		int r;
		r = m_asEngine->AddScriptSection(module, 0, script.c_str(), script.length());
		return r >= 0;
	}

	bool ScriptingEngine::BuildModule(const char* module)
	{
		return m_asEngine->Build(module) >= 0;
	}

	int ScriptingEngine::ExecuteModule(const char* module, const char* function)
	{
		int funcID = m_asEngine->GetFunctionIDByDecl(module, function);

		asIScriptContext* cont = m_asEngine->CreateContext();
		int r = cont->Prepare(funcID);
		if (r < 0)
			return r;
		//m_Contexts.push_back(cont);
		return cont->Execute();
	}

	int ScriptingEngine::ExecuteScript(Script *script, const char *function)
	{
		int funcID = m_asEngine->GetFunctionIDByDecl(script->GetModule(), function);

		asIScriptContext* cont = m_asEngine->CreateContext();
		int r = cont->Prepare(funcID);
		if (r < 0)
			return r;
		//m_Contexts.push_back(cont);
		return cont->Execute();
	}

	int ScriptingEngine::Execute(ScriptReference scref)
	{
		asIScriptContext* cont = m_asEngine->CreateContext();
		int r = cont->Prepare(scref.GetFunctionID());
		if (r < 0)
			return r;
		//m_Contexts.push_back(cont);
		return cont->Execute();
	}

	int ScriptingEngine::ExecuteString(const std::string &script, const char *module, int *context, int timeout, bool keep_context)
	{
		int r = 0;
		if (context != NULL)
		{
			asIScriptContext* pContext = m_asEngine->CreateContext();
			if (pContext != NULL)
			{
				m_Contexts.push_back(pContext);
				*context = m_Contexts.size()-1;

				r = m_asEngine->ExecuteString(module, script.c_str(), &pContext);
			}
		}
		else
			r = m_asEngine->ExecuteString(module, script.c_str());

		return r;
	}

	int ScriptingEngine::ReExecuteString(int context, const char *module)
	{
		int r;
		r = m_Contexts[context]->Execute();
		return r;
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

		//m_asEngine->RegisterGlobalProperty("g_Environment", Environment::getSingletonPtr());

		//! \todo Console, etc methods should be registered by the relavant classes. Weapons now fall under Entity, so there are no relavant methods to be registered for Weapons only

		// Register functions
		//registerWeaponMethods();
		//registerConsoleMethods();
	}

	
	//void ScriptingEngine::registerWeaponMethods()
	//{
	//	int r;

	//	r = m_asEngine->BeginConfigGroup(g_ASConfigWeapon.c_str()); cl_assert( r >= 0 );
	//	{
	//		if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	//		{
	//			r = m_asEngine->RegisterGlobalFunction("void DetonateProjectile(uint16)", asFUNCTION(SCR_DetonateProjectile), asCALL_CDECL); cl_assert( r >= 0 );

	//			r = m_asEngine->RegisterGlobalFunction("void ApplyEngineForce(uint16)", asFUNCTION(SCR_ApplyEngineForce), asCALL_CDECL); cl_assert( r >= 0 );
	//			r = m_asEngine->RegisterGlobalFunction("void ApplyForce(uint16, Vector)", asFUNCTION(SCR_ApplyForce), asCALL_CDECL); cl_assert( r >= 0 );
	//		}
	//		else
	//		{
	//			r = m_asEngine->RegisterGlobalFunction("void DetonateProjectile(uint16)", asFUNCTION(SCR_DetonateProjectileG), asCALL_GENERIC); cl_assert( r >= 0 );

	//			r = m_asEngine->RegisterGlobalFunction("void ApplyEngineForce(uint16)", asFUNCTION(SCR_ApplyEngineForceG), asCALL_GENERIC); cl_assert( r >= 0 );
	//			r = m_asEngine->RegisterGlobalFunction("void ApplyForce(uint16, Vector)", asFUNCTION(SCR_ApplyForceG), asCALL_GENERIC); cl_assert( r >= 0 );
	//		}
	//	}
	//	r = m_asEngine->EndConfigGroup(); cl_assert( r >= 0 );
	//}


	//void ScriptingEngine::registerConsoleMethods()
	//{
	//	int r;

	//	r = m_asEngine->BeginConfigGroup(g_ASConfigConsole.c_str()); cl_assert( r >= 0 );
	//	{
	//		if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	//		{
	//			r = m_asEngine->RegisterGlobalFunction("GetProjectileList(uint16)", asFUNCTION(CON_ListProjectiles), asCALL_CDECL); cl_assert( r >= 0 );

	//			r = m_asEngine->RegisterGlobalFunction("DetonateProjectile(uint16)", asFUNCTION(SCR_DetonateProjectile), asCALL_CDECL); cl_assert( r >= 0 );

	//			r = m_asEngine->RegisterGlobalFunction("ApplyEngineForce(uint16)", asFUNCTION(SCR_ApplyEngineForce), asCALL_CDECL); cl_assert( r >= 0 );
	//			r = m_asEngine->RegisterGlobalFunction("ApplyForce(uint16, Vector)", asFUNCTION(SCR_ApplyForce), asCALL_CDECL); cl_assert( r >= 0 );
	//		}
	//		else
	//		{
	//			r = m_asEngine->RegisterGlobalFunction("GetProjectileList(uint16)", asFUNCTION(CON_ListProjectilesG), asCALL_CDECL); cl_assert( r >= 0 );

	//			r = m_asEngine->RegisterGlobalFunction("DetonateProjectile(uint16)", asFUNCTION(SCR_DetonateProjectileG), asCALL_GENERIC); cl_assert( r >= 0 );
	//			
	//			r = m_asEngine->RegisterGlobalFunction("ApplyEngineForce(uint16)", asFUNCTION(SCR_ApplyEngineForceG), asCALL_GENERIC); cl_assert( r >= 0 );
	//			r = m_asEngine->RegisterGlobalFunction("ApplyForce(uint16, Vector)", asFUNCTION(SCR_ApplyForceG), asCALL_GENERIC); cl_assert( r >= 0 );
	//		}
	//	}
	//	r = m_asEngine->EndConfigGroup(); cl_assert( r >= 0 );
	//}

}
