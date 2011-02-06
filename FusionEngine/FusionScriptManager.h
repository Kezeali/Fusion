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

#ifndef Header_FusionEngine_ScriptManager
#define Header_FusionEngine_ScriptManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSingleton.h"

#include <ScriptUtils/Calling/Caller.h>
#include <boost/signals2.hpp>

#include "FusionScriptModule.h"
#include "FusionScriptReference.h"
#include "FusionScriptPreprocessor.h"
#include "FusionConsole.h"

#ifdef FSN_SCRIPTMANAGER_EXECUTE
#include <ClanLib/Core/System/system.h>
#define SCRIPT_ARG_USE_TEMPLATE
#endif

namespace FusionEngine
{

	//! Sent to event handlers
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
		ScriptManager *manager;
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
			return context->GetLineNumber();
		}

		int GetColumn() const
		{
			int column;
			context->GetLineNumber(0U, &column);
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
	 * \todo Rename ScriptManager -> ScriptingManager
	 *
	 * \sa
	 * Singleton
	 */
	class ScriptManager : public Singleton<ScriptManager>
	{
	public:
		typedef boost::signals2::signal<void (DebugEvent&)> DebugSignalType;
		typedef DebugSignalType::slot_type DebugSlotType;

	public:
		//! Basic constructor.
		ScriptManager();
		~ScriptManager();

	public:
		//! Returns a pointer to the AS engine.
		asIScriptEngine *GetEnginePtr() const;

		//! Returns the TypeId of the CScriptArray binding
		int GetArrayTypeId() const;
		//! Returns the TypeId of the ScriptVector (a Vector2D<float> wrapper) binding
		int GetVector2DTypeId() const;
		//! Returns the TypeId of the CScriptString binding
		int GetStringTypeId() const;

		bool IsScriptArray(int typeId);

		//! Adds the given script to a module, but doesn't build or execute it.
		//void RegisterScript(Script *script, const char *module);

		//! Registers a global object (type must already be registered)
		void RegisterGlobalObject(const char *decl, void* ptr);

		//! Runs preprocessors on the given script
		void Preprocess(std::string &script, const char *module_name);
		//! Runs preprocessors on the given script
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
		boost::signals2::connection SubscribeToModule(const char *module, Module::BuildModuleSlotType slot);

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

		//! Returns a class object corresponding to the given typename
		ScriptClass GetClass(const char* module, const std::string& type_name);
		//! Returns an object of the class corresponding to the given typename
		ScriptObject CreateObject(const char* module, const std::string& type_name);
		//! Returns an object of the class corresponding to the given type-id
		ScriptObject CreateObject(int type_id);

		ModulePtr GetModule(const char *module_name, asEGMFlags when = asGM_CREATE_IF_NOT_EXISTS);

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

		boost::signals2::connection SubscribeToDebugEvents(DebugSlotType slot);

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

		int m_ArrayTypeId;
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
		class StringScriptSection : public ScriptManager::ScriptSection
		{
		public:
			StringScriptSection();
			StringScriptSection(const std::string &code);

			std::string &GetCode();

			std::string m_Code;
		};
		class FileScriptSection : public ScriptManager::ScriptSection
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
			std::string section_name;
			std::string module_name;
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
		//! Returns the AngelScript module object with the given name or throws if nosuch exists
		asIScriptModule *getModuleOrThrow(const char *module) const;

		//! Registers global methods and functions which scripts can use.
		void registerTypes();

		static ScriptedSlotWrapper* Scr_ConnectDebugSlot(asIScriptObject *slot_object, ScriptManager *obj);
		static ScriptedSlotWrapper* Scr_ConnectDebugSlot(const std::string &decl, ScriptManager *obj);
	};

	bool operator ==(const ScriptManager::Breakpoint &lhs, const ScriptManager::Breakpoint &rhs);

}

#endif
