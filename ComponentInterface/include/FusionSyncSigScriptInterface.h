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

#ifndef H_FusionSynchronisedSignalingSystem
#define H_FusionSynchronisedSignalingSystem

#include "FusionPrerequisites.h"

#include "FusionExceptionFactory.h"

#include "FusionSynchronisedSignalingSystem.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include <ScriptUtils/Calling/Caller.h>

#include <angelscript.h>

namespace FusionEngine
{

	namespace SyncSig
	{

		static void RegisterHandlerConnection(asIScriptEngine* engine)
		{
			RegisterSharedPtrType<HandlerConnection_t>("SignalHandlerConnection", engine);
		}

		template <class SystemT>
		class SyncSigRegistrar
		{
		public:
			typedef typename SystemT::Key_t Key_t;
			static_assert(std::is_same<std::string, Key_t>::value, "Only string keys supported");

			static HandlerConnection_t AddIntHandler(const std::string& key, const std::string& fn_decl, SystemT* obj)
			{
				const auto engine = asGetActiveContext()->GetEngine();
				const auto module = engine->GetModule(asGetActiveContext()->GetFunction()->GetModuleName());
				ScriptUtils::Calling::Caller fn(module, fn_decl);
				return obj->AddHandler(key, fn);
			}

			static void RegisterSyncSig(asIScriptEngine* engine, const std::string& type_name)
			{
				auto cTypeName = type_name.c_str();
				RegisterSingletonType<SystemT>(type_name, engine);
				engine->RegisterObjectMethod(cTypeName, "SignalHandlerConnection AddHandler(const string &in, const string &in)", asFUNCTION(SyncSigRegistrar::AddIntHandler), asCALL_CDECL_OBJLAST);
			}
		};

	}

}

#endif
