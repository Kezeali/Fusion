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

#include "FusionScriptReference.h"

#include "FusionScriptingEngine.h"

namespace FusionEngine
{

	ScriptReference::ScriptReference(const char* module, ScriptFuncSig signature)
		: m_Module(module),
		m_Signature(signature)
	{
		m_FunctionID = ScriptingEngine::getSingleton().GetEnginePtr()->GetFunctionIDByDecl(module, signature.c_str());
	}

	ScriptReference::ScriptReference(const char* module, ScriptFuncSig signature, int funcID)
		: m_Module(module),
		m_Signature(signature),
		m_FunctionID(funcID)
	{
	}

	const char* ScriptReference::GetModule() const
	{
		return m_Module;
	}

	ScriptFuncSig ScriptReference::GetSignature() const
	{
		return m_Signature;
	}

	int ScriptReference::GetFunctionID() const
	{
		return m_FunctionID;
	}

}