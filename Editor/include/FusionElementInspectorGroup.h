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

#ifndef H_FusionElementInspectorGroup
#define H_FusionElementInspectorGroup

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionTypes.h"

#include "FusionEntityComponent.h"

#include <functional>
#include <unordered_map>
#include <string>

#include <boost/intrusive_ptr.hpp>

#include <Gwen/Controls/GroupBox.h>

#include "FusionEditorCircleTool.h"
#include "FusionEditorPolygonTool.h"
#include "FusionEditorRectangleTool.h"
#include "FusionResourceEditorFactory.h"

namespace FusionEngine { namespace Inspectors
{

	class ComponentInspector;

	class SubsectionCollection;

	struct EquivalentInspectorKey;

	class ElementGroup : public Gwen::Controls::Base
	{
	public:
		GWEN_CONTROL(ElementGroup, Gwen::Controls::Base);

		void AddEntity(const EntityPtr& entity);
		//! Passes all components to inspectors
		void DoneAddingEntities();

		typedef std::function<ComponentPtr (const EntityPtr&, std::string, std::string)> AddCallback_t;
		typedef std::function<void (const EntityPtr&, const ComponentPtr&)> RemoveCallback_t;

		void SetAddCallback(AddCallback_t fn) { m_AddCallback = fn; }
		void SetRemoveCallback(RemoveCallback_t fn) { m_RemoveCallback = fn; }

		void SetCircleToolExecutor(const CircleToolExecutor_t& executor) { m_CircleToolExecutor = executor; }
		void SetRectangleToolExecutor(const RectangleToolExecutor_t& executor) { m_RectangleToolExecutor = executor; }
		void SetPolygonToolExecutor(const PolygonToolExecutor_t& executor) { m_PolygonToolExecutor = executor; }

		void SetResourceEditorFactory(ResourceEditorFactory* factory) { m_ResourceEditorFactory = factory; }

		bool AddInspector(const ComponentPtr& component, const std::string& inspector_type, bool removable);

		void ProcessComponent(const ComponentPtr& component, bool removable = true);

		static Gwen::Controls::Base* AddSubsection(Gwen::Controls::Base* parent, const std::string& name, Gwen::Controls::Base* inspector);

		void AddFooter();

		Gwen::Controls::Base* GetSubsectionByChild(Gwen::Controls::Base* button)
		{
			Gwen::Controls::Base* parent = button->GetParent();
			while (parent != nullptr && std::string(parent->GetIdentifier()) != "inspector_section");
			{
				parent = parent->GetParent();
			} 
			return parent;
		}

	private:
		std::vector<EntityPtr> m_Entities;

		std::shared_ptr<SubsectionCollection> m_Subsections;

		Gwen::Controls::Base* body;

		std::vector<std::pair<EntityPtr, ComponentPtr>> m_ComponentsToProcess;
		std::vector<ComponentPtr> m_ComponentsToRemove;
		
		AddCallback_t m_AddCallback;
		RemoveCallback_t m_RemoveCallback;

		CircleToolExecutor_t m_CircleToolExecutor;
		RectangleToolExecutor_t m_RectangleToolExecutor;
		PolygonToolExecutor_t m_PolygonToolExecutor;

		ResourceEditorFactory* m_ResourceEditorFactory;

		unsigned short m_UpdatesSkipped;

		void AddSubsection(const EquivalentInspectorKey& key, const std::string& name, Inspectors::ComponentInspector* inspector, const ComponentPtr& initial_component, bool removable);

		//void ProcessEvent(Rocket::Core::Event& ev);

		void OnUpdate();
		
	};

	inline Gwen::Controls::Base* ElementGroup::AddSubsection(Gwen::Controls::Base* parent, const std::string& name, Gwen::Controls::Base* inspector)
	{
		Gwen::Controls::Base* subsection = nullptr;
		//// Subsection
		//subsection = Rocket::Core::Factory::InstanceElement(parent, "inspector_section", "inspector_section", Rocket::Core::XMLAttributes());
		//// Header (title)
		//auto header = Rocket::Core::Factory::InstanceElement(subsection.get(), "header", "header", Rocket::Core::XMLAttributes());
		//subsection->AppendChild(header);
		//Rocket::Core::Factory::InstanceElementText(header, Rocket::Core::String(name.data(), name.data() + name.length()));
		//header->RemoveReference();
		//// This element can be collapsed to hide the inspector
		//auto collapse = Rocket::Core::Factory::InstanceElement(subsection.get(), "collapsible", "collapsible", Rocket::Core::XMLAttributes());
		//collapse->SetPseudoClass("collapsed", true);

		//collapse->AppendChild(inspector);

		//subsection->AppendChild(collapse);
		//collapse->RemoveReference();

		//parent->AppendChild(subsection.get());
		//subsection->RemoveReference();

		return subsection;
	}

} }

#endif
