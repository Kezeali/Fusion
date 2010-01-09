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

#ifndef Header_FusionEngine_ScriptingEngine
#define Header_FusionEngine_ScriptingEngine

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Inherited
#include "FusionSingleton.h"

// External
#include <Calling/Caller.h>

// Fusion
#include "FusionBoostSignals2.h"

#include "FusionScriptModule.h"

#include "FusionScriptVector.h"
#include "FusionScriptReference.h"
#include "FusionScriptPreprocessor.h"

#include "FusionScriptTypeRegistrationUtils.h"

#include "FusionConsole.h"


#define SCRIPT_ARG_USE_TEMPLATE

namespace FusionEngine
{

	static const std::string g_ASConfigConsole = "Console";

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

	struct DebugEvent
	{
		//! Reason for stopping
		enum EventType
		{
			//! Context is in step-through mode
			Step,
			//! Breakpoint was hit
			Breakpoint,
			//! Exception was fired
			Exception
		};

		EventType type;
		ScriptingEngine *manager;
		asIScriptContext *context;

		int refCount;

		void AddRef()
		{
			refCount++;
		}

		void Release()
		{
			if (--refCount == 0)
				delete this;
		}

		int Resume()
		{
			return context->Execute();
		}

		int Abort()
		{
			return context->Abort();
		}

		int GetLine() const
		{
			return context->GetCurrentLineNumber();
		}

		int GetColumn() const
		{
			int column;
			context->GetCurrentLineNumber(&column);
			return column;
		}

		std::string GetExceptionMessage()
		{
			return context->GetExceptionString();
		}
	};

	/*!
	 * \brief
	 * Provides access to scripting for all FusionEngine objects.
	 *
	 * \todo Rename ScriptingEngine -> ScriptingManager
	 *
	 * \sa
	 * Singleton
	 */
	class ScriptingEngine : public Singleton<ScriptingEngine>
	{
	public:
		typedef bsig2::signal<void (DebugEvent&)> DebugSignalType;
		typedef DebugSignalType::slot_type DebugSlotType;

	public:
		//! Basic constructor.
		ScriptingEngine();
		~ScriptingEngine();

	public:
		//! Returns a pointer to the AS engine.
		asIScriptEngine *GetEnginePtr() const;

		int GetVectorTypeId() const;
		int GetStringTypeId() const;

		//! Adds the given script to a module, but doesn't build or execute it.
		//void RegisterScript(Script *script, const char *module);

		//! Registers a global object (type must already be registered)
		void RegisterGlobalObject(const char *decl, void* ptr);

		void Preprocess(std::string &script, const char *module_name);
		void Preprocess(std::string &script, const char *module_name, const std::string &filename);

		bool storeCodeString(const std::string &code, const char *section_name);

		//! Adds code to the given module
		bool AddCode(const std::string& script, const char *module, const char *section_name = "Script String");

		//! Adds code from a file
		bool AddFile(const std::string &filename, const char *module);

		//! Returns the code for the given script section
		/*!
		* Loads data from file if the script section was loaded from a file
		*/
		std::string &GetCode(const char *module, const char *section);

		void SetCode(const char *module, const char *section, std::string &code);

		//! Rebuilds the given module using code modified for debugging
		void DebugRebuild(const char *module);

		//! Builds the given module
		bool BuildModule(const char *module);

		//! Subscribes to rebuild events from module with the given name
		/*!
		* Wrapper for Module#ConnectToBuild()
		*/
		bsig2::connection SubscribeToModule(const char *module, Module::BuildModuleSlotType slot);

		//! Executes the given funcion in a module.
		/*!
		 * If the module is unbuilt, it will be built.
		 *
		 * \param[in] module A built or unbuilt module
		 * \param[in] function Function sig
		 * \param[in] timeout Timeout in miliseconds. Set to zero to disable timeout.
		 *
		 * \returns The exit status of the function
		 */
		ScriptReturn Execute(const char *module, const char *function, unsigned int timeout = 0);

#ifdef SCRIPT_ARG_USE_TEMPLATE
		//! Executes the given global method
		ScriptReturn Execute(UCScriptMethod &function)
		{
			if (!function.IsValid())
				return ScriptContext(NULL);

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
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			cont->Execute();
			return scxt;
		}

		//! Executes the given method
		ScriptReturn Execute(ScriptObject &object, UCScriptMethod &method)
		{
			if (!object.IsValid() || !method.IsValid())
				return ScriptContext(NULL);

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
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			r = cont->SetObject(object.GetScriptObject());
			if (r < 0)
				return scxt;

			cont->Execute();
			return scxt;
		}

		//! Executes the given global method
		template<typename T1>
		ScriptReturn Execute(UCScriptMethod& method, T1 p1)
		{
			if (!method.IsValid())
				return ScriptContext(NULL);

			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			try
			{
				setArgument(cont, 0, p1);
			}
			catch (InvalidArgumentException &ex)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::Execute",
					"Exception while setting arguments for " + method.GetSignature() + ":\n\t" + ex.ToString());
			}

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			cont->Execute();
			return scxt;
		}
		//! Executes the given global method
		template<typename T1, typename T2>
		ScriptReturn Execute(UCScriptMethod& method, T1 p1, T2 p2)
		{
			if (!method.IsValid())
				return ScriptContext(NULL);

			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			try
			{
				setArgument(cont, 0, p1);
				setArgument(cont, 1, p2);
			}
			catch (InvalidArgumentException &ex)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::Execute",
					"Exception while setting arguments for " + method.GetSignature() + ":\n\t" + ex.ToString());
			}

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			cont->Execute();
			return scxt;
		}
		//! Executes the given global method
		template<typename T1, typename T2, typename T3>
		ScriptReturn Execute(UCScriptMethod& method, T1 p1, T2 p2, T3 p3)
		{
			if (!method.IsValid())
				return ScriptContext(NULL);

			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(function.GetFunctionID());
			if (r < 0)
				return scxt;

			try
			{
				setArgument(cont, 0, p1);
				setArgument(cont, 1, p2);
				setArgument(cont, 2, p3);
			}
			catch (InvalidArgumentException &ex)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::Execute",
					"Exception while setting arguments for " + method.GetSignature() + ":\n\t" + ex.ToString());
			}

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			cont->Execute();
			return scxt;
		}
		//! Executes the given global method
		template<typename T1, typename T2, typename T3, typename T4>
		ScriptReturn Execute(UCScriptMethod& method, T1 p1, T2 p2, T3 p3, T4 p4)
		{
			if (!method.IsValid())
				return ScriptContext(NULL);

			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(function.GetFunctionID());
			if (r < 0)
				return scxt;

			try
			{
				setArgument(cont, 0, p1);
				setArgument(cont, 1, p2);
				setArgument(cont, 2, p3);
				setArgument(cont, 3, p4);
			}
			catch (InvalidArgumentException &ex)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::Execute",
					"Exception while setting arguments for " + method.GetSignature() + ":\n\t" + ex.ToString());
			}

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			cont->Execute();
			return scxt;
		}

		//! Executes the given method
		template<typename T1>
		ScriptReturn Execute(ScriptObject& object, UCScriptMethod& method, T1 p1)
		{
			if (!object.IsValid() || !method.IsValid())
				return ScriptContext(NULL);

			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			try
			{
				setArgument(cont, 0, p1);
			}
			catch (InvalidArgumentException &ex)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::Execute",
					"Exception while setting arguments for " + method.GetSignature() + ":\n\t" + ex.ToString());
			}

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			r = cont->SetObject(object.GetScriptStruct());
			if (r < 0)
				return scxt;

			cont->Execute();
			return scxt;
		}
		//! Executes the given method
		template<typename T1, typename T2>
		ScriptReturn Execute(ScriptObject& object, UCScriptMethod method, T1 p1, T2 p2)
		{
			if (!object.IsValid() || !method.IsValid())
				return ScriptContext(NULL);

			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			try
			{
				setArgument(cont, 0, p1);
				setArgument(cont, 1, p2);
			}
			catch (InvalidArgumentException &ex)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::Execute",
					"Exception while setting arguments for " + method.GetSignature() + ":\n\t" + ex.ToString());
			}

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			r = cont->SetObject(object.GetScriptStruct());
			if (r < 0)
				return scxt;

			cont->Execute();
			return scxt;
		}
		//! Executes the given method
		template<typename T1, typename T2, typename T3>
		ScriptReturn Execute(ScriptObject& object, UCScriptMethod method, T1 p1, T2 p2, T3 p3)
		{
			if (!object.IsValid() || !method.IsValid())
				return ScriptContext(NULL);

			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			try
			{
				setArgument(cont, 0, p1);
				setArgument(cont, 1, p2);
				setArgument(cont, 2, p3);
			}
			catch (InvalidArgumentException &ex)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::Execute",
					"Exception while setting arguments for " + method.GetSignature() + ":\n\t" + ex.ToString());
			}

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			r = cont->SetObject(object.GetScriptStruct());
			if (r < 0)
				return scxt;

			cont->Execute();
			return scxt;
		}
		//! Executes the given method
		template<typename T1, typename T2, typename T3, typename T4>
		ScriptReturn Execute(ScriptObject& object, UCScriptMethod method, T1 p1, T2 p2, T3 p3, T4 p4)
		{
			if (!object.IsValid() || !method.IsValid())
				return ScriptContext(NULL);

			asIScriptContext* cont = m_asEngine->CreateContext();
			ScriptReturn scxt(cont);

			int r = cont->Prepare(method.GetFunctionID());
			if (r < 0)
				return scxt;

			try
			{
				setArgument(cont, 0, p1);
				setArgument(cont, 1, p2);
				setArgument(cont, 2, p3);
				setArgument(cont, 3, p4);
			}
			catch (InvalidArgumentException &ex)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::Execute",
					"Exception while setting arguments for " + method.GetSignature() + ":\n\t" + ex.ToString());
			}

			int timeoutTime;
			if (method.GetTimeout() > 0 || m_DefaultTimeout > 0)
			{
				timeoutTime = CL_System::get_time() + (method.GetTimeout() > 0 ? method.GetTimeout() : m_DefaultTimeout);
				cont->SetLineCallback(asFUNCTION(TimeoutCallback), &timeoutTime, asCALL_CDECL);
			}
			cont->SetExceptionCallback(asMETHOD(ScriptingEngine, _exceptionCallback), this, asCALL_THISCALL);

			r = cont->SetObject(object.GetScriptStruct());
			if (r < 0)
				return scxt;

			cont->Execute();
			return scxt;
		}
#else
		//! Executes the given global method
		ScriptReturn Execute(UCScriptMethod scref, ScriptArgument p1, ScriptArgument p2, ScriptArgument p3, ScriptArgument p4);

		//! Executes the given class method
		ScriptReturn Execute(ScriptObject& obj, UCScriptMethod& method, ScriptArgument p1, ScriptArgument p2, ScriptArgument p3, ScriptArgument p4);
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
		UCScriptMethod GetFunction(const char* module, const std::string& signature);
		//! Returns a method object for the given function
		bool GetFunction(UCScriptMethod& out, const char* module, const std::string& signature);
		//! Returns a class object corresponding to the given typename
		ScriptClass GetClass(const char* module, const std::string& type_name);
		//! Returns an object of the class corresponding to the given typename
		ScriptObject CreateObject(const char* module, const std::string& type_name);
		//! Returns an object of the class corresponding to the given type-id
		ScriptObject CreateObject(int type_id);

		UCScriptMethod GetClassMethod(const char* module_name, const std::string& type_name, const std::string &signature);

		UCScriptMethod GetClassMethod(ScriptClass& type, const std::string& signature);

		ModulePtr ScriptingEngine::GetModule(const char *module_name, asEGMFlags when = asGM_CREATE_IF_NOT_EXISTS);

		//! Returns a global caller
		ScriptUtils::Calling::Caller GetCaller(const char* module, const std::string &signature);
		//! Returns a method caller
		ScriptUtils::Calling::Caller GetCaller(const ScriptObject &object, const std::string &signature);

		void ConnectToCaller(ScriptUtils::Calling::Caller &caller);

		//! Enables the simple, built-in debug output
		void EnableDebugOutput();
		//! Disables the simple, built-in debug output
		void DisableDebugOutput();
		//! Returns true if debug output is enabled
		bool DebugOutputIsEnabled();

		bsig2::connection SubscribeToDebugEvents(DebugSlotType slot);

		enum DebugModeFlags {
			Disabled    = 0x0,
			StepThrough = 0x1,
			Breakpoints = 0x2,
			Exceptions  = 0x4
		};
		void SetDebugMode(unsigned char flags);

		//! Options for debug settings
		struct DebugOptions
		{
			//! Default constructor
			DebugOptions()
				: storeCodeStrings(false)
			{}
			//! Store code passed as strings (in-memory)
			/*
			* If this setting is enabled, code added to modules with AddCode is
			* stored so it can be shown in debugger output.
			*/
			bool storeCodeStrings;
		}
		m_DebugSettings;

		void SetDebugOptions(const DebugOptions &settings);

		void SetBreakpoint(const char* module, const char* section, int line);

		static void printCallstack(asIScriptEngine *const engine, asIScriptContext *ctx, int current_func, std::string &to);

		//! Called when a script exception occors
		void _exceptionCallback(asIScriptContext *ctx);
		void _lineCallback(asIScriptContext *ctx);
		//! Message callback
		void _messageCallback(asSMessageInfo *msg);

	private:
		//! AngelScript Engine
		asIScriptEngine *m_asEngine;

		int m_VectorTypeId;
		int m_StringTypeId;

		typedef std::tr1::unordered_map<std::string, ModulePtr> ModuleMap;
		ModuleMap m_Modules;

		typedef std::tr1::shared_ptr<ScriptPreprocessor> PreprocessorPtr;
		typedef std::vector<PreprocessorPtr> PreprocessorArray;
		PreprocessorArray m_Preprocessors;

		unsigned int m_DefaultTimeout;

		bool m_DebugOutput;
		int m_DebugMode;

		DebugSignalType SigDebug;

		class ScriptSection
		{
		public:
			virtual ~ScriptSection() {}
			virtual std::string &GetCode() =0;
		};
		class StringScriptSection : public ScriptingEngine::ScriptSection
		{
		public:
			StringScriptSection();
			StringScriptSection(const std::string &code);

			std::string &GetCode();

			std::string m_Code;
		};
		class FileScriptSection : public ScriptingEngine::ScriptSection
		{
		public:
			FileScriptSection();
			FileScriptSection(const std::string &filename);

			std::string &GetCode();

			std::string m_Filename;
			std::string m_Code;
		};
		typedef std::tr1::shared_ptr<ScriptSection> ScriptSectionPtr;
		typedef std::tr1::unordered_map<std::string, ScriptSectionPtr> ScriptSectionMap;

		ScriptSectionMap m_ScriptSections;

		struct Breakpoint
		{
			int line;
			const char *section_name;
			const char *module_name;
		};
		friend bool operator ==(const Breakpoint &lhs, const Breakpoint &rhs);

		struct hash_Breakpoint : public std::unary_function<Breakpoint, std::size_t>
		{
			std::size_t operator() (const Breakpoint &value) const;
		};
		typedef std::tr1::unordered_set<Breakpoint, hash_Breakpoint> BreakpointSet;

		BreakpointSet m_Breakpoints;

		int m_SectionSerial;

	private:
		//! Throws if the given return value of a SetArgX Fn indicates failure
		inline void checkSetArgReturn(int r, int arg, const std::string &type = "object")
		{
			if (r == asCONTEXT_NOT_PREPARED)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::setArgument",
					"Context not prepared");
			}
			else if (r == asINVALID_ARG)
			{
				std::string wrongNumOfArgs, rightNumOfArgs;
				if (arg = 0)
				{
					wrongNumOfArgs = "one argument";
					rightNumOfArgs = "no arguments";
				}
				else if (arg = 1)
				{
					wrongNumOfArgs = "2 arguments";
					rightNumOfArgs = "one argument";
				}
				else
				{
					wrongNumOfArgs = CL_StringHelp::int_to_local8(arg+1) + " arguments";
					rightNumOfArgs = CL_StringHelp::int_to_local8(arg) + " arguments";
				}
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::setArgument",
					"Provided " + wrongNumOfArgs + " to method taking " + rightNumOfArgs);
			}
			else if (r == asINVALID_TYPE)
			{
				FSN_EXCEPT(ExCode::InvalidArgument, "ScriptingEngine::setArgument", 
					"The " + type + " passed to argument " + 
					std::string(CL_StringHelp::int_to_local8(arg)) + " is of incorrect type");
			}
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for value / reference arguments.
		 */
		template<class T>
		void setArgument(asIScriptContext* cxt, int arg, T value)
		{
			checkSetArgReturn( cxt->SetArgObject(arg, (void*)(&value)), arg );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for pointer arguments.
		 */
		template<class T>
		void setArgument(asIScriptContext* cxt, int arg, T* value)
		{
			checkSetArgReturn( cxt->SetArgObject(arg, (void*)value), arg, "pointer" );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for bool (byte) arguments.
		 */
		template<>
		void setArgument<bool>(asIScriptContext* cxt, int arg, bool value)
		{
			checkSetArgReturn( cxt->SetArgByte(arg, value), arg, "bool" );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for char (byte) arguments.
		 */
		template<>
		void setArgument<char>(asIScriptContext* cxt, int arg, char value)
		{
			checkSetArgReturn( cxt->SetArgByte(arg, value), arg, "char" );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for short (i.e. 16-bit - Word) arguments.
		 */
		template<>
		void setArgument<short>(asIScriptContext* cxt, int arg, short value)
		{
			checkSetArgReturn( cxt->SetArgWord(arg, value), arg, "short" );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for int (DWord) arguments.
		 */
		template<>
		void setArgument<int>(asIScriptContext* cxt, int arg, int value)
		{
			checkSetArgReturn( cxt->SetArgDWord(arg, value), arg, "int" );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for unsigned int (DWord) arguments.
		 */
		template<>
		void setArgument<unsigned int>(asIScriptContext* cxt, int arg, unsigned int value)
		{
			checkSetArgReturn( cxt->SetArgDWord(arg, value), arg, "unsigned int" );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for long (QWord) arguments.
		 */
		template<>
		void setArgument<long>(asIScriptContext* cxt, int arg, long value)
		{
			checkSetArgReturn( cxt->SetArgQWord(arg, value), arg, "long" );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for unsigned long (QWord) arguments.
		 */
		template<>
		void setArgument<unsigned long>(asIScriptContext* cxt, int arg, unsigned long value)
		{
			checkSetArgReturn( cxt->SetArgQWord(arg, value), arg, "unsigned long" );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for float arguments.
		 */
		template<>
		void setArgument<float>(asIScriptContext* cxt, int arg, float value)
		{
			checkSetArgReturn( cxt->SetArgFloat(arg, value), arg, "float" );
		}
		//! Sets the given argument in the given context
		/*!
		 * Specialization for double arguments.
		 */
		template<>
		void setArgument<double>(asIScriptContext* cxt, int arg, double value)
		{
			checkSetArgReturn( cxt->SetArgDouble(arg, value), arg, "double" );
		}

		//! Returns the AngelScript module object with the given name or throws if nosuch exists
		asIScriptModule *getModuleOrThrow(const char *module) const;

		//! Registers global methods and functions which scripts can use.
		void registerTypes();

		static ScriptedSlotWrapper* Scr_ConnectDebugSlot(asIScriptObject *slot_object, ScriptingEngine *obj);
		static ScriptedSlotWrapper* Scr_ConnectDebugSlot(const std::string &decl, ScriptingEngine *obj);
	};

	bool operator ==(const ScriptingEngine::Breakpoint &lhs, const ScriptingEngine::Breakpoint &rhs);

}

#endif
