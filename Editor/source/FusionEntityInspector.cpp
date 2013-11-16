/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#include "FusionEntityInspector.h"

#include "FusionEntity.h"

#include "FusionArchetypalEntityManager.h"

#include <boost/lexical_cast.hpp>

namespace FusionEngine { namespace Inspectors
{

	GWEN_CONTROL_CONSTRUCTOR(ElementEntityInspector)
	{
		InitUI();
	}

	void ElementEntityInspector::SetEntity(const EntityPtr& entity)
	{
		m_Entity = entity;

		if (m_Entity)
			ResetUIValues();
	}

	void ElementEntityInspector::InitUI()
	{
		/*using namespace std::placeholders;

		{
			InlineSection line(this);

			AddTextInput("Name", std::bind(&Entity::SetName, _2, _1), std::bind(&Entity::GetName, _1), 20);

			AddTextInput("Owner",
				[](const EntityPtr& entity)->std::string
			{
				try
				{
					return boost::lexical_cast<std::string>(entity->GetOwnerID());
				}
				catch (boost::bad_lexical_cast&){}
				return "";
			});

			AddTextInput("Authority",
				[](const EntityPtr& entity)->std::string
			{
				try
				{
					return boost::lexical_cast<std::string>(entity->GetAuthority());
				}
				catch (boost::bad_lexical_cast&){}
				return "";
			});
		}

		AddToggleInput("Streamed",
			[](bool streamed, const EntityPtr& entity)
		{
		},
			[](const EntityPtr& entity)
		{
			return entity->GetDomain() != SYSTEM_DOMAIN;
		});

		AddTextInput("Cell",
			[](const EntityPtr& entity)->std::string
		{
			try
			{
				return boost::lexical_cast<std::string>(entity->GetStreamingCellIndex().x) + ", " +
						boost::lexical_cast<std::string>(entity->GetStreamingCellIndex().y);
			}
			catch (boost::bad_lexical_cast&){}
			return "error";
		});

		AddToggleInput("Terrain",
			[](bool value, const EntityPtr& entity)
		{
			entity->SetTerrain(value);
		},
			[](const EntityPtr& entity)
		{
			return entity->IsTerrain();
		});

		AddTextInput("Archetype",
			[](const EntityPtr& entity)->std::string
		{
			if (entity->IsArchetypal())
				return "Instance: " + entity->GetArchetype();
			else if (entity->IsArchetype())
				return "Definition: " + entity->GetArchetype();
			else
				return "No";
		});

		{
			Rocket::Core::XMLAttributes attributes;
			attributes.Set("type", "submit");
			attributes.Set("name", Rocket::Core::String("apply"));
			attributes.Set("value", Rocket::Core::String(""));
			Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(this,
				"input",
				"input",
				attributes);

			Rocket::Core::Factory::InstanceElementText(element, "Apply To Instances");

			addControl(this, apply_button, element);
		}*/
	}

	void ElementEntityInspector::AddTextInput(const std::string& name, StringSetter_t setter, StringGetter_t getter, int size)
	{
		//auto parent = !m_InlineSections.empty() ? m_InlineSections.back() : boost::intrusive_ptr<Rocket::Core::Element>(this);

		//// Make sure title and textbox are on the same line
		//boost::intrusive_ptr<Rocket::Core::Element> block =
		//	Rocket::Core::Factory::InstanceElement(parent.get(), "line", "line", Rocket::Core::XMLAttributes());
		//block->RemoveReference();
		//parent->AppendChild(block.get());
		//

		//boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> input_element;

		//auto lowerName = fe_newlower(name);

		//Rocket::Core::Factory::InstanceElementText(block.get(), name.c_str());

		//Rocket::Core::XMLAttributes attributes;
		//attributes.Set("type", "text");
		//attributes.Set("id", Rocket::Core::String((lowerName + "_input").c_str()));
		//attributes.Set("name", Rocket::Core::String(lowerName.c_str()));
		//attributes.Set("enter_event", true);
		//attributes.Set("value", "");
		//if (size <= 0)
		//	attributes.Set("size", 10);
		//else
		//	attributes.Set("size", size);
		//if(!setter)
		//	attributes.Set("readonly", "true");
		//Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(block.get(),
		//	"input",
		//	"input",
		//	attributes);

		//addControl(block.get(), input_element, element);


		//if (input_element)
		//{
		//	Input inputData;
		//	inputData.ui_element = input_element;
		//	inputData.publishToEntity = [setter, input_element](const EntityPtr& entity) { setter(input_element->GetValue().CString(), entity); };
		//	inputData.receiveFromEntity = [getter, input_element](const EntityPtr& entity) { input_element->SetValue(getter(entity).c_str()); };
		//	m_Inputs[input_element] = inputData;
		//}
	}

	void ElementEntityInspector::AddTextInput(const std::string& name, StringGetter_t getter, int size)
	{
		AddTextInput(name, StringSetter_t(), getter, size);
	}

	/*namespace {
		bool SetCheckbox(const boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, bool value, bool first = true)
		{
			auto type = input->GetAttribute("type", Rocket::Core::String(""));
			if (type == "checkbox")
			{
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
						input->SetAttribute("checked", "some");
				}

				return true;
			}
			else
			{
				return false;
			}
		}

		bool GetCheckbox(const boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input)
		{
			auto type = input->GetAttribute("type", Rocket::Core::String(""));
			if (type == "checkbox")
			{
				return input->HasAttribute("checked");
			}
			else
			{
				return false;
			}
		}
	}*/

	void ElementEntityInspector::AddToggleInput(const std::string& name, BoolSetter_t setter, BoolGetter_t getter)
	{
		//auto line = Rocket::Core::Factory::InstanceElement(this, "p", "p", Rocket::Core::XMLAttributes());
		//this->AppendChild(line);

		//boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> input_element;

		//auto lowerName = fe_newlower(name);

		//Rocket::Core::Factory::InstanceElementText(line, name.c_str());

		//Rocket::Core::XMLAttributes attributes;
		//attributes.Set("type", "checkbox");
		//attributes.Set("id", Rocket::Core::String((lowerName + "_input").c_str()));
		//attributes.Set("name", Rocket::Core::String(lowerName.c_str()));
		//attributes.Set("value", Rocket::Core::String(lowerName.c_str()));
		//Rocket::Core::Element* element = Rocket::Core::Factory::InstanceElement(line,
		//	"input",
		//	"input",
		//	attributes);

		//addControl(line, input_element, element);

		//line->RemoveReference();

		//if (input_element)
		//{
		//	Input inputData;
		//	inputData.ui_element = input_element;
		//	inputData.publishToEntity = [setter, input_element](const EntityPtr& entity) { setter(GetCheckbox(input_element), entity); };
		//	inputData.receiveFromEntity = [getter, input_element](const EntityPtr& entity) { SetCheckbox(input_element, getter(entity)); };
		//	m_Inputs[input_element] = inputData;
		//}
	}

	//void ClearUIValue(boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& element)
	//{
	//	auto type = element->GetAttribute("type", Rocket::Core::String(""));
	//	if (type == "text")
	//	{
	//		element->SetValue("");
	//	}
	//	else if (type == "checkbox")
	//	{
	//		element->RemoveAttribute("checked");
	//	}
	//}

	//class SetUIValueVisitor
	//{
	//public:
	//	SetUIValueVisitor(const EntityPtr& entity_)
	//		: entity(entity_)
	//	{}
	//	EntityPtr entity;
	//	bool operator() (boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input, ElementEntityInspector::StringGetter_t& getter) const
	//	{
	//		auto type = input->GetAttribute("type", Rocket::Core::String(""));
	//		if (type == "text")
	//		{
	//			std::string value = getter(entity);
	//			input->SetValue(value.c_str());
	//			return true;
	//		}
	//		else
	//			return false;
	//	}
	//};

	void ElementEntityInspector::ResetUIValues()
	{
		//for (auto it = m_Inputs.begin(), end = m_Inputs.end(); it != end; ++it)
		//	ClearUIValue(it->second.ui_element);

		//for (auto inputIt = m_Inputs.begin(), inputEnd = m_Inputs.end(); inputIt != inputEnd; ++inputIt)
		//{
		//	inputIt->second.receiveFromEntity(m_Entity);
		//}
	}

	//void ElementEntityInspector::ProcessEvent(Rocket::Core::Event& ev)
	//{
	//	Rocket::Core::Element::ProcessEvent(ev);
	//	if (ev.GetTargetElement() == apply_button.get() && ev == "mouseup")
	//	{
	//		if (auto agent = m_Entity->GetArchetypeDefinitionAgent())
	//		{
	//			SendToConsole("Pushing archetype config");
	//			agent->PushState();
	//		}
	//	}
	//	try
	//	{
	//		const bool isSelectElem =
	//			dynamic_cast<Rocket::Controls::ElementFormControlSelect*>(ev.GetTargetElement()) ||
	//			dynamic_cast<Rocket::Controls::ElementFormControlDataSelect*>(ev.GetTargetElement());
	//		if (ev == "enter" || (isSelectElem && ev == "change"))
	//		{
	//			auto entry = m_Inputs.find(boost::intrusive_ptr<Rocket::Core::Element>(ev.GetTargetElement()));
	//			if (entry != m_Inputs.end())
	//			{
	//				auto& inputData = entry->second;
	//				if (inputData.publishToEntity)
	//					inputData.publishToEntity(m_Entity);

	//				if (ev == "enter")
	//					entry->first->Blur();
	//			}
	//		}
	//	}
	//	catch (boost::bad_lexical_cast&)
	//	{
	//		ResetUIValues();
	//	}
	//}

} }
