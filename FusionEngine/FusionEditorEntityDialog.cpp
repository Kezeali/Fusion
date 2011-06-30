/*
*  Copyright (c) 2010 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionEditorEntityDialog.h"

#include <boost/lexical_cast.hpp>
#include <boost/signals2.hpp>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Controls/ElementForm.h>
#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Controls/ElementDataGrid.h>
#include <Rocket/Controls/ElementDataGridRow.h>
#include <Rocket/Controls/ElementDataGridCell.h>

#include "FusionGUI.h"
#include "FusionEditorMapEntity.h"
#include "FusionEntity.h"
#include "FusionEntityManager.h"
#include "FusionExceptionFactory.h"
#include "FusionElementSelectableDataGrid.h"
#include "FusionEditorUndo.h"
#include "scriptstring.h"

namespace FusionEngine
{
	//! Undoable action for changes to entity properties
	template <class T>
	class ChangePropertyAction : public UndoableAction
	{
	public:
		ChangePropertyAction(const EditorMapEntityPtr &changed_entity, int property_index, int array_index, const T &from, const T &to);

		const std::string &GetTitle() const { return m_Title; }

	protected:
		void undoAction();
		void redoAction();

		std::string m_Title;

		EditorMapEntityPtr m_EditorEntity;
		int m_PropertyIndex, m_PropertyArrayIndex;
		T m_OldValue;
		T m_NewValue;
	};

	template <class T>
	ChangePropertyAction<T>::ChangePropertyAction(const EditorMapEntityPtr &changed_entity, int property_index, int array_index, const T &from, const T &to)
		: m_EditorEntity(changed_entity),
		m_PropertyIndex(property_index),
		m_PropertyArrayIndex(array_index),
		m_OldValue(from),
		m_NewValue(to)
	{
		m_Title = "Change [" + changed_entity->entity->GetType() + "] " + changed_entity->entity->GetName() + "." + changed_entity->entity->GetPropertyName(m_PropertyIndex);
	}

	template <class T>
	void ChangePropertyAction<T>::undoAction()
	{
		m_EditorEntity->SetPropertyValue(m_PropertyIndex, m_PropertyArrayIndex, m_OldValue);
	}

	template <class T>
	void ChangePropertyAction<T>::redoAction()
	{
		m_EditorEntity->SetPropertyValue(m_PropertyIndex, m_PropertyArrayIndex, m_NewValue);
	}

	//! Doesn't change where the pointer points to, but what it points to
	template <class T>
	class ChangePropertyAction<T*> : public UndoableAction
	{
		public:
		ChangePropertyAction(const EditorMapEntityPtr &changed_entity, int property_index, int array_index, const T &from, const T &to);

		const std::string &GetTitle() const { return m_Title; }

	protected:
		void undoAction();
		void redoAction();

		EditorMapEntityPtr m_EditorEntity;
		int m_PropertyIndex, m_PropertyArrayIndex;
		T m_OldValue;
		T m_NewValue;

		std::string m_Title;
	};

	template <class T>
	ChangePropertyAction<T*>::ChangePropertyAction(const EditorMapEntityPtr &changed_entity, int property_index, int array_index, const T &from, const T &to)
	{
		m_Title = "Change [" + changed_entity->entity->GetType() + "] " + changed_entity->entity->GetName() + "." + changed_entity->entity->GetPropertyName(m_PropertyIndex);
	}

	template <class T>
	void ChangePropertyAction<T*>::undoAction()
	{
		T* ptrToProperty = *static_cast<T**>( m_EditorEntity->entity->GetAddressOfProperty(m_PropertyIndex, m_PropertyArrayIndex) );
		(*ptrToProperty) = m_OldValue;
	}

	template <class T>
	void ChangePropertyAction<T*>::redoAction()
	{
		T* ptrToProperty = *static_cast<T**>( m_EditorEntity->entity->GetAddressOfProperty(m_PropertyIndex, m_PropertyArrayIndex) );
		(*ptrToProperty) = m_NewValue;
	}

	//! Undo changes to properties that store entity pointers
	template <>
	class ChangePropertyAction<EntityPtr> : public UndoableAction
	{
	public:
		ChangePropertyAction(const EditorMapEntityPtr &changed_entity, int property_index, int array_index, const EntityPtr &from, const EntityPtr &to)
		{
			m_Title = "Change [" + changed_entity->entity->GetType() + "] " + changed_entity->entity->GetName() + "." + changed_entity->entity->GetPropertyName(m_PropertyIndex);
		}

		const std::string &GetTitle() const { return m_Title; }

	protected:
		void undoAction()
		{
			m_EditorEntity->entity->SetPropertyEntity(m_PropertyIndex, m_PropertyArrayIndex, m_OldValue);
		}
		void redoAction()
		{
			m_EditorEntity->entity->SetPropertyEntity(m_PropertyIndex, m_PropertyArrayIndex, m_NewValue);
		}

		EditorMapEntityPtr m_EditorEntity;
		int m_PropertyIndex, m_PropertyArrayIndex;
		EntityPtr m_OldValue;
		EntityPtr m_NewValue;

		std::string m_Title;
	};


	//! Creates a form element for each property in the Properties data-grid
	class EditablePropertyFormatter : public Rocket::Controls::DataFormatter
	{
	public:
		EditablePropertyFormatter(const GameMapLoader::MapEntityPtr &map_entity);

		//! DataFormatter impl.
		void FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data);

	protected:
		GameMapLoader::MapEntityPtr m_MapEntity;

		template <typename T>
		void formatPrimativeElement(Rocket::Core::String &formatted_data, size_t index, size_t array_index, const Rocket::Core::String &str_index, const Rocket::Core::String &str_array_index);
	};

	EditablePropertyFormatter::EditablePropertyFormatter(const GameMapLoader::MapEntityPtr &map_entity)
		: m_MapEntity(map_entity)
	{
	}

	inline void toEmp(Rocket::Core::String &out, const std::string &str)
	{
		out.Assign(str.data(), str.length());
	}

	inline Rocket::Core::String formatId(const Rocket::Core::String &index, const Rocket::Core::String &array_index)
	{
		return index + "." + array_index;
	}

	template <typename T>
	void EditablePropertyFormatter::formatPrimativeElement(Rocket::Core::String &formatted_data, size_t index, size_t array_index, const Rocket::Core::String &index_str, const Rocket::Core::String &array_index_str)
	{
		EntityPtr &entity = m_MapEntity->entity;

		Rocket::Core::String name(entity->GetPropertyName(index).c_str());
		
		//T minValue = std::numeric_limits<T>::min(), maxValue = std::numeric_limits<T>::max();
		//Rocket::Core::String strMin;
		//toEmp(strMin, boost::lexical_cast<std::string>(minValue));
		//Rocket::Core::String strMax;
		//toEmp(strMax, boost::lexical_cast<std::string>(maxValue));

		if (!entity->PropertyIsArray(index) || array_index_str != "top")
		{
			T value = *static_cast<T*>(entity->GetAddressOfProperty(index, array_index));
			Rocket::Core::String strValue;
			Rocket::Core::TypeConverter<T, Rocket::Core::String>::Convert(value, strValue);

			formatted_data =
				"<input id=\"" + formatId(index_str, array_index_str) +
				"\" index=\"" + index_str + "\" array_index=\"" + array_index_str +
				"\" type=\"text\" name=\"" + name +
				"\" value=\"" + strValue + "\" />";
		}
		else
		{
			array_index = 0; // Just to make sure
			Rocket::Core::String strValue;
			// The first array value
			T *value = static_cast<T*>(entity->GetAddressOfProperty(index, array_index++));
			Rocket::Core::TypeConverter<T, Rocket::Core::String>::Convert(*value, strValue);
			formatted_data = strValue;
			// The rest of the values, comma seperated
			for (unsigned int array_size = entity->GetPropertyArraySize(index); array_index < array_size; ++array_index)
			{
				value = static_cast<T*>(entity->GetAddressOfProperty(index, array_index));
				Rocket::Core::TypeConverter<T, Rocket::Core::String>::Convert(*value, strValue);
				formatted_data += ", " + strValue;
			}
		}
	}

	template <typename T>
	void formatPrimative(Rocket::Core::String &formatted_data, void *prop, const Rocket::Core::String &index, const Rocket::Core::String &array_index, const Rocket::Core::String &name)
	{
		T value = *static_cast<T*>(prop);

		Rocket::Core::String strValue;
		Rocket::Core::TypeConverter<T, Rocket::Core::String>::Convert(value, strValue);

		formatted_data =
			"<input id=\"" + formatId(index, array_index) + "\" index=\"" + index + "\" array_index=\"" + array_index +
			"\" type=\"text\" name=\"" + name +
			"\" value=\"" + strValue + "\" />";
	}

	void formatStringElement(Rocket::Core::String &formatted_data, const std::string* const str, const Rocket::Core::String& index, const Rocket::Core::String &array_index, const Rocket::Core::String &name)
	{
		Rocket::Core::String value; toEmp(value, *str);
		formatted_data =
			"<input id=\"" + formatId(index, array_index) + "\" index=\"" + index + "\" array_index=\"" + array_index +
			"\" type=\"text\" name=\"" + name + "\" value=\"" + value + "\" />";
	}

	//template <typename T>
	//formatProperty(Rocket::Core::String &formatted_data, void *prop, const Rocket::Core::String &id, const Rocket::Core::String &name)
	//{
	//}

	//template <>
	//formatProperty<bool>(Rocket::Core::String &formatted_data, void *prop, const Rocket::Core::String &id, const Rocket::Core::String &name)
	//{
	//	bool checked = *static_cast<bool*>(prop);
	//	formatted_data =
	//		"<input id=\"" + id + "\" type=\"checkbox\" name=\"" + name + "\" value=\"true\" " + 
	//		Rocket::Core::String(checked ? "checked" : "") +
	//		"/>";
	//}

	void formatVectorElement(Rocket::Core::String &formatted_data, const Vector2 *prop, const Rocket::Core::String &index, const Rocket::Core::String &array_index, const Rocket::Core::String &name)
	{
		Rocket::Core::String x, y;
		Rocket::Core::TypeConverter<Vector2::type, Rocket::Core::String>::Convert(prop->x, x);
		Rocket::Core::TypeConverter<Vector2::type, Rocket::Core::String>::Convert(prop->y, y);

		formatted_data = x + ", " + y;
	}

	void EditablePropertyFormatter::FormatData(Rocket::Core::String &formatted_data, const Rocket::Core::StringList &raw_data)
	{
		if (raw_data.size() < 2)
			return;

		EntityPtr &entity = m_MapEntity->entity;

		Rocket::Core::String id = raw_data[0];

		const Rocket::Core::String &indexStr = raw_data[0];
		const Rocket::Core::String &arrayIndexStr = raw_data[1];

		bool container = true; // True if this line is expandable
		size_t index, array_index = 0;
		Rocket::Core::TypeConverter<Rocket::Core::String, size_t>::Convert(raw_data[0], index);
		if (raw_data[1] != "top")
		{
			if (Rocket::Core::TypeConverter<Rocket::Core::String, size_t>::Convert(raw_data[1], array_index))
			{
				container = false;
				id += "." + raw_data[1];
			}
		}

		Rocket::Core::String name; toEmp(name, entity->GetPropertyName(index));

		int type = entity->GetPropertyType(index) & ~Entity::pt_array_flag;
		if (type & Entity::pt_pointer_flag)
		{
			switch (type)
			{
			case Entity::pt_string:
				formatStringElement(formatted_data,
					*static_cast<std::string**>(entity->GetAddressOfProperty(index, array_index)),
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
						Rocket::Core::String(checked ? "checked" : "") +
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
				formatStringElement(formatted_data, static_cast<std::string*>(entity->GetAddressOfProperty(index, array_index)), indexStr, arrayIndexStr, name);
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
			FSN_EXCEPT(ExCode::NotImplemented, "The properties_dialog.rml document requires an element with the id: '" + name + "' to function.");
	}

	EntityEditorDialog::EntityEditorDialog(const GameMapLoader::MapEntityPtr &mapent, EntityManager *const entity_manager, UndoableActionManager *undo)
		: m_MapEntity(mapent),
		m_EntityManager(entity_manager),
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
		ElementForm *name_form = dynamic_cast<ElementForm*>( m_Document->GetElementById("name_form") ); verify(name_form, "name_form");

		m_InputX = dynamic_cast<ElementFormControlInput*>( m_Document->GetElementById("x") ); verify(m_InputX, "x");
		m_InputY = dynamic_cast<ElementFormControlInput*>( m_Document->GetElementById("y") ); verify(m_InputY, "y");
		m_InputName = dynamic_cast<ElementFormControlInput*>( name_form->GetElementById("name") ); verify(m_InputName, "name");
		m_InputType = dynamic_cast<ElementFormControlInput*>( m_Document->GetElementById("type") ); verify(m_InputType, "type");
		m_GridProperties = dynamic_cast<ElementSelectableDataGrid*>( m_Document->GetElementById("properties") ); verify(m_GridProperties, "properties");

		m_InputCommitName = dynamic_cast<ElementFormControlInput*>( name_form->GetElementById("commit_name") ); verify(m_InputCommitName, "commit_name");

		m_Document->AddEventListener("change", this);

		m_Document->AddEventListener("submit", this);

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
			SendToConsole("Opened properties editor and object that is not an Editor Entity - properties will not be displayed");

		Refresh();
	}

	EntityEditorDialog::~EntityEditorDialog()
	{
		if (m_Document != NULL)
		{
			m_Document->RemoveEventListener("submit", this);
			m_Document->RemoveEventListener("change", this);
			m_Document->Close();

			//m_GridProperties->RemoveEventListener("rowselected", this);
			//m_GridProperties->RemoveEventListener("rowdblclick", this);
			m_GridProperties->RemoveEventListener("keyup", this);

			m_Document->RemoveReference();
		}

		m_MapEntity.reset();
	}

	inline Rocket::Core::String to_emp(const std::string &str)
	{
		return Rocket::Core::String(str.data(), str.data() + str.length());
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

	template <typename T>
	void getProperty(T &value, const EditorMapEntityPtr &map_entity, unsigned int index, unsigned int array_index)
	{
		value = *static_cast<T*>( map_entity->entity->GetAddressOfProperty(index, array_index) );
	}

	// Used for primative types (except bool)
	template <typename T>
	void setPropertyPrimative(UndoableActionManager *undo, const EditorMapEntityPtr &map_entity, unsigned int index, unsigned int array_index, Rocket::Core::Event &ev)
	{
		Rocket::Core::String value = ev.GetTargetElement()->GetAttribute("value", Rocket::Core::String());
		try
		{
			T numericalValue = boost::lexical_cast<T>(value.CString());
			T oldValue = *static_cast<T*>( map_entity->entity->GetAddressOfProperty(index, array_index) );

			map_entity->SetPropertyValue(index, array_index, numericalValue);

			UndoableActionPtr action(new ChangePropertyAction<T>(map_entity, index, array_index, oldValue, numericalValue));
			undo->Add(action);
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
				const Rocket::Core::String &value = m_InputName->GetValue();
				//if (!value.Empty())
				//{
				//	m_MapEntity->hasName = true;
				//	m_MapEntity->entity->_setName(value.CString());
				//}
				//else
				//{
				//	m_MapEntity->hasName = false;
				//	m_MapEntity->entity->_setName("default");
				//}
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
		else if (ev == "submit")
		{
			Rocket::Core::String value = ev.GetParameter("name_value", Rocket::Core::String());
			if (!value.Empty())
			{
				m_MapEntity->hasName = true;
				//m_MapEntity->entity->_setName(value.CString());
				m_EntityManager->RenameEntity(m_MapEntity->entity, value.CString());
			}
			else
			{
				m_MapEntity->hasName = false;
				//m_MapEntity->entity->_setName("default");
				m_EntityManager->RenameEntity(m_MapEntity->entity, "default");
			}
		}
		else if (ev == "keyup")
		{
			if (ev.GetTargetElement()->GetAttribute("type", Rocket::Core::String()) == "text")
			{
				if (ev.GetParameter<int>("key_identifier", Rocket::Core::Input::KI_UNKNOWN) == Rocket::Core::Input::KI_RETURN)
				{
					EditorMapEntityPtr editorEntityPtr = boost::dynamic_pointer_cast<EditorMapEntity>(m_MapEntity);
					if (!editorEntityPtr)
						return;

					Rocket::Core::Element *elm = ev.GetTargetElement();
					unsigned int index = elm->GetAttribute<unsigned int>("index", 0);
					unsigned int array_index = elm->GetAttribute<unsigned int>("array_index", 0);

					Rocket::Core::String value = elm->GetAttribute("value", Rocket::Core::String());

					int type = m_MapEntity->entity->GetPropertyType(index) & ~Entity::pt_array_flag;
					switch (type)
					{
					case Entity::pt_int8:
						setPropertyPrimative<int8_t>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_int16:
						setPropertyPrimative<int16_t>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_int32:
						setPropertyPrimative<int32_t>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_int64:
						setPropertyPrimative<int64_t>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;

					case Entity::pt_uint8:
						setPropertyPrimative<uint8_t>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_uint16:
						setPropertyPrimative<uint16_t>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_uint32:
						setPropertyPrimative<uint32_t>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_uint64:
						setPropertyPrimative<uint64_t>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;

					case Entity::pt_float:
						setPropertyPrimative<float>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;
					case Entity::pt_double:
						setPropertyPrimative<double>(m_Undo, editorEntityPtr, index, array_index, ev);
						break;

					case Entity::pt_string:
						{
							std::string propValue(value.CString(), value.Length());
							std::string oldValue; getProperty(oldValue, editorEntityPtr, index, array_index);

							editorEntityPtr->SetPropertyValue(index, array_index, propValue);

							UndoableActionPtr action(new ChangePropertyAction<std::string>(editorEntityPtr, index, array_index, oldValue, propValue));
							m_Undo->Add(action);
						}
						break;
					case Entity::pt_string | Entity::pt_pointer_flag:
						{
							std::string *propValue = new std::string(value.CString(), value.Length());
							std::string* oldValue; getProperty(oldValue, editorEntityPtr, index, array_index);

							editorEntityPtr->SetPropertyValue(index, array_index, propValue);

							UndoableActionPtr action(new ChangePropertyAction<std::string*>(editorEntityPtr, index, array_index, *oldValue, *propValue));
							m_Undo->Add(action);
						}
						break;

					case Entity::pt_vector:
						{
							float numericVal;
							Rocket::Core::TypeConverter<Rocket::Core::String, float>::Convert(value, numericVal);
							Vector2 *vec = static_cast<Vector2*>(m_MapEntity->entity->GetAddressOfProperty(index, 0));

							Vector2 oldValue = *vec;
							(*vec)[array_index] = numericVal;

							editorEntityPtr->RefreshProperty(index);

							UndoableActionPtr action(new ChangePropertyAction<Vector2>(editorEntityPtr, index, array_index, oldValue, *vec));
							m_Undo->Add(action);
						}
						break;
					case Entity::pt_vector | Entity::pt_pointer_flag:
						{
							float numericVal;
							Rocket::Core::TypeConverter<Rocket::Core::String, float>::Convert(value, numericVal);
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
