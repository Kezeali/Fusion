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

#ifndef Header_FusionEngine_UndoMenu
#define Header_FusionEngine_UndoMenu

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionEditorUndoListener.h"
#include <Rocket/Controls/ElementFormControlSelect.h>

#include "FusionIDStack.h"


namespace FusionEngine
{

	//! Class that extends the ElementDataGrid in order to provide the ability to select rows.
	class ElementUndoMenu : public Rocket::Controls::ElementFormControlSelect, public UndoListener
	{
	public:
		ElementUndoMenu(const Rocket::Core::String &tag);
		virtual ~ElementUndoMenu();

		//void OnSetMaxActions(unsigned int max);
		void OnActionAdd(const UndoableActionPtr &action, bool to_end);
		void OnActionRemove(unsigned int first, UndoListener::Direction direction);

		//! So rad it adds in reverse
		//int RAdd(const Rocket::Core::String &rml, const Rocket::Core::String &value, int before = -1);
		//void RRemove(int index);

		static void RegisterElement();
		static void Register(asIScriptEngine *engine);

	protected:
		//unsigned int m_MaxActions;

		unsigned short m_NextId;

		//virtual void OnAttributeChange(const Rocket::Core::AttributeNameList& changed_attributes);

		virtual void ProcessEvent(Rocket::Core::Event& ev);

	};

}

#endif