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

#include "FusionElementInspectorGroup.h"

#include "FusionComponentInspector.h"
#include "FusionEntity.h"

#include <boost/lexical_cast.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

namespace FusionEngine { namespace Inspectors
{

	struct EquivalentInspectorKey
	{
		ComponentPtr component;
		EntityPtr entity;
		std::string inspector_type;
		std::string component_id;

		EquivalentInspectorKey()
		{}

		EquivalentInspectorKey(const EntityPtr& entity_, const std::string& type, const std::string& id)
			: entity(entity_), inspector_type(type), component_id(id)
		{}

		//EquivalentInspectorKey(const ComponentPtr& component_, const std::string& type);
		EquivalentInspectorKey(const ComponentPtr& component_, const std::string& type)
			: component(component_),
			entity(component_->GetParent()->shared_from_this()),
			inspector_type(type),
			component_id(component_->GetIdentifier())
		{}

		bool operator== (const EquivalentInspectorKey& other) const
		{
			if (entity != other.entity)
			{
				return inspector_type == other.inspector_type && component_id == other.component_id;
			}
			else
				return inspector_type == other.inspector_type && component == other.component;
		}
		EquivalentInspectorKey& operator= (const EquivalentInspectorKey& other)
		{
			component = other.component;
			entity = other.entity;
			inspector_type = other.inspector_type;
			component_id = other.component_id;
			return *this;
		}
	};

	/*EquivalentInspectorKey::EquivalentInspectorKey(const ComponentPtr& component_, const std::string& type)
		: component(component_),
		entity(component_->GetParent()->shared_from_this()),
		inspector_type(type),
		component_id(component_->GetIdentifier())
	{}*/

	namespace tags {
		struct button_code {};
		struct inspector_key {};
		struct component {};
	}

	std::size_t hash_value(const EquivalentInspectorKey& key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.inspector_type);
		boost::hash_combine(seed, key.component_id);

		return seed;
	}

	class SubsectionCollection
	{
	public:
		SubsectionCollection()
			: m_NextId(0)
		{}

		bool Contains(const EquivalentInspectorKey& key) const
		{
			const auto& byInspectorKey = m_Subsections.get<tags::inspector_key>();
			return byInspectorKey.find(key) != byInspectorKey.end();
		}

		void Add(const EquivalentInspectorKey& key, const ComponentPtr& component)
		{
			auto& byInspectorKey = m_Subsections.get<tags::inspector_key>();
			auto entry = byInspectorKey.find(key);
			FSN_ASSERT(entry != byInspectorKey.end());

			auto copy = *entry;
			copy.inspector_key = key;
			copy.component = component;
			auto r = m_Subsections.insert(copy);

#ifdef _DEBUG
			if (!r.second)
			{
				auto range = m_Subsections.get<tags::button_code>().equal_range(copy.button_code);
				for (; range.first != range.second; ++range.first)
				{
					auto t = *range.first;
				}
			}
#endif
			FSN_ASSERT(r.second);

			auto& inspectorComponents = m_Inspectors[copy.inspector];
			if (std::find(inspectorComponents.begin(), inspectorComponents.end(), component) == inspectorComponents.end())
				inspectorComponents.push_back(component);
		}

		int AddNewEntry(const EquivalentInspectorKey& key, Gwen::Controls::Base* subsection, Inspectors::ComponentInspector* inspector, const ComponentPtr& component)
		{
			SubsectionData newData;
			newData.button_code = m_NextId++;
			newData.inspector_key = key;
			newData.subsection = subsection;
			newData.inspector = inspector;
			newData.component = component;
			auto r = m_Subsections.insert(newData);

			if (r.second)
				m_Inspectors[inspector].push_back(component);
			else
				FSN_EXCEPT(InvalidArgumentException, "Inspector with the given key already exists");

			return newData.button_code;
		}

		void ForEachInspector(const std::function<void (const std::vector<ComponentPtr>&)>& action)
		{
			for (auto it = m_Inspectors.begin(), end = m_Inspectors.end(); it != end; ++it)
			{
				action(it->second);
			}
		}

		void InitInspectors()
		{
			for (auto it = m_Inspectors.begin(), end = m_Inspectors.end(); it != end; ++it)
			{
				it->first->SetComponents(it->second);
			}
		}

		std::vector<Gwen::Controls::Base*> GetSubsections(const ComponentPtr& component)
		{
			std::vector<Gwen::Controls::Base*> subsections;
			GetSubsections(component, std::back_inserter(subsections));
			return subsections;
		}

		template <class Iter>
		void GetSubsections(const ComponentPtr& component, Iter output_it)
		{
			auto& byButtonCode = m_Subsections.get<tags::component>();
			auto entry = byButtonCode.equal_range(component);
			for (; entry.first != entry.second; ++entry.first)
				*output_it++ = entry.first->subsection;
		}

		Gwen::Controls::Base* GetSubsection(int code)
		{
			auto& byButtonCode = m_Subsections.get<tags::button_code>();
			auto entry = byButtonCode.find(code);
			if (entry != byButtonCode.end())
				return entry->subsection;
			else
				return nullptr;
		}

		Inspectors::ComponentInspector* GetInspector(int code)
		{
			auto& byButtonCode = m_Subsections.get<tags::button_code>();
			auto entry = byButtonCode.find(code);
			if (entry != byButtonCode.end())
				return entry->inspector;
			else
				return nullptr;
		}

		const std::vector<ComponentPtr>& GetComponents(int code)
		{
			auto entry = m_Inspectors.find(GetInspector(code));
			if (entry != m_Inspectors.end())
				return entry->second;
			else
				FSN_EXCEPT(InvalidArgumentException, "No inspector with the given code");
		}

		bool RemoveEntry(int code)
		{
			auto& byButtonCode = m_Subsections.get<tags::button_code>();
			auto entry = byButtonCode.find(code);
			if (entry != byButtonCode.end())
			{
				m_Inspectors.erase(entry->inspector);
				byButtonCode.erase(entry);
				return true;
			}
			else
				return false;
		}

		void RemoveEntries(const ComponentPtr& component)
		{
			auto& byComponent = m_Subsections.get<tags::component>();
			auto range = byComponent.equal_range(component);
			for (; range.first != range.second; ++range.first)
				m_Inspectors.erase(range.first->inspector);
			byComponent.erase(component);
		}

	private:
		int m_NextId;

		struct SubsectionData
		{
			int button_code;

			EquivalentInspectorKey inspector_key;

			Gwen::Controls::Base* subsection;

			Inspectors::ComponentInspector* inspector;

			ComponentPtr component;
		};

		boost::multi_index_container<SubsectionData,
			boost::multi_index::indexed_by<
			boost::multi_index::ordered_non_unique<boost::multi_index::tag<tags::button_code>, boost::multi_index::member<SubsectionData, int, &SubsectionData::button_code>>,
			boost::multi_index::hashed_non_unique<boost::multi_index::tag<tags::inspector_key>, boost::multi_index::member<SubsectionData, EquivalentInspectorKey, &SubsectionData::inspector_key>>,
			boost::multi_index::hashed_non_unique<boost::multi_index::tag<tags::component>, boost::multi_index::member<SubsectionData, ComponentPtr, &SubsectionData::component>>
			>
			> m_Subsections;

		std::map<ComponentInspector*, std::vector<ComponentPtr>> m_Inspectors;
	};

	GWEN_CONTROL_CONSTRUCTOR(ElementGroup),
		m_Subsections(new SubsectionCollection())
	{
		//body = Rocket::Core::Factory::InstanceElement(this, "div", "div", Rocket::Core::XMLAttributes());
		//FSN_ASSERT(body);
		//this->AppendChild(body);
		//body->RemoveReference();
	}

	void ElementGroup::AddEntity(const EntityPtr& entity)
	{
		m_Entities.push_back(entity);

		ProcessComponent(entity->GetTransform(), false);

		const auto& components = entity->GetComponents();
		for (auto it = components.begin(), end = components.end(); it != end; ++it)
			if (*it != entity->GetTransform())
				ProcessComponent(*it);
	}

	void ElementGroup::DoneAddingEntities()
	{
		m_Subsections->InitInspectors();

		//SetPseudoClass("single_entity", m_Entities.size() == 1);
		//SetPseudoClass("multi_entity", m_Entities.size() > 1);
	}

	void ElementGroup::ProcessComponent(const ComponentPtr& component, bool removable)
	{
		bool added = AddInspector(component, component->GetType(), removable);

		// If there wasn't a specific inspector for the given type, try adding interface inspectors
		if (!added)
		{
			// Make sure the transform inspector is at the top
			const auto& actualInterfaces = component->GetInterfaces();
			auto entry = actualInterfaces.find("ITransform");
			if (entry == actualInterfaces.end())
			{
				for (auto it = actualInterfaces.begin(), end = actualInterfaces.end(); it != end; ++it)
					added |= AddInspector(component, *it, removable);
			}
			else
			{
				AddInspector(component, "ITransform", false);

				std::set<std::string> interfacesMinusTransform = actualInterfaces;
				interfacesMinusTransform.erase("ITransform");
				for (auto it = interfacesMinusTransform.begin(), end = interfacesMinusTransform.end(); it != end; ++it)
					added |= AddInspector(component, *it, removable);
			}
		}
		if (!added)
			SendToConsole("No inspector for component type: " + component->GetType());
	}

	bool ElementGroup::AddInspector(const ComponentPtr& component, const std::string& inspector_type, bool removable)
	{
		EquivalentInspectorKey key(component, inspector_type);
		if (!m_Subsections->Contains(key))
		{
			// New component / interface type
			auto tag = "inspector_" + inspector_type;
			fe_tolower(tag);
			Gwen::Controls::Base* inspector = nullptr;
			//Rocket::Core::String rocketTag(tag.data(), tag.data() + tag.length());
			//auto element = Rocket::Core::Factory::InstanceElement(body, rocketTag, "inspector", Rocket::Core::XMLAttributes());
			//auto inspector = dynamic_cast<Inspectors::ComponentInspector*>(element);
			//if (inspector)
			//{
			//	//auto& value = m_Inspectors[key];
			//	//value.first = inspector;
			//	//value.second.push_back(component);

			//	// The title format is "interface (actual type) - identifier"
			//	auto name = component->GetType();
			//	if (name != inspector_type)
			//		name = inspector_type + " (" + name + ")";
			//	if (!component->GetIdentifier().empty())
			//		name += " - " + component->GetIdentifier();

			//	// Pass the tool executors onward
			//	FSN_ASSERT(m_CircleToolExecutor);
			//	FSN_ASSERT(m_RectangleToolExecutor);
			//	FSN_ASSERT(m_PolygonToolExecutor);
			//	inspector->SetCircleToolExecutor(m_CircleToolExecutor);
			//	inspector->SetRectangleToolExecutor(m_RectangleToolExecutor);
			//	inspector->SetPolygonToolExecutor(m_PolygonToolExecutor);
			//	inspector->SetResourceEditorFactory(m_ResourceEditorFactory);

			//	//inspector->InitUI();

			//	AddSubsection(key, name, inspector, component, removable);
			//}
			//if (element)
			//	element->RemoveReference();

			return inspector != nullptr;
		}
		else
		{
			// Existing type
			//entry->second.second.push_back(component);
			m_Subsections->Add(key, component);
			//if (!removable)
			//{
			//	auto subsections = m_Subsections->GetSubsections(component);
			//	for (auto it = subsections.begin(); it != subsections.end(); ++it)
			//		(*it)->SetClass("locked_component", true);
			//}
			return true;
		}
	}

	void ElementGroup::AddSubsection(const EquivalentInspectorKey& key, const std::string& name, Inspectors::ComponentInspector* inspector, const ComponentPtr& initial_component, bool removable)
	{
		auto subsection = AddSubsection(body, name, inspector);

		int code =
			m_Subsections->AddNewEntry(key, subsection, inspector, initial_component);

		//if (!removable)
		//{
		//	subsection->SetClass("locked_component", true);
		//}

		//Rocket::Core::ElementList headerElems;
		//subsection->GetElementsByTagName(headerElems, "header");
		//FSN_ASSERT(!headerElems.empty());

		//auto header = headerElems.front();

		////  'Remove' button
		//Rocket::Core::XMLAttributes formAttributes;
		//formAttributes.Set("code", code);
		//auto form = Rocket::Core::Factory::InstanceElement(header, "form", "form", formAttributes);
		//header->AppendChild(form);

		//Rocket::Core::XMLAttributes buttonAttributes;
		//buttonAttributes.Set("type", "submit");
		//buttonAttributes.Set("name", "result");
		//buttonAttributes.Set("value", "remove_component");
		//auto removeButton = Rocket::Core::Factory::InstanceElement(header, "input", "input", buttonAttributes);
		//form->AppendChild(removeButton);
		//removeButton->RemoveReference();

		//form->RemoveReference();
	}

	void ElementGroup::AddFooter()
	{
		//Rocket::Core::XMLAttributes formAttributes;
		//formAttributes.Set("class", "footer");
		//auto footer = Rocket::Core::Factory::InstanceElement(this, "form", "form", formAttributes);

		//Rocket::Core::XMLAttributes inputAttributes;

		//inputAttributes.Set("type", "text");
		//inputAttributes.Set("name", "component_type");
		//inputAttributes.Set("size", 20);
		//auto typeBox = Rocket::Core::Factory::InstanceElement(footer, "input", "input", inputAttributes);
		//footer->AppendChild(typeBox);
		//typeBox->RemoveReference();

		//inputAttributes.Set("name", "component_id");
		//auto idBox = Rocket::Core::Factory::InstanceElement(footer, "input", "input", inputAttributes);
		//footer->AppendChild(idBox);
		//idBox->RemoveReference();

		//inputAttributes.Clear();
		//inputAttributes.Set("type", "submit");
		//inputAttributes.Set("name", "result");
		//inputAttributes.Set("value", "add_component");
		//auto addButton = Rocket::Core::Factory::InstanceElement(footer, "input", "input", inputAttributes);
		////addButton->SetClass("add_component", true);
		//Rocket::Core::Factory::InstanceElementText(addButton, "Add Component");
		//footer->AppendChild(addButton);
		//addButton->RemoveReference();

		//this->AppendChild(footer);
		//footer->RemoveReference();
	}

	/*
	void ElementGroup::ProcessEvent(Rocket::Core::Event& ev)
	{
		Rocket::Core::Element::ProcessEvent(ev);

		if (ev == "submit")
		{
			const bool add = ev.GetParameter("result", Rocket::Core::String()) == "add_component";
			const bool remove = ev.GetParameter("result", Rocket::Core::String()) == "remove_component";
			if (add)
			{
				auto elem = ev.GetTargetElement();
				std::string type = ev.GetParameter("component_type", Rocket::Core::String()).CString();
				std::string identifier = ev.GetParameter("component_id", Rocket::Core::String()).CString();
				for (auto it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
				{
					auto newCom = m_AddCallback(*it, type, identifier);

					if (newCom)
					{
						m_ComponentsToProcess.push_back(std::make_pair(*it, newCom));
					}
				}
			}
			else if (remove)
			{
				int code = ev.GetTargetElement()->GetAttribute("code", ev.GetParameter("code", int(-1)));

				if (code != -1)
				{
					auto components = m_Subsections->GetComponents(code);
					std::set<Gwen::Controls::Base*> subsectionsToRemove;

					for (auto it = components.begin(), end = components.end(); it != end; ++it)
					{
						const auto& com = *it;
						FSN_ASSERT(com->GetParent());
						m_RemoveCallback(com->GetParent()->shared_from_this(), com);

						m_Subsections->GetSubsections(com, std::inserter(subsectionsToRemove, subsectionsToRemove.begin()));
						m_Subsections->RemoveEntries(com);
					}
					for (auto it = subsectionsToRemove.begin(), end = subsectionsToRemove.end(); it != end; ++it)
					{
						body->RemoveChild(it->get());
					}
				}
			}
		}
	}
	*/

	void ElementGroup::OnUpdate()
	{
		//Rocket::Core::Element::OnUpdate();

		bool newComponentAdded = false;
		for (auto it = m_ComponentsToProcess.begin(); it != m_ComponentsToProcess.end();)
		{
			if (it->second->IsReady() || it->second->IsActive())
			{
				ProcessComponent(it->second);
				it = m_ComponentsToProcess.erase(it);
				newComponentAdded = true;
			}
			else
				++it;
		}

		if (m_UpdatesSkipped > 30)
		{
			m_UpdatesSkipped = 0;

			for (auto it = m_Entities.begin(); it != m_Entities.end(); ++it)
			{
				const auto& entity = *it;
				const auto& entityComponents = entity->GetComponents();
				for (auto cIt = entityComponents.begin(); cIt != entityComponents.end(); ++cIt)
				{
					auto subsections = m_Subsections->GetSubsections(*cIt);

					// No UI subsections have been created for this component yet: process it
					if (subsections.empty())
					{
						ProcessComponent(*cIt);
						newComponentAdded = true;
					}
				}

				std::set<ComponentPtr> inspectedComponents;
				m_Subsections->ForEachInspector([&inspectedComponents](const std::vector<ComponentPtr>& components)
				{
					inspectedComponents.insert(components.begin(), components.end());
				});
				for (auto cIt = inspectedComponents.begin(); cIt != inspectedComponents.end(); ++cIt)
				{
					const auto& component = *cIt;
					if (component->GetParent() == nullptr)
					{
						auto subsectionsToRemove = m_Subsections->GetSubsections(*cIt);
						for (auto subit = subsectionsToRemove.begin(), subend = subsectionsToRemove.end(); subit != subend; ++subit)
						{
							//body->RemoveChild(it->get());
						}

						m_Subsections->RemoveEntries(component);
					}
				}
			}
		}
		else
			++m_UpdatesSkipped;

		if (newComponentAdded)
			DoneAddingEntities();
	}

} }
