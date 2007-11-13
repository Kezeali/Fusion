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

#ifndef Header_FusionEngine_ScriptingEngine
#define Header_FusionEngine_ScriptingEngine

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

/// Fusion
#include "FusionScript.h"
#include "FusionScriptVector.h"
#include "FusionScriptReference.h"

#include "FusionConsole.h"


#define SCRIPT_ARG_USE_TEMPLATE

namespace FusionEngine
{

	static const std::string g_ASConfigConsole = "Console";
	static const std::string g_ASConfigWeapon = "Weapon";

	//! Aborts the given ctx if the current time is after the given time
	/*!
	 * \remarks
	 * This has to be static so we can (easily) have a different timeoutTime for
	 * concurrently running scripts.
	 */
	static void TimeoutCallback(asIScriptContext *ctx, int* timeoutTime)
	{
		// If the time out is reached, abort the script
		if( *timeoutTime < CL_System::get_time() )
			ctx->Abort();
	}

	/*!
	 * \brief
	 * Provides scripting support, and access to it, for all FusionEngine objects.
	 *
	 * \sa
	 * Singleton
	 */
	class ScriptingEngine : public Singleton<ScriptingEngine>
	{
	public:
		//! Basic constructor.
		ScriptingEngine();

	public:
		//! Returns a pointer to the AS engine.
		asIScriptEngine *GetEnginePtr() const;

		//! Adds the given script to a module, but doesn't build or execute it.
		void RegisterScript(Script *script, const char *module);

		//! Adds code to the given module
		bool AddCode(const std::string& script, const char *module);

		//! Builds the given module
		bool BuildModule(const char *module);

		//! Executes the given funcion in a module.
		/*!
		 * If the module is unbuilt, it will be built.
		 *
		 * \param[in] module A built or unbuilt module
		 *
		 * \returns The exit status of the function
		 */
		ScriptReturn Execute(const char *module, const char *function, unsigned int timeout = 0);

#ifdef SCRIPT_ARG_USE_TEMPLATE
		//! Executes the given function
		ScriptReturn Execute(ScriptMethod function)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(function.GetFunctionID());
			if (r < 0)
				return scxt;

			int timeoutTime;
			if (function.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (function.GetTimeout() > 0 ? function.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}

			cont->Execute();
			return scxt;
		}

		//! Executes the given method
		ScriptReturn Execute(ScriptObject object, ScriptMethod method)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			assert(method.GetFunctionID() == m_asEngine->GetMethodIDByDecl(object.GetTypeId(), method.GetSignature().c_str()));

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

		//! Executes the given function
		template<typename T1>
		ScriptReturn Execute(ScriptMethod method, T1 p1)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(function.GetFunctionID());
			if (r < 0)
				return scxt;

			setArgument(cont, 0, p1);

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}

			cont->Execute();
			return scxt;
		}
		//! Executes the given function
		template<typename T1, typename T2>
		ScriptReturn Execute(ScriptMethod method, T1 p1, T2 p2)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(function.GetFunctionID());
			if (r < 0)
				return scxt;

			setArgument(cont, 0, p1);
			setArgument(cont, 1, p2);

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}

			cont->Execute();
			return scxt;
		}
		//! Executes the given function
		template<typename T1, typename T2, typename T3>
		ScriptReturn Execute(ScriptMethod method, T1 p1, T2 p2, T3 p3)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(function.GetFunctionID());
			if (r < 0)
				return scxt;

			setArgument(cont, 0, p1);
			setArgument(cont, 1, p2);
			setArgument(cont, 2, p3);

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}

			cont->Execute();
			return scxt;
		}
		//! Executes the given function
		template<typename T1, typename T2, typename T3, typename T4>
		ScriptReturn Execute(ScriptMethod method, T1 p1, T2 p2, T3 p3, T4 p4)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(function.GetFunctionID());
			if (r < 0)
				return scxt;

			setArgument(cont, 0, p1);
			setArgument(cont, 1, p2);
			setArgument(cont, 2, p3);
			setArgument(cont, 3, p4);

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}

			cont->Execute();
			return scxt;
		}

		//! Executes the given method
		template<typename T1>
		ScriptReturn Execute(ScriptObject object, ScriptMethod method, T1 p1)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			setArgument(cont, 0, p1);

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
		//! Executes the given method
		template<typename T1, typename T2>
		ScriptReturn Execute(ScriptObject object, ScriptMethod method, T1 p1, T2 p2)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			setArgument(cont, 0, p1);
			setArgument(cont, 1, p2);

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
		//! Executes the given method
		template<typename T1, typename T2, typename T3>
		ScriptReturn Execute(ScriptObject object, ScriptMethod method, T1 p1, T2 p2, T3 p3)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			setArgument(cont, 0, p1);
			setArgument(cont, 1, p2);
			setArgument(cont, 2, p3);

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
		//! Executes the given method
		template<typename T1, typename T2, typename T3, typename T4>
		ScriptReturn Execute(ScriptObject object, ScriptMethod method, T1 p1, T2 p2, T3 p3, T4 p4)
		{
			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			setArgument(cont, 0, p1);
			setArgument(cont, 1, p2);
			setArgument(cont, 2, p3);
			setArgument(cont, 3, p4);

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
#else
		//! Executes the given function
		ScriptReturn Execute(ScriptMethod scref, ScriptArgument p1, ScriptArgument p2, ScriptArgument p3, ScriptArgument p4);

		//! Executes the given method
		ScriptReturn Execute(ScriptObject obj, ScriptMethod method, ScriptArgument p1, ScriptArgument p2, ScriptArgument p3, ScriptArgument p4);
#endif
		//! Executes the given code string.
		/*!
		 * \param[in] script A string containing as code to compile.
		 * \param[in] module The module to execute the code in.
		 * \param[out] context
		 * The context ID to use if you want to get return values
		 * (via GetReturnObject(context)).
		 * \param[in] timeout 
		 * Time the script can run before it's aborted. Default 1000 milis
		 * \returns The exit status of the script (asEXECUTION_ABORTED for timeout)
		 */
		ScriptReturn ExecuteString(const std::string &script, const char *module, int timeout = 5000);

		//! Re-executes the given stored context
		/*!
		 * Allows you to re-execute a suspended string execution. If you want
		 * to re-excute a Script, just call ExecuteScript() on it again.
		 *
		 * \param[in] context The context ID of the context which should be resumed.
		 * \param[in] module The module to execute the code in.
		 * \returns The exit status of the script.
		 * \retval asEXECUTION_ABORTED For timeout.
		 * \retval asERROR If the context doesn't exist.
		 *
		 * \sa
		 * ExecuteString()
		 */
		void ReExecute(ScriptContext& context);

		void SetDefaultTimeout(unsigned int timeout);
		unsigned int GetDefaultTimeout() const;

		//! Returns a method object for the given function
		ScriptMethod GetFunction(const char* module, const std::string& signature);
		//! Returns a method object for the given function
		bool GetFunction(ScriptMethod& out, const char* module, const std::string& signature);
		//! Returns a class object corresponding to the given typename
		ScriptClass GetClass(const char* module, const std::string& type_name);
		//! Returns an object of the class corresponding to the given typename
		ScriptObject CreateObject(const char* module, const std::string& type_name);
		ScriptMethod GetClassMethod(ScriptClass type, const std::string& signature);

		ScriptMethod GetClassMethod(ScriptObject type, const std::string& signature);

		//! Used internally
		void _messageCallback(asSMessageInfo *msg);

	private:
		//! AngelScript Engine
		asIScriptEngine *m_asEngine;

		//! Active contexts
		//std::vector<asIScriptContext*> m_Contexts;

		unsigned int m_DefaultTimeout;

	private:
		//! Sets the given argument in the given context
		template<class T>
		void setArgument(asIScriptContext* cxt, int arg, T value)
		{
			cxt->SetArgObject(arg, (void*)value);
		}
		//! Sets the given argument in the given context
		template<>
		void setArgument<void*>(asIScriptContext* cxt, int arg, void* value)
		{
			cxt->SetArgObject(arg, value);
		}
		//! Sets the given argument in the given context
		template<>
		void setArgument<bool>(asIScriptContext* cxt, int arg, bool value)
		{
			cxt->SetArgByte(arg, value);
		}
		//! Sets the given argument in the given context
		template<>
		void setArgument<short>(asIScriptContext* cxt, int arg, short value)
		{
			cxt->SetArgWord(arg, value);
		}
		//! Sets the given argument in the given context
		template<>
		void setArgument<int>(asIScriptContext* cxt, int arg, int value)
		{
			cxt->SetArgDWord(arg, value);
		}
		//! Sets the given argument in the given context
		template<>
		void setArgument<unsigned int>(asIScriptContext* cxt, int arg, unsigned int value)
		{
			cxt->SetArgDWord(arg, value);
		}
		//! Sets the given argument in the given context
		template<>
		void setArgument<float>(asIScriptContext* cxt, int arg, float value)
		{
			cxt->SetArgFloat(arg, value);
		}
		//! Sets the given argument in the given context
		template<>
		void setArgument<double>(asIScriptContext* cxt, int arg, double value)
		{
			cxt->SetArgDouble(arg, value);
		}
		//! Registers global methods and functions which scripts can use.
		void registerGlobals();

		////! Registers methods useful to weapon scripts
		//void registerWeaponMethods();
		////! Registers methods to be used from the console
		//void registerConsoleMethods();

	};

}

#endif
