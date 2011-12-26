/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#ifndef H_FusionElementSelectableDataGrid
#define H_FusionElementSelectableDataGrid

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <Rocket/Controls/ElementDataGrid.h>
#include <Rocket/Core/EventListener.h>


namespace FusionEngine
{

	//! Class that extends the ElementDataGrid in order to provide the ability to select rows.
	class ElementSelectableDataGrid : public Rocket::Controls::ElementDataGrid, public Rocket::Core::EventListener
	{
	public:
		ElementSelectableDataGrid(const Rocket::Core::String &tag);
		virtual ~ElementSelectableDataGrid();

		int GetSelectedRow();
		void SetSelectedRow(int newSelection);

		bool IsSelectable();
		void SetSelectable(bool selectable);

		int GetMinSelectableDepth();
		void SetMinSelectableDepth(int minDepth);

		static void RegisterElement(bool replace_datagird = true);

	protected:
		virtual void OnAttributeChange(const Rocket::Core::AttributeNameList& changed_attributes);

		virtual void ProcessEvent(Rocket::Core::Event& ev);

		void OnClick(Rocket::Core::Event &ev);
		void OnDoubleClick(Rocket::Core::Event &ev);

		static int GetRowDepth(Rocket::Controls::ElementDataGridRow *row);

	protected:
		bool m_Selectable;
		int m_MinSelectableDepth;
	};

}

#endif