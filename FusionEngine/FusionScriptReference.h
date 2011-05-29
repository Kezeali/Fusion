/*
  Copyright (c) 2007-2009 Fusion Project Team

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

#ifndef H_FusionEngine_ScriptReference
#define H_FusionEngine_ScriptReference

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include <ScriptUtils/Calling/Caller.h>

#include <boost/type_traits.hpp>

namespace FusionEngine
{

	static const unsigned int g_ScriptDefaultTimeout = 1000;

	//! Used by non-template versions of ScriptManager#Execute
	class ScriptArgument
	{
	public:
		enum DataType { NO_ARG, FLOAT, DOUBLE, INTEGER, UINT, QWORD, BYTE, POINTER, OBJECT };

	public:
		ScriptArgument();
		ScriptArgument(int value);
		ScriptArgument(unsigned int value);
		ScriptArgument(float value);
		ScriptArgument(double value);
		ScriptArgument(void* object);
		~ScriptArgument();

	public:
		int GetValueInt();
		unsigned int GetValueUInt();
		float GetValueFloat();
		double GetValueDouble();
		void* GetValuePointer();

		bool IsValid() const;

	protected:
		bool m_Valid;

		DataType m_Type;
		void* m_Value;
	};

	typedef ScriptArgument Arg;

	//! Stores execution state
	class ScriptContext
	{
		// So I can be lazy (may not be used, but just in case):
		friend class ScriptManager;
	public:
		//! Constructor
		ScriptContext(asIScriptContext* ctx);
		//! Copy constructor
		ScriptContext(const ScriptContext& other);
		//! Destructor.
		/*!
		 * Releases the context if this is the last reference to it
		 */
		~ScriptContext();

	public:
		int Execute() const;
		void Suspend() const;
		void Abort() const;

		bool WasSuccessful() const;
		bool RaisedException() const;
		virtual bool IsOk() const;

		float GetReturnValueFloat() const;
		double GetReturnValueDouble() const;
		void* GetReturnPointer() const;
		template<typename T>
		T* GetReturnValueObject() const
		{
			return (T*)m_Context->GetReturnObject();
		}

	protected:
		asIScriptContext* m_Context;

		asIScriptContext* GetContext() const;
	};

	typedef ScriptContext ScriptReturn;

	//! Stores data needed to execute a script function
	/*!
	 * \sa ScriptManager
	 */
	class ScriptMethod
	{
	private:
		typedef ScriptArgument::DataType DataType;
	public:
		ScriptMethod() : m_FunctionID(-1), m_Timeout(0), m_Module(0) {}
		//! Constructor
		ScriptMethod(ScriptManager* manager, const char* module, const std::string& signature);
		//! Constructor
		ScriptMethod(const char* module, const std::string& signature, int funcID, unsigned int timeout = g_ScriptDefaultTimeout);

	public:
		const char* GetModule() const;
		const std::string& GetSignature() const;
		int GetFunctionID() const;
		unsigned int GetTimeout() const;

		void SetModule(const char* module);
		void SetSignature(const std::string& sig);
		void SetFunctionID(int id);
		void SetTimeout(unsigned int timeout);

		DataType GetArgType(unsigned int argIndex);

		bool IsValid() const;

	protected:
		const char* m_Module;
		std::string m_Signature;
		int m_FunctionID;
		unsigned int m_Timeout;

		std::vector<DataType> m_ArgTypes;
		bool m_NoArgs;

		void parseSignature();
	};

	typedef ScriptMethod UCScriptMethod;

	//! Stores a script object
	/*!
	 * \sa ScriptManager
	 */
	class ScriptObject
	{
	public:
		ScriptObject() : m_Object(NULL) {}
		//! Constructor
		ScriptObject(asIScriptObject* object, bool add_ref = true);
		//! Copy constructor
		ScriptObject(const ScriptObject& other);
		//! Destructor
		~ScriptObject();

		//! Copy assignment
		ScriptObject& operator=(const ScriptObject& rhs);
		//{
		//	m_Object = rhs.m_Object;
		//	m_Object->AddRef();
		//}

	public:
		int GetTypeId() const;
		asIScriptObject* GetScriptObject() const;

		ScriptUtils::Calling::Caller GetCaller(const std::string &method_decl) const;
		
		bool IsValid() const;

		void Release();

	protected:
		asIScriptObject* m_Object;

		//ScriptManager *m_Engine;
	};


	//! Stores a script class
	/*!
	 * \sa ScriptManager
	 */
	class ScriptClass
	{
	public:
		//! Constructor
		ScriptClass(ScriptManager* manager, const char* module, const std::string& declaration);
		//! Constructor
		ScriptClass(ScriptManager* manager, const char* module, const std::string& declaration, int type_id);
		//! Constructor
		ScriptClass(const char *module, asIObjectType* as_type, int type_id);

	public:
		asIObjectType *GetType() const;
		int GetTypeId() const;
		const char* GetModule() const;
		const std::string& GetDeclaration() const;
		UCScriptMethod GetMethod(const std::string& signature) const;
		ScriptObject Instantiate();

		bool IsValid() const;

	protected:
		const char* m_Module;
		std::string m_Decl;
		int m_TypeID;
		asIObjectType *m_Type;

		//ScriptManager* m_ScriptManager;
	};

}

#endif