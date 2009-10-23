/*
  Copyright (c) 2006-2009 Fusion Project Team

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
#include "FusionXML.h"
//#include "FusionScriptDebugPreprocessor.h"

// Scripting extensions
#include "FusionScriptedSlots.h"
#include "FusionScriptVector.h"
#include "scriptstring.h"
#include "scriptmath.h"
#include "scriptstdvector.h"

// External
#include <boost/functional/hash.hpp>


namespace FusionEngine
{

	typedef std::pair<std::string::const_iterator, std::string::const_iterator> CharRange;

	// '#include' Preprocessor
	class IncludePreprocessor : public ScriptPreprocessor
	{
	public:
		IncludePreprocessor(ScriptingEngine *manager)
			: m_Manager(manager)
		{}
		virtual void Process(std::string &code, const char *module_name, MarkedLines &lines);

		bool isIncludeLine(const char *code_pos);
		// First is the beginning of the search area, last is the end of the search area
		CharRange getIncludeFile(std::string::const_iterator first, std::string::const_iterator last);

		ScriptingEngine *m_Manager; 
	};

	void IncludePreprocessor::Process(std::string &code, const char *module_name, ScriptPreprocessor::MarkedLines &marked)
	{
		std::string::size_type pos = 0;
		unsigned int line = 0;
		for (MarkedLines::iterator it = marked.begin(), end = marked.end(); it != end; ++it)
		{
			if ( it->type == '#' && isIncludeLine(code.c_str() + it->pos) )
			{
				CharRange range = getIncludeFile(code.begin() + it->pos+8, code.end());
				std::string filename = std::string(range.first, range.second);

				m_Manager->AddFile(filename, module_name);

				code.erase(range.first, range.second); // remove the processed line from the code
				it = marked.erase(it); // Remove the marker from the markers list
			}
		}
	}

	bool IncludePreprocessor::isIncludeLine(const char *start)
	{
		return std::string("#include").compare(0, 8, start, 8) == 0;

		//size_t confirmedChars = 0;
		//static const char *compareString = "#include";
		//while (true)
		//{
		//	if (*first == compareString[confirmedChars])
		//		confirmedChars++;
		//	else
		//		return false;

		//	if (confirmedChars == 8)
		//		return true;
		//}

		//return false;
	}

	CharRange IncludePreprocessor::getIncludeFile(std::string::const_iterator first, std::string::const_iterator last)
	{
		std::string::const_iterator begin = last;
		std::string::const_iterator end = last;

		unsigned int matched = 0;
		for (std::string::const_iterator pos = first; pos != last; ++pos)
		{
			if ((*pos) == '\"')
			{
				if (matched == 0)
					begin = pos;
				else
					end = pos;

				if (++matched == 2)
					break;
			}
		}

		return CharRange(begin, end);
	}

	////////////
	// ScriptSection implementations
	ScriptingEngine::StringScriptSection::StringScriptSection()
	{}

	ScriptingEngine::StringScriptSection::StringScriptSection(const std::string &code)
		: m_Code(code)
	{}

	std::string &ScriptingEngine::StringScriptSection::GetCode()
	{
		return m_Code;
	}

	ScriptingEngine::FileScriptSection::FileScriptSection()
	{}

	ScriptingEngine::FileScriptSection::FileScriptSection(const std::string &filename)
		: m_Filename(filename)
	{}

	std::string &ScriptingEngine::FileScriptSection::GetCode()
	{
		if (m_Code.empty())
		{
			OpenString_PhysFS(m_Code, fe_widen(m_Filename));
		}
		return m_Code;
	}

	/////////
	// Breakpoint hasher
	std::size_t ScriptingEngine::hash_Breakpoint::operator ()(const ScriptingEngine::Breakpoint &value) const 
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, value.section_name);
		boost::hash_combine(seed, value.line);

		return seed;
	}

	bool operator ==(const ScriptingEngine::Breakpoint &lhs, const ScriptingEngine::Breakpoint &rhs)
	{
		return lhs.line == rhs.line &&
			std::strcmp(lhs.module_name, rhs.module_name) == 0 &&
			std::strcmp(lhs.section_name, rhs.section_name) == 0;
	}

	//////////
	// ScriptingManager
	ScriptingEngine::ScriptingEngine()
		: m_DefaultTimeout(g_ScriptDefaultTimeout),
		m_DebugOutput(false),
		m_DebugMode(0),
		m_SectionSerial(0)
	{
		m_asEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		if (m_asEngine != NULL)
		{
			m_asEngine->SetMessageCallback(asMETHOD(ScriptingEngine,_messageCallback), this, asCALL_THISCALL);
			registerTypes();
		}

		m_Preprocessors.push_back(PreprocessorPtr(new IncludePreprocessor(this)));
	}

	ScriptingEngine::~ScriptingEngine()
	{
		if (m_asEngine != NULL)
		{
			m_asEngine->GarbageCollect();
			m_asEngine->Release();
		}
		m_ScriptSections.clear();
	}


	asIScriptEngine *ScriptingEngine::GetEnginePtr() const
	{
		return m_asEngine;
	}

	int ScriptingEngine::GetVectorTypeId() const
	{
		return m_VectorTypeId;
	}

	int ScriptingEngine::GetStringTypeId() const
	{
		return m_StringTypeId;
	}

	void ScriptingEngine::RegisterGlobalObject(const char *decl, void* ptr)
	{
		int r = m_asEngine->RegisterGlobalProperty(decl, ptr); FSN_ASSERT( r >= 0 );
	}

	void ScriptingEngine::Preprocess(std::string &script, const char *module_name)
	{
		ScriptPreprocessor::MarkedLines lines;
		// Parses the script to find lines with preprocessor markers
		ScriptPreprocessor::checkForMarkedLines(lines, script);

		for (PreprocessorArray::iterator it = m_Preprocessors.begin(), end = m_Preprocessors.end();
			it != end; ++it)
		{
			(*it)->Process(script, module_name, lines);
		}
	}

	bool ScriptingEngine::storeCodeString(const std::string &code, const char *section_name)
	{
		std::pair<ScriptSectionMap::iterator, bool> check =
			m_ScriptSections.insert( ScriptSectionMap::value_type(section_name, ScriptSectionPtr(new StringScriptSection(code))) );
		while (!check.second)
		{
			if (check.first->second->GetCode() == code)
				return false; // Code already added

			std::ostringstream str;
			str << section_name << m_SectionSerial++;
			check =
				m_ScriptSections.insert( ScriptSectionMap::value_type(str.str(), ScriptSectionPtr(new StringScriptSection(code))) );
		}

		return true;
	}

	bool ScriptingEngine::AddCode(const std::string &script, const char *module, const char *section_name)
	{
		if (!m_Preprocessors.empty())
		{
			std::string processedScript = script;
			Preprocess(processedScript, module);

			bool success = GetModule(module)->AddCode(section_name, processedScript) >= 0;

			if (!success)
				return false;

			// try to store the code string
			if (m_DebugSettings.storeCodeStrings && !storeCodeString(script, section_name))
				return false;

			return true;
		}

		bool success = GetModule(module)->AddCode(section_name, script) >= 0;

		// Copying is avoided if there are no preprocessors listed and
		//  the debug option 'storeCodeStrings' is disabled
		if (success && m_DebugSettings.storeCodeStrings)
			storeCodeString(script, section_name);

		return success;
	}

	bool ScriptingEngine::AddFile(const std::string& filename, const char *module)
	{
		int r = -1;

		if (m_ScriptSections.find(filename) != m_ScriptSections.end())
			return false; // File already added

		// Load the script from the file
		std::string script;
		OpenString_PhysFS(script, fe_widen(filename));
		// Preprocess the script
		Preprocess(script, module);
		// Add the script to the module
		r = GetModule(module)->AddCode(filename, script);
		// If the script was successfully added to the module, create a new
		//  ScriptSection for it (these are used for stepping through code
		//  in the debugger)
		if (r >= 0)
			m_ScriptSections[filename] = ScriptSectionPtr(new FileScriptSection(filename));

		return r >= 0;
	}

	void ScriptingEngine::DebugRebuild(const char *module)
	{
		ModulePtr mod = GetModule(module, asGM_ONLY_IF_EXISTS);
		if (mod == NULL) return;
		const StringVector& names = mod->GetSectionNames();
		for (StringVector::const_iterator it = names.begin(), end = names.end(); it != end; ++it)
		{
			ScriptSectionMap::iterator _where = m_ScriptSections.find(*it);
			mod->AddCode(_where->first, _where->second->GetCode());
		}
	}

	bool ScriptingEngine::BuildModule(const char* module)
	{
		//asIScriptModule* mod = m_asEngine->GetModule(module);
		ModulePtr mod = GetModule(module, asGM_ONLY_IF_EXISTS);
		// If the module doesn't exist, return false. Otherwise 
		//  return true on success
		if (mod == NULL) return false;
		else return mod->Build() >= 0;
	}

	bsig2::connection ScriptingEngine::SubscribeToModule(const char *module, Module::BuildModuleSlotType slot)
	{
		return GetModule(module)->ConnectToBuild(slot);
	}

	ScriptReturn ScriptingEngine::Execute(const char* module, const char* function, unsigned int timeout /* = 0 */)
	{
		int funcID = getModuleOrThrow(module)->GetFunctionIdByDecl(function);

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
		cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

		cont->Execute();

		return scxt;
	}

#ifndef SCRIPT_ARG_USE_TEMPLATE
	ScriptReturn ScriptingEngine::Execute(UCScriptMethod function, ScriptArgument p1, ScriptArgument p2, ScriptArgument p3, ScriptArgument p4)
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

	ScriptReturn ScriptingEngine::Execute(ScriptObject object, UCScriptMethod method, ScriptArgument p1, ScriptArgument p2, ScriptArgument p3, ScriptArgument p4)
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
		ctx->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);
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

	UCScriptMethod ScriptingEngine::GetFunction(const char* module, const std::string& signature)
	{
		int funcID = getModuleOrThrow(module)->GetFunctionIdByDecl(signature.c_str());
		return UCScriptMethod(module, signature, funcID);
	}

	bool ScriptingEngine::GetFunction(UCScriptMethod& out, const char* module, const std::string& signature)
	{
		int funcID = getModuleOrThrow(module)->GetFunctionIdByDecl(signature.c_str());
		if (funcID < 0)
			return false;

		out.SetModule(module);
		out.SetSignature(signature);
		out.SetFunctionID(funcID);

		return true;
	}

	ScriptClass ScriptingEngine::GetClass(const char* module, const std::string& type_name)
	{
		int id = getModuleOrThrow(module)->GetTypeIdByDecl(type_name.c_str());
		return ScriptClass(this, module, type_name, id);
	}

	ScriptObject ScriptingEngine::CreateObject(const char* module, const std::string& type_name)
	{
		int id = getModuleOrThrow(module)->GetTypeIdByDecl(type_name.c_str());
		asIScriptObject* obj = (asIScriptObject*)m_asEngine->CreateScriptObject(id);
		//obj->AddRef();
		return ScriptObject(obj, false);
	}

	UCScriptMethod ScriptingEngine::GetClassMethod(const char* module, const std::string& type_name, const std::string &signature)
	{
		int typeId = getModuleOrThrow(module)->GetTypeIdByDecl(type_name.c_str());
		if (typeId < 0)
			FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::GetClassMethod", "No such type: " + type_name);
		int methodId = m_asEngine->GetObjectTypeById(typeId)->GetMethodIdByDecl(signature.c_str());
		if (methodId < 0)
			FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::GetClassMethod", "No such method: " + signature + "\n in type " + type_name);

		//asIScriptFunction *function = m_asEngine->GetFunctionDescriptorById(methodId);
		return UCScriptMethod(module, signature, typeId);
	}

	UCScriptMethod ScriptingEngine::GetClassMethod(ScriptClass& type, const std::string& signature)
	{
		if (type.IsValid())
		{
			int id = m_asEngine->GetObjectTypeById(type.GetTypeId())->GetMethodIdByDecl(signature.c_str());
			UCScriptMethod method(type.GetModule(), signature, id);
			return method;
		}
		else
			return UCScriptMethod();
	}

	//UCScriptMethod ScriptingEngine::GetClassMethod(ScriptObject& type, const std::string& signature)
	//{
	//	FSN_ASSERT(false);
	//	asIObjectType *scriptType = m_asEngine->GetObjectTypeById(type.GetTypeId());
	//	int id = scriptType->GetMethodIdByDecl(signature.c_str());
	//	UCScriptMethod method(0, signature, id);
	//	return method;
	//}

	ModulePtr ScriptingEngine::GetModule(const char *module_name, asEGMFlags when)
	{
		ModuleMap::iterator _where = m_Modules.find(module_name);
		if (_where != m_Modules.end()) // Return the existing wrapper
		{
			return _where->second;
		}
		else if (when != asGM_ONLY_IF_EXISTS) // Create a new Module wrapper
		{
			ModulePtr modulePtr(new Module(m_asEngine->GetModule(module_name, when)));
			m_Modules[module_name] = modulePtr;
			return modulePtr;
		}
		else
		{
			return ModulePtr();
		}
	}

	ScriptUtils::Calling::Caller ScriptingEngine::GetCaller(const char * module_name, const std::string &signature)
	{
		ScriptUtils::Calling::Caller caller(m_asEngine->GetModule(module_name), signature.c_str());
		ConnectToCaller(caller);
		return caller;
	}

	ScriptUtils::Calling::Caller ScriptingEngine::GetCaller(const FusionEngine::ScriptObject &object, const std::string &signature)
	{
		ScriptUtils::Calling::Caller caller(object.GetScriptObject(), signature.c_str());
		ConnectToCaller(caller);
		return caller;
	}

	void ScriptingEngine::EnableDebugOutput()
	{
		m_DebugOutput = true;
	}

	void ScriptingEngine::DisableDebugOutput()
	{
		m_DebugOutput = false;
	}

	bool ScriptingEngine::DebugOutputIsEnabled()
	{
		return m_DebugOutput;
	}

	bsig2::connection ScriptingEngine::SubscribeToDebugEvents(ScriptingEngine::DebugSlotType slot)
	{
		return SigDebug.connect( slot );
	}

	void ScriptingEngine::SetDebugMode(unsigned char mode)
	{
		m_DebugMode = mode;
	}

	void ScriptingEngine::ConnectToCaller(ScriptUtils::Calling::Caller &caller)
	{
		caller.ConnectExceptionCallback( boost::bind(&ScriptingEngine::_exceptionCallback, this, _1) );
		caller.ConnectLineCallback( boost::bind(&ScriptingEngine::_lineCallback, this, _1) );
	}

	bool printVar(std::ostream &strstr, asIScriptContext *ctx, int var_ind, int stack_level)
	{
		asIScriptEngine *eng = ctx->GetEngine();
		void *varPtr = ctx->GetAddressOfVar(var_ind, stack_level);
		int typeId = ctx->GetVarTypeId(var_ind, stack_level);

		if (typeId == eng->GetTypeIdByDecl("int"))
		{
			strstr << *(int*)varPtr;
		}
		else if (typeId == eng->GetTypeIdByDecl("uint"))
		{
			strstr << *(unsigned int*)varPtr;
		}
		else if (typeId == eng->GetTypeIdByDecl("int8"))
		{
			strstr << *(char*)varPtr;
		}
		else if (typeId == eng->GetTypeIdByDecl("uint8"))
		{
			strstr << *(unsigned char*)varPtr;
		}
		else if (typeId == eng->GetTypeIdByDecl("string"))
		{
			CScriptString *str = (CScriptString*)varPtr;
			strstr << "\"" << str->buffer << "\"";
		}
		else if (typeId == eng->GetTypeIdByDecl("Vector"))
		{
			Vector2 *vec = (Vector2*)varPtr;
			strstr << vec->get_x() << "," << vec->get_y();
		}
		else if (typeId == eng->GetTypeIdByDecl("StringArray"))
		{
			const StringVector *vec = (const StringVector*)varPtr;
			strstr << "[" << vec->size() << "](";
			bool first = true;
			std::string::size_type length = 0;
			for (StringVector::const_iterator it = vec->begin(), end = vec->end(); it != end; ++it)
			{
				if (length > 20)
				{
					strstr << ",...";
					break;
				}
				if (first)
				{
					first = false;
					strstr << "\"" << *it << "\"";
				}
				else
					strstr << ",\"" << *it << "\"";
				
				length += it->length() + 3;
			}
			strstr << ")";
		}
		// Handle types
		else if ((typeId & ~asTYPEID_HANDLETOCONST) == eng->GetTypeIdByDecl("string@"))
		{
			const CScriptString **handle = (const CScriptString**)varPtr;
			if (handle != NULL)
			{
				strstr << "\"" << (*handle)->buffer << "\"";
			}
			else
			{
				strstr << "NULL";
			}
		}
		else if ((typeId & ~asTYPEID_HANDLETOCONST) == eng->GetTypeIdByDecl("Vector@"))
		{
			const Vector2 **handle = (const Vector2**)varPtr;
			if (handle != NULL)
			{
				strstr << (*handle)->get_x() << "," << (*handle)->get_y();
			}
			else
			{
				strstr << "NULL";
			}
		}
		// No conversion implemented for this type:
		else
			return false;

		return true;
	}

	static const size_t s_maxPregenSpaces = 13;
	static const char * s_spaces[14] = {
		"", " ", "  ", "   ", "    ", "     ", "      ", "       ",
		"        ", "         ", "          ", "           ", "            ",
		NULL
	};

	static void insertSpaces(std::ostream &into, size_t length)
	{
		if (length <= s_maxPregenSpaces)
		{
			into << s_spaces[length];
		}
		else
		{
			into << s_spaces[s_maxPregenSpaces];
			for (size_t k = s_maxPregenSpaces; k < length ; ++k)
				into << " ";
		}
	}

	void formatCallstackFunctionHeading(std::ostream &str, const char *sig, int line, int column)
	{
		str << "+ " << sig << " called at (" << line << "," << column << ")\n";
	}

	void ScriptingEngine::printCallstack(asIScriptEngine *const engine, asIScriptContext *ctx, int current_func, std::string &to)
	{
		std::stringstream str;

		// Print the current function
		asIScriptFunction *func = engine->GetFunctionDescriptorById(current_func);
		if (func != NULL)
		{
			const char *sig = func->GetDeclaration(true);
			int indent = ctx->GetCallstackSize();
			int column, line;
			if (indent > 0)
				line = ctx->GetCallstackLineNumber(indent-1, &column);
			else
				line = column = 0;

			insertSpaces(str, indent);
			formatCallstackFunctionHeading(str, sig, line, column);

			int vars = ctx->GetVarCount();
			for (int i = 0; i < vars; i++)
			{
				std::stringstream var_str;
				if (printVar(var_str, ctx, i, -1))
				{
					insertSpaces(str, indent);
					str << "|  " << ctx->GetVarDeclaration(i) << ": " << var_str.str() << "\n";
				}
			}
		}

		// Print the call-stack
		for (int i = ctx->GetCallstackSize()-1; i >= 0; i--)
		{
			const char *sig = engine->GetFunctionDescriptorById( ctx->GetCallstackFunction(i) )->GetDeclaration(true);
			int column, line;
			if (i > 0)
				line = ctx->GetCallstackLineNumber(i-1, &column);
			else
				line = column = 0;

			insertSpaces(str, i);
			formatCallstackFunctionHeading(str, sig, line, column);
			
			int vars = ctx->GetVarCount(i);
			for (int var_i = 0; var_i < vars; var_i++)
			{
				std::stringstream var_str;
				if (printVar(var_str, ctx, var_i, i))
				{
					insertSpaces(str, i);
					str << "|  " << ctx->GetVarDeclaration(var_i, i) << ": " << var_str.str() << "\n";
				}
			}
		}

		to += str.str();
	}

	void ScriptingEngine::_exceptionCallback(asIScriptContext *ctx)
	{
		asIScriptEngine *engine = ctx->GetEngine();

		std::string desc =
			CL_StringHelp::text_to_local8( cl_format("Script Exception:\n %1\n", ctx->GetExceptionString()) );

		int funcId = ctx->GetExceptionFunction();
		const asIScriptFunction *function = engine->GetFunctionDescriptorById(funcId);
		int column = 0;
		desc += CL_StringHelp::text_to_local8(
			cl_format(" In function: %1 (line %2, col %3)\n", function->GetDeclaration(), ctx->GetExceptionLineNumber(&column), column)
			);
		desc += CL_StringHelp::text_to_local8( 
			cl_format(" In module:   %1\n", function->GetModuleName())
			);
		desc += CL_StringHelp::text_to_local8(
			cl_format(" In section:  %1\n", function->GetScriptSectionName())
			);

		desc += "Call Trace (if available):\n";
		printCallstack(engine, ctx, ctx->GetExceptionFunction(), desc);

		SendToConsole(desc);

		if (m_DebugMode & Exceptions)
		{
			DebugEvent ev;
			ev.type = DebugEvent::Exception;
			ev.context = ctx;
			ev.manager = this;
			SigDebug(ev);
		}
	}

	/*std::string ScriptingEngine::GetCurrentScriptSection(ctx)
	{
	}*/

	void ScriptingEngine::_lineCallback(asIScriptContext *ctx)
	{
		if (m_DebugMode & StepThrough)
		{
			ctx->Suspend();

			DebugEvent ev;
			ev.type = DebugEvent::Step;
			ev.context = ctx;
			ev.manager = this;
			SigDebug(ev);
		}

		if (m_DebugMode & Breakpoints)
		{
			Breakpoint here;
			here.module_name = ctx->GetCurrentModule();

			here.section_name = 
				ctx->GetEngine()->GetModule(ctx->GetCurrentModule())->GetFunctionDescriptorById(ctx->GetCurrentFunction())->GetScriptSectionName();

			BreakpointSet::iterator _where = m_Breakpoints.find(here);
			if (_where != m_Breakpoints.end())
			{
				ctx->Suspend();

				DebugEvent ev;
				ev.type = DebugEvent::Breakpoint;
				ev.context = ctx;
				ev.manager = this;
				SigDebug(ev);
			}
		}

		if (m_DebugOutput)
		{
			int funcId = ctx->GetCurrentFunction();
			int column, line = ctx->GetCurrentLineNumber(&column);

			asIScriptFunction *function = ctx->GetEngine()->GetFunctionDescriptorById(funcId);

			std::ostringstream str;

			str << "(" << line << "," << column << ")";

			SendToConsole("Executing: " + std::string(function->GetDeclaration(true)), str.str());
		}
	}

	void ScriptingEngine::_messageCallback(asSMessageInfo* msg)
	{ 
		const char *msgType = 0;
		if( msg->type == asMSGTYPE_ERROR ) msgType = "Error  ";
		if( msg->type == asMSGTYPE_WARNING ) msgType = "Warning";
		if( msg->type == asMSGTYPE_INFORMATION ) msgType = "Info   ";

		std::wstring formatted(cl_format("ScriptingManager - %1 (%2, %3) : %4 : %5", msg->section, msg->row, msg->col, msgType, msg->message));
		SendToConsole(formatted);
	}

	asIScriptModule *ScriptingEngine::getModuleOrThrow(const char *module) const
	{
		asIScriptModule *mod = m_asEngine->GetModule(module, asGM_ONLY_IF_EXISTS);
		if (mod == NULL)
			FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingManager::getModuleOrThrow", "There is no module with the requested name");
		return mod;
	}

	ScriptedSlotWrapper* ScriptingEngine::Scr_ConnectDebugSlot(const std::string &decl, ScriptingEngine *obj)
	{
		asIScriptContext *context = asGetActiveContext();
		if (context != NULL)
		{
			// Make sure the callback has the right param
			if (decl.find("(DebugEvent@)") == std::string::npos)
			{
				context->SetException(("Passed callback '"+decl+"' has the wrong signature - should be: 'void somefn(DebugEvent@)'").c_str());
				return NULL;
			}
			asIScriptModule *module = context->GetEngine()->GetModule( context->GetCurrentModule() );
			ScriptedSlotWrapper *slot = new ScriptedSlotWrapper(module, decl);

			bsig2::connection c = obj->SigDebug.connect( boost::bind(&ScriptedSlotWrapper::Callback<DebugEvent&>, slot, _1) );
			slot->HoldConnection(c);

			return slot;
		}

		return NULL;
	}

	ScriptedSlotWrapper* ScriptingEngine::Scr_ConnectDebugSlot(asIScriptObject *slot_object, ScriptingEngine *obj)
	{
		ScriptedSlotWrapper *slot = new ScriptedSlotWrapper(slot_object, "void ProcessEvent(DebugEvent@)");

		bsig2::connection c = obj->SigDebug.connect( boost::bind(&ScriptedSlotWrapper::Callback<DebugEvent&>, slot, _1) );
		slot->HoldConnection(c);

		return slot;
	}

	void ScriptingEngine::registerTypes()
	{
		int r;

		// Register types
		RegisterScriptMath(m_asEngine);
		m_StringTypeId = RegisterScriptString(m_asEngine);
		RegisterScriptStringUtils(m_asEngine);
		m_VectorTypeId = Scripting::RegisterScriptVector(m_asEngine);
		
		RegisterVector<std::string>("StringArray", "string", m_asEngine);

		ScriptedSlotWrapper::Register(m_asEngine);

		//RefCounted::RegisterType<DebugEvent>(m_asEngine, "DebugEvent");

		// Listener interface
		//r = m_asEngine->RegisterInterface("IDebugListener"); FSN_ASSERT(r >= 0);
		//r = m_asEngine->RegisterInterfaceMethod("IDebugListener", "void ProcessEvent(DebugEvent@)");

		RegisterSingletonType<ScriptingEngine>("ScriptManager", m_asEngine);
		r = m_asEngine->RegisterObjectMethod(
			"ScriptManager", "void enableDebugOutput()",
			asMETHOD(ScriptingEngine, EnableDebugOutput),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = m_asEngine->RegisterObjectMethod(
			"ScriptManager", "void disableDebugOutput()",
			asMETHOD(ScriptingEngine, DisableDebugOutput),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = m_asEngine->RegisterObjectMethod(
			"ScriptManager", "void debugOutputIsEnabled(bool)",
			asMETHOD(ScriptingEngine, DebugOutputIsEnabled),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);

		//r = m_asEngine->RegisterObjectMethod(
		//	"ScriptManager", "CallbackConnection@ connectTo_Debugger(const string&in)",
		//	asFUNCTION(ScriptingEngine::Scr_ConnectDebugSlot),
		//	asCALL_THISCALL); FSN_ASSERT(r >= 0);

		//r = m_asEngine->RegisterObjectMethod(
		//	"ScriptManager", "CallbackConnection@ connectTo_Debugger(IDebugListener@)",
		//	asFUNCTION(ScriptingEngine::Scr_ConnectDebugSlot),
		//	asCALL_THISCALL); FSN_ASSERT(r >= 0);

		RegisterGlobalObject("ScriptManager scriptManager", this);
	}

}
