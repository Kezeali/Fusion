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

#ifndef Header_FusionEngine_ScriptedSlot
#define Header_FusionEngine_ScriptedSlot

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// External
#include <Calling/Caller.h>

// Fusion
#include "FusionRefCounted.h"

#include "FusionBoostSignals2.h"

#include "FusionScriptReference.h"
#include "FusionScriptTypeRegistrationUtils.h"


namespace FusionEngine
{

	//! Wrapper for boost#signals2#connection AND scripted slot caller
	class ScriptedSlotWrapper : public RefCounted, noncopyable
	{
	public:
		bsig2::connection m_Connection;

		ScriptUtils::Calling::Caller m_CallSlot;

		ScriptedSlotWrapper(asIScriptModule *module, const std::string &decl);
		ScriptedSlotWrapper(asIScriptObject *object, const std::string &decl);

		virtual ~ScriptedSlotWrapper();

		void HoldConnection(boost::signals2::connection &connection);

		void Disconnect();

		void Callback();

		// TODO: Use Boost Preprocessor to generate these params
		template <typename T0>
		void Callback(T0 p0)
		{
			m_CallSlot(p0);
		}

		template <typename T0>
		void Callback(T0& p0)
		{
			m_CallSlot(&p0);
		}

		static void Register(asIScriptEngine *engine);
	};

}

#endif