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

#include "Common.h"

#include "FusionElementUndoMenu.h"

#include <Inheritance/RegisterConversion.h>

#include <Rocket/Core/Factory.h>
#include <Rocket/Core/ElementInstancerGeneric.h>

#include <Rocket/AngelScript/Core/registration_utils/ras_RegistrationUtils.h>

#include <boost/lexical_cast.hpp>


namespace FusionEngine
{

	ElementUndoMenu::ElementUndoMenu(const EMP::Core::String &tag)
		: Rocket::Controls::ElementFormControlSelect(tag)
	{
	}

	ElementUndoMenu::~ElementUndoMenu()
	{
	}

	void ElementUndoMenu::OnSetMaxActions(unsigned int max)
	{
		m_MaxActions = max;
		// Remove items from the end to shrink to max
		for (unsigned int i = GetNumOptions()-1; i >= max; --i)
				Remove(i);
	}

	void ElementUndoMenu::OnActionAdd(const UndoableActionPtr &action, bool to_end)
	{
		if (to_end)
			Add(action->GetTitle().c_str(), action->GetTitle().c_str());
		else
			Add(action->GetTitle().c_str(), action->GetTitle().c_str(), 0);
	}

	void ElementUndoMenu::OnActionRemove(unsigned int first, UndoListener::Direction direction)
	{
		if (GetNumOptions() == 0)
			return;

		if (direction == UndoListener::NONE)
			Remove(first);
		else if (direction == UndoListener::FORWARD)
		{
			if (first == 0)
			{
				RemoveAll();
				return;
			}
			// Remove from the given index to the end of the list
			for (unsigned int i = GetNumOptions()-1; i >= first; --i)
				Remove(i);
		}
		else // REVERSE
		{
			// Remove from the given index to the beginning of the list
			for (unsigned int i = first; i > 0; i--)
				Remove(i);
			Remove(0);
		}
	}

	void ElementUndoMenu::ProcessEvent(Rocket::Core::Event& ev)
	{
		Rocket::Controls::ElementFormControlSelect::ProcessEvent(ev);
	}

	void ElementUndoMenu::RegisterElement()
	{
		Rocket::Core::ElementInstancer* instancer = new Rocket::Core::ElementInstancerGeneric< ElementUndoMenu >();
		Rocket::Core::Factory::RegisterElementInstancer("UndoMenu", instancer);
		instancer->RemoveReference();
	}

	void registerElementFormControlSelectMembers(asIScriptEngine *engine)
	{
		int r;
		r = engine->RegisterObjectMethod("ElementUndoMenu", "int Add(const e_String &in, const e_String &in, int, bool)",
			asMETHODPR(Rocket::Controls::ElementFormControlSelect, Add, (const EMP::Core::String&, const EMP::Core::String&, int, bool), int), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("ElementUndoMenu", "void Remove(int)",
			asMETHOD(Rocket::Controls::ElementFormControlSelect, Remove), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("ElementUndoMenu", "SelectOption& GetOption(int)",
			asMETHOD(Rocket::Controls::ElementFormControlSelect, GetOption), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("ElementUndoMenu", "int GetNumOptions()",
			asMETHOD(Rocket::Controls::ElementFormControlSelect, GetNumOptions), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("ElementUndoMenu", "int GetSelection()",
			asMETHOD(Rocket::Controls::ElementFormControlSelect, GetSelection), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("ElementUndoMenu", "void SetSelection(int)",
			asMETHOD(Rocket::Controls::ElementFormControlSelect, SetSelection), asCALL_THISCALL);
	}

	void ElementUndoMenu::Register(asIScriptEngine *engine)
	{
		using namespace Rocket::AngelScript::_registration_utils;
		registerType::referenceCountable<ElementUndoMenu>(engine, "ElementUndoMenu");
		registerElementMembers<Rocket::Core::Element>(engine, "ElementUndoMenu");

		{
			using namespace ScriptUtils::Inheritance;
			RegisterBaseOf<Rocket::Core::Element, ElementUndoMenu>(engine, "Element", "ElementUndoMenu");
			RegisterBaseOf<Rocket::Controls::ElementFormControl, ElementUndoMenu>(engine, "ElementFormControl", "ElementUndoMenu");
			RegisterBaseOf<Rocket::Controls::ElementFormControlSelect, ElementUndoMenu>(engine, "ElementFormControlSelect", "ElementUndoMenu");
		}

		registerElementFormControlSelectMembers(engine);
	}

}
