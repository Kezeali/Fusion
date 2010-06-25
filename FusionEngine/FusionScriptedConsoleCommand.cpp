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

#include "FusionStableHeaders.h"

#include "FusionScriptedConsoleCommand.h"

#include <boost/bind.hpp>
//#include <boost/algorithm/string/classification.hpp>
//#include <boost/algorithm/string/split.hpp>
//#include <boost/algorithm/string/find_iterator.hpp>
//#include <boost/range/iterator_range.hpp>
#include <boost/tokenizer.hpp>

#include <ClanLib/regexp.h>

#include <ScriptUtils/Calling/Caller.h>

#include "FusionConsole.h"
#include "FusionScriptManager.h"

namespace FusionEngine
{

	std::string ScriptedConsoleCommand(asIScriptModule* module, std::string decl, const StringVector &args)
	{
		ScriptUtils::Calling::Caller commandCaller(module, decl.c_str());
		if (commandCaller.ok())
		{
			ScriptManager *manager = ScriptManager::getSingletonPtr();
			if (manager != NULL)
				manager->ConnectToCaller(commandCaller);

			void *ret = commandCaller(&args);

			if (ret != NULL)
				return *static_cast<std::string*>( ret );
		}

		return std::string("");
	}

	StringVector ScriptedCCAutocomplete(asIScriptModule* module, std::string decl, int arg_num, const std::string &incomplete_val)
	{
		ScriptUtils::Calling::Caller autocompleteCaller(module, decl.c_str());
		if (autocompleteCaller.ok())
		{
			ScriptManager *manager = ScriptManager::getSingletonPtr();
			if (manager != NULL)
				manager->ConnectToCaller(autocompleteCaller);

			void *ret = autocompleteCaller(arg_num, incomplete_val);

			if (ret != NULL)
				return *static_cast<StringVector*>( ret );
		}

		return StringVector();
	}

	// Appends the components of the given parameter to the expression
	//  Unlimited whitespace is allowed between each component, so
	//  'const string&in' becomes 'const[\s]*string[\s]*&in[\s]*'
	void addComponents(const std::string &parameter, std::string &expression)
	{
		typedef boost::char_separator<char> tokFnType;
		typedef boost::tokenizer<tokFnType> tokenizer;

		// Start tokenizing the command string
		tokFnType tokFn(" ", "&@");
		tokenizer tok(parameter, tokFn);
		for (tokenizer::iterator it = tok.begin(), end = tok.end(); it != end; ++it)
		{
			if (*it == "&" || *it == "@")
				expression += *it;
			else
				expression += *it + "[\\s]*";
		}
	}

	const std::string &constructSignatureExpression(const std::string return_type, const StringVector &parameters, bool const_modifier, std::string &expression)
	{
		// Add the return-type and beginning of parameters matching bit 
		expression += "[\\s]*" + return_type + "[\\s]+.+[\\s]*([\\s]*";
		// Add the parameter matching bits
		{
			// Add the first parameter (without a ',' before it)
			StringVector::const_iterator it = parameters.begin();
			addComponents(*it, expression);
			// Add the rest of the parameters
			it++;
			for (StringVector::const_iterator end = parameters.end(); it != end; ++it)
			{
				expression += ",[\\s]*";
				addComponents(*it, expression);
			}
		}
		expression += ")[\\s]*";
		// Add the const-modifier matching bit
		if (const_modifier)
			expression += "const[\\s]*";

		return expression;
	}

	bool is_compatible(const std::string &function_declaration, const std::string &expected_signature)
	{
		// Find the parameters section of the signature (i.e. '(...)')
		std::string::size_type parametersBeginAt = expected_signature.find('(');
		std::string::size_type parametersEndAt = expected_signature.rfind(')');
		// Make sure that the expected_signature contains a valid parameters section
		if (parametersBeginAt == std::string::npos || parametersEndAt == std::string::npos || parametersBeginAt > parametersEndAt)
			return false;
		std::string::size_type parametersLength = parametersEndAt - parametersBeginAt;

		// Get the return type (i.e. everything before the open-bracket that indicates the beginning of the params)
		std::string returnType = fe_trim( expected_signature.substr(0, parametersBeginAt) );
		// Get the parameters
		StringVector parameters;
		if (parametersLength > 2) // Only need to split out the param.s if there are any!
			parameters = fe_splitstring(expected_signature.substr(parametersBeginAt+1, parametersLength-1), ",", false);
		// Check for const modifier
		bool is_const = expected_signature.find("const", parametersEndAt+1) != std::string::npos;

		std::string expressionString = "";
		constructSignatureExpression(returnType, parameters, is_const, expressionString);
		// Build and compile the expression
		CL_RegExp expression( expressionString.c_str() );
		return expression.search(function_declaration.c_str(), function_declaration.length()).is_match();
	}

	void Scr_BindConsoleCommand(const std::string &command, const std::string &func_name, Console *obj)
	{
		Scr_BindConsoleCommand(command, func_name, std::string(), obj);
	}
	void Scr_BindConsoleCommand(const std::string &command, const std::string &callback, const std::string &autocomplete, Console *obj)
	{
		asIScriptContext *context = asGetActiveContext();
		if (context != NULL)
		{
			asIScriptModule *module = ctxGetModule(context);

			// Expand the given string to a full command-callback decl. if it is just a name
			std::string callbackFullDecl;
			if (callback.find("string ", callback.find_first_not_of(" ")) == std::string::npos)
				callbackFullDecl = "string " + callback + "(const StringArray &in)";
			else
				callbackFullDecl = callback;

			// Make sure the indicated function exists
			if (module->GetFunctionIdByDecl(callbackFullDecl.c_str()) < 0)
			{
				// Set script exception? Log?
				return;
			}

			if (autocomplete.empty())
				obj->BindCommand(command, boost::bind(&ScriptedConsoleCommand, module, callbackFullDecl, _1) );
			else
			{
				// Expand the given string to a full autocomplete-callback decl. if it is
				//  just a name (as above with the command-callback)
				std::string autocompleteFullDecl;
				if (callback.find("string ", callback.find_first_not_of(" ")) == std::string::npos)
					autocompleteFullDecl = "string " + autocomplete + "(int, const string &in)";
				else
					autocompleteFullDecl = autocomplete;

				// Make sure the indicated function exists
				if (module->GetFunctionIdByDecl(autocompleteFullDecl.c_str()) < 0)
				{
					// Set script exception? Log?
					return;
				}

				obj->BindCommand(command,
					boost::bind(&ScriptedConsoleCommand, module, callbackFullDecl, _1), // Create command-callback fn.
					boost::bind(&ScriptedCCAutocomplete, module, autocompleteFullDecl, _1, _2) ); // and autocomplete fn.
			}
		}
	}

	void Scr_SetCCHelp(const std::string &command, const std::string &help_text, Console *obj)
	{
		obj->SetCommandHelpText(command, help_text, StringVector());
	}
	void Scr_SetCCHelp(const std::string &command, const std::string &help_text, const std::string &arg_names, Console *obj)
	{
		obj->SetCommandHelpText(command, help_text, fe_splitstring(arg_names, " "));
	}


	void RegisterScriptedConsoleCommand(asIScriptEngine *engine)
	{
		int r;
		r = engine->RegisterObjectMethod("Console",
			"void bindCommand(const string &in, const string &in)",
			asFUNCTIONPR(Scr_BindConsoleCommand, (const std::string&, const std::string&, Console*), void),
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"void bindCommand(const string &in, const string &in, const string &in)",
			asFUNCTIONPR(Scr_BindConsoleCommand, (const std::string&, const std::string&, const std::string&, Console*), void),
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"void setCommandHelpText(const string &in, const string &in)",
			asFUNCTIONPR(Scr_SetCCHelp, (const std::string&, const std::string&, Console*), void),
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"void setCommandHelpText(const string &in, const string &in, const string &in)",
			asFUNCTIONPR(Scr_SetCCHelp, (const std::string&, const std::string&, const std::string&, Console*), void),
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

}