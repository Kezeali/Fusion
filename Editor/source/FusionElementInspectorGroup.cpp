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

#include <Rocket/Core.h>
#include <Rocket/Controls.h>

#include <boost/lexical_cast.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

namespace FusionEngine { namespace Inspectors
{

	namespace tags {
		struct button_code {};
		struct inspector_key {};
		struct component {};
	}
	
	EquivalentInspectorKey::EquivalentInspectorKey(const ComponentPtr& component_, const std::string& type)
		: component(component_),
		entity(component_->GetParent()->shared_from_this()),
		inspector_type(type),
		component_id(component_->GetIdentifier())
	{}

	//std::size_t hash_value(const EquivalentInspectorKey& key)
	//{
	//}

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
			copy.component = component;
			auto r = m_Subsections.insert(copy);

			FSN_ASSERT(r.second);

			m_Inspectors[copy.inspector].push_back(component);
		}

		int AddNewEntry(const EquivalentInspectorKey& key, Rocket::Core::Element* subsection, Inspectors::ComponentInspector* inspector, const ComponentPtr& component)
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

		//void ForEachInspector()
		//{
		//}
		void InitInspectors()
		{
			for (auto it = m_Inspectors.begin(), end = m_Inspectors.end(); it != end; ++it)
			{
				it->first->SetComponents(it->second);
			}
		}

		std::vector<boost::intrusive_ptr<Rocket::Core::Element>> GetSubsections(const ComponentPtr& component)
		{
			std::vector<boost::intrusive_ptr<Rocket::Core::Element>> subsections;

		}

		template <class Iter>
		void GetSubsections(const ComponentPtr& component, Iter output_it)
		{
			auto& byButtonCode = m_Subsections.get<tags::component>();
			auto entry = byButtonCode.equal_range(component);
			for (; entry.first != entry.second; ++entry.first)
				*output_it++ = entry.first->subsection;
		}

		boost::intrusive_ptr<Rocket::Core::Element> GetSubsection(int code)
		{
			auto& byButtonCode = m_Subsections.get<tags::button_code>();
			auto entry = byButtonCode.find(code);
			if (entry != byButtonCode.end())
				return entry->subsection;
			else
				return boost::intrusive_ptr<Rocket::Core::Element>();
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

			Rocket::Core::Element* subsection;

			Inspectors::ComponentInspector* inspector;

			ComponentPtr component;
		};

		boost::multi_index_container<SubsectionData,
			boost::multi_index::indexed_by<
			boost::multi_index::hashed_unique<boost::multi_index::tag<tags::button_code>, boost::multi_index::member<SubsectionData, int, &SubsectionData::button_code>>,
			boost::multi_index::ordered_unique<boost::multi_index::tag<tags::inspector_key>, boost::multi_index::member<SubsectionData, EquivalentInspectorKey, &SubsectionData::inspector_key>>,
			boost::multi_index::hashed_non_unique<boost::multi_index::tag<tags::component>, boost::multi_index::member<SubsectionData, ComponentPtr, &SubsectionData::component>>
			>
			> m_Subsections;

		std::map<ComponentInspector*, std::vector<ComponentPtr>> m_Inspectors;
	};

	ElementGroup::ElementGroup(const Rocket::Core::String& tag)
		: Rocket::Core::Element(tag),
		m_Subsections(new SubsectionCollection())
	{
		body = Rocket::Core::Factory::InstanceElement(this, "div", "div", Rocket::Core::XMLAttributes());
		FSN_ASSERT(body);
		this->AppendChild(body);
		body->RemoveReference();
	}

	void ElementGroup::AddEntity(const EntityPtr& entity)
	{
		m_Entities.push_back(entity);

		m_EntityBeingProcessed = entity;

		const auto& components = entity->GetComponents();
		for (auto it = components.begin(), end = components.end(); it != end; ++it)
			ProcessComponent(*it);

		m_EntityBeingProcessed.reset();
	}

	void ElementGroup::DoneAddingEntities()
	{
		m_Subsections->InitInspectors();
		/*([](ComponentInspector* inspector, <something> components)
		{
			inspector->SetComponents(components);
		});*/
	}

	void ElementGroup::ProcessComponent(const ComponentPtr& component)
	{
		bool added = AddInspector(component, component->GetType());

		if (!added) // If there wasn't a specific inspector for the given type, try adding interface inspectors
		{
			for (auto iit = component->GetInterfaces().begin(), iend = component->GetInterfaces().end(); iit != iend; ++iit)
				added |= AddInspector(component, *iit);
		}
		if (!added)
			SendToConsole("No inspector for component type: " + component->GetType());
	}

	bool ElementGroup::AddInspector(const ComponentPtr& component, const std::string& inspector_type)
	{
		EquivalentInspectorKey key(component, inspector_type);
		if (!m_Subsections->Contains(key))
		{
			// New component / interface type
			auto tag = "inspector_" + inspector_type;
			fe_tolower(tag);
			Rocket::Core::String rocketTag(tag.data(), tag.data() + tag.length());
			auto element = Rocket::Core::Factory::InstanceElement(body, rocketTag, "inspector", Rocket::Core::XMLAttributes());
			auto inspector = dynamic_cast<Inspectors::ComponentInspector*>(element);
			if (inspector)
			{
				//auto& value = m_Inspectors[key];
				//value.first = inspector;
				//value.second.push_back(component);

				auto name = component->GetType();
				if (!component->GetIdentifier().empty())
					name += " - " + component->GetIdentifier();

				AddSubsection(key, name, inspector, component);
			}
			if (element)
				element->RemoveReference();

			return inspector != nullptr;
		}
		else
		{
			// Existing type
			//entry->second.second.push_back(component);
			m_Subsections->Add(key, component);
			return true;
		}
	}

	void ElementGroup::AddSubsection(const EquivalentInspectorKey& key, const std::string& name, Inspectors::ComponentInspector* inspector, const ComponentPtr& initial_component)
	{
		auto subsection = AddSubsection(body, name, inspector);

		int code =
			m_Subsections->AddNewEntry(key, subsection.get(), inspector, initial_component);

		Rocket::Core::ElementList headerElems;
		subsection->GetElementsByTagName(headerElems, "header");
		FSN_ASSERT(!headerElems.empty());
		
		auto header = headerElems.front();

		//  'Remove' button
		Rocket::Core::XMLAttributes formAttributes;
		formAttributes.Set("code", code);
		auto form = Rocket::Core::Factory::InstanceElement(header, "form", "form", formAttributes);
		header->AppendChild(form);

		Rocket::Core::XMLAttributes buttonAttributes;
		buttonAttributes.Set("type", "submit");
		buttonAttributes.Set("name", "result");
		buttonAttributes.Set("value", "remove_component");
		auto removeButton = Rocket::Core::Factory::InstanceElement(header, "input", "input", buttonAttributes);
		form->AppendChild(removeButton);
		removeButton->RemoveReference();

		form->RemoveReference();
	}

	void ElementGroup::AddFooter()
	{
		Rocket::Core::XMLAttributes formAttributes;
		formAttributes.Set("class", "footer");
		auto footer = Rocket::Core::Factory::InstanceElement(this, "form", "form", formAttributes);

		Rocket::Core::XMLAttributes inputAttributes;

		inputAttributes.Set("type", "text");
		inputAttributes.Set("name", "component_type");
		inputAttributes.Set("size", 20);
		auto typeBox = Rocket::Core::Factory::InstanceElement(footer, "input", "input", inputAttributes);
		footer->AppendChild(typeBox);
		typeBox->RemoveReference();

		inputAttributes.Set("name", "component_id");
		auto idBox = Rocket::Core::Factory::InstanceElement(footer, "input", "input", inputAttributes);
		footer->AppendChild(idBox);
		idBox->RemoveReference();

		inputAttributes.Clear();
		inputAttributes.Set("type", "submit");
		inputAttributes.Set("name", "result");
		inputAttributes.Set("value", "add_component");
		auto addButton = Rocket::Core::Factory::InstanceElement(footer, "input", "input", inputAttributes);
		//addButton->SetClass("add_component", true);
		Rocket::Core::Factory::InstanceElementText(addButton, "Add Component");
		footer->AppendChild(addButton);
		addButton->RemoveReference();

		this->AppendChild(footer);
		footer->RemoveReference();
	}

	void ElementGroup::ProcessEvent(Rocket::Core::Event& ev)
	{
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
						m_EntityBeingProcessed = *it;
						ProcessComponent(newCom);
					}
				}
				m_EntityBeingProcessed.reset();
				DoneAddingEntities();
			}
			else if (remove)
			{
				int code = ev.GetTargetElement()->GetAttribute("code", ev.GetParameter("code", int(-1)));

				if (code != -1)
				{
					auto components = m_Subsections->GetComponents(code);
					std::set<boost::intrusive_ptr<Rocket::Core::Element>> subsectionsToRemove;

					for (auto it = components.begin(), end = components.end(); it != end; ++it)
					{
						const auto& com = *it;
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

} }
