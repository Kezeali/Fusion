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
#include "FusionSpriteInspector.h"
#include "FusionASScriptInspector.h"

#include "FusionContextMenu.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <Rocket/Controls/DataSource.h>
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

	inline void RemoveEqualElems(const boost::filesystem::path& base, boost::filesystem::path& subdir)
	{
		boost::filesystem::path path;
		auto it = subdir.begin(), end = subdir.end();
		for (auto bit = base.begin(), bend = base.end(); it != end && bit != end; ++it, ++bit)
		{
			if (*bit != *it)
				break;
		}
		for (; it != end; ++it)
			path /= *it;
		subdir.swap(path);
	}

	inline std::string RemoveBasePath(const boost::filesystem::path& base, const std::string& subdir)
	{
		boost::filesystem::path subdirPath(subdir);
		RemoveEqualElems(base, subdirPath);
		return subdirPath.string();
	}

	class DialogListener : public Rocket::Core::EventListener
	{
	public:
		DialogListener(const std::function<void (const std::map<std::string, std::string>&)>& callback)
			: m_Callback(callback)
		{
		}

		void Attach(const boost::intrusive_ptr<Rocket::Core::ElementDocument>& document)
		{
			if (m_Document)
			{
				m_Document->RemoveEventListener("submit", this);
				m_Document->RemoveEventListener("close", this);
			}
			m_Document = document;
			if (m_Document)
			{
				m_Document->AddEventListener("submit", this);
				m_Document->AddEventListener("close", this);
			}
		}

		void SetCallback(const std::function<void (const std::map<std::string, std::string>&)>& callback)
		{
			m_Callback = callback;
		}

		void ProcessEvent(Rocket::Core::Event& e)
		{
			if (e == "submit")
			{
				std::map<std::string, std::string> results;

				auto params = e.GetParameters();
				Rocket::Core::String key, value;
				for (int pos = 0; params->Iterate(pos, key, value);)
				{
					results.insert(std::make_pair(key.CString(), value.CString()));
					value.Clear();
				}

				m_Callback(results);
			}
			else if (e == "close")
			{
				std::map<std::string, std::string> results;
				results["result"] = "cancel";
				m_Callback(results);
			}

			m_Document->Hide();
			m_Document->GetContext()->UnloadDocument(m_Document.get());
			m_Document.reset();
		}

		boost::intrusive_ptr<Rocket::Core::ElementDocument> m_Document;
		std::function<void (std::map<std::string, std::string>)> m_Callback;
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

	class EntitySelector : public Rocket::Core::Element
	{
	public:
		EntitySelector(const Rocket::Core::String& tag)
			: Rocket::Core::Element(tag)
		{
			Rocket::Core::XMLAttributes attributes;
			auto element = Rocket::Core::Factory::InstanceElement(this, "select", "select", attributes);FSN_ASSERT(element);
			if (m_Select = dynamic_cast<Rocket::Controls::ElementFormControlSelect*>(element))
			{
				AppendChild(m_Select.get());
			}
			element->RemoveReference();
		}

		void SetEntities(const std::vector<EntityPtr>& entities)
		{
			m_Entities = entities;
			size_t i = 0;
			for (auto it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it, ++i)
			{
				const auto& entity = *it;
				std::string name = entity->GetName();
				if (entity->IsSyncedEntity())
					name += "ID: " + boost::lexical_cast<std::string>(entity->GetID());
				if (name.empty() && entity->IsPseudoEntity())
					name += "Unnamed Pseudo-Entity " + boost::lexical_cast<std::string>(i);
				m_Select->Add(name.c_str(), name.c_str());
			}
		}

		typedef std::function<void (const EntityPtr&)> Callback_t;

		void SetCallback(Callback_t callback)
		{
			m_Callback = callback;
		}

		void ProcessEvent(Rocket::Core::Event& ev)
		{
			if (ev == "change")
			{
				auto selection = m_Select->GetSelection();
				if (selection >= 0 && (size_t)selection < m_Entities.size())
				{
					m_Callback(m_Entities[selection]);
				}
			}
		}

		std::vector<EntityPtr> m_Entities;
		boost::intrusive_ptr<Rocket::Controls::ElementFormControlSelect> m_Select;
		Callback_t m_Callback;
	};

	class InspectorSection : public Rocket::Core::Element/*, public Rocket::Core::EventListener*/
	{
	public:
		InspectorSection(const Rocket::Core::String& tag)
			: Rocket::Core::Element(tag),
			header(nullptr)
		{}

		void ProcessEvent(Rocket::Core::Event& ev)
		{
			if (ev.GetTargetElement() == header && ev == "click")
			{
				Rocket::Core::ElementList elements;
				GetElementsByTagName(elements, "collapsible");
				for (auto it = elements.begin(), end = elements.end(); it != end; ++it)
				{
					Rocket::Core::Element* elem = *it;
					auto currentStyle = elem->GetAttribute("style", Rocket::Core::String());
					if (currentStyle.Find(Rocket::Core::String("block")) != Rocket::Core::String::npos)
						elem->SetAttribute("style", "display: none;");
					else
						elem->SetAttribute("style", "display: block;");
				}
			}
		}

		Rocket::Core::Element* header;

		void OnChildAdd(Element* child)
		{
			if (child->GetTagName() == "header")
			{
				/*if (header)
					header->RemoveEventListener("click", this);*/
				header = child;
				//header->AddEventListener("click", this);
			}
		}

		void OnChildRemove(Element* child)
		{
			if (child == header)
			{
				//header->RemoveEventListener("click", this);
				header = nullptr;
			}
		}
	};

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

		m_SaveDialogListener = std::make_shared<DialogListener>([this](const std::map<std::string, std::string>& params)
		{
			if (params.at("result") == "ok")
			{
				auto path = params.at("path");
				if (!path.empty())
				{
					m_SaveName = (boost::filesystem::path(RemoveBasePath(PHYSFS_getWriteDir(), path)) / params.at("filename")).generic_string();
					m_SaveMap = true;
				}
			}
		});
		m_OpenDialogListener = std::make_shared<DialogListener>([this](const std::map<std::string, std::string>& params)
		{
			if (params.at("result") == "ok")
			{
				auto path = params.at("path");
				if (!path.empty())
				{
					m_SaveName = (boost::filesystem::path(RemoveBasePath(PHYSFS_getWriteDir(), path)) / params.at("filename")).generic_string();
					m_LoadMap = true;
				}
			}
		});

		//m_InspectorTypes.insert(std::make_pair(ITransform::GetTypeName(), []() { return std::make_shared<Inspectors::TransformInspector>(); }));
		{
			auto tag = "inspector_" + ITransform::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::TransformInspector>())->RemoveReference();

			tag = "inspector_" + ISprite::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::SpriteInspector>())->RemoveReference();

			Rocket::Core::Factory::RegisterElementInstancer("inspector_asscript",
				new Rocket::Core::ElementInstancerGeneric<Inspectors::ASScriptInspector>())->RemoveReference();
		}

		{
			std::string tag = "entity_selector";
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<EntitySelector>())->RemoveReference();

			tag = "inspector_section";
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<InspectorSection>())->RemoveReference();
		}

		m_RightClickMenu = boost::intrusive_ptr<ContextMenu>(new ContextMenu(m_GUIContext, true), false);
		m_PropertiesMenu = boost::intrusive_ptr<MenuItem>(new MenuItem("Properties", "properties"), false);
		m_RightClickMenu->AddChild(m_PropertiesMenu.get());
		m_EntitySelectionMenu = boost::intrusive_ptr<MenuItem>(new MenuItem("Select", "select"), false);
		m_RightClickMenu->AddChild(m_EntitySelectionMenu.get());
	}

	Editor::~Editor()
	{
		Console::getSingleton().UnbindCommand("le");
	}

	void Editor::CleanUp()
	{
		m_EntitySelectionMenu.reset();
		m_PropertiesMenu.reset();
		m_RightClickMenu.reset();
		// These need to be cleaned up before the GUI context is destroyed
		m_SaveDialogListener.reset();
		m_OpenDialogListener.reset();
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
		ScriptManager::getSingleton().RegisterGlobalObject("FilesystemDataSource filesystem_datasource", Rocket::Controls::DataSource::GetDataSource("filesystem"));
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

			if (m_RenderWorld)
				m_RenderWorld->SetPhysWorld(m_Box2DWorld->Getb2World());
		}
		else if (auto renderWorld = std::dynamic_pointer_cast<CLRenderWorld>(world))
		{
			m_RenderWorld = renderWorld;

			if (m_Box2DWorld)
				m_RenderWorld->SetPhysWorld(m_Box2DWorld->Getb2World());

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
			if (!v2Equal(m_CamVelocity, Vector2(0.f, 0.f)))
			{
				auto camPos = m_EditCam->GetPosition();
				auto camTrans = m_CamVelocity * dt;
				m_EditCam->SetPosition(camPos.x + camTrans.x, camPos.y + camTrans.y);

				if (m_CamVelocity.length() > 1.f)
					m_CamVelocity *= 0.5f * dt;
				else
					m_CamVelocity.x = m_CamVelocity.y = 0.f;
			}
		}

		for (auto it = m_ToDelete.begin(), end = m_ToDelete.end(); it != end; ++it)
			DeleteEntity(*it);
		m_ToDelete.clear();

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
			Save();
		}

		if (m_LoadMap)
		{
			m_LoadMap = false;
			Load();
		}
	}

	std::vector<std::shared_ptr<RendererExtension>> Editor::MakeRendererExtensions() const
	{
		return std::vector<std::shared_ptr<RendererExtension>>();
	}

	void Editor::GoToEntity(const EntityPtr& entity)
	{
		const auto entityPosition = entity->GetPosition();
		/*CL_Rectf area;
		Renderer::CalculateScreenArea(m_DisplayWindow.get_gc(), area, m_Viewport, true);
		area.left = ToSimUnits(area.left);
		area.top = ToSimUnits(area.top);
		area.right = ToSimUnits(area.right);
		area.bottom = ToSimUnits(area.bottom);*/
		m_EditCam->SetSimPosition(entityPosition);
	}

	void Editor::ShowSaveDialog()
	{
		auto document = m_GUIContext->LoadDocument("core/gui/file_dialog.rml");

		Rocket::Core::String title("Save Map");
		document->SetTitle(title);
		if (auto okButton = document->GetElementById("button_ok"))
			okButton->SetInnerRML("Save");
		if (auto fileList = document->GetElementById("file_list"))
			fileList->SetAttribute("source", "filesystem.#write_dir/Editor");

		m_SaveDialogListener->Attach(document);
		document->Show(Rocket::Core::ElementDocument::MODAL | Rocket::Core::ElementDocument::FOCUS);
		document->RemoveReference();
	}

	void Editor::ShowLoadDialog()
	{
		auto document = m_GUIContext->LoadDocument("core/gui/file_dialog.rml");

		Rocket::Core::String title("Open Map");
		document->SetTitle(title);
		if (auto okButton = document->GetElementById("button_ok"))
			okButton->SetInnerRML("Open");
		if (auto fileList = document->GetElementById("file_list"))
			fileList->SetAttribute("source", "filesystem.#write_dir/Editor");

		m_OpenDialogListener->Attach(document);
		document->Show(Rocket::Core::ElementDocument::MODAL | Rocket::Core::ElementDocument::FOCUS);
		document->RemoveReference();
	}

	void Editor::Save()
	{
		if (!m_SaveName.empty())
		{
			try
			{
				m_Saver->Save(m_SaveName, false);
			}
			catch (std::exception& e)
			{
				MessageBoxMaker::Show(Rocket::Core::GetContext("editor"), "error", std::string("title:Failed, message:") + e.what());
			}
		}
	}

	void Editor::Load()
	{
		if (!m_SaveName.empty())
		{
			try
			{
				m_Saver->Load(m_SaveName);
				m_NonStreamedEntities = m_EntityManager->GetNonStreamedEntities();
				m_StreamingManager->AddCamera(m_EditCam);
			}
			catch (std::exception& e)
			{
				MessageBoxMaker::Show(Rocket::Core::GetContext("editor"), "error", std::string("title:Failed, message:") + e.what());
			}
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

		for (int i = 0, num = m_GUIContext->GetNumDocuments(); i < num; ++i)
		{
			if (m_GUIContext->GetDocument(i)->IsPseudoClassSet("hover"))
				return;
		}

		if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_UP))
			m_CamVelocity.y = -400;
		if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_DOWN))
			m_CamVelocity.y = 400;
		if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_LEFT))
			m_CamVelocity.x = -400;
		if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_RIGHT))
			m_CamVelocity.x = 400;
	}

	void Editor::OnKeyUp(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (!m_Active)
			return;
		
		m_ShiftSelect = false;
		m_AltSelect = false;

		for (int i = 0, num = m_GUIContext->GetNumDocuments(); i < num; ++i)
		{
			if (m_GUIContext->GetDocument(i)->IsPseudoClassSet("hover"))
				return;
		}

		// Ctrl + Keys
		if (ev.ctrl)
		{
			switch (ev.id)
			{
			case CL_KEY_S:
				if (ev.shift || m_SaveName.empty())
					ShowSaveDialog();
				else
					m_SaveMap = true;
				break;
			case CL_KEY_O:
				ShowLoadDialog();
				break;
			}
		}
		// Keys
		else
		{
			switch (ev.id)
			{
			case CL_KEY_F3:
				m_RenderWorld->ToggleDebugDraw();
				break;
			case CL_KEY_F5:
				m_CompileMap = true;
				break;
			case CL_KEY_F7:
				m_RebuildScripts = true;
				break;
			case CL_KEY_P:
				{
					std::vector<EntityPtr> selectedEntities;
					ForEachSelected([&selectedEntities](const EntityPtr& entity)->bool {
						selectedEntities.push_back(entity);
						return true;
					});
					CreatePropertiesWindow(selectedEntities);
				}
				break;
			case CL_KEY_DELETE:
				if (ev.shift)
					ForEachSelected([this](const EntityPtr& entity)->bool { this->AddEntityToDelete(entity); return true; });
				else
				{
					std::vector<EntityPtr> selectedEntities;
					ForEachSelected([&selectedEntities](const EntityPtr& entity)->bool {
						selectedEntities.push_back(entity);
						return true;
					});
					//ShowDeleteDialog(selectedEntities);
				}
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
			if (m_ReceivedMouseDown)
			{
				OnMouseUp_Selection(ev);

				switch (ev.id)
				{
				case CL_MOUSE_RIGHT:
					ShowContextMenu(Vector2i(ev.mouse_pos.x, ev.mouse_pos.y), m_SelectedEntities);
					break;
				};
			}

			m_ReceivedMouseDown = false;
		}
	}

	void Editor::OnMouseMove(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (m_Active)
		{
			if (m_ReceivedMouseDown)
				UpdateSelectionRectangle(Vector2i(ev.mouse_pos.x, ev.mouse_pos.y), true);
		}
	}

	void Editor::OnMouseDown_Selection(const CL_InputEvent& ev)
	{
		m_DragFrom = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
		TranslateScreenToWorld(&m_DragFrom.x, &m_DragFrom.y);
		m_SelectionRectangle.right = m_SelectionRectangle.left = m_DragFrom.x;
		m_SelectionRectangle.bottom = m_SelectionRectangle.top = m_DragFrom.y;
	}

	void Editor::OnMouseUp_Selection(const CL_InputEvent& ev)
	{
		Vector2i mousePos(ReturnScreenToWorld((float)ev.mouse_pos.x, (float)ev.mouse_pos.y));
		// Detect click (no mouse movement between press and release)
		if (Vector2::distance(m_DragFrom, mousePos) <= Vector2(1.f, 1.f).length())
		{
			CL_Rectf nearRect(mousePos.x - 50.f, mousePos.y - 50.f, mousePos.x + 50.f, mousePos.y + 50.f);

			std::set<EntityPtr> entitiesUnselected;
			if (!m_ShiftSelect)
				entitiesUnselected = m_SelectedEntities;

			// Select entities found within the updated selection rectangle
			std::vector<EntityPtr> entitiesUnderMouse;
			GetEntitiesOverlapping(entitiesUnderMouse, nearRect, QueryType::General);
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
		}
		UpdateSelectionRectangle(mousePos, false);
	}

	void ClearCtxMenu(MenuItem *menu)
	{
		menu->RemoveAllChildren();
	}

	void AddMenuItem(MenuItem* parent, const std::string& title, const std::string& value, std::function<void (const MenuItemEvent&)> on_clicked)
	{
		MenuItem* item = new MenuItem(title, value);
		item->SignalClicked.connect(on_clicked);
		parent->AddChild(item);

		item->release();
	}

	void AddMenuItem(MenuItem* parent, const std::string& title, std::function<void (const MenuItemEvent&)> on_clicked)
	{
		AddMenuItem(parent, title, "", on_clicked);
	}

	void Editor::ShowContextMenu(const Vector2i& position, const std::set<EntityPtr>& entities)
	{
		ClearCtxMenu(m_PropertiesMenu.get());
		ClearCtxMenu(m_EntitySelectionMenu.get());

		AddMenuItem(m_EntitySelectionMenu.get(),
			"Deselect All",
			std::bind(&Editor::DeselectAll, this));

		for (auto it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const auto &entity = *it;

			const std::string title = entity->GetName().empty() ? std::string("Unnamed ") : entity->GetName() + "(" + entity->GetType() + ")";

			// Add an item for this entity to the Properties sub-menu
			AddMenuItem(m_PropertiesMenu.get(),
				title, entity->GetName(),
				[this, entity](const MenuItemEvent& e) { std::vector<EntityPtr> ents; ents.push_back(entity); CreatePropertiesWindow(ents); }
			);

			// Add an item for this entity to the Select sub-menu
			AddMenuItem(m_EntitySelectionMenu.get(),
				title, entity->GetName(),
				[this, entity](const MenuItemEvent& e) { if (!m_ShiftSelect) DeselectAll(); SelectEntity(entity); }
			);
		}

		m_RightClickMenu->Show(position.x, position.y);
	}

	void Editor::TranslateScreenToWorld(float* x, float* y) const
	{
		CL_Rectf area;
		Renderer::CalculateScreenArea(m_DisplayWindow.get_gc(), area, m_Viewport, true);
		*x += area.left; *y += area.top;
	}

	Vector2 Editor::ReturnScreenToWorld(float x, float y) const
	{
		TranslateScreenToWorld(&x, &y);
		return Vector2(x, y);
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

	void Editor::DeselectAll()
	{
		auto selectedEntities = m_SelectedEntities;
		for (auto it = selectedEntities.begin(), end = selectedEntities.end(); it != end; ++it)
			DeselectEntity(*it);
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
			{
				m_EntityManager->QueryRect([&out](const EntityPtr& ent)->bool { out.push_back(ent); return true; }, top_left, bottom_right);
				CL_Rectf simUnitsRectangle(top_left.x, top_left.y, bottom_right.x, bottom_right.y);
				for (auto it = m_NonStreamedEntities.begin(), end = m_NonStreamedEntities.end(); it != end; ++it)
				{
					auto pos = (*it)->GetPosition();
					if (simUnitsRectangle.contains(CL_Vec2f(pos.x, pos.y)))
						out.push_back(*it);
				}
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

	void Editor::DeleteEntity(const EntityPtr& entity)
	{
		DeselectEntity(entity);

		ClearCtxMenu(m_PropertiesMenu.get());
		ClearCtxMenu(m_EntitySelectionMenu.get());

		auto it = std::remove(m_NonStreamedEntities.begin(), m_NonStreamedEntities.end(), entity);
		if (it != m_NonStreamedEntities.end())
			m_NonStreamedEntities.erase(it, m_NonStreamedEntities.end());
		
		m_EntityManager->RemoveEntity(entity);
	}

	void Editor::AddEntityToDelete(const EntityPtr& entity)
	{
		m_ToDelete.push_back(entity);
	}

	void Editor::CreatePropertiesWindow(const std::vector<EntityPtr>& entities)
	{
		auto doc = m_GUIContext->LoadDocument("/core/gui/properties.rml");
		//{
		//	auto script = OpenString_PhysFS("/core/gui/gui_base.as");
		//	auto strm = new Rocket::Core::StreamMemory((const Rocket::Core::byte*)script.c_str(), script.size());
		//	doc->LoadScript(strm, "/core/gui/gui_base.as");
		//	strm->RemoveReference();
		//}

		class InspectorGenerator
		{
		public:
			Rocket::Core::ElementDocument* doc;

			std::map<std::string, std::pair<Inspectors::ComponentInspector*, std::vector<ComponentPtr>>> inspectors;

			boost::intrusive_ptr<EntitySelector> entity_selector;

			std::vector<EntityPtr> entities;

			InspectorGenerator(Rocket::Core::ElementDocument* doc_)
				: doc(doc_)
			{
				FSN_ASSERT(doc);

				auto element = doc->CreateElement("entity_selector"); FSN_ASSERT(element);
				if (entity_selector = dynamic_cast<EntitySelector*>(element))
				{
					auto body = doc->GetFirstChild()->GetElementById("content");
					body->AppendChild(entity_selector.get());
				}
				else
				{
					FSN_EXCEPT(Exception, "Failed to create entity_selector element.");
				}
				element->RemoveReference();
			}

			void ProcessEntity(const EntityPtr& entity)
			{
				entities.push_back(entity);

				const auto& components = entity->GetComponents();
				for (auto it = components.begin(), end = components.end(); it != end; ++it)
					ProcessComponent(*it);
			}

			void ProcessComponent(const ComponentPtr& component)
			{
				const bool added = AddInspector(component, component->GetType());

				if (!added) // If there wasn't a specific inspector for the given type, try adding interface inspectors
				{
					for (auto iit = component->GetInterfaces().begin(), iend = component->GetInterfaces().end(); iit != iend; ++iit)
						AddInspector(component, *iit);
				}
			}

			bool AddInspector(const ComponentPtr& component, const std::string& inspector_type)
			{
				const std::string inspector_identifier = inspector_type + component->GetIdentifier();
				auto entry = inspectors.find(inspector_identifier);
				if (entry == inspectors.end())
				{
					// New component / interface type
					auto tag = "inspector_" + inspector_type;
					fe_tolower(tag);
					Rocket::Core::String rocketTag(tag.data(), tag.data() + tag.length());
					auto element = Rocket::Core::Factory::InstanceElement(doc, rocketTag, "inspector", Rocket::Core::XMLAttributes());
					auto body = doc->GetFirstChild()->GetElementById("content");
					auto inspector = dynamic_cast<Inspectors::ComponentInspector*>(element);
					if (inspector)
					{
						auto& value = inspectors[inspector_identifier];
						value.first = inspector;
						value.second.push_back(component);

						auto name = component->GetType();

						// Subsection
						auto subsection = doc->CreateElement("inspector_section");
						// Title
						auto header = Rocket::Core::Factory::InstanceElement(subsection, "header", "header", Rocket::Core::XMLAttributes());
						subsection->AppendChild(header);
						Rocket::Core::Factory::InstanceElementText(header, Rocket::Core::String(name.data(), name.data() + name.length()));
						header->RemoveReference();
						// This element can be collapsed to hide the inspector
						auto p = Rocket::Core::Factory::InstanceElement(subsection, "collapsible", "collapsible", Rocket::Core::XMLAttributes());

						p->AppendChild(inspector);

						subsection->AppendChild(p);
						p->RemoveReference();

						body->AppendChild(subsection);
						subsection->RemoveReference();
					}
					if (element)
						element->RemoveReference();

					return inspector != nullptr;
				}
				else
				{
					// Existing type
					entry->second.second.push_back(component);
					return true;
				}
			}

			void Generate()
			{
				// Generate title
				std::string title;
				if (entities.size() > 1)
					title = boost::lexical_cast<std::string>(entities.size()) + " entities";
				else if (!entities.empty())
				{
					auto front = entities.front();
					if (front->GetName().empty() && front->IsPseudoEntity())
						title = "Unnamed Pseudo-Entity";
					else
					{
						title = front->GetName();
						if (front->IsSyncedEntity())
							title += " ID: " + boost::lexical_cast<std::string>(entities.size());
					}
				}
				if (!title.empty())
					doc->SetTitle(doc->GetTitle() + (": " + title).c_str());
				if (auto title = doc->GetElementById("title"))
					title->SetInnerRML(doc->GetTitle());

				// Set entities
				entity_selector->SetEntities(entities);

				for (auto it = inspectors.begin(), end = inspectors.end(); it != end; ++it)
				{
					it->second.first->SetComponents(it->second.second);
				}
			}
		};

		InspectorGenerator generator(doc);
		for (auto it = entities.begin(), end = entities.end(); it != end; ++it)
			generator.ProcessEntity(*it);
		generator.Generate();
		generator.entity_selector->SetCallback([this](const EntityPtr& entity) { this->GoToEntity(entity); });
		
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
