/*
  Copyright (c) 2009-2011 Fusion Project Team

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

#ifndef H_FusionEngine_ScriptedSlot
#define H_FusionEngine_ScriptedSlot

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// External
#include <ScriptUtils/Calling/Caller.h>

// Fusion
#include "FusionRefCounted.h"

//#include "FusionBoostSignals2.h"
#include <boost/signals2/signal.hpp>

#include "FusionScriptReference.h"
#include "FusionScriptTypeRegistrationUtils.h"


namespace FusionEngine
{

	//! Wrapper for boost#signals2#connection AND scripted slot caller
	class ScriptedSlotWrapper : public GarbageCollected<ScriptedSlotWrapper>, noncopyable
	{
	public:
		boost::signals2::connection m_Connection;
		ScriptUtils::Calling::Caller m_CallSlot;

		// this will be set to an object if the callback is an object method, otherwise this will be nullptr
		asIScriptObject *m_TargetObject;

		//! Ctor
		ScriptedSlotWrapper(asIScriptModule *module, const std::string &decl);
		//! Ctor
		ScriptedSlotWrapper(asIScriptObject *object, const std::string &decl);

		//! Dtor
		virtual ~ScriptedSlotWrapper();

		//! Sets the connection to keep open
		void HoldConnection(const boost::signals2::connection &connection);
		//! Disconnects the held connection
		void Disconnect();

		//! Executes the bound function (or 'callback') with no parameters
		void Callback();

		//! Same as Callback(void), but with a distinct name...
		/*!
		* ... because std::bind can't identify the correct method (it assumes the template
		* version, taking ScriptedSlotWrapper* as the arg), when used like: std::bind(&ScriptedSlotWrapper::Callback, slotWrapper)
		* The same syntax actually DOES compile with boost::bind, so this may be a bug in the MSVC implementation, or something
		* that differs in the standard vs. the TR1 spec.
		*/
		void CallbackNoParam();

		// TODO: Use varadic template to generate these params
		//! Executes the bound function, with the given parameters
		template <typename T0>
		void Callback(T0 p0)
		{
			if (m_CallSlot)
			{
				if (std::is_reference<T0>::value)
					m_CallSlot(&p0);
				else
					m_CallSlot(p0);
			}
		}

		//! You can use this version as the callback function when you know the passed param will be a ref
		template <typename T0>
		void CallbackRef(const T0& p0)
		{
			if (m_CallSlot)
				m_CallSlot(&p0);
		}

		//! Creates a slot for the named script method
		/*!
		* This will use the given script context to work out wheter the given
		* signature refers to an object method or a module method (function).
		*/
		static ScriptedSlotWrapper* CreateWrapperFor(asIScriptContext *context, const std::string &decl);

		virtual void EnumReferences(asIScriptEngine *engine);
		virtual void ReleaseAllReferences(asIScriptEngine *engine);

		//! Registers the script type that allows script objects to use this class' functionality
		static void Register(asIScriptEngine *engine);
	};

}

#endif
