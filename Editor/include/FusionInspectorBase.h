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

#ifndef H_FusionInspectorBase
#define H_FusionInspectorBase

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentInspector.h"

#include "FusionInspectorUtils.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>

#include <boost/variant.hpp>
#include <functional>
#include <map>
#include <string>
#include <type_traits>

namespace FusionEngine { namespace Inspectors
{

	/*class InspectorBaseImpl : public ComponentInspector
	{
	public:
		InspectorBaseImpl(const Rocket::Core::String& tag);

	private:
		typedef boost::variant<boost::intrusive_ptr<Rocket::Core::Element>> InputVariant_t;
		std::set<InputVariant_t> m_Inputs;

		void AddTextInput(const std::string& name);

		void InitUI();

		void ProcessEvent(Rocket::Core::Event& ev);
	};*/

	template <class ComponentT>
	class GenericInspector : public ComponentInspector
	{
	public:
		GenericInspector(const Rocket::Core::String& tag);

		virtual void InitUI() = 0;

	protected:

		typedef std::function<void (bool, ComponentIPtr<ComponentT>)> BoolSetter_t;
		typedef std::function<void (int, ComponentIPtr<ComponentT>)> IntSetter_t;
		typedef std::function<void (float, ComponentIPtr<ComponentT>)> FloatSetter_t;
		typedef std::function<void (std::string, ComponentIPtr<ComponentT>)> StringSetter_t;

		typedef std::function<bool (ComponentIPtr<ComponentT>)> BoolGetter_t;
		typedef std::function<int (ComponentIPtr<ComponentT>)> IntGetter_t;
		typedef std::function<float (ComponentIPtr<ComponentT>)> FloatGetter_t;
		typedef std::function<std::string (ComponentIPtr<ComponentT>)> StringGetter_t;

		typedef boost::variant<BoolSetter_t, IntSetter_t, FloatSetter_t, StringSetter_t> SetterCallbackVariant_t;
		typedef boost::variant<BoolGetter_t, IntGetter_t, FloatGetter_t, StringGetter_t> GetterCallbackVariant_t;

		void AddTextInput(const std::string& name, SetterCallbackVariant_t setter, GetterCallbackVariant_t getter);

		void AddToggleInput(const std::string& name, BoolSetter_t setter, BoolGetter_t getter);

		void AddRangeInput(const std::string& name, float min, float max, SetterCallbackVariant_t setter, GetterCallbackVariant_t getter);

		void AddSelectInput(const std::string& name, const std::vector<std::string>& options, StringSetter_t setter, StringGetter_t getter);

	private:
		typedef boost::variant<
			boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>,
			boost::intrusive_ptr<Rocket::Controls::ElementFormControlSelect>
		> InputVariant_t;

		struct Input
		{
			InputVariant_t ui_element;
			SetterCallbackVariant_t callback;
			GetterCallbackVariant_t get_callback;
		};

		std::map<boost::intrusive_ptr<Rocket::Core::Element>, Input> m_Inputs;

		std::vector<ComponentIPtr<ComponentT>> m_Components;

		void SetComponents(const std::vector<ComponentPtr>& components);
		void ReleaseComponents();

		void ResetUIValues();

		class SetUIValueVisitor : public boost::static_visitor<bool>
		{
		public:
			bool first;
			ComponentIPtr<ComponentT> component;
			SetUIValueVisitor(bool first_, const ComponentIPtr<ComponentT>& com_)
				: first(first_), component(com_)
			{}
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, BoolGetter_t& getter) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "checkbox")
				{
					const bool value = getter(component);
					if (first)
					{
						if (value)
							input->SetAttribute("checked", "");
						else
							input->RemoveAttribute("checked");
					}
					else
					{
						if (input->HasAttribute("checked") != value)
							input->RemoveAttribute("checked");
					}

					return true;
				}
				else
				{
					return false;
				}
			}

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, IntGetter_t& getter) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "text" || type == "range")
				{
					initUIValue(first, input, getter(component));
					return true;
				}
				else
				{
					return false;
				}
			}
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, FloatGetter_t& getter) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "text" || type == "range")
				{
					initUIValue(first, input, getter(component));
					return true;
				}
				else
				{
					return false;
				}
			}

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, StringGetter_t& getter) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				std::string currentUIValue;
				std::string value = getter(component);
				if (type == "text")
				{
					currentUIValue = input->GetValue().CString();
				}
				else if (type == "checkbox")
				{
					currentUIValue = input->HasAttribute("checked") ? "true" : "false";
				}
				else
				{
					return false;
				}

				if (!first && value != currentUIValue)
					input->SetValue("");
				else
					input->SetValue(value.c_str());
				return true;
			}

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlSelect>& input, StringGetter_t& getter) const
			{
				std::string currentUIValue = input->GetValue().CString();
				std::string value = getter(component);

				if (!first && value != currentUIValue)
					input->SetValue("");
				else
					input->SetValue(value.c_str());

				return true;
			}

			template <typename U>
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlSelect>& input, std::function<U (ComponentIPtr<ComponentT>)>& callback) const
			{
				return false;
			}

		};

		class GetUIValueVisitor : public boost::static_visitor<bool>
		{
		public:
			ComponentIPtr<ComponentT> component;
			GetUIValueVisitor(const ComponentIPtr<ComponentT>& com_)
				: component(com_)
			{}
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, BoolSetter_t& callback) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "checkbox")
				{
					callback(input->HasAttribute("checked"), component);
					return true;
				}
				else
				{
					return false;
				}
			}

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, IntSetter_t& callback) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "text" || type == "range")
				{
					callback(getUIValue<int>(input), component);
					return true;
				}
				else
				{
					return false;
				}
			}
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, FloatSetter_t& callback) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "text" || type == "range")
				{
					callback(getUIValue<float>(input), component);
					return true;
				}
				else
				{
					return false;
				}
			}

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, StringSetter_t& callback) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "text")
				{
					auto uiValue = input->GetValue();
					callback(std::string(uiValue.CString()), component);
					return true;
				}
				else
				{
					return false;
				}
			}

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlSelect>& input, StringSetter_t& callback) const
			{
				auto uiValue = input->GetValue();
				callback(std::string(uiValue.CString()), component);
				return true;
			}

			template <typename U>
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlSelect>& input, std::function<void (U, ComponentIPtr<ComponentT>)>& callback) const
			{
				return false;
			}

		};

		void ProcessEvent(Rocket::Core::Event& ev);
	};

	template <class ComponentT>
	GenericInspector<ComponentT>::GenericInspector(const Rocket::Core::String& tag)
		: ComponentInspector(tag)
	{
		//InitUI();
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::AddTextInput(const std::string& name, SetterCallbackVariant_t setter, GetterCallbackVariant_t getter)
	{
		auto line = Rocket::Core::Factory::InstanceElement(this, "p", "p", Rocket::Core::XMLAttributes());
		this->AppendChild(line);

		boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> input_element;

		auto lowerName = fe_newlower(name);

		Rocket::Core::Factory::InstanceElementText(line, name.c_str());

		Rocket::Core::XMLAttributes attributes;
		attributes.Set("type", "text");
		attributes.Set("id", Rocket::Core::String((lowerName + "_input").c_str()));
		attributes.Set("name", Rocket::Core::String(lowerName.c_str()));
		attributes.Set("value", "0");
		attributes.Set("size", "10");
		Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(line,
			"input",
			"input",
			attributes);

		addControl(line, input_element, element);

		line->RemoveReference();

		if (input_element)
		{
			Input inputData;
			inputData.ui_element = input_element;
			inputData.callback = setter;
			inputData.get_callback = getter;
			m_Inputs[input_element] = inputData;
		}
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::AddToggleInput(const std::string& name, BoolSetter_t setter, BoolGetter_t getter)
	{
		auto line = Rocket::Core::Factory::InstanceElement(this, "p", "p", Rocket::Core::XMLAttributes());
		this->AppendChild(line);

		boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> input_element;

		auto lowerName = fe_newlower(name);

		Rocket::Core::Factory::InstanceElementText(line, name.c_str());

		Rocket::Core::XMLAttributes attributes;
		attributes.Set("type", "checkbox");
		attributes.Set("id", Rocket::Core::String((lowerName + "_input").c_str()));
		attributes.Set("name", Rocket::Core::String(lowerName.c_str()));
		attributes.Set("value", Rocket::Core::String(lowerName.c_str()));
		Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(line,
			"input",
			"input",
			attributes);

		addControl(line, input_element, element);

		line->RemoveReference();

		if (input_element)
		{
			Input inputData;
			inputData.ui_element = input_element;
			inputData.callback = setter;
			inputData.get_callback = getter;
			m_Inputs[input_element] = inputData;
		}
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::AddRangeInput(const std::string& name, float min, float max, SetterCallbackVariant_t setter, GetterCallbackVariant_t getter)
	{
		auto line = Rocket::Core::Factory::InstanceElement(this, "p", "p", Rocket::Core::XMLAttributes());
		this->AppendChild(line);

		boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> input_element;

		auto lowerName = fe_newlower(name);

		Rocket::Core::Factory::InstanceElementText(line, name.c_str());

		Rocket::Core::XMLAttributes attributes;
		attributes.Set("type", "range");
		attributes.Set("id", Rocket::Core::String((lowerName + "_input").c_str()));
		attributes.Set("name", Rocket::Core::String(lowerName.c_str()));
		attributes.Set("min", min);
		attributes.Set("max", max);
		attributes.Set("value", min);
		Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(line,
			"input",
			"input",
			attributes);

		addControl(line, input_element, element);

		line->RemoveReference();

		if (input_element)
		{
			Input inputData;
			inputData.ui_element = input_element;
			inputData.callback = setter;
			inputData.get_callback = getter;
			m_Inputs[input_element] = inputData;
		}
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::AddSelectInput(const std::string& name, const std::vector<std::string>& options, StringSetter_t setter, StringGetter_t getter)
	{
		auto line = Rocket::Core::Factory::InstanceElement(this, "p", "p", Rocket::Core::XMLAttributes());
		this->AppendChild(line);

		boost::intrusive_ptr<Rocket::Controls::ElementFormControlSelect> select_element;

		auto lowerName = fe_newlower(name);

		Rocket::Core::Factory::InstanceElementText(line, name.c_str());

		Rocket::Core::XMLAttributes attributes;
		Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(line,
			"select",
			"select",
			attributes);

		addControl(line, select_element, element);

		for (auto it = options.begin(), end = options.end(); it != end; ++it)
		{
			Rocket::Core::String rktStr(it->data(), it->data() + it->length());
			select_element->Add(rktStr, rktStr);
		}

		line->RemoveReference();

		if (select_element)
		{
			Input inputData;
			inputData.ui_element = select_element;
			inputData.callback = setter;
			inputData.get_callback = getter;
			m_Inputs[select_element] = inputData;
		}
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::ProcessEvent(Rocket::Core::Event& ev)
	{
		try
		{
			const bool isSelectElem =
				dynamic_cast<Rocket::Controls::ElementFormControlSelect*>(ev.GetTargetElement()) ||
				dynamic_cast<Rocket::Controls::ElementFormControlDataSelect*>(ev.GetTargetElement());
			if (ev == "enter" || (isSelectElem && ev == "change"))
			{
				auto entry = m_Inputs.find(boost::intrusive_ptr<Rocket::Core::Element>(ev.GetTargetElement()));
				if (entry != m_Inputs.end())
				{
					auto& inputData = entry->second;
					for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
						boost::apply_visitor(GetUIValueVisitor(*it), inputData.ui_element, inputData.callback);
				}
			}
		}
		catch (boost::bad_lexical_cast&)
		{
			InitUI();
		}
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::SetComponents(const std::vector<ComponentPtr>& components)
	{
		for (auto it = components.begin(), end = components.end(); it != end; ++it)
		{
			if (auto typed = ComponentIPtr<ComponentT>(*it))
				m_Components.push_back(typed);
		}

		ResetUIValues();
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::ReleaseComponents()
	{
		m_Components.clear();
	}

	class ClearUIVisitor : public boost::static_visitor<>
	{
	public:
		void operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& element) const
		{
			auto type = element->GetAttribute("type", Rocket::Core::String(""));
			if (type == "text")
			{
				element->SetValue("");
			}
			else if (type == "checkbox")
			{
				element->RemoveAttribute("checked");
			}
		}
		void operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlSelect>& element) const
		{
			element->SetValue("");
		}
		//void operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControl>& element) const
		//{
		//	element->SetValue("");
		//}

	};

	template <class ComponentT>
	void GenericInspector<ComponentT>::ResetUIValues()
	{
		for (auto it = m_Inputs.begin(), end = m_Inputs.end(); it != end; ++it)
			boost::apply_visitor(ClearUIVisitor(), it->second.ui_element);

		bool first = true;
		for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
		{
			SetUIValueVisitor visitor(first, *it);
			for (auto inputIt = m_Inputs.begin(), inputEnd = m_Inputs.end(); inputIt != inputEnd; ++inputIt)
			{
				Input& inputData = inputIt->second;
				boost::apply_visitor(visitor, inputData.ui_element, inputData.get_callback);
			}

			first = false;
		}
	}

} }

#endif