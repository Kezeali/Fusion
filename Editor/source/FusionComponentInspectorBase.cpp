/*
*  Copyright (c) 2013 Fusion Project Team
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

#include "FusionComponentInspectorBase.h"

#include <Gwen/Controls/Property/Checkbox.h>

#include <boost/lexical_cast.hpp>

using namespace Gwen;

namespace FusionEngine { namespace Inspectors
{

	void ComponentInspectorBase::SetComponents(const std::vector<ComponentPtr>& components)
	{
		for (auto component : components)
		{
			// List the property
			m_Components.push_back(component);

			// Attach to properties to auto-refresh the UI
			auto& properties = component->GetProperties();
			for (size_t propertyIndex = 0; propertyIndex < properties.size(); ++propertyIndex)
			{
				const auto& propertyName = properties[propertyIndex].first;
				const auto& componentProperty = properties[propertyIndex].second;

				PropertySubscriptionData* propData = nullptr;
				auto entry = m_Properties.find(propertyName);
				if (entry == m_Properties.end())
				{
					propData = &m_Properties[propertyName];
					propData->propertyIndex = propertyIndex;
					auto typeId = componentProperty->GetImpl()->GetTypeId();
					if (typeId == Scripting::RegisteredAppType<std::string>::type_id)
					{
						propData->propertyRow = m_PropertiesControl->Add(propertyName);
					}
					else if (typeId == Scripting::RegisteredAppType<int>::type_id)
					{
						propData->propertyRow = m_PropertiesControl->Add(propertyName);
						propData->update = [](ComponentPtr com, size_t index, PropertyRowPtr row)
						{
							const auto& properties = com->GetProperties();
							if (properties.size() > index)
							{
								const auto& prop = properties[index].second;
								auto valueRef = prop->Get();
								auto value = *((int*)valueRef);
								row->GetProperty()->SetPropertyValue(boost::lexical_cast<std::string>(value));
							}
						};
					}
				}
				//if (propData->refreshAll)
				//{
				//	entry->second.connections.push_back(EvesdroppingManager::getSingleton().GetSignalingSystem().AddListener(it->second->GetID(), std::bind(&ComponentInspectorBase::ResetUIValues, this)));
				//}
				//else
				{
					std::string propName = propertyName;
					auto cback = [this, propName]() { this->RefreshUIForProp(propName); };
					propData->connections.push_back(EvesdroppingManager::getSingleton().GetSignalingSystem().AddListener(componentProperty->GetID(), cback));
				}
			}
		}

		ResetUIValues();
	}

	void ComponentInspectorBase::RefreshUIForProp(const std::string& name)
	{
		m_ResettingUI_DontApplyChangesToArchetypeInstance_YouDope = true;

		auto entry = m_Properties.find(name);
		if (entry != m_Properties.end())
		{
			auto& propertyRow = entry->second.propertyRow;

			bool first = true;
			for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
			{
				entry->second.update(*it, entry->second.propertyIndex, propertyRow);
				first = false;
			}
		}

		m_ResettingUI_DontApplyChangesToArchetypeInstance_YouDope = false;
	}

	void ComponentInspectorBase::ResetUIValues()
	{
		m_ResettingUI_DontApplyChangesToArchetypeInstance_YouDope = true;

		for (auto entry : m_Properties)
		{
			auto& propertyRow = entry.second.propertyRow;

			bool first = true;
			for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
			{
				entry.second.update(*it, entry.second.propertyIndex, propertyRow);
				first = false;
			}
		}

		m_ResettingUI_DontApplyChangesToArchetypeInstance_YouDope = false;
	}

	/*void ComponentInspectorBase::AddTextProperty(const std::string& label,
			std::function<std::string (ComponentPtr)> get,
			std::function<void (ComponentPtr, std::string)> set)
	{
			Controls::Properties* props = new Controls::Properties(this);
			props->SetBounds(10, 10, 150, 300);
			auto row = props->Add(label, new Controls::Property::Text(props));
			row->onChange.Add(this, [this, get, set](Controls::Base* control)
			{
				Controls::PropertyRow* row = (Controls::PropertyRow*)control;
				Controls::Property::Text* textbox = dynamic_cast<Controls::Property::Text*>(row->GetProperty());
				auto value = textbox->GetPropertyValue();
				for (auto component : m_Components)
				{
					set(component, value.c_str());
				}
			});
		}

		void ComponentInspectorBase::AddFloatProperty(const std::string& label,
			std::function<float (ComponentPtr)> get,
			std::function<void (ComponentPtr, float)> set)
		{
			Controls::Properties* props = new Controls::Properties(this);
			props->SetBounds(10, 10, 150, 300);
			auto row = props->Add(label, new Controls::Property::Text(props));
			row->onChange.Add(this, [this, get, set](Controls::Base* control)
			{
				Controls::PropertyRow* row = (Controls::PropertyRow*)control;
				Controls::Property::Text* textbox = dynamic_cast<Controls::Property::Text*>(row->GetProperty());
				auto value = textbox->GetPropertyValue();
				for (auto component : m_Components)
				{
					set(component, boost::lexical_cast<float>(value.c_str()));
				}
			});
		}

		void ComponentInspectorBase::AddBoolProperty(const std::string& label,
			std::function<bool (ComponentPtr)> get,
			std::function<void (ComponentPtr, bool)> set)
		{
			Controls::Properties* props = new Controls::Properties(this);
			props->SetBounds(10, 10, 150, 300);
			auto row = props->Add(label, new Controls::Property::Text(props));
			row->onChange.Add(this, [this, get, set](Controls::Base* control)
			{
				Controls::PropertyRow* row = (Controls::PropertyRow*)control;
				Controls::Property::Checkbox* checkbox = dynamic_cast<Controls::Property::Checkbox*>(row->GetProperty());
				auto value = checkbox->m_Checkbox->IsChecked();
				for (auto component : m_Components)
				{
					set(component, value);
				}
			});
		}*/

} }
