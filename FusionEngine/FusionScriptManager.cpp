/*
  Copyright (c) 2006-2011 Fusion Project Team

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

#include "FusionScriptManager.h"

#include <boost/functional/hash.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <tbb/scalable_allocator.h>

#include <ClanLib/core.h>

#include "FusionExceptionFactory.h"
//#include "FusionScriptDebugPreprocessor.h"
#include "FusionScriptReference.h"
#include "FusionXML.h"

// Scripting extensions
#include "FusionScriptedSlots.h"
#include "FusionScriptVector.h"
#include "scriptarray.h"
#include "scriptstring.h"
#include "scriptstdvector.h"
#include "scriptmath.h"

using namespace std::placeholders;

namespace FusionEngine
{

	typedef std::pair<std::string::const_iterator, std::string::const_iterator> CharRange;

	// '#include' Preprocessor
	class IncludePreprocessor : public ScriptPreprocessor
	{
	public:
		IncludePreprocessor(ScriptManager *manager)
			: m_Manager(manager)
		{}
		virtual void Process(std::string &code, const char *module_name, const std::string &filename, MarkedLines &lines);

		bool isIncludeLine(const char *code_pos);
		// First is the beginning of the search area, last is the end of the search area
		CharRange getIncludeFile(std::string::const_iterator first, std::string::const_iterator last);

		std::string resolvePath(const std::string &working_directory, const CharRange &path);

		ScriptManager *m_Manager;
	};

	void IncludePreprocessor::Process(std::string &code, const char *module_name, const std::string &filename, ScriptPreprocessor::MarkedLines &marked)
	{
		if (marked.empty())
			return;

		std::string workingDirectory = fe_getbasepath(filename);

		std::string::size_type pos = 0;
		unsigned int line = 0;
		MarkedLines::const_iterator it = marked.begin(), end = marked.end();
		while (it != end)
		{
			if ( it->type == '#' && isIncludeLine(code.c_str() + it->pos) )
			{
				CharRange range = getIncludeFile(code.begin() + it->pos+8, code.end());
				std::string fullPath = resolvePath(workingDirectory, range);

				m_Manager->AddFile(fullPath, module_name);

				std::string::size_type lineEnd = code.find('\n', it->pos);
				code.erase(it->pos, lineEnd - it->pos); // remove the processed line from the code
				it = marked.erase(it); // Remove the marker from the markers list
				end = marked.end();
			}
			else
				++it;
		}
	}

	bool IncludePreprocessor::isIncludeLine(const char *start)
	{
		return std::string("#include").compare(0, 8, start, 8) == 0;
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
					begin = pos+1;
				else
					end = pos;

				if (++matched == 2)
					break;
			}
		}

		return CharRange(begin, end);
	}

	std::string IncludePreprocessor::resolvePath(const std::string &working_directory, const CharRange &path)
	{
		if (*path.first == '/')
			return std::string(path.first, path.second);

		else
		{
			typedef std::vector<boost::iterator_range<std::string::iterator>> SplitResult;

			StringVector currentPath;
			boost::split(currentPath, working_directory, boost::is_any_of("/"));

			StringVector pathTokens;
			boost::split(pathTokens, path, boost::is_any_of("/"));
			for (StringVector::iterator it = pathTokens.begin(), end = pathTokens.end(); it != end; ++it)
			{
				if (*it == "..")
					currentPath.pop_back();
				else
					currentPath.push_back(*it);
			}

			return boost::join(currentPath, "/");
		}
	}

	////////////
	// ScriptSection implementations
	ScriptManager::StringScriptSection::StringScriptSection()
	{}

	ScriptManager::StringScriptSection::StringScriptSection(const std::string &code)
		: m_Code(code)
	{}

	std::string &ScriptManager::StringScriptSection::GetCode()
	{
		return m_Code;
	}

	ScriptManager::FileScriptSection::FileScriptSection()
	{}

	ScriptManager::FileScriptSection::FileScriptSection(const std::string &filename)
		: m_Filename(filename)
	{}

	std::string &ScriptManager::FileScriptSection::GetCode()
	{
		if (m_Code.empty())
		{
			OpenString_PhysFS(m_Code, m_Filename);
		}
		return m_Code;
	}

	/////////
	// Breakpoint hasher
	std::size_t ScriptManager::hash_Breakpoint::operator ()(const ScriptManager::Breakpoint &value) const 
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, value.section_name);
		boost::hash_combine(seed, value.line);

		return seed;
	}

	bool operator ==(const ScriptManager::Breakpoint &lhs, const ScriptManager::Breakpoint &rhs)
	{
		return lhs.line == rhs.line && 
			lhs.module_name == rhs.module_name &&
			lhs.section_name == rhs.section_name;
	}

	//! Aborts the given ctx if the current time is after the given time
	/*!
	 * \remarks
	 * This has to be static so we can (easily) have a different timeoutTime for
	 * concurrently running scripts.
	 */
	static void TimeoutCallback(asIScriptContext *ctx, unsigned int* timeoutTime)
	{
		// If the time out is reached, abort the script
		if( *timeoutTime < CL_System::get_time() )
			ctx->Abort();
	}

	static void* ScritAlloc(size_t size)
	{
	}

	static void ScriptFree(void* ptr)
	{
	}

	//////////
	// ScriptingManager
	ScriptManager::ScriptManager()
		: m_DefaultTimeout(g_ScriptDefaultTimeout),
		m_DebugOutput(false),
		m_DebugMode(0),
		m_SectionSerial(0),
		m_ArrayTypeId(0),
		m_StringTypeId(0),
		m_VectorTypeId(0)
	{
		int r = asSetGlobalMemoryFunctions(&scalable_malloc, &scalable_free); FSN_ASSERT(r >= 0);

		m_asEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		if (m_asEngine != NULL)
		{
			m_asEngine->SetMessageCallback(asMETHOD(ScriptManager,_messageCallback), this, asCALL_THISCALL);
			registerTypes();
		}

		m_Preprocessors.push_back(PreprocessorPtr(new IncludePreprocessor(this)));
	}

	ScriptManager::~ScriptManager()
	{
		if (m_asEngine != NULL)
		{
			m_asEngine->GarbageCollect();
			m_asEngine->Release();
		}
		m_ScriptSections.clear();
	}


	asIScriptEngine *ScriptManager::GetEnginePtr() const
	{
		return m_asEngine;
	}

	int ScriptManager::GetArrayTypeId() const
	{
		return m_ArrayTypeId;
	}

	int ScriptManager::GetVector2DTypeId() const
	{
		return m_VectorTypeId;
	}

	int ScriptManager::GetStringTypeId() const
	{
		return m_StringTypeId;
	}

	bool ScriptManager::IsScriptArray(int typeId)
	{
		if (typeId & asTYPEID_TEMPLATE)
		{
			asIObjectType *defaultArrayType = m_asEngine->GetObjectTypeById(m_ArrayTypeId);
			asIObjectType *arrayType = m_asEngine->GetObjectTypeById(typeId);
			return strcmp(arrayType->GetName(), defaultArrayType->GetName()) == 0;
		}
		return false;
	}

	void ScriptManager::RegisterGlobalObject(const char *decl, void* ptr)
	{
		int r = m_asEngine->RegisterGlobalProperty(decl, ptr); FSN_ASSERT( r >= 0 );
	}

	void ScriptManager::Preprocess(std::string &script, const char *module_name)
	{
		Preprocess(script, module_name, std::string());
	}

	void ScriptManager::Preprocess(std::string &script, const char *module_name, const std::string &filename)
	{
		ScriptPreprocessor::MarkedLines lines;
		// Parses the script to find lines with preprocessor markers
		ScriptPreprocessor::checkForMarkedLines(lines, script);

		for (PreprocessorArray::iterator it = m_Preprocessors.begin(), end = m_Preprocessors.end();
			it != end; ++it)
		{
			(*it)->Process(script, module_name, filename, lines);
		}
	}

	bool ScriptManager::storeCodeString(const std::string &code, const char *section_name)
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

	bool ScriptManager::AddCode(const std::string &script, const char *module, const char *section_name)
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

	bool ScriptManager::AddFile(const std::string& filename, const char *module_name)
	{
		int r = -1;

		auto module = GetModule(module_name);

		if (m_ScriptSections.find(filename) != m_ScriptSections.end()
			&& std::find(module->GetSectionNames().begin(), module->GetSectionNames().end(), filename) != module->GetSectionNames().end())
			return false; // File already added

		// Load the script from the file
		std::string script;
		OpenString_PhysFS(script, filename);
		// Preprocess the script
		Preprocess(script, module_name, filename);
		// Add the script to the module
		r = module->AddCode(filename, script);
		// If the script was successfully added to the module, create a new
		//  ScriptSection for it (these are used for stepping through code
		//  in the debugger)
		if (r >= 0)
			m_ScriptSections[filename] = ScriptSectionPtr(new FileScriptSection(filename));

		return r >= 0;
	}

	void ScriptManager::DebugRebuild(const char *module)
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

	bool ScriptManager::BuildModule(const char* module)
	{
		//asIScriptModule* mod = m_asEngine->GetModule(module);
		ModulePtr mod = GetModule(module, asGM_ONLY_IF_EXISTS);
		// If the module doesn't exist, return false. Otherwise 
		//  return true on success
		if (mod == NULL) return false;
		else return mod->Build() >= 0;
	}

	boost::signals2::connection ScriptManager::SubscribeToModule(const char *module, Module::BuildModuleSlotType slot)
	{
		return GetModule(module)->ConnectToBuild(slot);
	}

	ScriptContext ScriptManager::ExecuteString(const std::string &script, const char *module_name, int timeout)
	{
		asIScriptContext* ctx = m_asEngine->CreateContext();
		ScriptContext sctx(ctx);

		if (ctx != NULL)
		{
			ctx->SetExceptionCallback(asMETHOD(ScriptManager, _exceptionCallback), this, asCALL_THISCALL);

			asIScriptModule *module = m_asEngine->GetModule(module_name, asGM_CREATE_IF_NOT_EXISTS);
			std::string fnScript = "void ExecuteString() {\n";
			fnScript += script;
			fnScript += "\n;}";
			asIScriptFunction *fn;
			int r = module->CompileFunction("ExecuteString", fnScript.c_str(), -1, 0, &fn); if (r < 0) return sctx;
			r = ctx->Prepare(fn->GetId()); if (r < 0) return sctx;
			ctx->Execute();

			fn->Release();
		}

		return sctx;
	}

	void ScriptManager::ReExecute(ScriptContext& context)
	{
		context.Execute();
	}

	void ScriptManager::SetDefaultTimeout(unsigned int timeout)
	{
		m_DefaultTimeout = timeout;
	}

	unsigned int ScriptManager::GetDefaultTimeout() const
	{
		return m_DefaultTimeout;
	}

	ScriptClass ScriptManager::GetClass(const char* module, const std::string& type_name)
	{
		int id = getModuleOrThrow(module)->GetTypeIdByDecl(type_name.c_str());
		return ScriptClass(this, module, type_name, id);
	}

	ScriptObject ScriptManager::CreateObject(const char* module, const std::string& type_name)
	{
		int id = getModuleOrThrow(module)->GetTypeIdByDecl(type_name.c_str());
		asIScriptObject* obj = static_cast<asIScriptObject*>( m_asEngine->CreateScriptObject(id) );
		return ScriptObject(obj, false);
	}

	ScriptObject ScriptManager::CreateObject(int id)
	{
		asIScriptObject* obj = static_cast<asIScriptObject*>( m_asEngine->CreateScriptObject(id) );
		return ScriptObject(obj, false);
	}

	ModulePtr ScriptManager::GetModule(const char *module_name, asEGMFlags when)
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

	ScriptUtils::Calling::Caller ScriptManager::GetCaller(const char * module_name, const std::string &signature)
	{
		ScriptUtils::Calling::Caller caller(m_asEngine->GetModule(module_name), signature.c_str());
		ConnectToCaller(caller);
		return caller;
	}

	ScriptUtils::Calling::Caller ScriptManager::GetCaller(const FusionEngine::ScriptObject &object, const std::string &signature)
	{
		ScriptUtils::Calling::Caller caller(object.GetScriptObject(), signature.c_str());
		ConnectToCaller(caller);
		return caller;
	}

	void ScriptManager::EnableDebugOutput()
	{
		m_DebugOutput = true;
	}

	void ScriptManager::DisableDebugOutput()
	{
		m_DebugOutput = false;
	}

	bool ScriptManager::DebugOutputIsEnabled()
	{
		return m_DebugOutput;
	}

	boost::signals2::connection ScriptManager::SubscribeToDebugEvents(ScriptManager::DebugSlotType slot)
	{
		return SigDebug.connect( slot );
	}

	void ScriptManager::SetDebugMode(unsigned char mode)
	{
		m_DebugMode = mode;
	}

	void ScriptManager::SetDebugOptions(const ScriptManager::DebugOptions &settings)
	{
		m_DebugSettings = settings;
	}

	void ScriptManager::SetBreakpoint(const char *module, const char *section, int line)
	{
		Breakpoint bp;
		bp.module_name = module;
		bp.section_name = section;
		bp.line = line;
		m_Breakpoints.insert(bp);
	}

	void ScriptManager::ConnectToCaller(ScriptUtils::Calling::Caller &caller)
	{
		caller.ConnectExceptionCallback( std::bind(&ScriptManager::_exceptionCallback, this, _1) );
		if (m_DebugOutput || (m_DebugMode & DebugModeFlags::StepThrough) || (m_DebugMode & DebugModeFlags::Breakpoints))
			caller.ConnectLineCallback( std::bind(&ScriptManager::_lineCallback, this, _1) );
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

	void ScriptManager::printCallstack(asIScriptEngine *const engine, asIScriptContext *ctx, int current_func, std::string &to)
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
				line = ctx->GetLineNumber(indent-1, &column);
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
			const char *sig = ctx->GetFunction(i)->GetDeclaration(true);
			int column, line;
			if (i > 0)
				line = ctx->GetLineNumber(i-1, &column);
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

	void ScriptManager::_exceptionCallback(asIScriptContext *ctx)
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

	/*std::string ScriptManager::GetCurrentScriptSection(ctx)
	{
	}*/

	void ScriptManager::_lineCallback(asIScriptContext *ctx)
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

			asIScriptFunction *fn = ctx->GetFunction();
			here.module_name = fn->GetModuleName();
			here.section_name = fn->GetScriptSectionName();

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
			int funcId = ctx->GetFunction()->GetId();
			int column, line = ctx->GetLineNumber(0U, &column);

			asIScriptFunction *function = ctx->GetEngine()->GetFunctionDescriptorById(funcId);

			std::ostringstream str;

			str << "(" << line << "," << column << ")";

			SendToConsole("Executing: " + std::string(function->GetDeclaration(true)), str.str());
		}
	}

	void ScriptManager::_messageCallback(asSMessageInfo* msg)
	{ 
		const char *msgType = 0;
		if( msg->type == asMSGTYPE_ERROR ) msgType = "Error  ";
		if( msg->type == asMSGTYPE_WARNING ) msgType = "Warning";
		if( msg->type == asMSGTYPE_INFORMATION ) msgType = "Info   ";

		std::string formatted(cl_format("ScriptingManager - %1 (%2, %3) : %4 : %5", msg->section, msg->row, msg->col, msgType, msg->message));
		SendToConsole(formatted);
	}

	asIScriptModule *ScriptManager::getModuleOrThrow(const char *module) const
	{
		asIScriptModule *mod = m_asEngine->GetModule(module, asGM_ONLY_IF_EXISTS);
		if (mod == NULL)
			FSN_EXCEPT(ExCode::InvalidArgument, "There is no module with the requested name");
		return mod;
	}

	ScriptedSlotWrapper* ScriptManager::Scr_ConnectDebugSlot(const std::string &decl, ScriptManager *obj)
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
			asIScriptModule *module = ctxGetModule(context);
			ScriptedSlotWrapper *slot = new ScriptedSlotWrapper(module, decl);

			boost::signals2::connection c = obj->SigDebug.connect( std::bind(&ScriptedSlotWrapper::Callback<DebugEvent&>, slot, _1) );
			slot->HoldConnection(c);

			return slot;
		}

		return NULL;
	}

	ScriptedSlotWrapper* ScriptManager::Scr_ConnectDebugSlot(asIScriptObject *slot_object, ScriptManager *obj)
	{
		ScriptedSlotWrapper *slot = new ScriptedSlotWrapper(slot_object, "void ProcessEvent(DebugEvent@)");

		boost::signals2::connection c = obj->SigDebug.connect( std::bind(&ScriptedSlotWrapper::Callback<DebugEvent&>, slot, _1) );
		slot->HoldConnection(c);

		return slot;
	}

	void ScriptManager::registerTypes()
	{
		int r;

		// Register types
		RegisterScriptMath(m_asEngine);
		RegisterScriptArray(m_asEngine, true);
		m_ArrayTypeId = m_asEngine->GetDefaultArrayTypeId();
		m_StringTypeId = RegisterScriptString(m_asEngine);
		RegisterScriptStringUtils(m_asEngine);
		m_VectorTypeId = Scripting::RegisterScriptVector(m_asEngine);
		
		RegisterVector<std::string>("StringArray", "string", m_asEngine);

		ScriptedSlotWrapper::Register(m_asEngine);

		//RefCounted::RegisterType<DebugEvent>(m_asEngine, "DebugEvent");

		// Listener interface
		//r = m_asEngine->RegisterInterface("IDebugListener"); FSN_ASSERT(r >= 0);
		//r = m_asEngine->RegisterInterfaceMethod("IDebugListener", "void ProcessEvent(DebugEvent@)");

		RegisterSingletonType<ScriptManager>("ScriptManager", m_asEngine);
		r = m_asEngine->RegisterObjectMethod(
			"ScriptManager", "void enableDebugOutput()",
			asMETHOD(ScriptManager, EnableDebugOutput),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = m_asEngine->RegisterObjectMethod(
			"ScriptManager", "void disableDebugOutput()",
			asMETHOD(ScriptManager, DisableDebugOutput),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = m_asEngine->RegisterObjectMethod(
			"ScriptManager", "void debugOutputIsEnabled(bool)",
			asMETHOD(ScriptManager, DebugOutputIsEnabled),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);

		//r = m_asEngine->RegisterObjectMethod(
		//	"ScriptManager", "SignalConnection@ connectToDebugger(const string&in)",
		//	asFUNCTION(ScriptManager::Scr_ConnectDebugSlot),
		//	asCALL_THISCALL); FSN_ASSERT(r >= 0);

		//r = m_asEngine->RegisterObjectMethod(
		//	"ScriptManager", "SignalConnection@ connectToDebugger(IDebugListener@)",
		//	asFUNCTION(ScriptManager::Scr_ConnectDebugSlot),
		//	asCALL_THISCALL); FSN_ASSERT(r >= 0);

		RegisterGlobalObject("ScriptManager scriptManager", this);
	}

}
