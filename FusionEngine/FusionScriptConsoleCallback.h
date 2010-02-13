/*
  Copyright (c) 2009 Fusion Project Team

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

#ifndef Header_FusionEngine_ScriptConsoleCallback
#define Header_FusionEngine_ScriptConsoleCallback

#if _MSC_VER > 1000
# pragma once
#endif

#include "FusionCommon.h"

#include "FusionScriptManager.h"
#include "FusionConsole.h"

#include "scriptstring.h"

#include <boost/bind.hpp>

namespace FusionEngine
{

	//! Console CC (Command Callback) which calls a script method
	static std::string CC_ScriptFn(const StringVector &args, ScriptMethod script_fn)
	{
		ScriptManager *se = ScriptManager::getSingletonPtr();
		if (se != NULL)
		{
			ScriptReturn ctx = se->Execute(script_fn, args);
			if (ctx.IsOk())
				return *ctx.GetReturnValueObject<std::string>();
			else
				return "";
		}
		else
			return "Scripting engine hasn't been started.";
	}

	//! Binds the script method with the given decl to the given con. command
	static void Console_BindCommand_ScriptFn(
		const std::string &command, const std::string &as_function_decl,
		Console* console)
	{
		ScriptManager *se = ScriptManager::getSingletonPtr();
		if (se != NULL)
		{
			ScriptMethod method = se->GetFunction(0, as_function_decl);

			console->BindCommand(command, boost::bind(&CC_ScriptFn, _1, method));
		}
	}

	//! Registers bind_command()
	static void RegisterBindScriptFn(ScriptManager *eng)
	{
		FSN_ASSERT(eng != NULL);
		asIScriptEngine *asEngine = eng->GetEnginePtr();
		FSN_ASSERT(asEngine);

		int r = asEngine->RegisterObjectMethod("Console",
			"void bind_command(string &in, string &in)",
			asFUNCTION(Console_BindCommand_ScriptFn,
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r);
	}

}

#endif