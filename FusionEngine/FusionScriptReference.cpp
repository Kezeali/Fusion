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
#ifndef SCRIPT_ARG_USE_TEMPLATE
	ScriptArgument::ScriptArgument()
		: m_Valid(false)
	{}

	ScriptArgument::~ScriptArgument()
	{
		delete m_Value;
	}

	bool ScriptArgument::IsValid() const
	{
		return m_Valid;
	}
#endif

	ScriptContext::ScriptContext(asIScriptContext* ctx)
		: m_Context(ctx)
	{
		if (m_Context != NULL)
			ctx->AddRef();
	}

	ScriptContext::ScriptContext(const ScriptContext& other)
	{
		m_Context = other.m_Context;
		if (m_Context != NULL)
			m_Context->AddRef();
	}

	ScriptContext::~ScriptContext()
	{
		if (m_Context != NULL)
			m_Context->Release();
	}

	int ScriptContext::Execute() const
	{
		if (m_Context != NULL)
			return m_Context->Execute();
		else
			return -1;
	}

	void ScriptContext::Suspend() const
	{
		if (m_Context != NULL)
			m_Context->Suspend();
	}

	void ScriptContext::Abort() const
	{
		if (m_Context != NULL)
			m_Context->Abort();
	}

	asIScriptContext* ScriptContext::GetContext() const
	{
		return m_Context;
	}

	bool ScriptContext::WasSuccessful() const
	{
		if (m_Context == NULL)
			return false;

		return m_Context->GetState() >= 0;
	}

	bool ScriptContext::RaisedException() const
	{
		if (m_Context == NULL)
			return false;

		return m_Context->GetState() == asEXECUTION_EXCEPTION;
	}

	bool ScriptContext::IsOk() const
	{
		return WasSuccessful() && RaisedException();
	}

	float ScriptContext::GetValueFloat() const
	{
		if (m_Context == NULL)
			return 0.f;

		return m_Context->GetReturnFloat();
	}

	double ScriptContext::GetValueDouble() const
	{
		if (m_Context == NULL)
			return 0.0;

		return m_Context->GetReturnDouble();
	}

	void* ScriptContext::GetPointer() const
	{
		if (m_Context == NULL)
			return NULL;

		return m_Context->GetReturnPointer();
	}


	ScriptMethod::ScriptMethod(ScriptingEngine* manager, const char* module, const std::string& signature)
		: m_Module(module),
		m_Signature(signature),
		m_Timeout(0),
		m_NoArgs(false)
	{
		m_FunctionID = manager->GetEnginePtr()->GetFunctionIDByDecl(module, signature.c_str());
	}

	ScriptMethod::ScriptMethod(const char* module, const std::string& signature, int func_id, unsigned int timeout)
		: m_Module(module),
		m_Signature(signature),
		m_FunctionID(func_id),
		m_Timeout(timeout),
		m_NoArgs(false)
	{
	}

	const char* ScriptMethod::GetModule() const
	{
		return m_Module;
	}

	const std::string& ScriptMethod::GetSignature() const
	{
		return m_Signature;
	}

	int ScriptMethod::GetFunctionID() const
	{
		return m_FunctionID;
	}

	unsigned int ScriptMethod::GetTimeout() const
	{
		return m_Timeout;
	}

	void ScriptMethod::SetModule(const char* module)
	{
		m_Module = module;
	}

	void ScriptMethod::SetSignature(const std::string& sig)
	{
		m_Signature = sig;
	}

	void ScriptMethod::SetFunctionID(int id)
	{
		m_FunctionID = id;
	}

	void ScriptMethod::SetTimeout(unsigned int timeout)
	{
		m_Timeout = timeout;
	}

	ScriptArgument::DataType ScriptMethod::GetArgType(unsigned int argIndex)
	{
		if (!m_NoArgs || m_ArgTypes.empty())
			parseSignature();

		if (argIndex >= m_ArgTypes.size())
			return ScriptArgument::NO_ARG;

		return m_ArgTypes[argIndex];
	}

	bool ScriptMethod::IsValid() const
	{
		return m_FunctionID >= 0;
	}

	void ScriptMethod::parseSignature()
	{
		typedef std::string::size_type size_type;

		size_type argsBegin = m_Signature.find("(");
		size_type argsEnd = m_Signature.find_last_of(")");
		std::string working = m_Signature.substr(argsBegin, argsEnd-argsBegin);

		if (working.empty())
		{
			m_NoArgs = true;
			return;
		}

		std::vector<std::string> args = CL_String::tokenize(working, ",");

		m_ArgTypes.clear();
		for (int i = 0; i < args.size(); ++i)
		{
			std::string& arg = args[i];

			if (arg.empty())
				continue;

			// Pointers are easy to find...
			size_type indirectionMarkerPos = arg.find("@");
			if (indirectionMarkerPos != std::string::npos)
				m_ArgTypes[i] = Arg::POINTER;

			// Remove identifier names from the args
			size_type spacePos = arg.find(" ");
			if (spacePos != std::string::npos)
				arg = arg.substr(0, spacePos);

			// This probably means there is something wrong with the signature, but oh well :/
			if (arg.empty())
				continue;

			if (arg == "bool" || arg == "int8" || arg == "uint8")
				m_ArgTypes[i] = Arg::BYTE;
			else if (arg == "int16" || arg == "int32" || arg == "int")
				m_ArgTypes[i] = Arg::INTEGER;
			else if (arg == "float")
				m_ArgTypes[i] = Arg::FLOAT;
			else if (arg == "double")
				m_ArgTypes[i] = Arg::DOUBLE;
			else if (arg == "uint")
				m_ArgTypes[i] = Arg::UINT;
			else if (arg == "int64" || arg == "uint64")
				m_ArgTypes[i] = Arg::QWORD;
			else
				m_ArgTypes[i] = Arg::OBJECT;

		}

	}



	ScriptClass::ScriptClass(ScriptingEngine* manager, const char *module, const std::string& declaration)
		: m_Decl(declaration),
		m_Module(module),
		m_ScriptManager(manager)
	{
		m_TypeID = manager->GetEnginePtr()->GetTypeIdByDecl(module, declaration.c_str());
	}

	ScriptClass::ScriptClass(ScriptingEngine* manager, const char *module, const std::string& declaration, int type_id)
		: m_Decl(declaration),
		m_Module(module),
		m_ScriptManager(manager)
	{
		m_TypeID = manager->GetEnginePtr()->GetTypeIdByDecl(module, declaration.c_str());
	}

	int ScriptClass::GetTypeId() const
	{
		return m_TypeID;
	}

	const char* ScriptClass::GetModule() const
	{
		return m_Module;
	}

	const std::string& ScriptClass::GetDeclaration() const
	{
		return m_Decl;
	}

	ScriptMethod ScriptClass::GetMethod(const std::string& signature) const
	{
		int id = m_ScriptManager->GetEnginePtr()->GetMethodIDByDecl(m_TypeID, signature.c_str());
		return ScriptMethod(m_Module, signature, id);
	}

	ScriptObject ScriptClass::Instantiate()
	{
		if (IsValid())
		{
			asIScriptStruct* scriptStruct = (asIScriptStruct*)m_ScriptManager->GetEnginePtr()->CreateScriptObject(m_TypeID);
			ScriptObject object(scriptStruct);
			return object;
		}
		else
			return ScriptObject();
	}

	bool ScriptClass::IsValid() const
	{
		return m_TypeID >= 0;
	}

	
	ScriptObject::ScriptObject(asIScriptStruct *script_struct)
		: m_Struct(script_struct)
	{
	}

	ScriptObject::ScriptObject(const ScriptObject& other)
	{
		m_Struct = other.m_Struct;
		m_Struct->AddRef();
	}

	ScriptObject::~ScriptObject()
	{
		if (m_Struct != NULL)
			m_Struct->Release();
	}

	int ScriptObject::GetTypeId() const
	{
		return m_Struct->GetStructTypeId();
	}

	asIScriptStruct* ScriptObject::GetScriptStruct() const
	{
		return m_Struct;
	}

	bool ScriptObject::IsValid() const
	{
		return m_Struct != NULL;
	}

}