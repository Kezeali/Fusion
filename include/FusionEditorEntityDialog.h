/*
*  Copyright (c) 2010-2011 Fusion Project Team
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

#ifndef H_FusionEditorEntityDialog
#define H_FusionEditorEntityDialog

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionGameMapLoader.h"

#include <Rocket/Core/EventListener.h>

namespace EMP {
	namespace Core {
		class DataSource;
	}
}

namespace Rocket {
	namespace Core {
		class ElementDocument;
	}
	namespace Controls {
		class DataFormatter;
		class ElementFormControlInput;
	}
}


namespace FusionEngine
{

	class ElementSelectableDataGrid;
	class UndoableActionManager;

	//! EntityEditorDialog creates a GUI window for editing a given entity
	class EntityEditorDialog : public Rocket::Core::EventListener
	{
	public:
		EntityEditorDialog(const GameMapLoader::MapEntityPtr &map_entity, EntityManager *const entity_manager, UndoableActionManager *undo);
		~EntityEditorDialog();

		//! Called when, for example, an action is undone
		void Refresh();

		//! Displays the dialog
		void Show();

		//! Returns the GUI element maintained by this object
		Rocket::Core::ElementDocument* const GetDocument();

		//! Processes GUI events
		void ProcessEvent(Rocket::Core::Event &ev);

	protected:
		Rocket::Core::ElementDocument *m_Document;
		Rocket::Controls::ElementFormControlInput *m_InputX;
		Rocket::Controls::ElementFormControlInput *m_InputY;
		Rocket::Controls::ElementFormControlInput *m_InputName;
		Rocket::Controls::ElementFormControlInput *m_InputCommitName; // Button used to confirm name changes
		Rocket::Controls::ElementFormControlInput *m_InputType;
		ElementSelectableDataGrid *m_GridProperties;

		EMP::Core::DataSource *m_PropertiesDataSource;
		std::tr1::shared_ptr<Rocket::Controls::DataFormatter> m_PropertiesFormatter;

		EntityManager* m_EntityManager; // Used when renaming
		GameMapLoader::MapEntityPtr m_MapEntity;
		UndoableActionManager *m_Undo;
	};

}

#endif
