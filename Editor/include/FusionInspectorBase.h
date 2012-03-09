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
#include "FusionElementPropertyConnection.h"

#include "FusionInspectorUtils.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>

#include <boost/variant.hpp>
#include <functional>
#include <map>
#include <string>
#include <type_traits>

// TEMP: for hacky circle editor setup
#include "FusionEntity.h"

namespace FusionEngine { namespace Inspectors
{

	//! Generic inspector implementation for a given component type
	template <class ComponentT>
	class GenericInspector : public ComponentInspector
	{
	public:
		//! Constructor
		GenericInspector(const Rocket::Core::String& tag);

		//! Derived class should call AddXInput methods here
		virtual void InitUI() = 0;

	protected:

		/*! \defgroup GenericInspectorSetters Setters
		* Setter functor types
		* \{
		*/
		typedef std::function<void (bool, ComponentIPtr<ComponentT>)> BoolSetter_t;
		typedef std::function<void (int, ComponentIPtr<ComponentT>)> IntSetter_t;
		typedef std::function<void (float, ComponentIPtr<ComponentT>)> FloatSetter_t;
		typedef std::function<void (std::string, ComponentIPtr<ComponentT>)> StringSetter_t;
		/*! \} */

		/*! \defgroup GenericInspectorSetters Getters
		* Getter functor types
		* \{
		*/
		typedef std::function<bool (ComponentIPtr<ComponentT>)> BoolGetter_t;
		typedef std::function<int (ComponentIPtr<ComponentT>)> IntGetter_t;
		typedef std::function<float (ComponentIPtr<ComponentT>)> FloatGetter_t;
		typedef std::function<std::string (ComponentIPtr<ComponentT>)> StringGetter_t;
		/*! \} */

		//! Variant caller for all setter types
		typedef boost::variant<BoolSetter_t, IntSetter_t, FloatSetter_t, StringSetter_t> SetterCallbackVariant_t;
		//! Variant caller for all getter types
		typedef boost::variant<BoolGetter_t, IntGetter_t, FloatGetter_t, StringGetter_t> GetterCallbackVariant_t;

		CircleToolExecutor_t m_CircleToolExecutor;
		RectangleToolExecutor_t m_RectangleToolExecutor;
		PolygonToolExecutor_t m_PolygonToolExecutor;

		//! Adds a text input
		void AddTextInput(const std::string& name, SetterCallbackVariant_t setter, GetterCallbackVariant_t getter, int size = 0);
		//! Adds a toggle (checkbox) input
		void AddToggleInput(const std::string& name, BoolSetter_t setter, BoolGetter_t getter);
		//! Adds a range (slider) input
		void AddRangeInput(const std::string& name, float min, float max, SetterCallbackVariant_t setter, GetterCallbackVariant_t getter);
		//! Adds a select (popup) input
		void AddSelectInput(const std::string& name, const std::vector<std::string>& options, StringSetter_t setter, StringGetter_t getter);
		//! Adds a button input
		void AddButtonInput(const std::string& text, const std::string& value, StringSetter_t setter);
		//! Adds a circle input
		void AddCircleInput(FloatSetter_t x_setter, FloatGetter_t x_getter, FloatSetter_t y_setter, FloatGetter_t y_getter, FloatSetter_t radius_setter, FloatGetter_t radius_getter);
		//! Adds a rectangle input
		void AddRectangleInput(FloatSetter_t x_setter, FloatGetter_t x_getter, FloatSetter_t y_setter, FloatGetter_t y_getter, FloatSetter_t hw_setter, FloatGetter_t hw_getter, FloatSetter_t hh_setter, FloatGetter_t hh_getter, FloatSetter_t angle_setter, FloatGetter_t angle_getter);
		
		//! Updates the UI values from the components
		void ResetUIValues();

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

		void SetCircleToolExecutor(const CircleToolExecutor_t& executor) { m_CircleToolExecutor = executor; }
		void SetRectangleToolExecutor(const RectangleToolExecutor_t& executor) { m_RectangleToolExecutor = executor; }
		void SetPolygonToolExecutor(const PolygonToolExecutor_t& executor) { m_PolygonToolExecutor = executor; }
		

		//! 'Input' callback for the Edit Circle button
		class EditCircleButtonFunctor
		{
		public:
			EditCircleButtonFunctor(GenericInspector<ComponentT>* executor_, FloatSetter_t x_setter_, FloatGetter_t x_getter_, FloatSetter_t y_setter_, FloatGetter_t y_getter_, FloatSetter_t radius_setter_, FloatGetter_t radius_getter_)
				: executor(executor_),
				x_setter(x_setter_), x_getter(x_getter_), y_setter(y_setter_), y_getter(y_getter_), radius_setter(radius_setter_), radius_getter(radius_getter_)
			{
			}

			//EditCircleButtonFunctor(CircleToolExecutor_t executor_, FloatSetter_t x_setter_, FloatGetter_t x_getter_, FloatSetter_t y_setter_, FloatGetter_t y_getter_, FloatSetter_t radius_setter_, FloatGetter_t radius_getter_)
			//	: executor(executor_),
			//	x_setter(x_setter_), x_getter(x_getter_), y_setter(y_setter_), y_getter(y_getter_), radius_setter(radius_setter_), radius_getter(radius_getter_)
			//{
			//}

			void operator() (bool, ComponentIPtr<ComponentT> component)
			{
				auto offset = dynamic_cast<IComponent*>(component.get())->GetParent()->GetPosition();

				const Vector2 c(ToRenderUnits(x_getter(component) + offset.x), ToRenderUnits(y_getter(component) + offset.y));
				const float r = ToRenderUnits(radius_getter(component));
				executor->m_CircleToolExecutor(c, r, [this, component](const Vector2& c, float r)
				{
					auto offset = dynamic_cast<IComponent*>(component.get())->GetParent()->GetPosition();
					x_setter(ToSimUnits(c.x) - offset.x, component);
					y_setter(ToSimUnits(c.y) - offset.y, component);
					radius_setter(ToSimUnits(r), component);
				});
			}

			GenericInspector<ComponentT>* executor;
			//CircleToolExecutor_t executor;
			FloatSetter_t x_setter, y_setter, radius_setter;
			FloatGetter_t x_getter, y_getter, radius_getter;
			Vector2 offset;
		};

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
				if (getter)
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
				else
					return false;
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
				else if (type == "submit")
				{
					callback(true, component);
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
				if (type == "text" || type == "submit")
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
	void GenericInspector<ComponentT>::AddTextInput(const std::string& name, SetterCallbackVariant_t setter, GetterCallbackVariant_t getter, int size)
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
		attributes.Set("value", "");
		if (size <= 0)
			attributes.Set("size", 10);
		else
			attributes.Set("size", size);
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
	void GenericInspector<ComponentT>::AddButtonInput(const std::string& text, const std::string& value, StringSetter_t setter)
	{
		auto line = Rocket::Core::Factory::InstanceElement(this, "p", "p", Rocket::Core::XMLAttributes());
		this->AppendChild(line);

		boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> input_element;

		auto lowerText = fe_newlower(text);

		Rocket::Core::XMLAttributes attributes;
		attributes.Set("type", "submit");
		attributes.Set("name", Rocket::Core::String(lowerText.c_str()));
		attributes.Set("value", Rocket::Core::String(value.c_str()));
		Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(line,
			"input",
			"input",
			attributes);
		
		Rocket::Core::Factory::InstanceElementText(element, text.c_str());

		addControl(line, input_element, element);

		line->RemoveReference();

		if (input_element)
		{
			Input inputData;
			inputData.ui_element = input_element;
			inputData.callback = setter;
			m_Inputs[input_element] = inputData;
		}
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::AddCircleInput(FloatSetter_t x_setter, FloatGetter_t x_getter, FloatSetter_t y_setter, FloatGetter_t y_getter, FloatSetter_t radius_setter, FloatGetter_t radius_getter)
	{
		using namespace Rocket::Core;
		using namespace Rocket::Controls;

		auto line = Factory::InstanceElement(this, "p", "p", Rocket::Core::XMLAttributes());
		this->AppendChild(line);

		boost::intrusive_ptr<ElementFormControlInput> input_element;

		Factory::InstanceElementText(line, "circle");

		auto addComponentInput = [&](const FloatSetter_t& setter, const FloatGetter_t& getter)
		{
			Rocket::Core::XMLAttributes attributes;
			//attributes.Set("class", "circle_input");
			attributes.Set("type", "text");
			attributes.Set("size", 10);
			Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(line,
				"input",
				"input",
				attributes);

			addControl(line, input_element, element);

			if (input_element)
			{
				Input inputData;
				inputData.ui_element = input_element;
				inputData.callback = setter;
				inputData.get_callback = getter;
				m_Inputs[input_element] = inputData;
			}
		};
		addComponentInput(x_setter, x_getter);
		addComponentInput(y_setter, y_getter);
		addComponentInput(radius_setter, radius_getter);

		{
			XMLAttributes attributes;
			//attributes.Set("class", "circle_input");
			attributes.Set("type", "submit");
			Element* element = Factory::InstanceElement(line,
				"input",
				"input",
				attributes);
			element->SetId("circle_editor");
			Factory::InstanceElementText(element, "E");

			addControl(line, input_element, element);

			if (input_element)
			{
				Input inputData;
				inputData.ui_element = input_element;
				inputData.callback = BoolSetter_t(EditCircleButtonFunctor(this, x_setter, x_getter, y_setter, y_getter, radius_setter, radius_getter));
				m_Inputs[input_element] = inputData;
			}
		}

		line->RemoveReference();
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::AddRectangleInput(FloatSetter_t x_setter, FloatGetter_t x_getter, FloatSetter_t y_setter, FloatGetter_t y_getter, FloatSetter_t hw_setter, FloatGetter_t hw_getter, FloatSetter_t hh_setter, FloatGetter_t hh_getter, FloatSetter_t angle_setter, FloatGetter_t angle_getter)
	{
		using namespace Rocket::Core;
		using namespace Rocket::Controls;

		auto line = Factory::InstanceElement(this, "p", "p", Rocket::Core::XMLAttributes());
		this->AppendChild(line);

		boost::intrusive_ptr<ElementFormControlInput> input_element;

		Factory::InstanceElementText(line, "rectangle");

		auto addComponentInput = [&](const FloatSetter_t& setter, const FloatGetter_t& getter)
		{
			Rocket::Core::XMLAttributes attributes;
			//attributes.Set("class", "circle_input");
			attributes.Set("type", "text");
			attributes.Set("size", 10);
			Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(line,
				"input",
				"input",
				attributes);

			addControl(line, input_element, element);

			if (input_element)
			{
				Input inputData;
				inputData.ui_element = input_element;
				inputData.callback = setter;
				inputData.get_callback = getter;
				m_Inputs[input_element] = inputData;
			}
		};
		addComponentInput(x_setter, x_getter);
		addComponentInput(y_setter, y_getter);
		addComponentInput(hw_setter, hw_getter);
		addComponentInput(hh_setter, hh_getter);
		addComponentInput(angle_setter, angle_getter);

		{
			XMLAttributes attributes;
			//attributes.Set("class", "circle_input");
			attributes.Set("type", "submit");
			Element* element = Factory::InstanceElement(line,
				"input",
				"input",
				attributes);
			element->SetId("rectangle_editor");
			Factory::InstanceElementText(element, "E");

			addControl(line, input_element, element);

			if (input_element)
			{
				Input inputData;
				inputData.ui_element = input_element;
				inputData.callback = BoolSetter_t(EditCircleButtonFunctor(this, x_setter, x_getter, y_setter, y_getter, radius_setter, radius_getter));
				m_Inputs[input_element] = inputData;
			}
		}

		line->RemoveReference();
	}

	template <class ComponentT>
	void GenericInspector<ComponentT>::ProcessEvent(Rocket::Core::Event& ev)
	{
		Rocket::Core::Element::ProcessEvent(ev);
		try
		{
			const bool isSelectElem =
				dynamic_cast<Rocket::Controls::ElementFormControlSelect*>(ev.GetTargetElement()) ||
				dynamic_cast<Rocket::Controls::ElementFormControlDataSelect*>(ev.GetTargetElement());
			auto inputElem = dynamic_cast<Rocket::Controls::ElementFormControlInput*>(ev.GetTargetElement());
			const bool isCheckboxElem = inputElem && inputElem->GetAttribute("type", Rocket::Core::String()) == "checkbox";
			const bool isButton = ev.GetTargetElement()->GetTagName() == "button" || (inputElem && inputElem->GetAttribute("type", Rocket::Core::String()) == "submit");
			if (ev == "enter" || ((isSelectElem || isCheckboxElem) && ev == "change") || (isButton && ev == "click"))
			{
				auto entry = m_Inputs.find(boost::intrusive_ptr<Rocket::Core::Element>(ev.GetTargetElement()));
				if (entry != m_Inputs.end())
				{
					auto& inputData = entry->second;
					for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
						boost::apply_visitor(GetUIValueVisitor(*it), inputData.ui_element, inputData.callback);

					if (ev == "enter")
						entry->first->Blur();
				}
			}
			//else if (ev.GetTargetElement()->IsClassSet("circle_input") && ev == "focus")
			//{
			//	RequestCircleInput(ev.GetTargetElement()->GetAttribute("name", ""));
			//}
			else if (ev == "dragdrop")
			{
				Rocket::Core::Element* dest_container = ev.GetTargetElement();
				Rocket::Core::Element* drag_element = *static_cast<Rocket::Core::Element**>(ev.GetParameter<void*>("drag_element", NULL));

				if (drag_element)
				{
					auto entry = m_Inputs.find(boost::intrusive_ptr<Rocket::Core::Element>(dest_container));
					if (entry != m_Inputs.end())
					{
						{
							Rocket::Core::ElementList fnameInfoElems;
							drag_element->GetElementsByTagName(fnameInfoElems, "fileinfo");
							if (!fnameInfoElems.empty())
							{
								auto finfo = fnameInfoElems.front();
								auto textval = finfo->GetAttribute("path", Rocket::Core::String());
								if (!textval.Empty())
								{
									std::string tvstr(textval.CString(), textval.CString() + textval.Length());
									if (auto* cb = boost::get<StringSetter_t>(&entry->second.callback))
									{
										for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
											(*cb)(tvstr, *it);
										SetUIValueVisitor v(true, m_Components.front());
										GetterCallbackVariant_t fgetter = StringGetter_t([tvstr](ComponentIPtr<ComponentT>)->std::string { return tvstr; });
										boost::apply_visitor(v, entry->second.ui_element, fgetter);
									}
								}
							}
						}
						{
							Rocket::Core::ElementList propertyLinkElems;
							drag_element->GetElementsByTagName(propertyLinkElems, "proplink");
							if (!propertyLinkElems.empty())
							{
								auto elem = propertyLinkElems.front();
								if (auto linkInfo = dynamic_cast<ElementPropertyConnection*>(elem))
								{
								}
							}
						}
					}
				}
			}
		}
		catch (boost::bad_lexical_cast&)
		{
			ResetUIValues();
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
