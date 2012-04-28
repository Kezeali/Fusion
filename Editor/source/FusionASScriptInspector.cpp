/*
*  Copyright (c) 2012 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionASScriptInspector.h"

#include "FusionInspectorUtils.h"

#include "scriptany.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>

#include <boost/lexical_cast.hpp>

namespace FusionEngine { namespace Inspectors
{

	ASScriptInspector::ASScriptInspector(const Rocket::Core::String& tag)
		: ComponentInspector(tag)
	{
	}

	void ASScriptInspector::SetComponents(const std::vector<ComponentPtr>& components)
	{
		if (!components.empty())
		{
			auto firstScript = ComponentIPtr<ASScript>(components.front());
			FSN_ASSERT(firstScript);
			if (!firstScript->GetScriptInterface())
			{
				Rocket::Core::Factory::InstanceElementText(this, "Script component not instantiated");
				return;
			}
			FSN_ASSERT(firstScript->GetScriptInterface()->object);
			auto type = firstScript->GetScriptInterface()->object->GetTypeId();
			for (auto it = components.begin(), end = components.end(); it != end; ++it)
			{
				if (auto typed = ComponentIPtr<ASScript>(*it))
				{
					if (typed->GetScriptInterface() && typed->GetScriptInterface()->object->GetTypeId() == type)
						m_Components.push_back(typed);
					else
					{
						m_Components.clear();
						Rocket::Core::Factory::InstanceElementText(this, "Script types inconsistent");
						return;
					}
				}
			}

			if (!m_Components.empty())
			{
				FSN_ASSERT(m_Components.front()->GetScriptInterface());
				FSN_ASSERT(m_Components.front()->GetScriptInterface()->object);
				m_ScriptEngine = m_Components.front()->GetScriptInterface()->object->GetEngine();

				InitUI();
			}
			else
			{
				Rocket::Core::Factory::InstanceElementText(this, "Invalid component types");
			}
		}
	}

	void ASScriptInspector::ReleaseComponents()
	{
		m_Components.clear();
	}

	void ASScriptInspector::AddPropertyControl(Rocket::Core::Element* parent, unsigned int index, const std::string& name, unsigned int type_id)
	{
		auto line = Rocket::Core::Factory::InstanceElement(parent, "p", "p", Rocket::Core::XMLAttributes());

		Rocket::Core::Factory::InstanceElementText(line, Rocket::Core::String(name.data(), name.data() + name.length()));

		auto lowerName = fe_newlower(name);

		ScriptPropertyInput p;
		p.name = name;
		p.index = index;
		p.type_id = type_id;

		if (type_id == asTYPEID_BOOL)
		{
			Rocket::Core::XMLAttributes attributes;
			attributes.Set("type", "checkbox");
			attributes.Set("id", Rocket::Core::String((lowerName + "_input").c_str()));
			attributes.Set("name", Rocket::Core::String(lowerName.c_str()));
			//attributes.Set("value", "true");
			Rocket::Core::Element* check_input = Rocket::Core::Factory::InstanceElement(line,
				"input",
				"input",
				attributes);

			addControl(line, p.input, check_input);
		}
		else if (type_id >= asTYPEID_INT8 && type_id <= asTYPEID_DOUBLE)
		{
			Rocket::Core::XMLAttributes attributes;
			attributes.Set("type", "text");
			attributes.Set("id", Rocket::Core::String((lowerName + "_input").c_str()));
			attributes.Set("name", Rocket::Core::String(lowerName.c_str()));
			attributes.Set("value", "0");
			attributes.Set("size", "10");
			Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(line,
				"input",
				"input",
				attributes);

			addControl(line, p.input, text_input);
		}
		else if (type_id == ScriptManager::getSingleton().GetStringTypeId())
		{
			Rocket::Core::XMLAttributes attributes;
			attributes.Set("type", "text");
			attributes.Set("id", Rocket::Core::String((lowerName + "_input").c_str()));
			attributes.Set("name", Rocket::Core::String(lowerName.c_str()));
			attributes.Set("value", "0");
			attributes.Set("size", "30");
			Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(line,
				"input",
				"input",
				attributes);

			addControl(line, p.input, text_input);
		}
		else if (type_id == ScriptManager::getSingleton().GetVector2DTypeId())
		{
			p.array_index = 0;
			{
				Rocket::Core::XMLAttributes attributes;
				attributes.Set("type", "text");
				attributes.Set("id", Rocket::Core::String((lowerName + "_x_input").c_str()));
				attributes.Set("name", Rocket::Core::String((lowerName + "_x").c_str()));
				attributes.Set("value", "0");
				attributes.Set("size", "10");
				Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(line,
					"input",
					"input",
					attributes);

				addControl(line, p.input, text_input);
			}

			ScriptPropertyInput p2 = p;
			p2.array_index = 1;

			{
				Rocket::Core::XMLAttributes attributes;
				attributes.Set("type", "text");
				attributes.Set("id", Rocket::Core::String((lowerName + "_y_input").c_str()));
				attributes.Set("name", Rocket::Core::String((lowerName + "_y").c_str()));
				attributes.Set("value", "0");
				attributes.Set("size", "10");
				Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(line,
					"input",
					"input",
					attributes);

				addControl(line, p2.input, text_input);
			}

			if (p2.input)
				m_Inputs.push_back(p2);
		}
		else
		{
			const std::string message = "(No editor UI-element has been defined for this type.)";
			Rocket::Core::Factory::InstanceElementText(line, Rocket::Core::String(message.data(), message.data() + message.length()));
		}

		parent->AppendChild(line);
		line->RemoveReference();

		if (p.input)
			m_Inputs.push_back(p);
	}
	
	void ASScriptInspector::InitUI()
	{
		auto object = m_Components.front();

		m_Connections.clear();

		if (m_PropertiesSection)
		{
			this->RemoveChild(m_PropertiesSection.get());
			m_PropertiesSection.reset();
		}

		m_PropertiesSection = Rocket::Core::Factory::InstanceElement(this, "div", "div", Rocket::Core::XMLAttributes());
		this->AppendChild(m_PropertiesSection.get());

		auto typeName = object->GetScriptInterface()->object->GetObjectType()->GetName();
		Rocket::Core::Factory::InstanceElementText(m_PropertiesSection.get(), "Script Type: " + Rocket::Core::String(typeName));

		const auto& props = object->GetScriptProperties();
		unsigned int index = 0;
		for (auto it = props.begin(), end = props.end(); it != end; ++it, ++index)
		{
			const auto& prop = *it;
			AddPropertyControl(m_PropertiesSection.get(), index, prop.name, prop.type_id);

			for (auto cit = m_Components.begin(); cit != m_Components.end(); ++cit)
			{
				auto componentProperty = (*cit)->GetProperty(index);
				m_Connections.push_back(EvesdroppingManager::getSingleton().GetSignalingSystem().AddListener(componentProperty->GetID(), std::bind(&ASScriptInspector::RefreshPropertyValue, this, index)));
			}
		}

		m_PropertiesSection->RemoveReference();

		ResetUIValues();
	}

	void ASScriptInspector::RefreshPropertyValue(const ScriptPropertyInput& property_input_data, const ComponentIPtr<ASScript>& object, bool first)
	{
		boost::intrusive_ptr<CScriptAny> prop = object->GetPropertyRaw(property_input_data.index);
		if (!prop)
			return;
		prop->Release();
		FSN_ASSERT(prop->GetTypeId() == property_input_data.type_id);

		// Check for built-in types
		switch (property_input_data.type_id)
		{
		case asTYPEID_BOOL:
			{
				if (first)
				{
					if (prop->value.valueInt != 0)
						property_input_data.input->SetAttribute("checked", "");
					else
						property_input_data.input->RemoveAttribute("checked");
				}
				else
				{
					if (property_input_data.input->HasAttribute("checked") != (prop->value.valueInt != 0))
						property_input_data.input->RemoveAttribute("checked");
				}
			}
			break;
			// Int types
		case asTYPEID_INT8:
			{
				initUIValue(first, property_input_data.input, (std::int8_t)prop->value.valueInt);
			}
			break;
		case asTYPEID_INT16:
			{
				initUIValue(first, property_input_data.input, (std::int16_t)prop->value.valueInt);
			}
			break;
		case asTYPEID_INT32:
			{
				initUIValue(first, property_input_data.input, (std::int32_t)prop->value.valueInt);
			}
			break;
		case asTYPEID_INT64:
			{
				initUIValue(first, property_input_data.input, (std::int64_t)prop->value.valueInt);
			}
			break;
			// Unsigned types
		case asTYPEID_UINT8:
			{
				initUIValue(first, property_input_data.input, (std::uint8_t)prop->value.valueInt);
			}
			break;
		case asTYPEID_UINT16:
			{
				initUIValue(first, property_input_data.input, (std::uint16_t)prop->value.valueInt);
			}
			break;
		case asTYPEID_UINT32:
			{
				initUIValue(first, property_input_data.input, (std::uint32_t)prop->value.valueInt);
			}
			break;
		case asTYPEID_UINT64:
			{
				initUIValue(first, property_input_data.input, (std::uint64_t)prop->value.valueInt);
			}
			break;
			// float types
		case asTYPEID_FLOAT:
			{
				float v = 0.f;
				if (prop->Retrieve(&v, asTYPEID_FLOAT))
					initUIValue(first, property_input_data.input, v);
			}
			break;
		case asTYPEID_DOUBLE:
			{
				double v = 0.0;
				if (prop->Retrieve(&v, asTYPEID_DOUBLE))
				{
					initUIValue(first, property_input_data.input, v);
				}
			}
			break;
		default:
			{
				if (property_input_data.type_id == m_ScriptEngine->GetStringFactoryReturnTypeId())
				{
					std::string strval = *static_cast<std::string*>(prop->value.valueObj);
					Rocket::Core::String rktStrval(strval.data(), strval.data() + strval.size());
					if (!first && property_input_data.input->GetValue() != rktStrval)
						property_input_data.input->SetValue("");
					else
						property_input_data.input->SetValue(rktStrval);
				}
				else if (property_input_data.type_id == ScriptManager::getSingleton().GetVector2DTypeId())
				{
					FSN_ASSERT(property_input_data.array_index == 0 || property_input_data.array_index == 1);

					auto vecVal = *static_cast<Vector2*>(prop->value.valueObj);
					if (property_input_data.array_index == 0)
						initUIValue(first, property_input_data.input, vecVal.x);
					else
						initUIValue(first, property_input_data.input, vecVal.y);
				}
			}
			break;
		}
	}

	void ASScriptInspector::RefreshPropertyValue(unsigned int index)
	{
		const auto& indexIndex = m_Inputs.get<2>();
		auto range = indexIndex.equal_range(index);
		bool first = true;
		for (auto cit = m_Components.begin(), cend = m_Components.end(); cit != cend; ++cit)
		{
			for (; range.first != range.second; ++range.first)
			{
				RefreshPropertyValue(*range.first, *cit, first);
				first = false;
			}
		}
	}

	void ASScriptInspector::ResetUIValues()
	{
		for (auto it = m_Inputs.get<0>().begin(), end = m_Inputs.get<0>().end(); it != end; ++it)
			it->input->SetValue("");

		bool first = true;
		for (auto cit = m_Components.begin(), cend = m_Components.end(); cit != cend; ++cit)
		{
			const auto& object = *cit;
			unsigned int index = 0;
			for (auto it = m_Inputs.get<0>().begin(), end = m_Inputs.get<0>().end(); it != end; ++it, ++index)
			{
				RefreshPropertyValue(*it, object, first);
			}
			first = false;
		}
	}

	void ASScriptInspector::ProcessEvent(Rocket::Core::Event& ev)
	{
		Rocket::Core::Element::ProcessEvent(ev);
		try
		{
			const bool enterEvent = ev == "enter";
			const bool changeEvent = ev == "change";
			if (enterEvent || changeEvent)
			{
				const auto& controls = m_Inputs.get<1>();
				auto entry = controls.find(
					boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>(dynamic_cast<Rocket::Controls::ElementFormControlInput*>(
					ev.GetTargetElement()
					)));
				if (entry != controls.end())
				{
					if (changeEvent && entry->input->GetAttribute("type", Rocket::Core::String()) != "checkbox")
						return;

					const unsigned int index = entry->index;
					// Check for built-in types
					switch (entry->type_id)
					{
					case asTYPEID_BOOL:
						{
							auto value = entry->input->HasAttribute("checked");
							setValue(index, value, entry->type_id);
						}
						break;
						// Int types
					case asTYPEID_INT8:
						{
							auto value = getUIValue<std::int8_t>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
					case asTYPEID_INT16:
						{
							auto value = getUIValue<std::int16_t>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
					case asTYPEID_INT32:
						{
							auto value = getUIValue<std::int32_t>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
					case asTYPEID_INT64:
						{
							auto value = getUIValue<std::int64_t>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
					// Unsigned types
					case asTYPEID_UINT8:
						{
							auto value = getUIValue<std::uint8_t>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
					case asTYPEID_UINT16:
						{
							auto value = getUIValue<std::uint16_t>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
					case asTYPEID_UINT32:
						{
							auto value = getUIValue<std::uint32_t>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
					case asTYPEID_UINT64:
						{
							auto value = getUIValue<std::uint64_t>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
						// float types
					case asTYPEID_FLOAT:
						{
							auto value = getUIValue<float>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
					case asTYPEID_DOUBLE:
						{
							auto value = getUIValue<double>(entry->input);
							setValue(index, value, entry->type_id);
						}
						break;
					default:
						{
							if (entry->type_id == m_ScriptEngine->GetStringFactoryReturnTypeId())
							{
								auto rocketString = entry->input->GetValue();
								std::string value(rocketString.CString(), rocketString.CString() + rocketString.Length());
								setValue(index, value, entry->type_id);
							}
						}
						break;
					}
				}
			}
		}
		catch (boost::bad_lexical_cast&)
		{
			ResetUIValues();
		}
	}

} }
