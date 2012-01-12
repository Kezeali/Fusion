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

#include "FusionAngelScriptSystem.h"
#include "FusionCLRenderSystem.h"
#include "FusionRenderer.h"

// TYPES (will be removed when createEntity is replaced with a script)
#include "FusionPhysicalComponent.h"
#include "FusionRender2DComponent.h"
#include "FusionScriptComponent.h"
#include "FusionTransformComponent.h"

#include <boost/lexical_cast.hpp>
#include <Rocket/Core/ElementDocument.h>

namespace FusionEngine
{

	void intrusive_ptr_add_ref(Rocket::Core::ReferenceCountable *ptr)
	{
		ptr->AddReference();
	}

	void intrusive_ptr_release(Rocket::Core::ReferenceCountable *ptr)
	{
		ptr->RemoveReference();
	}

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

	Editor::Editor(const std::vector<CL_String>& args)
		: m_RebuildScripts(false),
		m_CompileMap(false),
		m_SaveMap(false),
		m_LoadMap(false)
	{
		auto& context = GUI::getSingleton().CreateContext("editor");
		context->SetMouseShowPeriod(500);

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
	}

	Editor::~Editor()
	{
	}

	void Editor::SetDisplay(const CL_DisplayWindow& display)
	{
		m_DisplayWindow = display;
		m_KeyUpSlot = m_DisplayWindow.get_ic().get_keyboard().sig_key_up().connect(this, &Editor::onKeyUp);
	}

	void Editor::SetMapLoader(const std::shared_ptr<RegionMapLoader>& map_loader)
	{
		m_MapLoader = map_loader;
		// TODO: do this when the editor is enabled, undo when not
		m_MapLoader->SetSavePath("Editor/");
	}

	void Editor::OnWorldCreated(const std::shared_ptr<ISystemWorld>& world)
	{
		if (auto asw = std::dynamic_pointer_cast<AngelScriptWorld>(world))
		{
			SetAngelScriptWorld(asw);
		}
		else if (auto renderWorld = std::dynamic_pointer_cast<CLRenderWorld>(world))
		{
			m_RenderWorld = renderWorld;

			auto camera = std::make_shared<Camera>();
			camera->SetPosition(0.f, 0.f);
			auto viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
			m_RenderWorld->AddViewport(viewport);
			m_StreamingManager->AddCamera(camera);

			m_EditCam = camera;
		}
	}

	void Editor::Update(float time, float dt)
	{
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
		}

		if (m_CompileMap)
		{
			m_CompileMap = false;

			m_StreamingManager->StoreAllCells(false);
			m_MapLoader->Stop();
			try
			{
				IO::PhysFSStream file("default.gad", IO::Write);
				GameMap::CompileMap(file, m_StreamingManager->GetCellSize(), m_MapLoader->GetCellCache(), m_NonStreamedEntities);
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
			m_Saver->Save(m_SaveName);
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

	void Editor::onKeyUp(const CL_InputEvent& ev, const CL_InputState& state)
	{
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

			Vector2 pos((float)ev.mouse_pos.x, (float)ev.mouse_pos.y);
			CL_Rectf area;
			Renderer::CalculateScreenArea(m_DisplayWindow.get_gc(), area, vp, true);
			pos.x += area.left; pos.y += area.top;

			auto entity = createEntity(true, num, pos, m_EntityInstantiator.get(), m_ComponentFactory.get(), m_EntityManager.get());
			if (entity && entity->GetDomain() == SYSTEM_DOMAIN)
				m_NonStreamedEntities.push_back(entity);
		}
	}

}
