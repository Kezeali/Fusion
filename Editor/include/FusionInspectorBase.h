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

		typedef std::function<void (bool, ComponentIPtr<ComponentT>)> BoolInputCallback_t;
		typedef std::function<void (int, ComponentIPtr<ComponentT>)> IntInputCallback_t;
		typedef std::function<void (float, ComponentIPtr<ComponentT>)> FloatInputCallback_t;
		typedef std::function<void (std::string, ComponentIPtr<ComponentT>)> StringInputCallback_t;

		typedef std::function<bool (ComponentIPtr<ComponentT>)> BoolPropertyCallback_t;
		typedef std::function<int (ComponentIPtr<ComponentT>)> IntPropertyCallback_t;
		typedef std::function<float (ComponentIPtr<ComponentT>)> FloatPropertyCallback_t;
		typedef std::function<std::string (ComponentIPtr<ComponentT>)> StringPropertyCallback_t;

		typedef boost::variant<BoolInputCallback_t, IntInputCallback_t, FloatInputCallback_t, StringInputCallback_t> InputCallbackVariant_t;
		typedef boost::variant<BoolPropertyCallback_t, IntPropertyCallback_t, FloatPropertyCallback_t, StringPropertyCallback_t> PropertyCallbackVariant_t;

		void AddTextInput(const std::string& name, InputCallbackVariant_t setter, PropertyCallbackVariant_t getter);

	private:
		typedef boost::variant<boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>> InputVariant_t;

		struct Input
		{
			InputVariant_t ui_element;
			InputCallbackVariant_t callback;
			PropertyCallbackVariant_t get_callback;
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
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, BoolPropertyCallback_t& callback) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "checkbox")
				{
					const bool value = callback(component);
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

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, std::function<int (ComponentIPtr<ComponentT>)>& callback) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "text" || type == "range")
				{
					initUIValue(first, input, callback(component));
					return true;
				}
				else
				{
					return false;
				}
			}
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, std::function<float (ComponentIPtr<ComponentT>)>& callback) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "text" || type == "range")
				{
					initUIValue(first, input, callback(component));
					return true;
				}
				else
				{
					return false;
				}
			}

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, StringPropertyCallback_t& callback) const
			{
				auto type = input->GetAttribute("type", Rocket::Core::String(""));
				if (type == "text")
				{
					std::string currentValue = input->GetValue().CString();
					std::string value = callback(component);
					if (!first && value != currentValue)
						input->SetValue("");
					else
						input->SetValue(value.c_str());
					return true;
				}
				else
				{
					return false;
				}
			}

		};

		class GetUIValueVisitor : public boost::static_visitor<bool>
		{
		public:
			ComponentIPtr<ComponentT> component;
			GetUIValueVisitor(const ComponentIPtr<ComponentT>& com_)
				: component(com_)
			{}
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, BoolInputCallback_t& callback) const
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

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, std::function<void (int, ComponentIPtr<ComponentT>)>& callback) const
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
			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, std::function<void (float, ComponentIPtr<ComponentT>)>& callback) const
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

			bool operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, StringInputCallback_t& callback) const
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
	void GenericInspector<ComponentT>::AddTextInput(const std::string& name, InputCallbackVariant_t setter, PropertyCallbackVariant_t getter)
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
		Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(line,
			"input",
			"input",
			attributes);

		addControl(line, input_element, text_input);

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
	void GenericInspector<ComponentT>::ProcessEvent(Rocket::Core::Event& ev)
	{
		try
		{
			if (ev == "enter")
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
		void operator()(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& i) const
		{
			auto type = i->GetAttribute("type", Rocket::Core::String(""));
			if (type == "text")
			{
				i->SetValue("");
			}
			else if (type == "checkbox")
			{
				i->RemoveAttribute("checked");
			}
		}

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