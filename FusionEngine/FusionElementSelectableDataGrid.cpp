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

#include "FusionStableHeaders.h"

#include "FusionElementSelectableDataGrid.h"

#include <Rocket/Controls/ElementDataGridRow.h>
#include <EMP/Core/Dictionary.h>
#include <Rocket/Core/Factory.h>
#include <Rocket/Core/ElementInstancerGeneric.h>

#include "FusionExceptionFactory.h"


namespace FusionEngine
{

	ElementSelectableDataGrid::ElementSelectableDataGrid(const EMP::Core::String &tag)
		: Rocket::Controls::ElementDataGrid(tag)
	{
		m_Selectable = false;
		m_MinSelectableDepth = 0;
		//AddEventListener("rowadd", this);
	}

	ElementSelectableDataGrid::~ElementSelectableDataGrid()
	{
	}

	int ElementSelectableDataGrid::GetSelectedRow()
	{
		for (int i = 0, end = GetNumRows(); i < end; ++i)
		{
			Rocket::Controls::ElementDataGridRow *row = GetRow(i);
			if (row->IsClassSet("selected"))
				return i;
		}
		return -1;
	}

	void ElementSelectableDataGrid::SetSelectedRow(int newSelection)
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

	bool ElementSelectableDataGrid::IsSelectable()
	{
		return m_Selectable;
	}

	void ElementSelectableDataGrid::SetSelectable(bool selectable)
	{
		m_Selectable = selectable;
		if (selectable)
			SetSelectedRow(-1);
	}

	int ElementSelectableDataGrid::GetMinSelectableDepth()
	{
		return m_MinSelectableDepth;
	}

	void ElementSelectableDataGrid::SetMinSelectableDepth(int minDepth)
	{
		m_MinSelectableDepth = minDepth;
	}


	void ElementSelectableDataGrid::OnAttributeChange(const Rocket::Core::AttributeNameList& changed_attributes)
	{
		Rocket::Controls::ElementDataGrid::OnAttributeChange(changed_attributes);
		for (Rocket::Core::AttributeNameList::const_iterator it = changed_attributes.begin(), end = changed_attributes.end(); it != end; ++it)
		{
			const EMP::Core::String &attr = *it;
			if (attr == "selectable")
				// The attribute can validly have no value, in which case it defaults to
				//  true
				m_Selectable = HasAttribute("selectable") && GetAttribute<EMP::Core::String>("selectable", "") != "false";
		}
	}

	void ElementSelectableDataGrid::ProcessEvent(Rocket::Core::Event& ev)
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

	void ElementSelectableDataGrid::OnClick(Rocket::Core::Event &ev)
	{
		if (!m_Selectable)
			return;

		Rocket::Controls::ElementDataGridRow *row = dynamic_cast<Rocket::Controls::ElementDataGridRow*>( ev.GetCurrentElement() );
		if (row == NULL)
			return;

		int depth = GetRowDepth(row);
		if (depth < m_MinSelectableDepth)
			return;

		int prevSelection = GetSelectedRow();
		int newSelection = row->GetTableRelativeIndex();

		SetSelectedRow(newSelection);

		EMP::Core::Dictionary parameters;
		parameters.Set("row_index", newSelection);
		if (prevSelection != -1)
			parameters.Set("prev_row_index", prevSelection);
		DispatchEvent("rowselected", parameters);
	}

	void ElementSelectableDataGrid::OnDoubleClick(Rocket::Core::Event &ev)
	{
		Rocket::Controls::ElementDataGridRow *row = dynamic_cast<Rocket::Controls::ElementDataGridRow*>( ev.GetCurrentElement() );
		if (row != NULL)
		{
			EMP::Core::Dictionary parameters;
			parameters.Set("row_index", row->GetTableRelativeIndex());
			DispatchEvent("rowdblclick", parameters);
		}
	}


	void ElementSelectableDataGrid::RegisterElement(bool replace_datagird)
	{
		Rocket::Core::ElementInstancer* instancer = new Rocket::Core::ElementInstancerGeneric< ElementSelectableDataGrid >();
		Rocket::Core::Factory::RegisterElementInstancer(replace_datagird ? "datagrid" : "selectable_datagrid", instancer);
		instancer->RemoveReference();
	}

	int ElementSelectableDataGrid::GetRowDepth(Rocket::Controls::ElementDataGridRow *row)
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

}
