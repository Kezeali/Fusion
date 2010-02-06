/*
  Copyright (c) 2010 Fusion Project Team

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

#include "FusionEditorEntityDialog.h"

#include "FusionBoostSignals2.h"
#include "FusionEditorMapEntity.h"
#include "FusionEntity.h"
#include "FusionGUI.h"
#include "FusionElementSelectableDataGrid.h"
#include "FusionEditorUndo.h"

#include <boost/lexical_cast.hpp>

#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Controls/ElementDataGrid.h>
#include <Rocket/Controls/ElementDataGridRow.h>
#include <Rocket/Controls/ElementDataGridCell.h>


namespace FusionEngine
{

	//! Creates a form element for each property in the Properties data-grid
	class EditablePropertyFormatter : public Rocket::Controls::DataFormatter
	{
	public:
		EditablePropertyFormatter(const GameMapLoader::GameMapEntityPtr &map_entity);

		//! DataFormatter impl.
		void FormatData(EMP::Core::String& formatted_data, const EMP::Core::StringList& raw_data);

	protected:
		GameMapLoader::GameMapEntityPtr m_MapEntity;

		template <typename T>
		void formatPrimativeElement(EMP::Core::String &formatted_data, size_t index, const EMP::Core::String &id = EMP::Core::String());
	};

	EditablePropertyFormatter::EditablePropertyFormatter(const GameMapLoader::GameMapEntityPtr &map_entity)
		: m_MapEntity(map_entity)
	{
	}

	inline void toEmp(EMP::Core::String &out, const std::string &str)
	{
		out.Assign(str.data(), str.length());
	}

	template <typename T>
	void EditablePropertyFormatter::formatPrimativeElement(EMP::Core::String &formatted_data, size_t index, const EMP::Core::String &id)
	{
		EntityPtr &entity = m_MapEntity->entity;

		//if (id.Empty())
		//	toEmp(id, boost::lexical_cast<std::string>(index));

		EMP::Core::String name(entity->GetPropertyName(index).c_str());

		T value = *static_cast<T*>(entity->GetAddressOfProperty(index));
		//T minValue = std::numeric_limits<T>::min(), maxValue = std::numeric_limits<T>::max();

		EMP::Core::String strValue;
		toEmp(strValue, boost::lexical_cast<std::string>(value));

		//EMP::Core::String strMin;
		//toEmp(strMin, boost::lexical_cast<std::string>(minValue));
		//EMP::Core::String strMax;
		//toEmp(strMax, boost::lexical_cast<std::string>(maxValue));

		formatted_data =
			"<input id=\"" + id + "\" type=\"text\" name=\"" + name +
			"\" value=\"" + strValue + "\" />";
	}

	void EditablePropertyFormatter::FormatData(EMP::Core::String &formatted_data, const EMP::Core::StringList &raw_data)
	{
		EntityPtr &entity = m_MapEntity->entity;

		size_t index = boost::lexical_cast<size_t>(raw_data[0].CString());

		const EMP::Core::String &id = raw_data[0];

		EMP::Core::String name; toEmp(name, entity->GetPropertyName(index));

		switch (entity->GetPropertyType(index))
		{
		case Entity::pt_bool:
			{
				bool checked = *static_cast<bool*>(entity->GetAddressOfProperty(index));
				formatted_data =
					"<input id=\"" + id + "\" type=\"checkbox\" name=\"" + name + "\" value=\"true\" " + 
					EMP::Core::String(checked ? "checked" : "") +
					"/>";
			}
			break;

		case Entity::pt_int8:
			formatPrimativeElement<int8_t>(formatted_data, index, id);
			break;
		case Entity::pt_int16:
			formatPrimativeElement<int16_t>(formatted_data, index, id);
			break;
		case Entity::pt_int32:
			formatPrimativeElement<int32_t>(formatted_data, index, id);
			break;
		case Entity::pt_int64:
			formatPrimativeElement<int64_t>(formatted_data, index, id);
			break;

			case Entity::pt_uint8:
			formatPrimativeElement<uint8_t>(formatted_data, index, id);
			break;
		case Entity::pt_uint16:
			formatPrimativeElement<uint16_t>(formatted_data, index, id);
			break;
		case Entity::pt_uint32:
			formatPrimativeElement<uint32_t>(formatted_data, index, id);
			break;
		case Entity::pt_uint64:
			formatPrimativeElement<uint64_t>(formatted_data, index, id);
			break;
		}
	}

	void verify(Rocket::Core::Element *element, const std::string &name)
	{
		if (element == NULL)
			FSN_EXCEPT(ExCode::NotImplemented, "EntityEditorDialog", "The properties_dialog.rml document requires an element with the id: '" + name + "' to function.");
	}

	EntityEditorDialog::EntityEditorDialog(const GameMapLoader::GameMapEntityPtr &mapent, UndoableActionManager *undo)
		: m_MapEntity(mapent),
		m_Undo(undo),
		m_Document(NULL),
		m_InputX(NULL),
		m_InputY(NULL),
		m_InputName(NULL),
		m_InputType(NULL),
		m_GridProperties(NULL)
	{
		using namespace Rocket::Controls;

		Rocket::Core::Context *guiCtx = GUI::getSingleton().GetContext();
		m_Document = guiCtx->LoadDocument("core/gui/properties_dialog.rml");
		if (m_Document == NULL)
			return;

		// Grab the elements from the doc.
		m_InputX = dynamic_cast<ElementFormControlInput*>( m_Document->GetElementById("x") ); verify(m_InputX, "x");
		m_InputY = dynamic_cast<ElementFormControlInput*>( m_Document->GetElementById("y") ); verify(m_InputY, "y");
		m_InputName = dynamic_cast<ElementFormControlInput*>( m_Document->GetElementById("name") ); verify(m_InputName, "name");
		m_InputType = dynamic_cast<ElementFormControlInput*>( m_Document->GetElementById("type") ); verify(m_InputType, "type");
		m_GridProperties = dynamic_cast<ElementSelectableDataGrid*>( m_Document->GetElementById("properties") ); verify(m_GridProperties, "properties");

		m_Document->AddEventListener("change", this);

		m_PropertiesFormatter.reset(new EditablePropertyFormatter(m_MapEntity));

		// Set up the properties listbox
		EditorMapEntityPtr editorEntityPtr = boost::dynamic_pointer_cast<EditorMapEntity>(m_MapEntity);
		if (editorEntityPtr)
		{
			m_GridProperties->AddColumn("index", m_PropertiesFormatter->GetDataFormatterName(), 0.f, "Value");
			m_GridProperties->SetDataSource(editorEntityPtr->GetDataSourceName() + ".properties");
			//m_GridProperties->AddEventListener("rowselected", this);
			//m_GridProperties->AddEventListener("rowdblclick", this);

			m_GridProperties->AddEventListener("keyup", this);
		}
		else
			SendToConsole("Can't display editable properties in an Entity property window that was just opened: not an Editor-Entity");

		Refresh();
	}

	EntityEditorDialog::~EntityEditorDialog()
	{
		if (m_Document != NULL)
		{
			m_Document->RemoveEventListener("change", this);
			m_Document->Close();

			//m_GridProperties->RemoveEventListener("rowselected", this);
			//m_GridProperties->RemoveEventListener("rowdblclick", this);
			m_GridProperties->RemoveEventListener("keyup", this);

			m_Document->RemoveReference();
		}

		m_MapEntity.reset();
	}

	inline EMP::Core::String to_emp(const std::string &str)
	{
		return EMP::Core::String(str.data(), str.data() + str.length());
	}

	void EntityEditorDialog::Refresh()
	{
		try
		{
			Vector2 position = m_MapEntity->entity->GetPosition();
			std::string value = boost::lexical_cast<std::string>(position.x);
			m_InputX->SetValue( to_emp(value) );

			value = boost::lexical_cast<std::string>(position.y);
			m_InputY->SetValue( to_emp(value) );
		}
		catch (const boost::bad_lexical_cast &)
		{
		}

		if (m_MapEntity->hasName)
			m_InputName->SetValue( to_emp(m_MapEntity->entity->GetName()) );

		m_InputType->SetValue( to_emp(m_MapEntity->entity->GetType()) );
	}

	void EntityEditorDialog::Show()
	{
		m_Document->Show();
	}

	Rocket::Core::ElementDocument* const EntityEditorDialog::GetDocument()
	{
		return m_Document;
	}

	// Used for primative types (except bool)
	template <typename T>
	void setPropertyPrimative(const EditorMapEntityPtr &map_entity, unsigned int index, Rocket::Core::Event &ev)
	{
		if (ev.GetParameter<int>("key_identifier", Rocket::Core::Input::KI_UNKNOWN) == Rocket::Core::Input::KI_RETURN)
		{
			EMP::Core::String value = ev.GetTargetElement()->GetAttribute("value", EMP::Core::String());
			try
			{
				T numericalValue = boost::lexical_cast<T>(value.CString());
				map_entity->SetPropertyValue(index, numericalValue);
			}
			catch (boost::bad_lexical_cast &ex)
			{
				std::string valStr(value.CString());
				SendToConsole("Failed to set property '" + map_entity->entity->GetPropertyName(index) + "' to " + valStr);
				SendToConsole(ex.what());
				map_entity->RefreshProperty(index);
			}
		}
	}

	void EntityEditorDialog::ProcessEvent(Rocket::Core::Event &ev)
	{
		if (ev == "change")
		{
			if (ev.GetTargetElement() == m_InputX || ev.GetTargetElement() == m_InputY)
			{
				try
				{
					Vector2 position;
					position.x = boost::lexical_cast<float>( m_InputX->GetValue().CString() );
					position.y = boost::lexical_cast<float>( m_InputY->GetValue().CString() );
					m_MapEntity->entity->SetPosition(position);
				}
				catch (const boost::bad_lexical_cast &)
				{
				}
			}

			else if (ev.GetTargetElement() == m_InputName)
			{
				const EMP::Core::String &value = m_InputName->GetValue();
				if (!value.Empty())
				{
					m_MapEntity->hasName = true;
					m_MapEntity->entity->_setName(value.CString());
				}
				else
				{
					m_MapEntity->hasName = false;
					m_MapEntity->entity->_setName("default");
				}
			}

			else if (ev.GetTargetElement()->GetParentNode()->GetTagName() == "datagridcell")
			{
				EditorMapEntityPtr editorEntityPtr = boost::dynamic_pointer_cast<EditorMapEntity>(m_MapEntity);
				if (!editorEntityPtr)
					return;
				
				size_t index = boost::lexical_cast<size_t>( ev.GetTargetElement()->GetId().CString() );
				
				if (m_MapEntity->entity->GetPropertyType(index) == Entity::pt_bool)
					editorEntityPtr->SetPropertyValue<bool>(index, ev.GetParameter("value", false));
			}
		}
		else if (ev == "keyup")
		{
			if (ev.GetTargetElement()->GetAttribute("type", EMP::Core::String()) == "text")
			{
				EditorMapEntityPtr editorEntityPtr = boost::dynamic_pointer_cast<EditorMapEntity>(m_MapEntity);
				if (!editorEntityPtr)
					return;

				Rocket::Core::Element *elm = ev.GetTargetElement();
				unsigned int index = boost::lexical_cast<unsigned int>( elm->GetId().CString() );

				switch (m_MapEntity->entity->GetPropertyType(index))
				{
				case Entity::pt_int8:
					setPropertyPrimative<int8_t>(editorEntityPtr, index, ev);
					break;
				case Entity::pt_int16:
					setPropertyPrimative<int16_t>(editorEntityPtr, index, ev);
					break;
				case Entity::pt_int32:
					setPropertyPrimative<int32_t>(editorEntityPtr, index, ev);
					break;
				case Entity::pt_int64:
					setPropertyPrimative<int64_t>(editorEntityPtr, index, ev);
					break;

				case Entity::pt_uint8:
					setPropertyPrimative<uint8_t>(editorEntityPtr, index, ev);
					break;
				case Entity::pt_uint16:
					setPropertyPrimative<uint16_t>(editorEntityPtr, index, ev);
					break;
				case Entity::pt_uint32:
					setPropertyPrimative<uint32_t>(editorEntityPtr, index, ev);
					break;
				case Entity::pt_uint64:
					setPropertyPrimative<uint64_t>(editorEntityPtr, index, ev);
					break;

				case Entity::pt_float:
					setPropertyPrimative<float>(editorEntityPtr, index, ev);
					break;
				case Entity::pt_double:
					setPropertyPrimative<double>(editorEntityPtr, index, ev);
					break;
				}
			}
		}
		else if (ev == "rowselected")
		{
		}
		else if (ev == "rowdblclick")
		{
			int selectedIndex = ev.GetParameter("row_index", (int)-1);
		}

		//else if (ev == "rowadd")
		//{
		//	int i = ev.GetParameter("first_row_added", 0);
		//	int end = i + ev.GetParameter("num_rows_added", 0);
		//	for (; i < end; ++i)
		//	{
		//		Rocket::Controls::ElementDataGridRow *row = m_GridProperties->GetRow(i);
		//		Rocket::Controls::ElementDataGridCell *cell = dynamic_cast<Rocket::Controls::ElementDataGridCell*>(row->GetChild(1));
		//		if (cell != NULL)
		//		{
		//			Rocket::Core::Element *inputElement = cell->GetChild(0);
		//			if (inputElement != NULL)
		//				inputElement->AddEventListener("change", this);
		//		}
		//	}
		//}
	}

}
