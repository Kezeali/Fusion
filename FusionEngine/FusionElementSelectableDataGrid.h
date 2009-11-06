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

#ifndef Header_FusionEngine_SelectableDataGrid
#define Header_FusionEngine_SelectableDataGrid

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <Rocket/Controls/ElementDataGrid.h>

// IMPL (TODO: put in .cpp)
#include <Rocket/Controls/ElementDataGridRow.h>
#include <EMP/Core/Dictionary.h>
#include <Rocket/Core/Factory.h>


namespace FusionEngine
{

	//! Class that extends the ElementDataGrid in order to provide the ability to select rows.
	class ElementSelectableDataGrid : public Rocket::Controls::ElementDataGrid, public Rocket::Core::EventListener
	{
	public:
		ElementSelectableDataGrid(const EMP::Core::String &tag)
			: Rocket::Controls::ElementDataGrid(tag)
		{
			m_Selectable = false;
			m_MinSelectableDepth = 0;
			AddEventListener("rowadd", this);
		}

		virtual ~ElementSelectableDataGrid()
		{
		}

		virtual void ProcessEvent(Rocket::Core::Event& ev)
		{
			Rocket::Controls::ElementDataGrid::ProcessEvent(ev);

			if (ev.GetType() == "rowadd")
			{
				int i = ev.GetParameter("first_row_added", 0);
				int end = i + ev.GetParameter("num_rows_added", 0);
				for (; i < end; ++i)
				{
					Rocket::Controls::ElementDataGridRow *row = GetRow(i);
					row->AddEventListener("click", this);
					row->AddEventListener("dblclick", this);
				}
			}
			else if (ev.GetType() == "click")
			{
				OnClick(ev);
			}
			else if (ev.GetType() == "dblclick")
			{
				OnDoubleClick(ev);
			}
		}

		void OnClick(Rocket::Core::Event &ev)
		{
			if (!m_Selectable)
				return;

			Rocket::Controls::ElementDataGridRow *row = dynamic_cast<Rocket::Controls::ElementDataGridRow*>( ev.GetTargetElement() );
			if (!row)
				return;

			int depth = GetRowDepth(row);
			if (depth < m_MinSelectableDepth)
				return;

			int prevSelection = GetSelectedRow();
			int newSelection = row->GetTableRelativeIndex();
			if (newSelection == prevSelection)
				return;

			SetSelectedRow(newSelection);

			EMP::Core::Dictionary parameters;
			parameters.Set("row_index", newSelection);
			if (prevSelection != -1)
				parameters.Set("prev_row_index", prevSelection);
			DispatchEvent("rowselected", parameters);
		}

		void OnDoubleClick(Rocket::Core::Event &ev)
		{
			Rocket::Controls::ElementDataGridRow *row = dynamic_cast<Rocket::Controls::ElementDataGridRow*>( ev.GetTargetElement() );
			EMP::Core::Dictionary parameters;
			parameters.Set("row_index", row->GetTableRelativeIndex());
			DispatchEvent("rowdblclick", parameters);
		}

		static int GetRowDepth(Rocket::Controls::ElementDataGridRow *row)
		{
			int depth = 0;
			Rocket::Controls::ElementDataGridRow *parent = row->GetParentRow();
			while (parent != NULL)
			{
				++depth;
				parent = parent->GetParentRow();
			}
			return depth;
		}

		int GetSelectedRow()
		{
			for (int i = 0, end = GetNumRows(); i < end; ++i)
			{
				Rocket::Controls::ElementDataGridRow *row = GetRow(i);
				if (row->IsClassSet("selected"))
					return i;
			}
			return -1;
		}

		void SetSelectedRow(int newSelection)
		{
			if (newSelection >= 0 && !m_Selectable)
				FSN_EXCEPT(ExCode::InvalidArgument, "ElementSelectableDataGrid::SetSelectedRow", "The datagrid is not selectable");
			if (newSelection >= GetNumRows())
				FSN_EXCEPT(ExCode::InvalidArgument, "ElementSelectableDataGrid::SetSelectedRow", "Index out of range");

			for (int i = 0; i < GetNumRows(); ++i)
			{
				Rocket::Controls::ElementDataGridRow *row = GetRow(i);
				row->SetClass("selected", i == newSelection);
			}
		}

		bool IsSelectable()
		{
			return m_Selectable;
		}

		void SetSelectable(bool selectable)
		{
			m_Selectable = selectable;
			if (selectable)
				SetSelectedRow(-1);
		}

		int GetMinSelectableDepth()
		{
			return m_MinSelectableDepth;
		}

		void SetMinSelectableDepth(int minDepth)
		{
			m_MinSelectableDepth = minDepth;
		}

		static void RegisterElement(bool replace_datagird = true)
		{
			Rocket::Core::ElementInstancer* instancer = new Rocket::Core::ElementInstancerGeneric< ElementSelectableDataGrid >();
			Rocket::Core::Factory::RegisterElementInstancer(replace_datagird ? "datagrid" : "selectable_datagrid", instancer);
			instancer->RemoveReference();
		}

	protected:
		bool m_Selectable;
		int m_MinSelectableDepth;
	};


}

#endif