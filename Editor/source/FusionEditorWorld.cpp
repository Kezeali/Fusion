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

#include "FusionEditorWorld.h"

#include "FusionSystemTask.h"

#include "FusionArchetypeFactory.h"
#include "FusionArchetypalEntityManager.h"
#include "FusionArchetypeResourceEditor.h"
#include "FusionCamera.h"
#include "FusionEditorCircleTool.h"
#include "FusionEditorPolygonTool.h"
#include "FusionEditorRectangleTool.h"
#include "FusionElementInspectorGroup.h"
#include "FusionEntity.h"
#include "FusionEntityInspector.h"
#include "FusionPolygonResourceEditor.h"
#include "FusionResource.h"
#include "FusionResourceDatabase.h"
#include "Visual/FusionDebugDraw.h"

#include "Gwen/DebugRenderer.h"
#include <Gwen/Input/ClanLib.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <Gwen/Controls/Canvas.h>
#include <Gwen/Controls/ComboBox.h>
#include <Gwen/Controls/Menu.h>
#include <Gwen/Controls/Dialogs/Query.h>
#include <Gwen/Controls/WindowControl.h>
#include <Gwen/Skins/Simple.h>
#include <Gwen/Skins/TexturedBase.h>

namespace FusionEngine
{

	EditorWorld::EditorWorld(System::ISystem* system)
		: System::WorldBase(system),
		m_Tool(Tool::None)
	{
		m_ShapeTools[Tool::Polygon] = m_ShapeTools[Tool::Line] = m_PolygonTool = std::make_shared<EditorPolygonTool>();
		m_ShapeTools[Tool::Rectangle] = m_RectangleTool = std::make_shared<EditorRectangleTool>();
		m_ShapeTools[Tool::Elipse] = m_CircleTool = std::make_shared<EditorCircleTool>();

		{
			auto polygonResourceEditor = std::make_shared<PolygonResourceEditor>();
			m_ResourceEditors["POLYGON"] = polygonResourceEditor;
			polygonResourceEditor->SetPolygonToolExecutor([this](const std::vector<Vector2>& verts, const PolygonToolCallback_t& done_cb)
			{
				this->m_PolygonTool->Start(verts, done_cb, EditorPolygonTool::Freeform);
				this->m_Tool = Tool::Polygon;
			});
			//using namespace std::placeholders;
			//polygonResourceEditor->SetPolygonToolExecutor(std::bind(&EditorPolygonTool::Start, m_PolygonTool, _1, _2, EditorPolygonTool::Freeform));
		}

		{
			auto archetypeResourceEditor = std::make_shared<ArchetypeResourceEditor>();
			m_ResourceEditors["ArchetypeFactory"] = archetypeResourceEditor;
			archetypeResourceEditor->SetEntityInspectorExecutor([this](const EntityPtr& entity, const std::function<void (void)>& done_cb)
			{
				this->CreatePropertiesWindow(entity, done_cb);
			});
		}
	}

	class InspectorGenerator
	{
	public:
		Gwen::Controls::WindowControl* window;
		Gwen::Controls::Base* body;

		//boost::intrusive_ptr<EntitySelector> entity_selector;
		Inspectors::ElementEntityInspector* entity_inspector;
		Inspectors::ElementGroup* inspector_group;

		Gwen::Controls::ComboBox* entitySelector;

		std::vector<EntityPtr> entities;

		std::function<void (const EntityPtr& entity)> entitySelectorCallback;

		InspectorGenerator(Gwen::Controls::Canvas* gui_canvas)
		{
			window = new Gwen::Controls::WindowControl(gui_canvas);

			window->SetTitle(L"Properties");
			window->SetSize(200, 200);
			window->SetPos(0, 0);
			window->SetDeleteOnClose(true);

			entitySelector = new Gwen::Controls::ComboBox(window);
			//entitySelector->onSelection.Add(this, &InspectorGenerator::EntitySelector_OnSelection);
			/*
			auto element = doc->CreateElement("entity_selector"); FSN_ASSERT(element);
			if (entity_selector = dynamic_cast<EntitySelector*>(element))
			{
				auto body = doc->GetFirstChild()->GetElementById("content");
				Rocket::Core::Factory::InstanceElementText(body, "Go To:");
				body->AppendChild(entity_selector.get());
			}
			else
			{
				FSN_EXCEPT(Exception, "Failed to create entity_selector element.");
			}
			element->RemoveReference();

			element = Rocket::Core::Factory::InstanceElement(doc, "inspector_entity", "inspector", Rocket::Core::XMLAttributes()); FSN_ASSERT(element);
			if (entity_inspector = dynamic_cast<Inspectors::ElementEntityInspector*>(element))
			{
				entity_inspector->SetClass("entity", true);
				Inspectors::ElementGroup::AddSubsection(body, "Entity", entity_inspector.get());
			}
			element->RemoveReference();

			element = Rocket::Core::Factory::InstanceElement(doc, "inspector_group", "inspector_group", Rocket::Core::XMLAttributes()); FSN_ASSERT(element);
			if (inspector_group = dynamic_cast<Inspectors::ElementGroup*>(element))
			{
				body->AppendChild(inspector_group.get());
			}
			else
			{
				FSN_EXCEPT(Exception, "Failed to create inspector_group element.");
			}
			element->RemoveReference();
			*/
		}

		void ProcessEntity(const EntityPtr& entity)
		{
			entities.push_back(entity);

			inspector_group->AddEntity(entity);
		}

		void SetEntitySelectorCallback(std::function<void (const EntityPtr& entity)> fn)
		{
			entitySelectorCallback = fn;
		}

		void EntitySelector_OnSelection()
		{
			EntityPtr entitySelected;
			entitySelectorCallback(entitySelected);
		}

		void Generate()
		{
			// Generate title
			{
				std::string title;
				if (entities.size() > 1)
				{
					title = boost::lexical_cast<std::string>(entities.size()) + " entities";
				}
				else if (!entities.empty())
				{
					auto front = entities.front();
					if (front->GetName().empty())
						title = "Unnamed";
					else
						title = front->GetName();
					if (front->IsPseudoEntity())
						title += " Pseudo-Entity";
					else
						title += " - ID: " + boost::lexical_cast<std::string>(front->GetID());

					entity_inspector->SetEntity(front);
				}
				if (!title.empty())
					window->SetTitle(Gwen::Utility::Format(L"Properties: %s", title));
			}

			inspector_group->AddFooter();

			inspector_group->DoneAddingEntities();

			// Set entities
			for (auto entity : entities)
			{
				entitySelector->AddItem(Gwen::Utility::StringToUnicode(entity->GetName()));
			}
		}
	};

	void EditorWorld::InitInspectorGenerator(InspectorGenerator& generator, const std::function<void (void)>& close_callback)
	{
		generator.inspector_group->SetCircleToolExecutor([this](const Vector2& c, float r, const CircleToolCallback_t& done_cb)
		{
			this->m_CircleTool->Start(c, r, done_cb);
			this->m_Tool = EditorWorld::Tool::Elipse;
		});
		generator.inspector_group->SetRectangleToolExecutor([this](const Vector2& size, const Vector2& c, float r, const RectangleToolCallback_t& done_cb)
		{
			this->m_RectangleTool->Start(size, c, r, done_cb);
			this->m_Tool = EditorWorld::Tool::Rectangle;
		});
		generator.inspector_group->SetPolygonToolExecutor([this](const std::vector<Vector2>& verts, const PolygonToolCallback_t& done_cb)
		{
			this->m_PolygonTool->Start(verts, done_cb, EditorPolygonTool::Freeform);
			this->m_Tool = EditorWorld::Tool::Polygon;
		});
		generator.inspector_group->SetResourceEditorFactory(this);
		
		generator.SetEntitySelectorCallback([this](const EntityPtr& entity) { this->GoToEntity(entity); });
		generator.inspector_group->SetAddCallback([this](const EntityPtr& entity, const std::string& type, const std::string& id)->ComponentPtr
		{
			if (!entity->GetArchetypeDefinitionAgent())
				return this->m_EntityInstantiator->AddComponent(entity, type, id);
			else
			{
				// Don't activate components being added to the arc definition
				auto com = this->m_ComponentFactory->InstantiateComponent(type);
				entity->AddComponent(com, id);
				return com;
			}
		});
		generator.inspector_group->SetRemoveCallback([this](const EntityPtr& entity, const ComponentPtr& component)
		{
			this->m_EntityInstantiator->RemoveComponent(entity, component);
		});

		generator.entity_inspector->SetCloseCallback(close_callback);
	}

	void EditorWorld::CreatePropertiesWindow(const EntityPtr& entity, const std::function<void (void)>& close_callback)
	{
		std::vector<EntityPtr> e;
		e.push_back(entity);
		CreatePropertiesWindow(e, close_callback);
	}

	void EditorWorld::CreatePropertiesWindow(const std::vector<EntityPtr>& entities, const std::function<void (void)>& close_callback)
	{
		//auto doc = m_GUIContext->LoadDocument("/Data/core/gui/properties.rml");

		//{
		//	auto script = OpenString_PhysFS("/Data/core/gui/gui_base.as");
		//	auto strm = new Rocket::Core::StreamMemory((const Rocket::Core::byte*)script.c_str(), script.size());
		//	doc->LoadScript(strm, "/Data/core/gui/gui_base.as");
		//	strm->RemoveReference();
		//}

		InspectorGenerator generator(m_GUIContext);
		InitInspectorGenerator(generator, close_callback);

		for (auto it = entities.begin(), end = entities.end(); it != end; ++it)
			generator.ProcessEntity(*it);
		generator.Generate();

		//doc->Show();
		//doc->RemoveReference();
	}

	void EditorWorld::CreatePropertiesWindowForSelected()
	{
		//auto doc = m_GUIContext->LoadDocument("/Data/core/gui/properties.rml");

		//{
		//	auto script = OpenString_PhysFS("/Data/core/gui/gui_base.as");
		//	auto strm = new Rocket::Core::StreamMemory((const Rocket::Core::byte*)script.c_str(), script.size());
		//	doc->LoadScript(strm, "/Data/core/gui/gui_base.as");
		//	strm->RemoveReference();
		//}

		InspectorGenerator generator(m_GUIContext);
		InitInspectorGenerator(generator, std::function<void (void)>());

		ForEachSelected([&generator](const EntityPtr& entity) { generator.ProcessEntity(entity); });
		generator.Generate();
		
		//doc->Show();
		//doc->RemoveReference();
	}

	class OverlayTask : public System::TaskBase
	{
	public:
		friend class EditorWorld;

		OverlayTask(EditorWorld* world, const std::shared_ptr<EditorPolygonTool>& poly_tool, const std::shared_ptr<EditorRectangleTool>& rect_tool, const std::shared_ptr<EditorCircleTool>& circle_tool);

		void Update() override;

	private:
		DebugDraw m_DebugDraw;

		clan::Colorf GetColour(const std::set<EntityPtr>::const_iterator& it) const
		{
			const auto fraction = std::distance(it, m_Selected.end()) / (float)m_Selected.size();
			return clan::ColorHSVf(fraction * 360.f, 0.8f, 0.8f, 1.0f);
		}

		clan::Colorf GetColour(const EntityPtr& entity) const
		{
			return GetColour(m_Selected.find(entity));
		}

		std::set<EntityPtr> m_Selected;
		Vector2 m_Offset;

		std::shared_ptr<Camera> m_EditCam;
		float m_CamRange;

		std::shared_ptr<EditorPolygonTool> m_PolygonTool;
		std::shared_ptr<EditorRectangleTool> m_RectangleTool;
		std::shared_ptr<EditorCircleTool> m_CircleTool;
	};

	OverlayTask::OverlayTask(EditorWorld* world, const std::shared_ptr<EditorPolygonTool>& poly_tool, const std::shared_ptr<EditorRectangleTool>& rect_tool, const std::shared_ptr<EditorCircleTool>& circle_tool)
		: System::TaskBase(world, "Editor"),
		m_PolygonTool(poly_tool),
		m_RectangleTool(rect_tool),
		m_CircleTool(circle_tool)
	{
	}

	void OverlayTask::Update()
	{
		for (auto it = m_Selected.begin(), end = m_Selected.end(); it != end; ++it)
		{
			const auto& entity = *it;

			auto pos = entity->GetPosition();
			pos.x = ToRenderUnits(pos.x), pos.y = ToRenderUnits(pos.y);
			pos += m_Offset;

			clan::Rectf box(clan::Sizef(50, 50));
			box.translate(pos.x - box.get_width() * 0.5f, pos.y - box.get_height() * 0.5f);

			m_DebugDraw.DrawRectangle(box, GetColour(it));
		}

		if (m_EditCam)
		{
			clan::Colorf rangeColour(1.0f, 0.6f, 0.6f, 0.95f);
			auto center = m_EditCam->GetPosition();
			auto radius = ToRenderUnits(m_CamRange);
			clan::Rectf camRect(center.x - radius, center.y - radius, center.x + radius, center.y + radius);
			m_DebugDraw.DrawRectangle(camRect, rangeColour);
		}

		if (m_PolygonTool && m_PolygonTool->IsActive())
			m_PolygonTool->Draw();
		if (m_RectangleTool && m_RectangleTool->IsActive())
			m_RectangleTool->Draw();
		if (m_CircleTool && m_CircleTool->IsActive())
			m_CircleTool->Draw();
	}

	class SelectionDrawerTask : public System::TaskBase
	{
	public:
		void Update() override;

		void SetSelectionBox(const clan::Rectf& box) { m_SelectionBox = box; }

		clan::Rectf m_SelectionBox;

		DebugDraw m_DebugDraw;
	};

	void SelectionDrawerTask::Update()
	{
		if (!fe_fzero(m_SelectionBox.get_width()) || !fe_fzero(m_SelectionBox.get_height()))
		{
			auto fillC = clan::Colorf::aquamarine;
			fillC.set_alpha(0.20f);
			m_DebugDraw.DrawRectangle(m_SelectionBox, clan::Colorf::white);
			m_DebugDraw.DrawSolidRectangle(m_SelectionBox, fillC);
		}
	}

	System::TaskList_t EditorWorld::MakeTasksList() const
	{
		System::TaskList_t taskList;
		taskList.push_back(*m_OverlayTask);
		taskList.push_back(*m_SelectionDrawerTask);
		return std::move(taskList);
	}

	void EditorWorld::SetSelectionBox(const clan::Rectf& box)
	{
		m_SelectionDrawerTask->SetSelectionBox(box);
	}
	
	void EditorWorld::SelectEntity(const EntityPtr& entity)
	{
		m_OverlayTask->m_Selected.insert(entity);
	}

	void EditorWorld::DeselectEntity(const EntityPtr& entity)
	{
		m_OverlayTask->m_Selected.erase(entity);
	}

	void EditorWorld::SetOffset(const Vector2& offset)
	{
		m_OverlayTask->m_Offset = offset;
	}

	void EditorWorld::ForEachSelected(std::function<void (const EntityPtr&)> fn)
	{
		for (auto it = m_OverlayTask->m_Selected.begin(); it != m_OverlayTask->m_Selected.end(); ++it)
		{
			fn(*it);
		}
	}

	void EditorWorld::ForEachSelectedWithColours(std::function<void (const EntityPtr&, const clan::Colorf&)> fn)
	{
		for (auto it = m_OverlayTask->m_Selected.begin(); it != m_OverlayTask->m_Selected.end(); ++it)
		{
			fn(*it, m_OverlayTask->GetColour(it));
		}
	}

}
