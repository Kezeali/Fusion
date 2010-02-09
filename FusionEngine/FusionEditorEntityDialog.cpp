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
#include "scriptstring.h"

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
		void formatPrimativeElement(EMP::Core::String &formatted_data, size_t index, size_t array_index, const EMP::Core::String &str_index, const EMP::Core::String &str_array_index);
	};

	EditablePropertyFormatter::EditablePropertyFormatter(const GameMapLoader::GameMapEntityPtr &map_entity)
		: m_MapEntity(map_entity)
	{
	}

	inline void toEmp(EMP::Core::String &out, const std::string &str)
	{
		out.Assign(str.data(), str.length());
	}

	inline EMP::Core::String formatId(const EMP::Core::String &index, const EMP::Core::String &array_index)
	{
		return index + "." + array_index;
	}

	template <typename T>
	void EditablePropertyFormatter::formatPrimativeElement(EMP::Core::String &formatted_data, size_t index, size_t array_index, const EMP::Core::String &index_str, const EMP::Core::String &array_index_str)
	{
		EntityPtr &entity = m_MapEntity->entity;

		//if (index_str.Empty())
		//	toEmp(index_str, boost::lexical_cast<std::string>(index));

		EMP::Core::String name(entity->GetPropertyName(index).c_str());

		T value = *static_cast<T*>(entity->GetAddressOfProperty(index, array_index));
		//T minValue = std::numeric_limits<T>::min(), maxValue = std::numeric_limits<T>::max();

		EMP::Core::String strValue;
		//toEmp(strValue, boost::lexical_cast<std::string>(value));
		EMP::Core::TypeConverter<T, EMP::Core::String>::Convert(value, strValue);

		//EMP::Core::String strMin;
		//toEmp(strMin, boost::lexical_cast<std::string>(minValue));
		//EMP::Core::String strMax;
		//toEmp(strMax, boost::lexical_cast<std::string>(maxValue));

		if (array_index_str != "top")
		{
			formatted_data =
				"<input id=\"" + formatId(index_str, array_index_str) +
				"\" index=\"" + index_str + "\" array_index=\"" + array_index_str +
				"\" type=\"text\" name=\"" + name +
				"\" value=\"" + strValue + "\" />";
		}
		else
			formatted_data = strValue;
	}

	template <typename T>
	void formatPrimative(EMP::Core::String &formatted_data, void *prop, const EMP::Core::String &index, const EMP::Core::String &array_index, const EMP::Core::String &name)
	{
		T value = *static_cast<T*>(prop);

		EMP::Core::String strValue;
		EMP::Core::TypeConverter<T, EMP::Core::String>::Convert(value, strValue);

		formatted_data =
			"<input id=\"" + formatId(index, array_index) + "\" index=\"" + index + "\" array_index=\"" + array_index +
			"\" type=\"text\" name=\"" + name +
			"\" value=\"" + strValue + "\" />";
	}

	void formatStringElement(EMP::Core::String &formatted_data, const CScriptString *str, const EMP::Core::String &index, const EMP::Core::String &array_index, const EMP::Core::String &name)
	{
		EMP::Core::String value; toEmp(value, str->buffer);
		formatted_data =
			"<input id=\"" + formatId(index, array_index) + "\" index=\"" + index + "\" array_index=\"" + array_index +
			"\" type=\"text\" name=\"" + name + "\" value=\"" + value + "\" />";
	}

	//template <typename T>
	//formatProperty(EMP::Core::String &formatted_data, void *prop, const EMP::Core::String &id, const EMP::Core::String &name)
	//{
	//}

	//template <>
	//formatProperty<bool>(EMP::Core::String &formatted_data, void *prop, const EMP::Core::String &id, const EMP::Core::String &name)
	//{
	//	bool checked = *static_cast<bool*>(prop);
	//	formatted_data =
	//		"<input id=\"" + id + "\" type=\"checkbox\" name=\"" + name + "\" value=\"true\" " + 
	//		EMP::Core::String(checked ? "checked" : "") +
	//		"/>";
	//}

	void formatVectorElement(EMP::Core::String &formatted_data, const Vector2 *prop, const EMP::Core::String &index, const EMP::Core::String &array_index, const EMP::Core::String &name)
	{
		EMP::Core::String x, y;
		EMP::Core::TypeConverter<Vector2::type, EMP::Core::String>::Convert(prop->x, x);
		EMP::Core::TypeConverter<Vector2::type, EMP::Core::String>::Convert(prop->y, y);

		formatted_data = x + ", " + y;
	}

	void EditablePropertyFormatter::FormatData(EMP::Core::String &formatted_data, const EMP::Core::StringList &raw_data)
	{
		if (raw_data.size() < 2)
			return;

		EntityPtr &entity = m_MapEntity->entity;

		EMP::Core::String id = raw_data[0];

		const EMP::Core::String &indexStr = raw_data[0];
		const EMP::Core::String &arrayIndexStr = raw_data[1];

		bool container = true; // True if this line is expandable
		size_t index, array_index = 0;
		EMP::Core::TypeConverter<EMP::Core::String, size_t>::Convert(raw_data[0], index);
		if (raw_data[1] != "top")
		{
			if (EMP::Core::TypeConverter<EMP::Core::String, size_t>::Convert(raw_data[1], array_index))
			{
				container = false;
				id += "." + raw_data[1];
			}
		}

		EMP::Core::String name; toEmp(name, entity->GetPropertyName(index));

		int type = entity->GetPropertyType(index) & ~Entity::pt_array_flag;
		if (type & Entity::pt_pointer_flag)
		{
			switch (type)
			{
			case Entity::pt_string:
				formatStringElement(formatted_data,
					*static_cast<CScriptString**>(entity->GetAddressOfProperty(index, array_index)),
					indexStr, arrayIndexStr, name);
				break;
			case Entity::pt_vector:
				{
					Vector2 *vec = *static_cast<Vector2**>(entity->GetAddressOfProperty(index));
					if (container) // The main row for this vector (displays data, non-editable
						formatVectorElement(formatted_data, vec, indexStr, arrayIndexStr, name);
					else // This is an expanded row (for one of the vector components)
						formatPrimative<Vector2::type>(formatted_data, &((*vec)[array_index]), indexStr, arrayIndexStr, name);
				}
				break;
			}
		}
		else
		{
			switch (type)
			{
			case Entity::pt_bool:
				{
					bool checked = *static_cast<bool*>(entity->GetAddressOfProperty(index, array_index));
					formatted_data =
						"<input id=\"" + id + "\" index=\"" + indexStr + "\" array_index=\"" + arrayIndexStr + "\" type=\"checkbox\" name=\"" + name + "\" value=\"true\" " + 
						EMP::Core::String(checked ? "checked" : "") +
						"/>";
				}
				break;

			case Entity::pt_int8:
				formatPrimativeElement<int8_t>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;
			case Entity::pt_int16:
				formatPrimativeElement<int16_t>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;
			case Entity::pt_int32:
				formatPrimativeElement<int32_t>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;
			case Entity::pt_int64:
				formatPrimativeElement<int64_t>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;

			case Entity::pt_uint8:
				formatPrimativeElement<uint8_t>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;
			case Entity::pt_uint16:
				formatPrimativeElement<uint16_t>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;
			case Entity::pt_uint32:
				formatPrimativeElement<uint32_t>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;
			case Entity::pt_uint64:
				formatPrimativeElement<uint64_t>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;

			case Entity::pt_float:
				formatPrimativeElement<float>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;
			case Entity::pt_double:
				formatPrimativeElement<double>(formatted_data, index, array_index, indexStr, arrayIndexStr);
				break;

			case Entity::pt_string:
					formatStringElement(formatted_data, static_cast<CScriptString*>(entity->GetAddressOfProperty(index)), indexStr, arrayIndexStr, name);
				break;

			case Entity::pt_vector:
				{
					Vector2 *vec = static_cast<Vector2*>(entity->GetAddressOfProperty(index));
					if (container) // The main row for this vector (displays data, non-editable
						formatVectorElement(formatted_data, vec, indexStr, arrayIndexStr, name);
					else // This is an expanded row (for one of the vector components)
						formatPrimative<Vector2::type>(formatted_data, &((*vec)[array_index]), indexStr, arrayIndexStr, name);
				}
				break;


			default:
				formatted_data = "<span class=\"error\">Can't display this property (unknown type)</span>";
				break;
			}
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
			m_GridProperties->AddColumn("index,array_index", m_PropertiesFormatter->GetDataFormatterName(), 0.f, "Value");
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
	void setPropertyPrimative(const EditorMapEntityPtr &map_entity, unsigned int index, unsigned int array_index, Rocket::Core::Event &ev)
	{
		EMP::Core::String value = ev.GetTargetElement()->GetAttribute("value", EMP::Core::String());
		try
		{
			T numericalValue = boost::lexical_cast<T>(value.CString());
			map_entity->SetPropertyValue(index, array_index, numericalValue);
		}
		catch (boost::bad_lexical_cast &ex)
		{
			std::string valStr(value.CString());
			SendToConsole("Failed to set property '" + map_entity->entity->GetPropertyName(index) + "' to " + valStr);
			SendToConsole(ex.what());
			map_entity->RefreshProperty(index);
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

				Rocket::Core::Element *elm = ev.GetTargetElement();
				unsigned int index = elm->GetAttribute<unsigned int>("index", 0);
				unsigned int array_index = elm->GetAttribute<unsigned int>("array_index", 0);
				
				if (m_MapEntity->entity->GetPropertyType(index) == Entity::pt_bool)
					editorEntityPtr->SetPropertyValue<bool>(index, array_index, ev.GetParameter("value", false));
			}
		}
		else if (ev == "keyup")
		{
			if (ev.GetTargetElement()->GetAttribute("type", EMP::Core::String()) == "text")
			{
				if (ev.GetParameter<int>("key_identifier", Rocket::Core::Input::KI_UNKNOWN) == Rocket::Core::Input::KI_RETURN)
				{
					EditorMapEntityPtr editorEntityPtr = boost::dynamic_pointer_cast<EditorMapEntity>(m_MapEntity);
					if (!editorEntityPtr)
						return;

					Rocket::Core::Element *elm = ev.GetTargetElement();
					unsigned int index = elm->GetAttribute<unsigned int>("index", 0);
					unsigned int array_index = elm->GetAttribute<unsigned int>("array_index", 0);

					EMP::Core::String value = elm->GetAttribute("value", EMP::Core::String());

					int type = m_MapEntity->entity->GetPropertyType(index) & ~Entity::pt_array_flag;
					switch (type)
					{
					case Entity::pt_int8:
						setPropertyPrimative<int8_t>(editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_int16:
						setPropertyPrimative<int16_t>(editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_int32:
						setPropertyPrimative<int32_t>(editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_int64:
						setPropertyPrimative<int64_t>(editorEntityPtr, index, array_index, ev);
						break;

					case Entity::pt_uint8:
						setPropertyPrimative<uint8_t>(editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_uint16:
						setPropertyPrimative<uint16_t>(editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_uint32:
						setPropertyPrimative<uint32_t>(editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_uint64:
						setPropertyPrimative<uint64_t>(editorEntityPtr, index, array_index, ev);
						break;

					case Entity::pt_float:
						setPropertyPrimative<float>(editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_double:
						setPropertyPrimative<double>(editorEntityPtr, index, array_index, ev);
						break;

					case Entity::pt_string:
						{
							CScriptString propValue(value.CString(), value.Length());
							editorEntityPtr->SetPropertyValue(index, array_index, propValue);
						}
						break;
					case Entity::pt_string | Entity::pt_pointer_flag:
						{
							CScriptString *propValue = new CScriptString(value.CString(), value.Length());
							editorEntityPtr->SetPropertyValue(index, array_index, propValue);
						}
						break;

					case Entity::pt_vector:
						{
							float numericVal;
							EMP::Core::TypeConverter<EMP::Core::String, float>::Convert(value, numericVal);
							Vector2 *vec = static_cast<Vector2*>(m_MapEntity->entity->GetAddressOfProperty(index, 0));
							(*vec)[array_index] = numericVal;
							editorEntityPtr->RefreshProperty(index);
						}
						break;
					case Entity::pt_vector | Entity::pt_pointer_flag:
						{
							float numericVal;
							EMP::Core::TypeConverter<EMP::Core::String, float>::Convert(value, numericVal);
							Vector2 *vec = *static_cast<Vector2**>(m_MapEntity->entity->GetAddressOfProperty(index, 0));
							(*vec)[array_index] = numericVal;
							editorEntityPtr->RefreshProperty(index);
						}
						break;
					}
				}
			}
		}
	}

}
