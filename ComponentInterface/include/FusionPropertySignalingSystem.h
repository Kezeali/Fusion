/*
*  Copyright (c) 2012 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef H_FusionPropertySignalingSystem
#define H_FusionPropertySignalingSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSingleton.h"

#include "FusionSynchronisedSignalingSystem.h"
#include "FusionPropertySyncSigDetail.h"

#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{

	typedef SyncSig::SynchronisedSignalingSystem<int, SyncSig::PropertyCallback> PropertySignalingSystem_t;

	class EvesdroppingManager : public Singleton<EvesdroppingManager>
	{
	public:
		EvesdroppingManager();

		PropertySignalingSystem_t& GetSignalingSystem()
		{
			return m_SignalingSystem;
		}

		static void RegisterScriptInterface(asIScriptEngine* engine);

	protected:
		PropertySignalingSystem_t m_SignalingSystem;
	};

	EvesdroppingManager::EvesdroppingManager()
	{
		ScriptManager::getSingleton().RegisterGlobalObject("EvesdroppingManager evesdropping", this);
	}

	SyncSig::HandlerConnection_t EvesdroppingManager_Connect(IComponentProperty* prop, const std::string& fn_decl, EvesdroppingManager* obj)
	{
		const auto engine = asGetActiveContext()->GetEngine();
		const auto module = engine->GetModule(asGetActiveContext()->GetFunction()->GetModuleName());
		ScriptUtils::Calling::Caller fn(module, fn_decl.c_str());

		return obj->GetSignalingSystem().AddHandler<int>(prop->GetID(), fn);
	}

	void EvesdroppingManager::RegisterScriptInterface(asIScriptEngine* engine)
	{
		RegisterSharedPtrType<boost::signals2::scoped_connection>("EvesdroppingConnection", engine);
		RegisterSingletonType<EvesdroppingManager>("EvesdroppingManager", engine);
		int r = engine->RegisterObjectMethod("EvesdroppingManager", "EvesdroppingConnection connect(Property@, const string &in)", asFUNCTION(EvesdroppingManager_Connect), asCALL_CDECL_OBJLAST);
		FSN_ASSERT(r >= 0);
	}

}

#endif
