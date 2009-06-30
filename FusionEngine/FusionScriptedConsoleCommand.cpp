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

#include "Common.h"

#include "FusionScriptedConsoleCommand.h"

#include "FusionConsole.h"

#include <Calling/Caller.h>
#include <boost/bind.hpp>

namespace FusionEngine
{

	std::string ScriptedConsoleCommand(asIScriptModule* module, std::string decl, const StringVector &args)
	{
		ScriptUtils::Calling::Caller commandCaller(module, decl.c_str());
		if (commandCaller.ok())
		{
			void *ret = commandCaller(/*&args*/);

			if (ret != NULL)
				return *static_cast<std::string*>( ret );
		}

		return std::string();
	}

	void Scr_BindConsoleCommand(const std::string &command, const std::string &decl, Console *obj)
	{
		asIScriptContext *context = asGetActiveContext();
		if (context != NULL)
		{
			asIScriptModule *module = context->GetEngine()->GetModule( context->GetCurrentModule() );

			obj->BindCommand(command, boost::bind(&ScriptedConsoleCommand, module, decl, _1) );
		}
	}

	void RegisterScriptedConsoleCommand(asIScriptEngine *engine)
	{
		int r;
		r = engine->RegisterObjectMethod("Console",
			"void bindCommand(const string &in, const string &in)",
			asFUNCTION(Scr_BindConsoleCommand),
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

}