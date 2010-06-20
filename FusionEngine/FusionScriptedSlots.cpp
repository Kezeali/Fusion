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

#include "FusionScriptedSlots.h"


namespace FusionEngine
{

	ScriptedSlotWrapper* ScriptedSlotWrapper::CreateWrapperFor(asIScriptContext *context, const std::string &decl)
	{
		ScriptedSlotWrapper *slot = nullptr;

		if (context != nullptr)
		{
			asIScriptObject *object = ctxGetObject(context);
			if (object != nullptr)
				slot = new ScriptedSlotWrapper(object, decl);
			else
			{
				asIScriptModule *module = ctxGetModule(context);
				slot = new ScriptedSlotWrapper(module, decl);
			}
		}

		return slot;
	}

	ScriptedSlotWrapper::ScriptedSlotWrapper(asIScriptModule *module, const std::string &decl)
		: m_CallSlot(module, decl.c_str()),
		m_TargetObject(nullptr)
	{
	}

	ScriptedSlotWrapper::ScriptedSlotWrapper(asIScriptObject *object, const std::string &decl)
		: m_TargetObject(object),
		m_CallSlot(object, decl.c_str())
	{
		m_TargetObject->AddRef();
	}

	ScriptedSlotWrapper::~ScriptedSlotWrapper()
	{
		m_Connection.disconnect();
	}

	void ScriptedSlotWrapper::Disconnect()
	{
		m_Connection.disconnect();
	}

	void ScriptedSlotWrapper::HoldConnection(boost::signals2::connection &connection)
	{
		m_Connection = connection;
	}

	void ScriptedSlotWrapper::Callback()
	{
		if (m_CallSlot.ok())
			m_CallSlot();
	}

	void ScriptedSlotWrapper::EnumReferences(asIScriptEngine *engine)
	{
		if (m_TargetObject != nullptr)
			engine->GCEnumCallback((void*)m_TargetObject);
		//if (m_CallSlot.ok())
		//	engine->GCEnumCallback((void*)m_TargetObject);
	}

	void ScriptedSlotWrapper::ReleaseAllReferences(asIScriptEngine *engine)
	{
		if (m_TargetObject != nullptr)
		{
			m_CallSlot.release();
			m_TargetObject->Release();
			m_TargetObject = nullptr;
		}
	}

	void ScriptedSlotWrapper::Register(asIScriptEngine *engine)
	{
		ScriptedSlotWrapper::RegisterGCType(engine, "SignalConnection");
	}

}
