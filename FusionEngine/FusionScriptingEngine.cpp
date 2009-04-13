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

#include "FusionScriptingEngine.h"

//#include "FusionScriptingFunctions.h"
#include "FusionScriptReference.h"
// Scripting extensions
#include "FusionScriptVector.h"
#include "stdstring.h"
#include "scriptmath.h"

namespace FusionEngine
{

	ScriptingEngine::ScriptingEngine()
		: m_DefaultTimeout(g_ScriptDefaultTimeout)
	{
		m_asEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		if (m_asEngine != NULL)
		{
			m_asEngine->SetMessageCallback(asMETHOD(ScriptingEngine,_messageCallback), this, asCALL_THISCALL);
			registerTypes();
		}
	}


	asIScriptEngine *ScriptingEngine::GetEnginePtr() const
	{
		return m_asEngine;
	}

	void ScriptingEngine::RegisterGlobalObject(const char *decl, void* ptr)
	{
		int r = m_asEngine->RegisterGlobalProperty(decl, ptr); assert( r >= 0 );
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

	ScriptReturn ScriptingEngine::Execute(const char* module, const char* function, unsigned int timeout /* = 0 */)
	{
		int funcID = m_asEngine->GetFunctionIDByDecl(module, function);

		asIScriptContext* cont = m_asEngine->CreateContext();
		ScriptReturn scxt(cont);

		int r = cont->Prepare(funcID);
		if (r < 0)
			return scxt;

		int timeoutTime;
		if (timeout > 0)
		{
			timeoutTime = CL_System::get_time() + timeout;
			cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_STDCALL);
		}

		cont->Execute();

		return scxt;
	}

#ifndef SCRIPT_ARG_USE_TEMPLATE
	ScriptReturn ScriptingEngine::Execute(ScriptMethod function, ScriptArgument p1, ScriptArgument p2, ScriptArgument p3, ScriptArgument p4)
	{
		asIScriptContext* cont = m_asEngine->CreateContext();
		ScriptReturn scxt(cont);

		int r = cont->Prepare(function.GetFunctionID());
		if (r < 0)
			return scxt;

		int timeoutTime;
		if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
		{
			timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
			cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
		}

		cont->Execute();
		return scxt;
	}

	ScriptReturn ScriptingEngine::Execute(ScriptObject object, ScriptMethod method, ScriptArgument p1, ScriptArgument p2, ScriptArgument p3, ScriptArgument p4)
	{
		asIScriptContext* cont = m_asEngine->CreateContext();
		ScriptReturn scxt(cont);

		int r = cont->Prepare(method.GetFunctionID());
		if (r < 0)
			return scxt;

		int timeoutTime;
		if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
		{
			timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
			cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
		}

		r = cont->SetObject(object.GetScriptStruct());
		if (r < 0)
			return scxt;

		cont->Execute();
		return scxt;
	}
#endif

	ScriptContext ScriptingEngine::ExecuteString(const std::string &script, const char *module, int timeout)
	{
		asIScriptContext* ctx = m_asEngine->CreateContext();
		ScriptContext sctx(ctx);

		if (ctx != NULL)
			m_asEngine->ExecuteString(module, script.c_str(), &ctx);

		return sctx;
	}

	void ScriptingEngine::ReExecute(ScriptContext& context)
	{
		context.Execute();
	}

	void ScriptingEngine::SetDefaultTimeout(unsigned int timeout)
	{
		m_DefaultTimeout = timeout;
	}

	unsigned int ScriptingEngine::GetDefaultTimeout() const
	{
		return m_DefaultTimeout;
	}

	ScriptMethod ScriptingEngine::GetFunction(const char* module, const std::string& signature)
	{
		int funcID = m_asEngine->GetFunctionIDByDecl(module, signature.c_str());
		return ScriptMethod(module, signature, funcID);
	}

	bool ScriptingEngine::GetFunction(ScriptMethod& out, const char* module, const std::string& signature)
	{
		int funcID = m_asEngine->GetFunctionIDByDecl(module, signature.c_str());
		if (funcID < 0)
			return false;

		out.SetModule(module);
		out.SetSignature(signature);
		out.SetFunctionID(funcID);

		return true;
	}

	ScriptClass ScriptingEngine::GetClass(const char* module, const std::string& type_name)
	{
		int id = m_asEngine->GetTypeIdByDecl(module, type_name.c_str());
		return ScriptClass(this, module, type_name, id);
	}

	ScriptObject ScriptingEngine::CreateObject(const char* module, const std::string& type_name)
	{
		int id = m_asEngine->GetTypeIdByDecl(module, type_name.c_str());
		asIScriptObject* obj = (asIScriptObject*)m_asEngine->CreateScriptObject(id);
		obj->AddRef();
		return ScriptObject(obj);
	}

	ScriptMethod ScriptingEngine::GetClassMethod(ScriptClass& type, const std::string& signature)
	{
		int id = m_asEngine->GetMethodIDByDecl(type.GetTypeId(), signature.c_str());
		ScriptMethod method(type.GetModule(), signature, id);
		return method;
	}

	ScriptMethod ScriptingEngine::GetClassMethod(ScriptObject& type, const std::string& signature)
	{
		int id = m_asEngine->GetMethodIDByDecl(type.GetTypeId(), signature.c_str());
		ScriptMethod method(0, signature, id);
		return method;
	}

	void ScriptingEngine::_messageCallback(asSMessageInfo* msg)
	{ 
		const char *msgType = 0;
		if( msg->type == 0 ) msgType = "Error  ";
		if( msg->type == 1 ) msgType = "Warning";
		if( msg->type == 2 ) msgType = "Info   ";

		std::string formatted = CL_String::format("ScriptManager - %1 (%2, %3) : %4 : %5", msg->section, msg->row, msg->col, msgType, msg->message);
		SendToConsole(formatted);
	}

	void ScriptingEngine::registerTypes()
	{
		// Register types
		RegisterScriptMath(m_asEngine);
		RegisterStdString(m_asEngine);
		RegisterScriptVector(m_asEngine);

		//RegisterScriptString(m_asEngine);

		//m_asEngine->RegisterGlobalProperty("g_Environment", Environment::getSingletonPtr());

		//! \todo Console, etc methods should be registered by the relavant classes. Weapons now fall under Entity, so there are no relavant methods to be registered for Weapons only

		// Register functions
		//registerWeaponMethods();
		//registerConsoleMethods();
	}

	//void ScriptingEngine::RegisterScript(Script *script, const char *module)
	//{
	//	int r;
	//	r = m_asEngine->AddScriptSection(module, 0, script->GetScript().c_str(), script->GetScript().length());
	//	if (r >= 0)
	//	{
	//		script->_setModule(module);
	//		script->_notifyRegistration();
	//	}
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
