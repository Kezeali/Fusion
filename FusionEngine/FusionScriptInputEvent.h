/*
*  Copyright (c) 2011 Fusion Project Team
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

#ifndef H_FusionScriptInputEvent
#define H_FusionScriptInputEvent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionRefCounted.h"

#include "FusionInputHandler.h"

namespace FusionEngine
{

	class ScriptInputEvent : public RefCounted
	{
	public:
		InputEvent input_event;

		explicit ScriptInputEvent(const InputEvent& ie)
			: input_event(ie)
		{}

		explicit ScriptInputEvent(InputEvent&& ie)
			: input_event(std::move(ie))
		{}

		int GetPlayer() const { return input_event.Player; }
		const std::string& GetInputName() const { return input_event.Input; }
		InputEvent::InputType GetType() const { return input_event.Type; }
		bool IsDown() const { return input_event.Down; }
		double GetValue() const { return input_event.Value; }

		static void Register(asIScriptEngine* engine);

	};

	inline void ScriptInputEvent::Register(asIScriptEngine* engine)
	{
		RegisterType<ScriptInputEvent>(engine, "InputEvent");
		int r;
		r = engine->RegisterEnum("InputType"); FSN_ASSERT(r >= 0);
		r = engine->RegisterEnumValue("InputType", "Binary", InputEvent::Binary); FSN_ASSERT(r >= 0);
		r = engine->RegisterEnumValue("InputType", "AnalogNormalized", InputEvent::AnalogNormalized); FSN_ASSERT(r >= 0);
		r = engine->RegisterEnumValue("InputType", "AnalogAbsolute", InputEvent::AnalogAbsolute); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("InputEvent", "int get_player() const", asMETHOD(ScriptInputEvent, GetPlayer), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("InputEvent", "const string & get_inputName() const", asMETHOD(ScriptInputEvent, GetInputName), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("InputEvent", "InputType get_type() const", asMETHOD(ScriptInputEvent, GetType), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("InputEvent", "bool get_isDown() const", asMETHOD(ScriptInputEvent, IsDown), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("InputEvent", "double get_value() const", asMETHOD(ScriptInputEvent, GetValue), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

}

#endif
