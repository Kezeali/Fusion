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

namespace FusionEngine
{

	static const std::string g_ASConfigConsole = "Console";
	static const std::string g_ASConfigWeapon = "Weapon";

	class COutStream
	{
	public:
		void Callback(asSMessageInfo *msg) 
		{ 
			const char *msgType = 0;
			if( msg->type == 0 ) msgType = "Error  ";
			if( msg->type == 1 ) msgType = "Warning";
			if( msg->type == 2 ) msgType = "Info   ";

			char buffer[255];
			sprintf(buffer, "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, msgType, msg->message);
			SendToConsole(std::string(buffer), Console::MTWARNING);
		}
	};

	/*!
	 * \brief
	 * Provides scripting support, and access to it, for all FusionEngine objects.
	 *
	 * \sa
	 * Singleton
	 */
	class ScriptingEngine : public Singleton<ScriptingEngine>
	{
		COutStream m_Out;
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
		int ExecuteModule(const char *module, const char *function);

		//! Executes the given funcion in Script
		/*!
		 * If the Script is already registered, it's entire containing module
		 * will be executed. The script / module will be built if necessary.
		 *
		 * \param[in] script A loaded Script object 
		 * \returns The exit status of the function
		 */
		int ExecuteScript(Script *script, const char *function);

		//! Executes the given ScriptReference
		int Execute(ScriptReference scref);

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
		int ExecuteString(const std::string &script, const char *module, int* context = NULL, int timeout = 1000, bool keep_context = false);

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
		int ReExecuteString(int context, const char *module);

		//! Used internally
		void _lineCallback(asIScriptContext *ctx, int *timeOut);

	private:
		//! AngelScript Engine
		asIScriptEngine *m_asEngine;

		//! Active contexts
		std::vector<asIScriptContext*> m_Contexts;

	private:
		//! Registers global methods and functions which scripts can use.
		void registerGlobals();

		////! Registers methods useful to weapon scripts
		//void registerWeaponMethods();
		////! Registers methods to be used from the console
		//void registerConsoleMethods();

	};

}

#endif
