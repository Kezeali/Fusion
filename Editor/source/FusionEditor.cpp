/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionEditor.h"

#include "FusionCamera.h"
#include "FusionCellCache.h"
#include "FusionComponentFactory.h"
#include "FusionEntityInstantiator.h"
#include "FusionEntityManager.h"
#include "FusionGUI.h"
#include "FusionMessageBox.h"
#include "FusionRegionCellCache.h"
#include "FusionRegionMapLoader.h"
#include "FusionStreamingManager.h"
#include "FusionWorldSaver.h"
#include "FusionScriptTypeRegistrationUtils.h"

#include "FusionAngelScriptSystem.h"
#include "FusionBox2DSystem.h"
#include "FusionCLRenderSystem.h"
#include "FusionCLRenderExtension.h"
#include "FusionRenderer.h"

#include "FusionBox2DComponent.h"

// TYPES (will be removed when createEntity is replaced with a script)
#include "FusionPhysicalComponent.h"
#include "FusionRender2DComponent.h"
#include "FusionScriptComponent.h"
#include "FusionTransformComponent.h"

#include "FusionTransformInspector.h"

#include <boost/lexical_cast.hpp>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/StreamMemory.h>

void intrusive_ptr_add_ref(asIScriptFunction *ptr)
{
	ptr->AddRef();
}

void intrusive_ptr_release(asIScriptFunction *ptr)
{
	ptr->Release();
}

namespace FusionEngine
{

	class GUIDialog
	{
	public:
		virtual ~GUIDialog()
		{
			m_Document->RemoveReference();
		}

	protected:
		Rocket::Core::ElementDocument* m_Document;
	};

	//! Implements b2QueryCallback, returning all entities within an AABB that satisfy the passed function
	class EntityQuery : public b2QueryCallback
	{
	public:
		EntityQuery(std::vector<EntityPtr>* output_array, std::function<bool (b2Fixture*, bool&)> test_fn)
			: m_Test(test_fn),
			m_Entities(output_array)
		{
		}

		explicit EntityQuery(std::vector<EntityPtr>* output_array)
			: m_Entities(output_array)
		{
		}

		bool ReportFixture(b2Fixture* fixture)
		{
			bool continue_query = true;
			if (!m_Test || m_Test(fixture, continue_query))
			{
				auto* component = static_cast<Box2DFixture*>(fixture->GetUserData());
				if (component != nullptr)
				{
					m_Entities->push_back(component->GetParent()->shared_from_this());
				}
			}
			// Continue if the test func. said to
			return continue_query;
		}

		std::function<bool (b2Fixture*, bool&)> m_Test;
		std::vector<EntityPtr>* m_Entities;

	private:
		EntityQuery(const EntityQuery& other)
			: m_Test(other.m_Test),
			m_Entities(other.m_Entities)
		{}
		EntityQuery& operator= (const EntityQuery& other)
		{
			m_Test = other.m_Test;
			m_Entities = other.m_Entities;
			return *this;
		}
	};

	class EditorOverlay : public CLRenderExtension
	{
	public:
		void Draw(const CL_GraphicContext& gc);

		void SelectEntity(const EntityPtr& entity)
		{
			m_Selected.insert(entity);
		}

		void DeselectEntity(const EntityPtr& entity)
		{
			m_Selected.erase(entity);
		}

		std::set<EntityPtr> m_Selected;
	};

	void EditorOverlay::Draw(const CL_GraphicContext& gc_)
	{
		auto gc = gc_;

		for (auto it = m_Selected.begin(), end = m_Selected.end(); it != end; ++it)
		{
			const auto& entity = *it;
			auto pos = entity->GetPosition();
			pos.x = ToRenderUnits(pos.x), pos.y = ToRenderUnits(pos.y);
			CL_Rectf box(CL_Sizef(50, 50));
			box.translate(pos.x - box.get_width() * 0.5f, pos.y - box.get_height() * 0.5f);
			CL_Draw::box(gc, box, CL_Colorf::powderblue);
		}
	}

	class SelectionDrawer : public CLRenderExtension
	{
	public:
		void Draw(const CL_GraphicContext& gc);

		void SetSelectionBox(const CL_Rectf& box) { m_SelectionBox = box; }

		CL_Rectf m_SelectionBox;
	};

	void SelectionDrawer::Draw(const CL_GraphicContext& gc_)
	{
		auto gc = gc_;
		auto fillC = CL_Colorf::aquamarine;
		fillC.set_alpha(0.20f);
		CL_Draw::box(gc, m_SelectionBox, CL_Colorf::white);
		CL_Draw::fill(gc, m_SelectionBox, fillC);
	}

	Editor::Editor(const std::vector<CL_String>& args)
		: m_Active(false),
		m_RebuildScripts(false),
		m_CompileMap(false),
		m_SaveMap(false),
		m_LoadMap(false),
		m_ShiftSelect(false),
		m_AltSelect(false),
		m_ReceivedMouseDown(false)
	{
		auto& context = GUI::getSingleton().CreateContext("editor");
		context->SetMouseShowPeriod(500);

		m_GUIContext = context->m_Context;

		MessageBoxMaker::AddFactory("error",
			[](Rocket::Core::Context* context, const MessageBoxMaker::ParamMap& params)->MessageBox*
		{
			boost::intrusive_ptr<MessageBox> messageBox(new MessageBox(context, "core/gui/message_box.rml"));

			messageBox->SetType("error_message");
			messageBox->SetTitle(MessageBoxMaker::GetParam(params, "title"));
			messageBox->SetElement("message_label", MessageBoxMaker::GetParam(params, "message"));

			MessageBox* messageBoxRawPtr = messageBox.get();
			messageBox->GetEventSignal("accept_clicked").connect([messageBoxRawPtr](Rocket::Core::Event& ev) {
				messageBoxRawPtr->release();
			});

			return messageBox.get();
		});

		Console::getSingleton().BindCommand("le", [this](const std::vector<std::string>& cmdargs)->std::string
		{
			std::string retval = "Selected entities: ";
			for (auto it = this->m_SelectedEntities.begin(), end = this->m_SelectedEntities.end(); it != end; ++it)
				retval += "\n" + (*it)->GetName();
			return retval;
		});
		Console::getSingleton().SetCommandHelpText("le", "List selected entities", StringVector());

		m_EditorOverlay = std::make_shared<EditorOverlay>();
		m_SelectionDrawer = std::make_shared<SelectionDrawer>();

		//m_InspectorTypes.insert(std::make_pair(ITransform::GetTypeName(), []() { return std::make_shared<Inspectors::TransformInspector>(); }));
		{
			auto tag = "inspector_" + ITransform::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::TransformInspector>())->RemoveReference();
		}
	}

	Editor::~Editor()
	{
		Console::getSingleton().UnbindCommand("le");
	}

	void Editor::Activate()
	{
		m_Active = true;

		m_OriginalSavePath = m_MapLoader->GetSavePath();
		m_MapLoader->SetSavePath("Editor/");

		BuildCreateEntityScript();

		auto camera = std::make_shared<Camera>();
		camera->SetPosition(0.f, 0.f);
		m_Viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
		m_RenderWorld->AddViewport(m_Viewport);
		m_StreamingManager->AddCamera(camera);

		m_EditCam = camera;

		m_RenderWorld->AddRenderExtension(m_EditorOverlay, m_Viewport);
		m_RenderWorld->AddRenderExtension(m_SelectionDrawer, m_Viewport);
	}

	void Editor::Deactivate()
	{
		m_Active = false;

		m_MapLoader->SetSavePath(m_OriginalSavePath);
	}

	void Editor::SetDisplay(const CL_DisplayWindow& display)
	{
		m_DisplayWindow = display;
		m_KeyDownSlot = m_DisplayWindow.get_ic().get_keyboard().sig_key_down().connect(this, &Editor::OnKeyDown);
		m_KeyUpSlot = m_DisplayWindow.get_ic().get_keyboard().sig_key_up().connect(this, &Editor::OnKeyUp);
		m_MouseDownSlot = m_DisplayWindow.get_ic().get_mouse().sig_key_down().connect(this, &Editor::OnMouseDown);
		m_MouseUpSlot = m_DisplayWindow.get_ic().get_mouse().sig_key_up().connect(this, &Editor::OnMouseUp);
		m_MouseMoveSlot = m_DisplayWindow.get_ic().get_mouse().sig_pointer_move().connect(this, &Editor::OnMouseMove);
	}

	void Editor::SetMapLoader(const std::shared_ptr<RegionMapLoader>& map_loader)
	{
		m_MapLoader = map_loader;	
	}

	void Editor::SetAngelScriptWorld(const std::shared_ptr<AngelScriptWorld>& asw)
	{
		m_AngelScriptWorld = asw;

		ScriptManager::getSingleton().RegisterGlobalObject("Editor editor", this);
		if (m_Active)
			BuildCreateEntityScript();
	}

	void Editor::OnWorldCreated(const std::shared_ptr<ISystemWorld>& world)
	{
		if (auto asw = std::dynamic_pointer_cast<AngelScriptWorld>(world))
		{
			SetAngelScriptWorld(asw);
		}
		else if (auto b2World = std::dynamic_pointer_cast<Box2DWorld>(world))
		{
			m_Box2DWorld = b2World;
		}
		else if (auto renderWorld = std::dynamic_pointer_cast<CLRenderWorld>(world))
		{
			m_RenderWorld = renderWorld;

			if (m_Active)
			{
				auto camera = std::make_shared<Camera>();
				camera->SetPosition(0.f, 0.f);
				m_Viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
				m_RenderWorld->AddViewport(m_Viewport);
				m_StreamingManager->AddCamera(camera);

				m_EditCam = camera;
			}
		}
	}

	void Editor::Update(float time, float dt)
	{
		// Bodies have to be forced to create since the simulation isn't running
		m_Box2DWorld->InitialiseActiveComponents();

		if (m_EditCam)
		{
			auto camPos = m_EditCam->GetPosition();
			if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_UP))
				camPos.y -= 400 * dt;
			if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_DOWN))
				camPos.y += 400 * dt;
			if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_LEFT))
				camPos.x -= 400 * dt;
			if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_RIGHT))
				camPos.x += 400 * dt;

			m_EditCam->SetPosition(camPos.x, camPos.y);
		}

		if (m_RebuildScripts && m_AngelScriptWorld)
		{
			m_RebuildScripts = false;
			m_AngelScriptWorld->BuildScripts();

			BuildCreateEntityScript();
		}

		if (m_CompileMap)
		{
			m_CompileMap = false;

			m_StreamingManager->StoreAllCells(false);
			m_MapLoader->Stop();
			try
			{
				IO::PhysFSStream file("default.gad", IO::Write);
				GameMap::CompileMap(file, m_StreamingManager->GetCellSize(), m_MapLoader->GetCellCache(), m_NonStreamedEntities, m_EntityInstantiator.get());
				m_MapLoader->SaveEntityLocationDB("default.endb");
			}
			catch (FileSystemException& e)
			{
				SendToConsole("Failed to compile map: " + e.GetDescription());
				MessageBoxMaker::Show(Rocket::Core::GetContext("editor"), "error", "title:Compilation Failed, message:" + e.GetDescription());
			}
			m_MapLoader->Start();

			m_StreamingManager->Update(true);

			auto mb = MessageBoxMaker::Create(Rocket::Core::GetContext("editor"), "error", "title:Success, message:Compiled default.gad");
			mb->Show();
		}

		if (m_SaveMap)
		{
			m_SaveMap = false;
			m_SaveName = "save";
			Save();
		}

		if (m_LoadMap)
		{
			m_LoadMap = false;
			m_SaveName = "save";
			Load();
		}
	}

	std::vector<std::shared_ptr<RendererExtension>> Editor::MakeRendererExtensions() const
	{
		return std::vector<std::shared_ptr<RendererExtension>>();
	}

	void Editor::ShowSaveDialog()
	{
		//auto document = m_GUIContext->LoadDocument("core/gui/SaveDialog");
		//document->RemoveReference();
		//m_Dialog = std::make_shared<SaveDialog>(m_SaveName);
	}

	void Editor::ShowLoadDialog()
	{
	}

	void Editor::Save()
	{
		if (m_SaveName.empty())
		{
			ShowSaveDialog();
		}

		try
		{
			m_Saver->Save(m_SaveName, false);
		}
		catch (std::exception& e)
		{
			MessageBoxMaker::Show(Rocket::Core::GetContext("editor"), "error", std::string("title:Failed, message:") + e.what());
		}
	}

	void Editor::Load()
	{
		if (m_SaveName.empty())
		{
			ShowLoadDialog();
		}

		try
		{
			m_Saver->Load(m_SaveName);
			m_StreamingManager->AddCamera(m_EditCam);
		}
		catch (std::exception& e)
		{
			MessageBoxMaker::Show(Rocket::Core::GetContext("editor"), "error", std::string("title:Failed, message:") + e.what());
		}
	}

	EntityPtr createEntity(bool add_to_scene, unsigned int i, Vector2 position, EntityInstantiator* instantiator, ComponentFactory* factory, EntityManager* entityManager)
	{
		position.x = ToSimUnits(position.x); position.y = ToSimUnits(position.y);

		ComponentPtr transformCom;
		if (i == 1 || i == 2)
		{
			transformCom = factory->InstantiateComponent("StaticTransform");
		}
		else if (i == 4)
		{
			transformCom = factory->InstantiateComponent("b2Kinematic");
		}
		else
		{
			transformCom = factory->InstantiateComponent("b2RigidBody");
		}

		auto entity = std::make_shared<Entity>(entityManager, &entityManager->m_PropChangedQueue, transformCom);

		if (i == 2 || i == 3)
		{
			ObjectID id = 0;
			id = instantiator->GetFreeGlobalID();
			entity->SetID(id);

			std::stringstream str;
			str << i << "_" << id;
			entity->SetName("edit" + str.str());

			if (i == 2)
				entity->SetDomain(SYSTEM_DOMAIN);
		}
		//else
		//{
		//	std::stringstream str;
		//	str << reinterpret_cast<uintptr_t>(entity.get());
		//	entity->SetName("edit" + str.str());
		//}

		{
			auto transform = entity->GetComponent<ITransform>();
			transform->Position.Set(position);
			transform->Angle.Set(0.f);
		}

		if (add_to_scene)
			entityManager->AddEntity(entity);

		if (i == 1)
		{
			auto transform = entity->GetComponent<ITransform>();
			transform->Depth.Set(-1);
		}

		ComponentPtr b2CircleFixture;
		if (i == 3 || i == 4)
		{
			b2CircleFixture = factory->InstantiateComponent("b2Circle");
			entity->AddComponent(b2CircleFixture);
			{
				auto fixture = entity->GetComponent<FusionEngine::IFixture>();
				fixture->Density.Set(0.8f);
				fixture->Sensor.Set(i > 80);
				auto shape = entity->GetComponent<ICircleShape>();
				shape->Radius.Set(ToSimUnits(50.f / 2.f));
			}
			entity->SynchroniseParallelEdits();
		}

		auto clSprite = factory->InstantiateComponent("CLSprite");
		entity->AddComponent(clSprite);

		ComponentPtr asScript, asScript2;
		if (i == 4)
		{
			asScript = factory->InstantiateComponent("ASScript");
			entity->AddComponent(asScript, "script_a");
		}

		if (i == 2)
		{
			asScript2 = factory->InstantiateComponent("ASScript");
			entity->AddComponent(asScript2, "spawn_script");
		}

		if (i == 4)
		{
			auto transform = entity->GetComponent<ITransform>();
			transform->Depth.Set(1);
		}

		{
			auto sprite = entity->GetComponent<ISprite>();
			if (i == 1)
			{
				sprite->ImagePath.Set("Entities/Dirt.png");
			}
			else if (i == 3)
			{
				sprite->ImagePath.Set("Entities/Test/Gfx/spaceshoot_body_moving.png");
				sprite->AnimationPath.Set("Entities/Test/test_anim.yaml");
			}
			else
				sprite->ImagePath.Set("Entities/Test/Gfx/spaceshoot_body_moving1.png");
		}

		if (i == 5)
		{
			auto script = entity->GetComponent<IScript>("script_a");
			if (script)
				script->ScriptPath.Set("Scripts/test_script.as");
		}
		if (i == 2)
		{
			auto script = entity->GetComponent<IScript>("spawn_script");
			if (script)
				script->ScriptPath.Set("Scripts/SpawnPoint.as");
		}
		entity->SynchroniseParallelEdits();

		{
			auto body = entity->GetComponent<IRigidBody>();
			if (body)
			{
				//body->ApplyTorque(10.f);
				//body->ApplyForce(Vector2(2000, 0), body->GetCenterOfMass() + Vector2(2, -1));
				//body->AngularVelocity.Set(CL_Angle(180, cl_degrees).to_radians());
				body->LinearDamping.Set(0.1f);
				body->AngularDamping.Set(0.9f);
			}
		}

		return entity;
	}

	void Editor::OnKeyDown(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (!m_Active)
			return;

		if (ev.shift)
			m_ShiftSelect = true;
		if (ev.alt)
			m_AltSelect = true;
	}

	void Editor::OnKeyUp(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (!m_Active)
			return;

		m_ShiftSelect = false;
		m_AltSelect = false;

		// Ctrl + Keys
		if (ev.ctrl)
		{
			switch (ev.id)
			{
			case CL_KEY_S:
				m_SaveMap = true;
				break;
			case CL_KEY_O:
				m_LoadMap = true;
				break;
			}
		}
		// Keys
		else
		{
			switch (ev.id)
			{
			case CL_KEY_F7:
				m_RebuildScripts = true;
				break;
			case CL_KEY_F5:
				m_CompileMap = true;
				break;
			case CL_KEY_P:
				ForEachSelected([this](const EntityPtr& entity)->bool {
					this->CreatePropertiesWindow(entity);
					return true;
				});
				break;
			}
		}
		// Numbers
		if (ev.id >= CL_KEY_0 && ev.id <= CL_KEY_9)
		{
			unsigned int num = (unsigned int)(ev.id - CL_KEY_0);

			auto vps = m_RenderWorld->GetViewports();
			if (vps.empty())
				return;
			auto vp = vps.front();

			srand(CL_System::get_time());

			bool randomAngle = ev.shift;
			
			auto caller = ScriptUtils::Calling::Caller::CallerForGlobalFuncId(ScriptManager::getSingleton().GetEnginePtr(), m_CreateEntityFn->GetId());
			if (caller)
			{
				CL_Rectf area;
				Renderer::CalculateScreenArea(m_DisplayWindow.get_gc(), area, vp, true);

				Vector2 pos((float)ev.mouse_pos.x, (float)ev.mouse_pos.y);
				pos.x += area.left; pos.y += area.top;

				if (m_SelectionRectangle.contains(CL_Vec2f(pos.x, pos.y)))
				{
					for (pos.y = m_SelectionRectangle.top; pos.y < m_SelectionRectangle.bottom; pos.y += 100)
						for (pos.x = m_SelectionRectangle.left; pos.x < m_SelectionRectangle.right; pos.x += 100)
						{
							Vector2 simPos(ToSimUnits(pos.x), ToSimUnits(pos.y));

							float angle = 0.f;
							if (randomAngle)
								angle = 2.f * s_pi * (rand() / (float)RAND_MAX);

							caller(num, &simPos, angle);
						}
				}
				else
				{
					pos.x = ToSimUnits(pos.x);
					pos.y = ToSimUnits(pos.y);

					float angle = 0.f;

					caller(num, &pos, angle);
				}
			}
		}
	}

	void Editor::OnMouseDown(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (m_Active)
		{
			//[process global inputs here]

			for (int i = 0, num = m_GUIContext->GetNumDocuments(); i < num; ++i)
			{
				if (m_GUIContext->GetDocument(i)->IsPseudoClassSet("hover"))
					return;
			}

			m_ReceivedMouseDown = true;

			OnMouseDown_Selection(ev);
		}
	}

	void Editor::OnMouseUp(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (m_Active)
		{
			m_ReceivedMouseDown = false;
		}
	}

	void Editor::OnMouseMove(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (m_Active)
		{
			if (m_ReceivedMouseDown)
				UpdateSelectionRectangle(Vector2(ev.mouse_pos.x, ev.mouse_pos.y), true);
		}
	}

	void Editor::OnMouseDown_Selection(const CL_InputEvent& ev)
	{
		m_DragFrom = Vector2(ev.mouse_pos.x, ev.mouse_pos.y);
		TranslateScreenToWorld(&m_DragFrom.x, &m_DragFrom.y);
		m_SelectionRectangle.right = m_SelectionRectangle.left = m_DragFrom.x;
		m_SelectionRectangle.bottom = m_SelectionRectangle.top = m_DragFrom.y;
	}

	void Editor::TranslateScreenToWorld(float* x, float* y) const
	{
		CL_Rectf area;
		Renderer::CalculateScreenArea(m_DisplayWindow.get_gc(), area, m_Viewport, true);
		*x += area.left; *y += area.top;
	}

	Vector2 Editor::ReturnScreenToWorld(float x, float y) const
	{
		CL_Rectf area;
		Renderer::CalculateScreenArea(m_DisplayWindow.get_gc(), area, m_Viewport, true);
		return Vector2(x + area.left, y + area.top);
	}

	void Editor::UpdateSelectionRectangle(const Vector2& pointer_position, bool translate_position)
	{
		float oldWidth = std::abs(m_SelectionRectangle.get_width());
		float oldHeight = std::abs(m_SelectionRectangle.get_height());

		m_SelectionRectangle.right = pointer_position.x;
		m_SelectionRectangle.bottom = pointer_position.y;
		if (translate_position)
			TranslateScreenToWorld(&m_SelectionRectangle.right, &m_SelectionRectangle.bottom);

		std::set<EntityPtr> entitiesUnselected;
		if (!m_ShiftSelect && (std::abs(m_SelectionRectangle.get_width()) < oldWidth || std::abs(m_SelectionRectangle.get_height()) < oldHeight))
			entitiesUnselected = m_SelectedEntities;

		// Select entities found within the updated selection rectangle
		std::vector<EntityPtr> entitiesUnderMouse;
		GetEntitiesOverlapping(entitiesUnderMouse, m_SelectionRectangle, QueryType::General);
		for (auto it = entitiesUnderMouse.begin(), end = entitiesUnderMouse.end(); it != end; ++it)
		{
			const EntityPtr& map_entity = *it;
			if (!m_ShiftSelect)
				entitiesUnselected.erase(map_entity);
			if (!m_AltSelect)
				SelectEntity(map_entity);
			else
				DeselectEntity(map_entity);
		}
		// Deselect all the entities not found in the updated rectangle
		for (auto it = entitiesUnselected.begin(), end = entitiesUnselected.end(); it != end; ++it)
		{
			const EntityPtr& map_entity = *it;
			if (!m_AltSelect)
				DeselectEntity(map_entity);
			else
				SelectEntity(map_entity);
		}

		m_SelectionDrawer->SetSelectionBox(m_SelectionRectangle);
	}

	void Editor::SelectEntity(const EntityPtr& entity)
	{
		m_SelectedEntities.insert(entity);
		m_EditorOverlay->SelectEntity(entity);
	}

	void Editor::DeselectEntity(const EntityPtr& entity)
	{
		m_SelectedEntities.erase(entity);
		m_EditorOverlay->DeselectEntity(entity);
	}

	void Editor::ForEachSelected(std::function<bool (const EntityPtr&)> fn)
	{
		for (auto it = m_EditorOverlay->m_Selected.begin(), end = m_EditorOverlay->m_Selected.end(); it != end; ++it)
		{
			if (!fn(*it))
				break;
		}
	}

	void Editor::GetEntitiesOverlapping(std::vector<EntityPtr> &out, const CL_Rectf &rectangle, const Editor::QueryType query_type)
	{
		// Figure out where the rectangle is in the world
		Vector2 top_left(ToSimUnits(std::min(rectangle.left, rectangle.right)), ToSimUnits(std::min(rectangle.top, rectangle.bottom)));
		Vector2 bottom_right(ToSimUnits(std::max(rectangle.left, rectangle.right)), ToSimUnits(std::max(rectangle.top, rectangle.bottom)));

		switch (query_type)
		{
		case QueryType::General:
			m_EntityManager->QueryRect([&out](const EntityPtr& ent)->bool { out.push_back(ent); return true; }, top_left, bottom_right);
			for (auto it = m_NonStreamedEntities.begin(), end = m_NonStreamedEntities.end(); it != end; ++it)
			{
				auto pos = (*it)->GetPosition();
				if (rectangle.contains(CL_Vec2f(pos.x, pos.y)))
					out.push_back(*it);
			}
			break;

		case QueryType::Physical:
			{
				// Make an AABB for the given rect
				b2AABB aabb;
				aabb.lowerBound.Set(top_left.x, top_left.y);
				aabb.upperBound.Set(bottom_right.x, bottom_right.y);

				// Query the world for overlapping shapes
				EntityQuery callback(&out, [](b2Fixture*, bool&) {return true;});
				m_Box2DWorld->Getb2World()->QueryAABB(&callback, aabb);
			}
			break;
		};
	}

	void Editor::BuildCreateEntityScript()
	{
		auto& scriptManager = ScriptManager::getSingleton();

		m_CreateEntityFn.reset();

		auto module = scriptManager.GetModule("core_create_entity");
		// Load the script file
		auto script = OpenString_PhysFS("/core/create_entity.as");
		// Generate and add the basecode section (also preprocess the script)
		int r = module->AddCode("basecode", m_AngelScriptWorld->GenerateBaseCodeForScript(script));
		FSN_ASSERT(r >= 0);
		// Add the pre-processed script
		r = module->AddCode("/core/create_entity.as", script);
		FSN_ASSERT(r >= 0);
		// Attempt to build
		r = module->Build();
		if (r < 0)
			SendToConsole("Failed to build /core/create_entity.as");

		m_CreateEntityFn = module->GetASModule()->GetFunctionByName("createEntity");
	}

	EntityPtr Editor::CreateEntity(const std::string& transform_type, const Vector2& position, float angle, bool synced, bool streaming)
	{
		ComponentPtr transformCom = m_ComponentFactory->InstantiateComponent(transform_type);

		auto entity = std::make_shared<Entity>(m_EntityManager.get(), &m_EntityManager->m_PropChangedQueue, transformCom);

		ObjectID id = 0;
		if (synced)
			id = m_EntityInstantiator->GetFreeGlobalID();
		entity->SetID(id);

		if (!streaming)
		{
			entity->SetDomain(SYSTEM_DOMAIN);
			// TODO: limit max non-streamed-entities
			m_NonStreamedEntities.push_back(entity);
		}

		{
			auto transform = entity->GetComponent<ITransform>();
			transform->Position.Set(position);
			transform->Angle.Set(angle);
		}

		m_EntityManager->AddEntity(entity);

		return entity;
	}

	void Editor::CreatePropertiesWindow(const EntityPtr& entity)
	{
		auto doc = m_GUIContext->LoadDocument("/core/gui/properties.rml");
		{
			auto script = OpenString_PhysFS("/core/gui/gui_base.as");
			auto strm = new Rocket::Core::StreamMemory((const Rocket::Core::byte*)script.c_str(), script.size());
			doc->LoadScript(strm, "/core/gui/gui_base.as");
			strm->RemoveReference();
		}

		auto component = entity->GetTransform();
		//auto subsection = doc->CreateElement("inspector_section");
		//subsection->CreateTextElement(component->GetType());
		for (auto iit = component->GetInterfaces().begin(), iend = component->GetInterfaces().end(); iit != iend; ++iit)
		{
			auto tag = "inspector_" + *iit;
			fe_tolower(tag);
			auto element = doc->CreateElement(tag.c_str());
			auto body = doc->GetFirstChild();
			auto inspector = dynamic_cast<Inspectors::ComponentInspector*>(element);
			if (inspector)
			{
				inspector->SetComponent(component);
				body->AppendChild(inspector);
			}
			if (element)
				element->RemoveReference();
		}
		
		doc->Show();
		doc->RemoveReference();
	}

	void Editor::RegisterScriptType(asIScriptEngine* engine)
	{
		int r;
		RegisterSingletonType<Editor>("Editor", engine);
		r = engine->RegisterObjectMethod("Editor", "Entity CreateEntity(const string &in, const Vector &in, float, bool, bool)", asMETHOD(Editor, CreateEntity), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

}
