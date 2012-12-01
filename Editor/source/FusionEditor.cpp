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

#include "FusionArchetypeFactory.h"
#include "FusionArchetypalEntityManager.h"
#include "FusionArchetypeResourceEditor.h"
#include "FusionCamera.h"
#include "FusionCellCache.h"
#include "FusionComponentFactory.h"
#include "FusionEditorCircleTool.h"
#include "FusionEditorPolygonTool.h"
#include "FusionEditorRectangleTool.h"
#include "FusionEntityInstantiator.h"
#include "FusionEntityManager.h"
#include "FusionFilesystemDataSource.h"
#include "FusionGUI.h"
#include "FusionMessageBox.h"
#include "FusionClientOptions.h"
#include "FusionPolygonResourceEditor.h"
#include "FusionRegionCellCache.h"
#include "FusionRegionMapLoader.h"
#include "FusionResource.h"
#include "FusionResourceDatabase.h"
#include "FusionStreamingManager.h"
#include "FusionWorldSaver.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionBinaryStream.h"

#include "FusionAngelScriptSystem.h"
#include "FusionBox2DSystem.h"
#include "FusionCLRenderSystem.h"
#include "FusionCLRenderExtension.h"
#include "FusionRenderer.h"

#include "FusionBox2DComponent.h"

#include "FusionElementInspectorGroup.h"

#include "FusionTransformInspector.h"
#include "FusionRigidBodyInspector.h"
#include "FusionFixtureInspector.h"
#include "FusionCircleShapeInspector.h"
#include "FusionPolygonShapeInspector.h"
#include "FusionSpriteInspector.h"
#include "FusionASScriptInspector.h"

#include "FusionEntityInspector.h"

#include "FusionContextMenu.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <Rocket/Controls/DataFormatter.h>
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

	class ResourceBrowserDataSource : public Rocket::Controls::DataSource
	{
	public:
		ResourceBrowserDataSource(const std::shared_ptr<ResourceDatabase>& database)
			: Rocket::Controls::DataSource("resource_browser"),
			filesystem_ds(nullptr),
			m_ResourceDatabase(database)
		{
			using namespace std::placeholders;
			m_OnResourceLoadedConnection =
				ResourceManager::getSingleton().SignalResourceLoaded.connect(std::bind(&ResourceBrowserDataSource::OnResourceLoaded, this, _1));

			filesystem_ds = dynamic_cast<FilesystemDataSource*>(Rocket::Controls::DataSource::GetDataSource("filesystem"));
		}
		~ResourceBrowserDataSource()
		{
			m_OnResourceLoadedConnection.disconnect();
		}

		void OnResourceLoaded(const ResourceDataPtr& resource);

	private:
		void GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns);
		int GetNumRows(const Rocket::Core::String& table);

		FilesystemDataSource* filesystem_ds;

		std::shared_ptr<ResourceDatabase> m_ResourceDatabase;

		boost::signals2::connection m_OnResourceLoadedConnection;
	};

	void ResourceBrowserDataSource::OnResourceLoaded(const ResourceDataPtr& resource)
	{
		if (filesystem_ds)
		{
			try
			{
				auto entry = filesystem_ds->ReverseLookup(resource->GetPath());
				NotifyRowChange(entry.first, entry.second, 1);
			}
			catch (FileSystemException&)
			{
			}
		}
	}

	void ResourceBrowserDataSource::GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns)
	{
		if (filesystem_ds)
		{
			filesystem_ds->GetRow(row, table, row_index, columns);
			for (auto it = columns.begin(); it != columns.end(); ++it)
			{
				if (*it == "resource_type")
				{
					const std::string path = filesystem_ds->GetPath(filesystem_ds->PreproPath(table.CString()), row_index);
					const std::string type = m_ResourceDatabase->GetResourceType(path);
					row.insert(row.begin() + std::distance(columns.begin(), it), Rocket::Core::String(type.c_str()));
				}
			}
		}
	}

	int ResourceBrowserDataSource::GetNumRows(const Rocket::Core::String& table)
	{
		if (filesystem_ds)
		{
			return filesystem_ds->GetNumRows(table);
		}
		else
			return 0;
	}

	class PreviewFormatter : public Rocket::Controls::DataFormatter
	{
	public:
		PreviewFormatter()
			: Rocket::Controls::DataFormatter("resource_preview")
		{}

	private:
		void FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data)
		{
			if (!raw_data.empty())
			{
				formatted_data =
					"<span style=\"icon-decorator: image; icon-image:" + raw_data[0] + ";\" "
					"onmouseover=\"%this:GeneratePreviewPopup('" + raw_data[0] + "', event);\" "
					"onmouseover=\"%this:HidePreviewPopup('" + raw_data[0] + "', event);\">" +
					raw_data[0] +
					"</span>";
			}
		}
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
		EditorOverlay(const std::shared_ptr<EditorPolygonTool>& poly_tool, const std::shared_ptr<EditorRectangleTool>& rect_tool, const std::shared_ptr<EditorCircleTool>& circle_tool)
			: m_PolygonTool(poly_tool),
			m_RectangleTool(rect_tool),
			m_CircleTool(circle_tool)
		{
		}

		void Draw(const CL_GraphicContext& gc);

		void SelectEntity(const EntityPtr& entity)
		{
			m_Selected.insert(entity);
		}

		void DeselectEntity(const EntityPtr& entity)
		{
			m_Selected.erase(entity);
		}

		void SetOffset(const Vector2& offset)
		{
			m_Offset = offset;
		}

		CL_Colorf GetColour(const std::set<EntityPtr>::const_iterator& it) const
		{
			const auto fraction = std::distance(it, m_Selected.end()) / (float)m_Selected.size();
			return CL_ColorHSVf(fraction * 360.f, 0.8f, 0.8f, 1.0f);
		}

		CL_Colorf GetColour(const EntityPtr& entity) const
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

	void EditorOverlay::Draw(const CL_GraphicContext& gc_)
	{
		auto gc = gc_;

		for (auto it = m_Selected.begin(), end = m_Selected.end(); it != end; ++it)
		{
			const auto& entity = *it;

			auto pos = entity->GetPosition();
			pos.x = ToRenderUnits(pos.x), pos.y = ToRenderUnits(pos.y);
			pos += m_Offset;

			CL_Rectf box(CL_Sizef(50, 50));
			box.translate(pos.x - box.get_width() * 0.5f, pos.y - box.get_height() * 0.5f);

			CL_Draw::box(gc, box, GetColour(it));
		}

		if (m_EditCam)
		{
			CL_Colorf rangeColour(1.0f, 0.6f, 0.6f, 0.95f);
			auto center = m_EditCam->GetPosition();
			auto radius = ToRenderUnits(m_CamRange);
			CL_Rectf camRect(center.x - radius, center.y - radius, center.x + radius, center.y + radius);
			CL_Draw::box(gc, camRect, rangeColour);
		}

		if (m_PolygonTool && m_PolygonTool->IsActive())
			m_PolygonTool->Draw(gc);
		if (m_RectangleTool && m_RectangleTool->IsActive())
			m_RectangleTool->Draw(gc);
		if (m_CircleTool && m_CircleTool->IsActive())
			m_CircleTool->Draw(gc);
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
		if (!fe_fzero(m_SelectionBox.get_width()) || !fe_fzero(m_SelectionBox.get_height()))
		{
			auto gc = gc_;
			auto fillC = CL_Colorf::aquamarine;
			fillC.set_alpha(0.20f);
			CL_Draw::box(gc, m_SelectionBox, CL_Colorf::white);
			CL_Draw::fill(gc, m_SelectionBox, fillC);
		}
	}

	class EntitySelector : public Rocket::Core::Element
	{
	public:
		EntitySelector(const Rocket::Core::String& tag)
			: Rocket::Core::Element(tag)
		{
			Rocket::Core::XMLAttributes attributes;
			auto element = Rocket::Core::Factory::InstanceElement(this, "select", "select", attributes); FSN_ASSERT(element);
			if (m_Select = dynamic_cast<Rocket::Controls::ElementFormControlSelect*>(element))
			{
				AppendChild(m_Select.get());
			}
			element->RemoveReference();

			element = Rocket::Core::Factory::InstanceElement(this, "button", "button", attributes); FSN_ASSERT(element);
			AppendChild(element);
			Rocket::Core::Factory::InstanceElementText(element, "Go");
			element->RemoveReference();
		}
		~EntitySelector()
		{
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
			Rocket::Core::Element::ProcessEvent(ev);
			if (ev == "change" || (ev.GetTargetElement()->GetTagName() == "button" && ev == "click"))
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
			Rocket::Core::Element::ProcessEvent(ev);
			if (ev.GetTargetElement() == header && ev == "click")
			{
				Rocket::Core::ElementList elements;
				GetElementsByTagName(elements, "collapsible");
				for (auto it = elements.begin(), end = elements.end(); it != end; ++it)
				{
					Rocket::Core::Element* elem = *it;
					elem->SetPseudoClass("collapsed", !elem->IsPseudoClassSet("collapsed"));
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

	class DockedWindowManager
	{
	public:
		DockedWindowManager(Editor* editor)
			: m_Listener(editor)
		{
			// TODO: add an OnResize method (called by the editor's OnWindowResize handler)
			//  that updates the docked window sizes to relative to the window (requires another
			//  setting for each window defining their resize mode - static, relative, fill edge)
		}

		~DockedWindowManager()
		{
		}

		enum Side
		{
			Left,
			Top,
			Right,
			Bottom
		};

		void AddWindow(Rocket::Core::ElementDocument* window, Side dock_side)
		{
			m_Listener.AddWindow(window, dock_side);
		}

		void RemoveWindow(Rocket::Core::ElementDocument* window)
		{
			m_Listener.RemoveWindow(window);
		}

	private:
		class EventListener : public Rocket::Core::EventListener
		{
		public:
			EventListener(Editor* editor)
				: m_Editor(editor)
			{
				m_Editor->GetGUIContext()->AddEventListener("resize", this);
			}

			~EventListener()
			{
				if (m_Editor->GetGUIContext())
					m_Editor->GetGUIContext()->RemoveEventListener("resize", this);

				auto attached = m_AttachedDocuments;
				m_AttachedDocuments.clear();
				for (auto it = attached.begin(); it != attached.end(); ++it)
				{
					it->first->RemoveEventListener("close", this);
					it->first->RemoveEventListener("hide", this);
					it->first->RemoveEventListener("resize", this);
				}
			}

			void AddWindow(Rocket::Core::ElementDocument* window, DockedWindowManager::Side dock_side)
			{
				auto r = m_AttachedDocuments.insert(std::make_pair(window, dock_side));
				FSN_ASSERT(r.second); // don't allow duplicates

				window->AddEventListener("close", this);
				window->AddEventListener("hide", this);
				window->AddEventListener("resize", this);
			}

			void RemoveWindow(Rocket::Core::ElementDocument* window)
			{
				window->RemoveEventListener("close", this);
				window->RemoveEventListener("hide", this);
				window->RemoveEventListener("resize", this);
				m_AttachedDocuments.erase(window);
			}

			void ShrinkArea(boost::intrusive_ptr<Rocket::Core::ElementDocument> document, DockedWindowManager::Side side)
			{
				switch (side)
				{
				case DockedWindowManager::Left:
					m_Area.left = std::max(m_Area.left, document->GetOffsetWidth() / m_Editor->GetGUIContext()->GetDimensions().x);
					break;
				case DockedWindowManager::Top:
					m_Area.top = std::max(m_Area.top, document->GetOffsetHeight() / m_Editor->GetGUIContext()->GetDimensions().y);
					break;
				case DockedWindowManager::Right:
					m_Area.right = std::min(m_Area.right, document->GetOffsetLeft() / m_Editor->GetGUIContext()->GetDimensions().x);
					break;
				case DockedWindowManager::Bottom:
					m_Area.bottom = std::min(m_Area.bottom, document->GetOffsetTop() / m_Editor->GetGUIContext()->GetDimensions().y);
					break;
				};
			}

			void ProcessEvent(Rocket::Core::Event& ev)
			{
				FSN_ASSERT(m_Editor);

				if (ev == "resize" || ev == "hide" || ev == "close")
				{
					m_Area.left = m_Area.top = 0.f;
					m_Area.right = m_Area.bottom = 1.f;

					for (auto it = m_AttachedDocuments.begin(); it != m_AttachedDocuments.end(); ++it)
					{
						if (it->first->IsVisible())
							ShrinkArea(it->first, it->second);
					}

					m_Editor->GetViewport()->SetArea(m_Area);
				}
			}

			void OnDetach(Rocket::Core::Element* element)
			{
				if (auto window = dynamic_cast<Rocket::Core::ElementDocument*>(element))
					m_AttachedDocuments.erase(window);
			}

			Editor* m_Editor;

			CL_Rectf m_Area;

			std::map<Rocket::Core::ElementDocument*, DockedWindowManager::Side> m_AttachedDocuments;
		};

		EventListener m_Listener;
	};

	inline bool MouseOverUI(Rocket::Core::Context* context)
	{
		for (int i = 0, num = context->GetNumDocuments(); i < num; ++i)
		{
			auto doc = context->GetDocument(i);
			if (doc->GetTitle() != "background" && doc->IsPseudoClassSet("hover"))
				return true;
		}
		if (GUI::getSingleton().GetConsoleWindow()->IsPseudoClassSet("hover"))
			return true;
		return false;
	}

	Editor::Editor(const std::vector<CL_String>& args)
		: m_EditCamRange(-1.f),
		m_Active(false),
		m_RebuildScripts(false),
		m_CompileMap(false),
		m_SaveMap(false),
		m_LoadMap(false),
		m_ShiftSelect(false),
		m_AltSelect(false),
		m_ReceivedMouseDown(false),
		m_Dragging(false),
		m_Tool(Tool::None)
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

		Console::getSingleton().BindCommand("editor_resourcedb_reset", [this](const std::vector<std::string>& cmdargs)->std::string
		{
			if (this->m_ResourceDatabase)
			{
				this->m_ResourceDatabase->Clear();
				return "resource database cleared";
			}
			else return "resource database not initialised";
		});
		Console::getSingleton().SetCommandHelpText("editor_resourcedb_reset", "Clear stored resource types", StringVector());

		Console::getSingleton().BindCommand("editor_resourcedb_delete", [this](const std::vector<std::string>& cmdargs)->std::string
		{
			if (this->m_ResourceDatabase)
			{
				if (cmdargs.size() >= 2)
				{
					if (this->m_ResourceDatabase->RemoveResourceType(cmdargs[1]))
						return cmdargs[1] + "removed from database";
					else return cmdargs[1] + "not found";
				}
				return "";
			}
			else return "resource database not initialised";
		});
		Console::getSingleton().SetCommandHelpText("editor_resourcedb_delete", "Remove the stored type for the given resource", StringVector());

		{
			Console::getSingleton().BindCommand("resource_load", [](const std::vector<std::string>& cmdargs)->std::string
			{
				if (cmdargs.size() >= 3)
				{
					auto con = ResourceManager::getSingleton().GetResource(cmdargs[2], cmdargs[1], ResourceContainer::LoadedFn());
					con.disconnect();
					return "Getting resource";
				}
				else
					return "Enter a path and type";
			}, [](int arg_i, const std::string& arg)->std::vector<std::string>
			{
				std::vector<std::string> completions;
				if (arg_i == 2)
				{
					auto types = ResourceManager::getSingleton().GetResourceLoaderTypes();
					completions.assign(types.begin(), types.end());
					std::sort(completions.begin(), completions.end());
				}
				return completions;
			});
			std::vector<std::string> args; args.push_back("path"); args.push_back("type");
			Console::getSingleton().SetCommandHelpText("resource_load", "Load the given resource. Can be used to populate the editor's resource-database", args);
		}

		m_ShapeTools[Tool::Polygon] = m_ShapeTools[Tool::Line] = m_PolygonTool = std::make_shared<EditorPolygonTool>();
		m_ShapeTools[Tool::Rectangle] = m_RectangleTool = std::make_shared<EditorRectangleTool>();
		m_ShapeTools[Tool::Elipse] = m_CircleTool = std::make_shared<EditorCircleTool>();

		m_EditorOverlay = std::make_shared<EditorOverlay>(m_PolygonTool, m_RectangleTool, m_CircleTool);
		m_SelectionDrawer = std::make_shared<SelectionDrawer>();

		m_SaveDialogListener = std::make_shared<DialogListener>([this](const std::map<std::string, std::string>& params)
		{
			if (params.at("result") == "ok")
			{
				auto path = params.at("path");
				if (!path.empty())
				{
					auto basepath = boost::filesystem::path(PHYSFS_getWriteDir()) / "Editor";
					if (path.empty() || path == "ok")
						path = basepath.string();
					else
						path = FilesystemDataSource::PreprocessPath(path);
					auto saveName = boost::filesystem::path(RemoveBasePath(basepath, path)) / params.at("filename");
					saveName.replace_extension();
					m_SaveName = saveName.generic_string();
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
					auto basepath = boost::filesystem::path(PHYSFS_getWriteDir()) / "Editor";
					if (path.empty() || path == "ok")
						path = basepath.string();
					else
						path = FilesystemDataSource::PreprocessPath(path);
					auto saveName = boost::filesystem::path(RemoveBasePath(basepath, path)) / params.at("filename");
					saveName.replace_extension();
					m_SaveName = saveName.generic_string();
					m_LoadMap = true;
				}
			}
		});

		{
			auto polygonResourceEditor = std::make_shared<PolygonResourceEditor>();
			m_ResourceEditors["POLYGON"] = polygonResourceEditor;
			polygonResourceEditor->SetPolygonToolExecutor([this](const std::vector<Vector2>& verts, const PolygonToolCallback_t& done_cb)
			{
				this->m_PolygonTool->Start(verts, done_cb, EditorPolygonTool::Freeform);
				this->m_Tool = Editor::Tool::Polygon;
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

		m_ResourceDatabase = std::make_shared<ResourceDatabase>();
		m_ResourceBrowserDataSource = std::make_shared<ResourceBrowserDataSource>(m_ResourceDatabase);

		{
			auto tag = "inspector_" + ITransform::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::TransformInspector>())->RemoveReference();

			tag = "inspector_" + IRigidBody::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::RigidBodyInspector>())->RemoveReference();

			tag = "inspector_" + IFixture::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::FixtureInspector>())->RemoveReference();

			tag = "inspector_" + ICircleShape::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::CircleShapeInspector>())->RemoveReference();

			tag = "inspector_" + IPolygonShape::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::PolygonShapeInspector>())->RemoveReference();

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

			tag = "inspector_entity";
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::ElementEntityInspector>())->RemoveReference();

			tag = "inspector_group";
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::ElementGroup>())->RemoveReference();

			tag = "inspector_section";
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<InspectorSection>())->RemoveReference();
		}

		//m_PreviewFormatter.reset(new PreviewFormatter);

		m_RightClickMenu = boost::intrusive_ptr<ContextMenu>(new ContextMenu(m_GUIContext, true), false);
		m_PropertiesMenu = boost::intrusive_ptr<MenuItem>(new MenuItem("Properties", "properties"), false);
		m_RightClickMenu->AddChild(m_PropertiesMenu.get());
		m_EntitySelectionMenu = boost::intrusive_ptr<MenuItem>(new MenuItem("Select", "select"), false);
		m_RightClickMenu->AddChild(m_EntitySelectionMenu.get());

		m_PropertiesMenu->SignalClicked.connect([this](const MenuItemEvent& e) { CreatePropertiesWindowForSelected(); });
	}

	Editor::~Editor()
	{
		Console::getSingleton().UnbindCommand("le");
		Console::getSingleton().UnbindCommand("editor_resourcedb_reset");
		Console::getSingleton().UnbindCommand("editor_resourcedb_delete");
		Console::getSingleton().UnbindCommand("resource_load");
	}

	void Editor::CleanUp()
	{
		m_ResourceBrowser.reset();
		m_DockedWindows.reset();
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

		m_EditorOverlay->m_EditCam = m_EditCam;
		if (m_EditCamRange < 0.f)
			m_EditorOverlay->m_CamRange = m_StreamingManager->GetRange();
		else
			m_EditorOverlay->m_CamRange = m_EditCamRange;

		m_RenderWorld->AddRenderExtension(m_EditorOverlay, m_Viewport);
		m_RenderWorld->AddRenderExtension(m_SelectionDrawer, m_Viewport);

		m_DockedWindows = std::make_shared<DockedWindowManager>(this);

		m_Background = m_GUIContext->LoadDocument("core/gui/editor_background.rml");
		m_Background->RemoveReference();

		m_Background->SetProperty("width", Rocket::Core::Property(m_DisplayWindow.get_gc().get_width(), Rocket::Core::Property::PX));
		m_Background->SetProperty("height", Rocket::Core::Property(m_DisplayWindow.get_gc().get_height(), Rocket::Core::Property::PX));

		m_Background->Show();
	}

	void Editor::Deactivate()
	{
		m_Active = false;

		m_Background.reset();

		m_EditorOverlay->m_EditCam.reset();

		m_RenderWorld->RemoveViewport(m_Viewport);

		m_EditCam.reset();
		m_Viewport.reset();

		m_MapLoader->SetSavePath(m_OriginalSavePath);
	}

	void Editor::SetOptions(const ClientOptions& options)
	{
		try
		{
			m_EditCamRange = boost::lexical_cast<float>(options.GetOption_str("editor_cam_range"));
			m_EditorOverlay->m_CamRange = m_EditCamRange;
		}
		catch (boost::bad_lexical_cast&)
		{}
	}

	void Editor::SetDisplay(const CL_DisplayWindow& display)
	{
		m_DisplayWindow = display;
		m_KeyDownSlot = m_DisplayWindow.get_ic().get_keyboard().sig_key_down().connect(this, &Editor::OnKeyDown);
		m_KeyUpSlot = m_DisplayWindow.get_ic().get_keyboard().sig_key_up().connect(this, &Editor::OnKeyUp);
		m_MouseDownSlot = m_DisplayWindow.get_ic().get_mouse().sig_key_down().connect(this, &Editor::OnMouseDown);
		m_MouseUpSlot = m_DisplayWindow.get_ic().get_mouse().sig_key_up().connect(this, &Editor::OnMouseUp);
		m_MouseMoveSlot = m_DisplayWindow.get_ic().get_mouse().sig_pointer_move().connect(this, &Editor::OnMouseMove);
		m_WindowResizeSlot = m_DisplayWindow.sig_resize().connect(this, &Editor::OnWindowResize);
	}

	void Editor::SetMapLoader(const std::shared_ptr<RegionCellArchivist>& map_loader)
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

		ScriptManager::getSingleton().AddFile("/core/gui/gui_popup.as", "gui_popup.as");
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

		if (m_EditCam && !MouseOverUI(m_GUIContext))
		{
			if (!m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_CONTROL))
			{
				if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_UP))
					m_CamVelocity.y = -400;
				if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_DOWN))
					m_CamVelocity.y = 400;
				if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_LEFT))
					m_CamVelocity.x = -400;
				if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(CL_KEY_RIGHT))
					m_CamVelocity.x = 400;
			}
			if (!v2Equal(m_CamVelocity, Vector2(0.f, 0.f)))
			{
				auto camPos = m_EditCam->GetPosition();
				auto camTrans = m_CamVelocity * dt;
				m_EditCam->SetPosition(camPos.x + camTrans.x, camPos.y + camTrans.y);

				const auto kb = m_DisplayWindow.get_ic().get_keyboard();
				if (!(kb.get_keycode(CL_KEY_UP) || kb.get_keycode(CL_KEY_DOWN)))
				{
					if (m_CamVelocity.length() > 1.f)
						m_CamVelocity.y *= 0.75f * dt;
					else
						m_CamVelocity.y = 0.f;
				}
				if (!(kb.get_keycode(CL_KEY_LEFT) || kb.get_keycode(CL_KEY_RIGHT)))
				{
					if (m_CamVelocity.length() > 1.f)
						m_CamVelocity.x *= 0.5f * dt;
					else
						m_CamVelocity.x = 0.f;
				}
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

	bool Editor::IsResourceEditable(const std::string& filename) const
	{
		return m_ResourceEditors.find(m_ResourceDatabase->GetResourceType(filename)) != m_ResourceEditors.end();
	}
	
	void Editor::StartResourceEditor(const std::string& filename)
	{
		auto type = m_ResourceDatabase->GetResourceType(filename);
		if (!type.empty())
		{
			auto editorEntry = m_ResourceEditors.find(type);
			if (editorEntry != m_ResourceEditors.end())
			{
				auto path = boost::filesystem::path(filename).parent_path();
				if (path.has_parent_path())
					PHYSFS_mkdir(path.string().c_str());

				Vector2 offset(m_EditCam->GetPosition().x, m_EditCam->GetPosition().y);				
				if (GetNumSelected() > 0)
				{
					auto c = GetBBOfSelected().get_center();
					offset = Vector2(ToRenderUnits(c.x), ToRenderUnits(c.y));
				}

				auto editor = editorEntry->second;
				ResourceManager::getSingleton().GetResource(type, filename, [editor, offset](ResourceDataPtr d) { editor->SetResource(d, offset); });
			}
			else
				SendToConsole(filename + " is not present in the resource-db. Load it to assign a type before attempting to edit.");
		}
		else
			SendToConsole(filename + " is not present in the resource-db. Load it to assign a type before attempting to edit.");
	}

	void Editor::GoToEntity(const EntityPtr& entity)
	{
		const auto entityPosition = entity->GetPosition();
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
				if (auto editor_metadata = m_MapLoader->CreateDataFile("editor_metadata"))
				{
					IO::Streams::CellStreamWriter writer(editor_metadata.get());
					// Write the map bounds
					CL_Rectf bounds = m_MapLoader->GetCellCache()->GetUsedBounds();
					writer.Write(bounds.top);
					writer.Write(bounds.left);
					writer.Write(bounds.right);
					writer.Write(bounds.bottom);
				}
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
				if (auto editor_metadata = m_MapLoader->LoadDataFile("editor_metadata"))
				{
					IO::Streams::CellStreamReader reader(editor_metadata.get());
					// Read the map bounds
					CL_Rectf bounds;
					reader.Read(bounds.top);
					reader.Read(bounds.left);
					reader.Read(bounds.right);
					reader.Read(bounds.bottom);
					m_MapLoader->GetCellCache()->SetupEditMode(true, bounds);
				}
				m_NonStreamedEntities = m_EntityManager->GetLastLoadedNonStreamedEntities();
				m_StreamingManager->AddCamera(m_EditCam, m_EditCamRange);
			}
			catch (std::exception& e)
			{
				MessageBoxMaker::Show(Rocket::Core::GetContext("editor"), "error", std::string("title:Failed, message:") + e.what());
			}
		}
	}

	void Editor::ToggleResourceBrowser()
	{
		if (IsResourceBrowserVisible())
			HideResourceBrowser();
		else
			ShowResourceBrowser();
	}

	bool Editor::IsResourceBrowserVisible() const
	{
		return m_ResourceBrowser && m_ResourceBrowser->IsVisible();
	}

	class FunctorEventListener : public Rocket::Core::EventListener
	{
	public:
		FunctorEventListener()
		{}

		FunctorEventListener(const std::function<void (Rocket::Core::Event&)>& handler)
			: m_Handler(handler)
		{}

		~FunctorEventListener()
		{
		}

	private:
		void ProcessEvent(Rocket::Core::Event& ev) { FSN_ASSERT(m_Handler); m_Handler(ev); }

		void OnAttach(Rocket::Core::Element* element)
		{
			m_AttachedElements.insert(element);
		}

		void OnDetach(Rocket::Core::Element* element)
		{
			m_AttachedElements.erase(element);
		}

		std::function<void (Rocket::Core::Event&)> m_Handler;

		std::set<Rocket::Core::Element*> m_AttachedElements;
	};

	void Editor::ShowResourceBrowser()
	{
		if (!m_ResourceBrowser)
		{
			m_ResourceBrowser = m_GUIContext->LoadDocument("core/gui/resource_browser.rml");
			m_ResourceBrowser->RemoveReference();

			m_DockedWindows->AddWindow(m_ResourceBrowser.get(), DockedWindowManager::Left);
		}
		if (m_ResourceBrowser)
		{
			m_ResourceBrowser->Show();

			auto area = m_Viewport->GetArea();
			area.left = m_ResourceBrowser->GetOffsetWidth() / m_DisplayWindow.get_gc().get_width();
			m_Viewport->SetArea(area);
		}
	}

	void Editor::HideResourceBrowser()
	{
		if (m_ResourceBrowser)
		{
			m_ResourceBrowser->Hide();
			m_Viewport->SetArea(0.f, 0.f, 1.f, 1.f);
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

		auto entity = std::make_shared<Entity>(entityManager, transformCom);

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

	void Editor::NudgeSelectedEntities(const Vector2& delta_pixels)
	{
		Vector2 delta = delta_pixels;
		delta *= 1.f / m_EditCam->GetZoom();
		delta.x = ToSimUnits(delta.x); delta.y = ToSimUnits(delta.y);

		ForEachSelected([delta](const EntityPtr& entity)->bool { entity->SetPosition(entity->GetPosition() + delta); return true; });

		m_EditorOverlay->SetOffset(Vector2());
	}

	void Editor::OnKeyDown(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (!m_Active)
			return;

		if (ev.shift)
			m_ShiftSelect = true;
		if (ev.alt)
			m_AltSelect = true;

		if (MouseOverUI(m_GUIContext))
			return;

		if (m_Tool != Tool::None)
		{
			if (auto& tool = m_ShapeTools[m_Tool])
				tool->KeyChange(ev.shift, ev.ctrl, ev.alt);
		}

		// Ctrl + Keys
		if (ev.ctrl && ev.repeat_count == 0)
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

			case CL_KEY_C:
				CopySelectedEntities();
				break;
			case CL_KEY_V:
				{
					Vector2 offset = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
					TranslateScreenToWorld(&offset.x, &offset.y);
					PasteEntities(offset);
				}
				break;

			case CL_KEY_R:
				{
					ToggleResourceBrowser();
				}
				break;

			case CL_KEY_0:
				m_EditCam->SetZoom(1.0);
				break;

			case CL_KEY_P:
				{
					CreatePropertiesWindowForSelected();
				}
				break;

			case CL_KEY_LEFT:
				if (!m_Dragging)
					NudgeSelectedEntities(Vector2(-1.f, 0.f));
				break;
			case CL_KEY_RIGHT:
				if (!m_Dragging)
					NudgeSelectedEntities(Vector2(1.f, 0.f));
				break;
			case CL_KEY_UP:
				if (!m_Dragging)
					NudgeSelectedEntities(Vector2(0.f, -1.f));
				break;
			case CL_KEY_DOWN:
				if (!m_Dragging)
					NudgeSelectedEntities(Vector2(0.f, 1.f));
				break;
			}
		}
		else
		{
			switch (ev.id)
			{
			case CL_KEY_L:
				{
					std::vector<Vector2> verts;
					m_PolygonTool->Start(verts, [](const std::vector<Vector2>& v) {}, EditorPolygonTool::Line);
					m_Tool = Tool::Polygon;
				}
				break;
			case CL_KEY_K:
				{
					Vector2 center;
					m_CircleTool->Start(center, 0.0f, [](const Vector2& c, float r) {
						SendToConsole("Circle: " + boost::lexical_cast<std::string>(c.x) + "," + boost::lexical_cast<std::string>(c.x) + ", " + boost::lexical_cast<std::string>(r));
					});
					m_Tool = Tool::Elipse;
				}
				break;
			case CL_KEY_M:
				{
					Vector2 hsize(1.f, 1.f);
					Vector2 center;
					m_RectangleTool->Start(hsize, center, 0.0f, [](const Vector2& hs, const Vector2& c, float r) {
						SendToConsole("Rect: " + boost::lexical_cast<std::string>(c.x) + "," + boost::lexical_cast<std::string>(c.x) +
							" size:" + boost::lexical_cast<std::string>(hs.x) + "," + boost::lexical_cast<std::string>(hs.x) +
							" r:" + boost::lexical_cast<std::string>(r));
					});
					m_Tool = Tool::Rectangle;
				}
				break;
			case CL_KEY_RETURN:
				if (m_Tool != Tool::None)
				{
					if (auto& tool = m_ShapeTools[m_Tool])
						tool->Finish();
				}
				m_Tool = Tool::None;
				break;
			case CL_KEY_ESCAPE:
				if (m_Tool != Tool::None)
				{
					if (auto& tool = m_ShapeTools[m_Tool])
						tool->Cancel();
				}
				m_Tool = Tool::None;
				break;
			};
			//if (m_Tool == Tool::Polygon || m_Tool == Tool::Line)
			//{
			//	if (ev.id == CL_KEY_RETURN)
			//	{
			//		m_Tool = Tool::None;
			//		m_PolygonTool->Finish();
			//	}
			//	else if (ev.id == CL_KEY_ESCAPE)
			//	{
			//		m_Tool = Tool::None;
			//		m_PolygonTool->Cancel();
			//	}
			//}
		}
	}

	void Editor::OnKeyUp(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (!m_Active)
			return;
		
		m_ShiftSelect = false;
		m_AltSelect = false;

		if (MouseOverUI(m_GUIContext))
			return;

		if (m_Tool != Tool::None)
		{
			if (auto& tool = m_ShapeTools[m_Tool])
				tool->KeyChange(ev.shift, ev.ctrl, ev.alt);
		}

		// Ctrl + Keys
		if (ev.ctrl)
		{
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
			case CL_KEY_F9:
				Console::getSingleton().Interpret("prof_savetimes on");
				break;
			case CL_KEY_F10:
				Console::getSingleton().Interpret("prof_savetimes off");
				break;
			case CL_KEY_DELETE:
				if (ev.shift)
					ForEachSelected([this](const EntityPtr& entity)->bool { this->AddEntityToDelete(entity); return true; });
				else
				{
					//ShowDeleteDialog();
				}
				break;
			case CL_KEY_PRIOR:
				{
				float interval = m_StreamingManager->GetPollArchiveInterval();
				m_StreamingManager->SetPollArchiveInterval(interval += 0.05f);
				}
				break;
			case CL_KEY_NEXT:
				{
				float interval = m_StreamingManager->GetPollArchiveInterval();
				m_StreamingManager->SetPollArchiveInterval(interval -= 0.05f);
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

			if (ev.id == CL_KEY_8)
			{
				if (!ev.shift)
				{
					EntityPtr entity;
					if (!m_EditorOverlay->m_Selected.empty())
					{
						entity = *m_EditorOverlay->m_Selected.begin();
					}
					else
					{
						entity = createEntity(false, 3, Vector2::zero(), m_EntityInstantiator.get(), m_ComponentFactory.get(), m_EntityManager.get());
						//entity->SetName(m_EntityManager->GenerateName(entity));
					}

					std::string archetypeName = entity->GetName() + ".archetype";

					std::shared_ptr<ArchetypeFactory> archetypeFactory = std::make_shared<ArchetypeFactory>();
					archetypeFactory->DefineArchetypeFromEntity(m_ComponentFactory.get(), archetypeName, entity);

					IO::PhysFSStream archetypeFile("/Data/" + archetypeName, IO::OpenMode::Write);
					archetypeFactory->Save(archetypeFile);
				}

				return;
			}

			// TODO: if Caller gets destroyed without being called an exception is fired: investigate
			auto caller = ScriptUtils::Calling::Caller::CallerForGlobalFuncId(ScriptManager::getSingleton().GetEnginePtr(), m_CreateEntityFn->GetId());
			if (caller)
			{
				Vector2 pos((float)ev.mouse_pos.x, (float)ev.mouse_pos.y);
				TranslateScreenToWorld(&pos.x, &pos.y);

				// Fix the selection rectangle
				if (m_SelectionRectangle.bottom < m_SelectionRectangle.top)
					std::swap(m_SelectionRectangle.top, m_SelectionRectangle.bottom);
				if (m_SelectionRectangle.right < m_SelectionRectangle.left)
					std::swap(m_SelectionRectangle.left, m_SelectionRectangle.right);

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

	inline ShapeTool::MouseInput GetMouseInputType(const int id)
	{
		switch (id)
		{
		case CL_MOUSE_LEFT: return ShapeTool::MouseInput::LeftButton;
		case CL_MOUSE_WHEEL_UP: return ShapeTool::MouseInput::ScrollUp;
		case CL_MOUSE_WHEEL_DOWN: return ShapeTool::MouseInput::ScrollDown;
		default: return ShapeTool::MouseInput::None;
		}
	}

	void Editor::OnMouseDown(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (m_Active)
		{
			//[process global inputs here]

			if (MouseOverUI(m_GUIContext))
				return;

			m_ReceivedMouseDown = true;

			if (m_Tool == Tool::None)
			{
				if (ev.ctrl)
				{
					m_Dragging = true;
					GUI::getSingleton().GetContext()->GetRootElement()->SetAttribute("style", "cursor: Move;");
					GUI::getSingleton().GetContext()->SetMouseCursor("Move");
				}

				OnMouseDown_Selection(ev);
			}
			else
			{
				if (auto& tool = m_ShapeTools[m_Tool])
				{
					Vector2 mpos = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
					const auto mouseInputType = GetMouseInputType(ev.id);
					if (mouseInputType != ShapeTool::MouseInput::None)
						tool->MousePress(ReturnScreenToWorld(mpos.x, mpos.y), mouseInputType, ev.shift, ev.ctrl, ev.alt);
				}
			}
		}
	}

	void Editor::OnMouseUp(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (m_Active)
		{
			if (m_ReceivedMouseDown)
			{
				bool inputBlockedByTool = false;
				if (m_Tool == Tool::None)
				{
					if (m_Dragging/*m_Tool == Tool::Move*/)
					{
						Vector2 mouseInWorld = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
						TranslateScreenToWorld(&mouseInWorld.x, &mouseInWorld.y);

						auto delta = mouseInWorld - m_DragFrom;
						delta.x = ToSimUnits(delta.x); delta.y = ToSimUnits(delta.y);

						ForEachSelected([delta](const EntityPtr& entity)->bool { entity->SetPosition(entity->GetPosition() + delta); return true; });

						m_EditorOverlay->SetOffset(Vector2());
					}
					else
					{
						switch (ev.id)
						{
						case CL_MOUSE_LEFT:
							OnMouseUp_Selection(ev);
							break;
						};
					}
				}
				else
				{
					if (auto& tool = m_ShapeTools[m_Tool])
					{
						Vector2 mpos = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
						const auto mouseInputType = GetMouseInputType(ev.id);
						if (mouseInputType != ShapeTool::MouseInput::None)
							inputBlockedByTool = tool->MouseRelease(ReturnScreenToWorld(mpos.x, mpos.y), mouseInputType, ev.shift, ev.ctrl, ev.alt);
					}
				}
				if (!inputBlockedByTool)
				{
					switch (ev.id)
					{
					case CL_MOUSE_RIGHT:
						if (m_EditorOverlay->m_Selected.empty())
							OnMouseUp_Selection(ev);
						ShowContextMenu(Vector2i(ev.mouse_pos.x, ev.mouse_pos.y), m_EditorOverlay->m_Selected);
						break;
					case CL_MOUSE_WHEEL_UP:
						if (ev.ctrl)
						{
							if (ev.alt)
								ForEachSelected([](const EntityPtr& entity)->bool { entity->SetAngle(entity->GetAngle() + s_pi * 0.01f); return true; });
							else
								ForEachSelected([](const EntityPtr& entity)->bool { entity->SetAngle(entity->GetAngle() + s_pi * 0.1f); return true;  });
						}
						else
							m_EditCam->SetZoom(m_EditCam->GetZoom() + 0.05f);
						break;
					case CL_MOUSE_WHEEL_DOWN:
						if (ev.ctrl)
						{
							if (ev.alt)
								ForEachSelected([](const EntityPtr& entity)->bool { entity->SetAngle(entity->GetAngle() - s_pi * 0.01f); return true;  });
							else
								ForEachSelected([](const EntityPtr& entity)->bool { entity->SetAngle(entity->GetAngle() - s_pi * 0.1f); return true;  });
						}
						else
							m_EditCam->SetZoom(m_EditCam->GetZoom() - 0.05f);
						break;
					};
				}
			}

			GUI::getSingleton().GetContext()->GetRootElement()->SetAttribute("style", "cursor: Arrow;");
			GUI::getSingleton().GetContext()->SetMouseCursor("Arrow");
			m_Dragging = false;
			m_ReceivedMouseDown = false;
		}
	}

	void Editor::OnMouseMove(const CL_InputEvent& ev, const CL_InputState& state)
	{
		if (m_Active)
		{
			if (m_Tool != Tool::None)
			{
				if (auto& tool = m_ShapeTools[m_Tool])
				{
					Vector2 mpos = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
					tool->MouseMove(ReturnScreenToWorld(mpos.x, mpos.y), ev.shift, ev.ctrl, ev.alt);
				}
			}
			else if (m_ReceivedMouseDown)
			{
				if (m_Dragging)
				{
					OnMouseMove_Move(ev);
				}
				else
					UpdateSelectionRectangle(Vector2i(ev.mouse_pos.x, ev.mouse_pos.y), true);
			}
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
		Vector2 mousePos = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
		TranslateScreenToWorld(&mousePos.x, &mousePos.y);

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

	void Editor::OnMouseMove_Move(const CL_InputEvent& ev)
	{
		Vector2i mousePos(ev.mouse_pos.x, ev.mouse_pos.y);

		Vector2 mouseInWorld = mousePos;
		TranslateScreenToWorld(&mouseInWorld.x, &mouseInWorld.y);

		m_EditorOverlay->SetOffset(mouseInWorld - m_DragFrom);
	}

	void Editor::OnWindowResize(int x, int y)
	{
		if (m_Active)
		{
			m_Background->SetProperty("width", Rocket::Core::Property(m_DisplayWindow.get_gc().get_width(), Rocket::Core::Property::PX));
			m_Background->SetProperty("height", Rocket::Core::Property(m_DisplayWindow.get_gc().get_height(), Rocket::Core::Property::PX));
		}
	}

	void ClearCtxMenu(MenuItem *menu)
	{
		menu->RemoveAllChildren();
	}

	boost::intrusive_ptr<MenuItem> AddMenuItem(MenuItem* parent, const std::string& title, const std::string& value, std::function<void (const MenuItemEvent&)> on_clicked)
	{
		boost::intrusive_ptr<MenuItem> item(new MenuItem(title, value), false);
		item->SignalClicked.connect(on_clicked);
		parent->AddChild(item.get());

		return item;
	}

	boost::intrusive_ptr<MenuItem> AddMenuItem(MenuItem* parent, const std::string& title, std::function<void (const MenuItemEvent&)> on_clicked)
	{
		return AddMenuItem(parent, title, "", on_clicked);
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

			const std::string title = (entity->GetName().empty() ? std::string("Unnamed") : entity->GetName()) +
				(entity->IsSyncedEntity() ? " (" +  boost::lexical_cast<std::string>(entity->GetID()) + ")" : "");

			// Add an item for this entity to the Properties sub-menu
			auto item = AddMenuItem(m_PropertiesMenu.get(),
				title, entity->GetName(),
				[this, entity](const MenuItemEvent& e) { CreatePropertiesWindow(entity); }
			);
			item->SetBGColour(m_EditorOverlay->GetColour(entity));

			// Add an item for this entity to the Select sub-menu
			item = AddMenuItem(m_EntitySelectionMenu.get(),
				title, entity->GetName(),
				[this, entity](const MenuItemEvent& e) { if (!m_ShiftSelect) DeselectAll(); SelectEntity(entity); }
			);
			item->SetBGColour(m_EditorOverlay->GetColour(entity));
		}

		m_RightClickMenu->Show(position.x, position.y);
	}

	//void Editor::ShowContextMenu(const Vector2i& position)
	//{
	//	ClearCtxMenu(m_PropertiesMenu.get());
	//	ClearCtxMenu(m_EntitySelectionMenu.get());

	//	AddMenuItem(m_EntitySelectionMenu.get(),
	//		"Deselect All",
	//		std::bind(&Editor::DeselectAll, this));

	//	ForEachSelectedWithColours([this](const EntityPtr& entity, const CL_Colorf& colour)->bool
	//	{
	//		const std::string title = entity->GetName().empty() ? std::string("Unnamed ") : entity->GetName() + "(" + entity->GetType() + ")";

	//		// Add an item for this entity to the Properties sub-menu
	//		AddMenuItem(this->m_PropertiesMenu.get(),
	//			title, entity->GetName(),
	//			[this, entity](const MenuItemEvent& e) { std::vector<EntityPtr> ents; ents.push_back(entity); CreatePropertiesWindow(ents); }
	//		);

	//		// Add an item for this entity to the Select sub-menu
	//		AddMenuItem(m_EntitySelectionMenu.get(),
	//			title, entity->GetName(),
	//			[this, entity](const MenuItemEvent& e) { if (!m_ShiftSelect) DeselectAll(); SelectEntity(entity); }
	//		);
	//	});

	//	m_RightClickMenu->Show(position.x, position.y);
	//}

	bool Editor::TranslateScreenToWorld(float* x, float* y) const
	{
		CL_Rectf worldArea, screenArea;
		Renderer::CalculateScreenArea(m_DisplayWindow.get_gc(), worldArea, m_Viewport, true);
		Renderer::CalculateScreenArea(m_DisplayWindow.get_gc(), screenArea, m_Viewport, false);

		const bool withinViewport = *x >= screenArea.left && *y >= screenArea.top;
		*x -= screenArea.left, *y -= screenArea.top;

		*x *= (1 / m_EditCam->GetZoom());
		*y *= (1 / m_EditCam->GetZoom());
		*x += worldArea.left, *y += worldArea.top;

		return withinViewport;
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

	size_t Editor::GetNumSelected() const
	{
		return m_EditorOverlay->m_Selected.size();
	}

	void Editor::ForEachSelected(std::function<bool (const EntityPtr&)> fn)
	{
		for (auto it = m_EditorOverlay->m_Selected.begin(), end = m_EditorOverlay->m_Selected.end(); it != end; ++it)
		{
			if (!fn(*it))
				break;
		}
	}

	void Editor::ForEachSelectedWithColours(std::function<bool (const EntityPtr&, const CL_Colorf&)> fn)
	{
		for (auto it = m_EditorOverlay->m_Selected.begin(), end = m_EditorOverlay->m_Selected.end(); it != end; ++it)
		{
			if (!fn(*it, m_EditorOverlay->GetColour(it)))
				break;
		}
	}

	void Editor::DoWithArchetypeFactory(const std::string& archetype_name, std::function<void (const ResourcePointer<ArchetypeFactory>&)> fn)
	{
		if (m_ArchetypeFactoryLoadConnections.empty())
			m_NextFactoryId = 0;

		m_ArchetypeFactoryLoadConnections[m_NextFactoryId++] = std::make_shared<boost::signals2::scoped_connection>(
			ResourceManager::getSingleton().GetResource(
			"ArchetypeFactory", archetype_name, [fn](ResourceDataPtr& resource)
		{
			fn(ResourcePointer<ArchetypeFactory>(resource));
		}));
	}

	CL_Rectf Editor::GetBBOfSelected()
	{
		CL_Rectf bb(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
		ForEachSelected([&bb](const EntityPtr& entity)->bool
		{
			auto transform = entity->GetComponent<ITransform>(); FSN_ASSERT(transform);
			auto pos = transform->GetPosition();
			if (pos.x < bb.left)
				bb.left = pos.x;
			if (pos.y < bb.top)
				bb.top = pos.y;
			if (pos.x > bb.right)
				bb.right = pos.x;
			if (pos.y > bb.bottom)
				bb.bottom = pos.y;
			return true;
		});
		return bb;
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

		auto entity = std::make_shared<Entity>(m_EntityManager.get(), transformCom);

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
		entity->SynchroniseParallelEdits();

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

	void Editor::CreateArchetypeInstance(const std::string& archetype_name, const Vector2& position, float angle)
	{
		DoWithArchetypeFactory(archetype_name, [this, position, angle](const ResourcePointer<ArchetypeFactory>& archetype_factory)
		{
			Vector2 p(position);
			if (m_SelectionRectangle.contains(CL_Vec2f(p.x, p.y)))
			{
				for (p.y = m_SelectionRectangle.top; p.y < m_SelectionRectangle.bottom; p.y += 100)
					for (p.x = m_SelectionRectangle.left; p.x < m_SelectionRectangle.right; p.x += 100)
					{
						Vector2 simPos(ToSimUnits(p.x), ToSimUnits(p.y));

						auto entity = archetype_factory->MakeInstance(m_ComponentFactory.get(), simPos, angle);
						m_EntityManager->AddEntity(entity);

						SendToConsole("Instantiate " + boost::lexical_cast<std::string>(p.x));
					}
			}
			else
			{
				Vector2 simPos(ToSimUnits(p.x), ToSimUnits(p.y));
				auto entity = archetype_factory->MakeInstance(m_ComponentFactory.get(), simPos, angle);
				m_EntityManager->AddEntity(entity);
			}
		});
	}

	void Editor::CopySelectedEntities()
	{
		auto bb = GetBBOfSelected();
		auto c = bb.get_center();
		Vector2 center(c.x, c.y);
		if (auto file = m_DataArchiver->CreateDataFile("editor.entity_clipboard"))
		{
			IO::Streams::CellStreamWriter writer(file.get());
			writer.Write(GetNumSelected());
			ForEachSelected([&](const EntityPtr& entity)->bool
			{
				{
					auto transform = entity->GetComponent<ITransform>();
					auto pos = transform->GetPosition();
					pos -= center;
					writer.Write(pos.x);
					writer.Write(pos.y);
					writer.Write(transform->Angle.Get());
				}
				EntitySerialisationUtils::SaveEntity(*file, entity, true, EntitySerialisationUtils::EditableBinary);
				return true;
			});
		}
		else
			FSN_EXCEPT(FileSystemException, "Failed to create entity clipboard file");
	}

	void Editor::PasteEntities(const Vector2& offset, float base_angle)
	{
		if (auto selfishFile = m_DataArchiver->LoadDataFile("editor.entity_clipboard"))
		{
			std::shared_ptr<std::istream> file(std::move(selfishFile));

			Vector2 simOffset(ToSimUnits(offset.x), ToSimUnits(offset.y));

			IO::Streams::CellStreamReader reader(file.get());
			size_t numEntities = reader.ReadValue<size_t>();
			for (size_t i = 0; i < numEntities; ++i)
			{
				Vector2 entityPos;
				float entityAngle = 0.f;
				{
					reader.Read(entityPos.x);
					reader.Read(entityPos.y);
					reader.Read(entityAngle);
				}

				EntityPtr entity;
				std::tie(entity, file) = EntitySerialisationUtils::LoadEntityImmeadiate(std::move(file), true, 0, EntitySerialisationUtils::EditableBinary, m_ComponentFactory.get(), m_EntityManager.get(), m_EntityInstantiator.get());

				if (entity->GetID())
					entity->SetID(m_EntityInstantiator->GetFreeGlobalID());
				entity->SetName("");

				{
					auto transform = entity->GetComponent<ITransform>();
					transform->Position.Set(simOffset + entityPos);
					transform->Angle.Set(base_angle + entityAngle);
				}

				m_EntityManager->AddEntity(entity);
				if (entity->GetDomain() == SYSTEM_DOMAIN)
					m_NonStreamedEntities.push_back(entity);
			}
		}
	}

	class InspectorGenerator
	{
	public:
		Rocket::Core::ElementDocument* doc;
		Rocket::Core::Element* body;

		boost::intrusive_ptr<EntitySelector> entity_selector;
		boost::intrusive_ptr<Inspectors::ElementEntityInspector> entity_inspector;
		boost::intrusive_ptr<Inspectors::ElementGroup> inspector_group;

		std::vector<EntityPtr> entities;

		InspectorGenerator(Rocket::Core::ElementDocument* doc_)
			: doc(doc_)
		{
			FSN_ASSERT(doc);

			body = doc->GetFirstChild()->GetElementById("content");
			if (!body)
			{
				FSN_EXCEPT(Exception, (doc->GetSourceURL() + " is missing the content element").CString());
			}

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
		}

		void ProcessEntity(const EntityPtr& entity)
		{
			entities.push_back(entity);

			inspector_group->AddEntity(entity);
		}

		void Generate()
		{
			// Generate title
			{
				std::string title;
				if (entities.size() > 1)
				{
					title = boost::lexical_cast<std::string>(entities.size()) + " entities";

					entity_inspector->SetPseudoClass("unavailable", true);
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

					entity_inspector->SetPseudoClass("unavailable", false);
					entity_inspector->SetEntity(front);
				}
				if (!title.empty())
					doc->SetTitle(doc->GetTitle() + (": " + title).c_str());
			}
			if (auto titleElem = doc->GetElementById("title"))
				titleElem->SetInnerRML(doc->GetTitle());

			inspector_group->AddFooter();

			inspector_group->DoneAddingEntities();

			// Set entities
			entity_selector->SetEntities(entities);
		}
	};

	void Editor::InitInspectorGenerator(InspectorGenerator& generator, const std::function<void (void)>& close_callback)
	{
		generator.inspector_group->SetCircleToolExecutor([this](const Vector2& c, float r, const CircleToolCallback_t& done_cb)
		{
			this->m_CircleTool->Start(c, r, done_cb);
			this->m_Tool = Editor::Tool::Elipse;
		});
		generator.inspector_group->SetRectangleToolExecutor([this](const Vector2& size, const Vector2& c, float r, const RectangleToolCallback_t& done_cb)
		{
			this->m_RectangleTool->Start(size, c, r, done_cb);
			this->m_Tool = Editor::Tool::Rectangle;
		});
		generator.inspector_group->SetPolygonToolExecutor([this](const std::vector<Vector2>& verts, const PolygonToolCallback_t& done_cb)
		{
			this->m_PolygonTool->Start(verts, done_cb, EditorPolygonTool::Freeform);
			this->m_Tool = Editor::Tool::Polygon;
		});
		
		generator.entity_selector->SetCallback([this](const EntityPtr& entity) { this->GoToEntity(entity); });
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

	void Editor::CreatePropertiesWindow(const EntityPtr& entity, const std::function<void (void)>& close_callback)
	{
		std::vector<EntityPtr> e;
		e.push_back(entity);
		CreatePropertiesWindow(e, close_callback);
	}

	void Editor::CreatePropertiesWindow(const std::vector<EntityPtr>& entities, const std::function<void (void)>& close_callback)
	{
		auto doc = m_GUIContext->LoadDocument("/core/gui/properties.rml");
		//{
		//	auto script = OpenString_PhysFS("/core/gui/gui_base.as");
		//	auto strm = new Rocket::Core::StreamMemory((const Rocket::Core::byte*)script.c_str(), script.size());
		//	doc->LoadScript(strm, "/core/gui/gui_base.as");
		//	strm->RemoveReference();
		//}

		InspectorGenerator generator(doc);
		InitInspectorGenerator(generator, close_callback);

		for (auto it = entities.begin(), end = entities.end(); it != end; ++it)
			generator.ProcessEntity(*it);
		generator.Generate();

		doc->Show();
		doc->RemoveReference();
	}

	void Editor::CreatePropertiesWindowForSelected()
	{
		auto doc = m_GUIContext->LoadDocument("/core/gui/properties.rml");
		//{
		//	auto script = OpenString_PhysFS("/core/gui/gui_base.as");
		//	auto strm = new Rocket::Core::StreamMemory((const Rocket::Core::byte*)script.c_str(), script.size());
		//	doc->LoadScript(strm, "/core/gui/gui_base.as");
		//	strm->RemoveReference();
		//}

		InspectorGenerator generator(doc);
		InitInspectorGenerator(generator, std::function<void (void)>());

		ForEachSelected([&generator](const EntityPtr& entity)->bool { generator.ProcessEntity(entity); return true; });
		generator.Generate();
		
		doc->Show();
		doc->RemoveReference();
	}

	ViewportPtr Editor_GetViewport(const std::string& name, Editor* obj)
	{
		return obj->GetViewport();
	}

	void Editor::RegisterScriptType(asIScriptEngine* engine)
	{
		int r;
		RegisterSingletonType<Editor>("Editor", engine);
		r = engine->RegisterObjectMethod("Editor", "Entity CreateEntity(const string &in, const Vector &in, float, bool, bool)", asMETHOD(Editor, CreateEntity), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "void createArchetypeInstance(const string &in, const Vector &in, float)", asMETHOD(Editor, CreateArchetypeInstance), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "bool isResourceEditable(const string &in) const", asMETHOD(Editor, IsResourceEditable), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "void startResourceEditor(const string &in)", asMETHOD(Editor, StartResourceEditor), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "bool goToEntity(const Entity &in)", asMETHOD(Editor, GoToEntity), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "Viewport getViewport(const string &in)", asFUNCTION(Editor_GetViewport), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

}
