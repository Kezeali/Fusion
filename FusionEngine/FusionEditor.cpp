/*
*  Copyright (c) 2009-2010 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionEditor.h"

#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/signals2.hpp>
#include <Rocket/Core.h>
#include <Rocket/Controls.h>

#include "FusionEditorEntityDialog.h"
#include "FusionEditorMapEntity.h"
#include "FusionEditorMoveAction.h"
#include "FusionEditorMultiAction.h"
#include "FusionEntityFactory.h"
#include "FusionEntityManager.h"
#include "FusionExceptionFactory.h"
#include "FusionGUI.h"
#include "FusionInstanceSynchroniser.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIODeviceProvider.h"
#include "FusionPhysicalEntityManager.h"
#include "FusionRenderer.h"
#include "FusionScriptedEntity.h"
#include "FusionScriptManager.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionXml.h"

namespace FusionEngine
{

	// Editor DataSource
	EditorDataSource::EditorDataSource()
		: EMP::Core::DataSource("editor")
	{
	}

	EditorDataSource::~EditorDataSource()
	{
	}

	void EditorDataSource::UpdateSuggestions(const StringVector &list)
	{
		size_t previousSize = m_Suggestions.size();
		m_Suggestions = list;
		// Cause the element to update:
		if (previousSize > 0 && list.size() > 0)
			NotifyRowChange("type_suggestions", 0, fe_min(previousSize, list.size()));
		if (previousSize > list.size())
			NotifyRowRemove("type_suggestions", list.size(), previousSize-list.size());
		else if (list.size() > previousSize)
			NotifyRowAdd("type_suggestions", previousSize, list.size()-previousSize);
	}

	const std::string &EditorDataSource::GetSuggestion(size_t index)
	{
		return m_Suggestions[index];
	}

	void EditorDataSource::GetRow(EMP::Core::StringList& row, const EMP::Core::String& table, int row_index, const EMP::Core::StringList& columns)
	{
		if (row_index < 0 || (size_t)row_index >= m_Suggestions.size())
			return;

		if (table == "type_suggestions")
		{
			for (size_t i = 0; i < columns.size(); i++)
			{
				if (columns[i] == "name")
				{
					row.push_back(m_Suggestions[row_index].c_str());
				}
			}
		}
	}

	int EditorDataSource::GetNumRows(const EMP::Core::String& table)
	{
		if (table == "type_suggestions")
		{
			return m_Suggestions.size();
		}

		return 0;
	}

	//! An action
	/*!
	* \todo Make sure the entity is re-created with the correct ID (so redo actions that restore entity-pointer properties are valid)
	*/
	class AddRemoveEntityAction : public UndoableAction
	{
	public:
		typedef std::pair<unsigned int, boost::any> ChangedProperty;
		typedef std::vector<ChangedProperty> ChangedPropertyArray;

		AddRemoveEntityAction(Editor *editor, EntityFactory *factory, EntityManager *manager, const EditorMapEntityPtr &map_entity, bool added = true);

		const std::string &GetTitle() const;
	protected:
		bool m_Added;
		EditorMapEntityPtr m_EditorEntity;
		Editor *m_Editor;

		EntityFactory *m_Factory;
		EntityManager *m_Manager;

		// Used to recreate the entity
		struct
		{
			bool synced;
			ObjectID id;
			std::string type;
			std::string name;
			Vector2 position;
		} m_Parameters;

		std::string m_Title;
		
		void undoAction();
		void redoAction();

		void create();
		void destroy();
	};
	typedef std::tr1::shared_ptr<AddRemoveEntityAction> AddRemoveEntityActionPtr;

	AddRemoveEntityAction::AddRemoveEntityAction(Editor *editor, EntityFactory *factory, EntityManager *manager, const EditorMapEntityPtr &map_entity, bool added)
		: m_Editor(editor),
		m_Factory(factory),
		m_Manager(manager),
		m_EditorEntity(map_entity),
		m_Added(added)
	{
		m_Title = std::string(added ? "Add " : "Remove ") + "[" + map_entity->entity->GetType() + "] ";
		if (map_entity->hasName)
			m_Title += map_entity->entity->GetName();

		// Store the entity info
		m_Parameters.synced = map_entity->synced;
		m_Parameters.type = map_entity->entity->GetType();
		m_Parameters.name = map_entity->entity->GetName();
		m_Parameters.position = map_entity->entity->GetPosition();

		//if (!m_Added)
		//	m_EditorEntity.reset();
	}

	const std::string &AddRemoveEntityAction::GetTitle() const
	{
		return m_Title;
	}

	void AddRemoveEntityAction::create()
	{
		//m_EditorEntity = m_Editor->CreateEntity(m_Parameters.type, m_Parameters.name, m_Parameters.synced, m_Parameters.position.x, m_Parameters.position.y);

		m_EditorEntity->RestoreEntity(m_Factory, m_Manager);
		m_Editor->AddEntity(m_EditorEntity);
	}

	void AddRemoveEntityAction::destroy()
	{
		m_Parameters.synced = m_EditorEntity->synced;
		m_Parameters.type = m_EditorEntity->entity->GetType();
		m_Parameters.name = m_EditorEntity->entity->GetName();
		m_Parameters.position = m_EditorEntity->entity->GetPosition();

		//m_Editor->RemoveEntity(m_EditorEntity);
		//m_EditorEntity.reset();

		// Save the entity's state and remove it from the Editor
		m_Editor->RemoveEntity(m_EditorEntity);
		m_EditorEntity->ArchiveEntity();
		ScriptManager::getSingleton().GetEnginePtr()->GarbageCollect();
	}

	void AddRemoveEntityAction::undoAction()
	{
		if (m_Added)
			destroy();
		else
			create();
	}

	void AddRemoveEntityAction::redoAction()
	{
		if (m_Added)
			create();
		else
			destroy();
	}


	void Editor::EditorEntityDeserialiser::ListEntity(const EntityPtr &entity)
	{
		m_EntityMap[entity->GetID()] = entity;
	}

	EntityPtr Editor::EditorEntityDeserialiser::GetEntity(ObjectID id) const
	{
		EntityIdMap::const_iterator _where = m_EntityMap.find(id);
		if (_where != m_EntityMap.end())
			return _where->second;
		else
			return EntityPtr();
	}

	Editor::Editor(InputManager *input, EntityFactory *entity_factory, Renderer *renderer, InstancingSynchroniser *instancing, PhysicalWorld *world, StreamingManager *streaming_manager, GameMapLoader *map_util, EntityManager *entity_manager)
		: m_Input(input),
		m_EntityFactory(entity_factory),
		m_Renderer(renderer),
		m_InstanceSynchroniser(instancing),
		m_PhysicalWorld(world),
		m_Streamer(streaming_manager),
		m_MapUtil(map_util),
		m_EntityManager(entity_manager),
		m_MainDocument(nullptr),
		m_PropertiesMenu(nullptr),
		m_EntitySelectionMenu(nullptr),
		m_RightClickMenu(nullptr),
		m_UndoManager(256),
		m_Enabled(false),
		m_ActiveTool(tool_move),
		m_ShiftSelect(false),
		m_Dragging(false),
		m_ReceivedMouseDown(false)
	{
		m_EditorDataSource = new EditorDataSource();
	}

	Editor::~Editor()
	{
		CleanUp();
		delete m_EditorDataSource;
	}

	const std::string s_EditorSystemName = "Editor";

	const std::string &Editor::GetName() const
	{
		return s_EditorSystemName;
	}

	void generateCircleLineLoop(CL_Vec2f *verts, int num_verts, float radius, const CL_Vec2f &middle = CL_Vec2f())
	{
		int i;
		//float x, y;
		float theta;
		float wedgeAngle;	// Size of angle between two points on the circle (single wedge)
		//int points = num_verts-1;

		// Precompute wedge angle
		wedgeAngle = (float)((2*s_pi) / num_verts);

		// Set up vertices for a circle
		for(i = 0; i < num_verts; i++)
		{
			// Calculate theta for this vertex
			theta = i * wedgeAngle;

			// Compute X and Y locations
			verts[i].x = (float)(middle.x + radius * cos(theta));
			verts[i].y = (float)(middle.y - radius * sin(theta));
		}

	}

	void Editor::generateSelectionOverlay()
	{
		CL_GraphicContext gc = m_Renderer->GetGraphicContext();
		CL_Texture offtex(gc, 32, 32);

		CL_FrameBuffer offscreen(gc);
		offscreen.attach_color_buffer(0, offtex);
		gc.set_frame_buffer(offscreen);

		// The main selection box
		gc.clear(CL_Colorf(0.0f, 0.0f, 0.0f, 0.0f));
		// attributes
		CL_Colorf colour(0.8f, 0.4f, 0.3f, 0.8f);
		CL_Vec2f cross[4] =
		{
			CL_Vec2f(1, 1),
			CL_Vec2f(32, 32),
			CL_Vec2f(1, 32),
			CL_Vec2f(32, 1),
		};
		CL_Vec2f box[4] =
		{
			CL_Vec2f(1, 1),
			CL_Vec2f(32, 1),
			CL_Vec2f(32, 32),
			CL_Vec2f(1, 32),
		};
		// draw it
		{
			CL_PrimitivesArray prim_array(gc);
			prim_array.set_attributes(0, cross);
			prim_array.set_attribute(1, colour);
			gc.set_program_object(cl_program_color_only);
			gc.draw_primitives(cl_lines, 4, prim_array);
			prim_array.set_attributes(0, box);
			gc.draw_primitives(cl_line_loop, 4, prim_array);
			gc.reset_program_object();
		}

		{
			CL_SpriteDescription spriteDesc;
			spriteDesc.add_frame(offtex);
			m_SelectionOverlay = CL_Sprite(gc, spriteDesc);
		}

		offscreen.detach_color_buffer(0, offtex);
		offtex = CL_Texture(gc, 10, 10);
		offscreen.attach_color_buffer(0, offtex);
		// Rotation indicator
		gc.clear(CL_Colorf(0.0f, 0.0f, 0.0f, 0.0f));
		// attributes
		CL_Vec2f circle[24];
		generateCircleLineLoop(circle, 24, 5, CL_Vec2f(5.f, 5.f)); 
		// draw it
		{
			CL_PrimitivesArray prim_array(gc);
			prim_array.set_attributes(0, circle);
			prim_array.set_attribute(1, colour);
			gc.set_program_object(cl_program_color_only);
			gc.draw_primitives(cl_line_loop, 24, prim_array);
			gc.reset_program_object();
		}

		{
			CL_SpriteDescription spriteDesc;
			spriteDesc.add_frame(offtex);
			m_SelectionOverlay_Rotate = CL_Sprite(gc, spriteDesc);
		}

		gc.reset_frame_buffer();
	}

	bool Editor::Initialise()
	{
		// Load gui documents
		Rocket::Core::Context *guiCtx = GUI::getSingleton().GetContext();
		m_MainDocument = guiCtx->LoadDocument("core/gui/editor.rml");
		if (m_MainDocument == NULL)
			return false;
		m_MainDocument->RemoveReference();

		// Create context menu
		m_RightClickMenu = new ContextMenu(m_MainDocument->GetContext(), m_Input);
		m_PropertiesMenu = new MenuItem("Properties", "properties");
		m_RightClickMenu->AddChild(m_PropertiesMenu);
		m_EntitySelectionMenu = new MenuItem("Select", "select");
		m_RightClickMenu->AddChild(m_EntitySelectionMenu);

		m_Viewport.reset(new Viewport());
		m_Camera.reset( new Camera(ScriptManager::getSingleton().GetEnginePtr()) );
		m_Camera->release();
		m_Viewport->SetCamera(m_Camera);

		m_RawInputConnection = m_Input->SignalRawInput.connect( boost::bind(&Editor::OnRawInput, this, _1) );

		generateSelectionOverlay();

		return true;
	}

	void Editor::CleanUp()
	{
		// Release context menu
		if (m_PropertiesMenu != nullptr)
		{
			m_PropertiesMenu->release();
			m_PropertiesMenu = nullptr;
		}
		if (m_EntitySelectionMenu != nullptr)
		{
			m_EntitySelectionMenu->release();
			m_EntitySelectionMenu = nullptr;
		}
		if (m_RightClickMenu != nullptr)
		{
			m_RightClickMenu->release();
			m_RightClickMenu = nullptr;
		}
		m_PropertiesMenuConnections.clear();
		m_SelectionMenuConnections.clear();

		if (m_MainDocument != nullptr)
			m_MainDocument->Close();
		m_MainDocument = nullptr;

		m_EntityDialogs.clear();

		m_UndoManager.Clear();
		m_UndoManager.DetachAllListeners();

		DeselectAll();

		//m_PlainEntityArray.clear();
		m_UsedTypes.clear();
		m_Archetypes.clear();
		m_PseudoEntities.clear();
		m_Entities.clear();

		m_EntityManager->Clear();
	}

	// TODO: put this in FusionEntity.h, or make Renerer update renderables instead
	//typedef std::tr1::unordered_set<uintptr_t> ptr_set;
	//void updateRenderables(EntityPtr &entity, float split, ptr_set &updated_sprites)
	//{
	//	RenderableArray &renderables = entity->GetRenderables();
	//	for (RenderableArray::iterator it = renderables.begin(), end = renderables.end(); it != end; ++it)
	//	{
	//		RenderablePtr &renderable = *it;
	//		if (renderable->GetSpriteResource().IsLoaded())
	//		{
	//			std::pair<ptr_set::iterator, bool> result = updated_sprites.insert((uintptr_t)renderable->GetSpriteResource().Get());
	//			if (result.second)
	//				renderable->GetSpriteResource()->update((int)(split * 1000));
	//		}
	//		renderable->UpdateAABB();
	//	}
	//}

	void Editor::Update(float split)
	{
		//m_PhysicalWorld->Step(split);
		// TODO: seperate PhysicalWorld::Draw from PhysicalWorld::Step (so debug-draw can be called any time - e.g. Editor::Draw)

		const CL_Vec2f &currentPos = m_Camera->GetPosition();
		m_Camera->SetPosition(currentPos.x + m_CamVelocity.x, currentPos.y + m_CamVelocity.y);
		m_Camera->Update(split);

		m_Streamer->Update();

		m_EntityManager->Update(split);

		//ptr_set updatedSprites;
		//for (EntityArray::iterator it = m_PlainEntityArray.begin(), end = m_PlainEntityArray.end(); it != end; ++it)
		//{
		//	EntityPtr &entity = *it;
		//	updateRenderables(entity, split, updatedSprites);
		//}
	}

	void Editor::Draw()
	{
		m_EntityManager->Draw(m_Renderer, m_Viewport, 0);
		if (m_DragSelect)
		{
			CL_GraphicContext gc = m_Renderer->GetGraphicContext();
			//m_Viewport->SetupGC(gc);
			CL_Draw::box(gc, m_SelectionRectangle, CL_Colorf::white);
			//m_Viewport->ResetGC(gc);
		}
	}

	void Editor::Start()
	{
		m_Streamer->SetPlayerCamera(255, m_Camera);

		m_PhysicalWorld->SetDebugDrawViewport(m_Viewport);
		m_PhysicalWorld->EnableDebugDraw();

		//m_EntityManager->SetDomainState(ALL_DOMAINS, DS_STREAMING | DS_SYNCH);
		m_EntityManager->SetDomainState(GAME_DOMAIN, DS_STREAMING | DS_SYNCH);

		this->PushMessage(new SystemMessage(SystemMessage::RESUME));
		this->PushMessage(new SystemMessage(SystemMessage::SHOW));

		GUI::getSingleton().GetContext()->SetMouseCursor("Arrow");

		m_MainDocument->Show();

		// Allows input to be caputred by OnRawInput
		m_Enabled = true;
	}

	void Editor::Stop()
	{
		m_Streamer->RemovePlayerCamera(255);

		//m_EntityManager->SetDomainState(ALL_DOMAINS, DS_ALL);
		m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL);

		this->PushMessage(new SystemMessage(SystemMessage::PAUSE));
		this->PushMessage(new SystemMessage(SystemMessage::HIDE));

		if (m_MainDocument != nullptr)
			m_MainDocument->Hide();

		m_Enabled = false;
	}

	void Editor::SetEntityModule(const ModulePtr &module)
	{
		m_EntityBuildConnection = module->ConnectToBuild( boost::bind(&Editor::OnBuildEntities, this, _1) );
	}

	void Editor::OnBuildEntities(BuildModuleEvent &ev)
	{
		if (ev.type == BuildModuleEvent::PostBuild)
		{
			StringVector types;
			m_EntityFactory->GetTypes(types, true);
			m_EntityTypes.insert(types.begin(), types.end());
			//for (StringVector::iterator it = types.begin(), end = types.end(); it != end; ++it)
			//{
			//	m_EntityTypes.insert(*it);
			// something
			//}
			m_EditorDataSource->UpdateSuggestions(types);
		}
	}

	void Editor::Enable(bool enable)
	{
		if (!m_Camera)
			return;

		if (enable)
		{
			m_Streamer->SetPlayerCamera(255, m_Camera);

			m_PhysicalWorld->SetDebugDrawViewport(m_Viewport);
			m_PhysicalWorld->EnableDebugDraw();

			this->PushMessage(new SystemMessage(SystemMessage::RESUME));
			this->PushMessage(new SystemMessage(SystemMessage::SHOW));

			GUI::getSingleton().GetContext()->SetMouseCursor("Arrow");

			m_MainDocument->Show();
		}
		else
		{
			m_Streamer->RemovePlayerCamera(255);

			this->PushMessage(new SystemMessage(SystemMessage::PAUSE));
			this->PushMessage(new SystemMessage(SystemMessage::HIDE));

			if (m_MainDocument != NULL)
				m_MainDocument->Hide();
		}
		m_Enabled = enable;
	}

	void Editor::OnRawInput(const RawInput &ev)
	{
		if (!m_Enabled)
			return;

		if (ev.InputType == RawInput::Pointer)
		{
			if (m_ReceivedMouseDown)
			{
				float moveSize = (ev.PointerPosition - m_LastDragMove).length();

				m_Dragging = true;
				// Decide whether the user intends to create a new selection box, or drag selected entities
				if ((m_ActiveTool == tool_move && m_ShiftSelect)
					|| ((m_DragSelect || m_SelectedEntities.empty()) && (m_ActiveTool == tool_move || ev.Control || moveSize > 5.0f)))
				{
					m_LastDragMove = ev.PointerPosition;
					m_DragSelect = true;

					m_SelectionRectangle.right = (float)ev.PointerPosition.x;
					m_SelectionRectangle.bottom = (float)ev.PointerPosition.y;

					GameMapLoader::MapEntityArray entitiesUnderMouse;
					GetEntitiesOverlapping(entitiesUnderMouse, m_SelectionRectangle, true);
					if (!m_ShiftSelect)
						DeselectAll();
					std::for_each(entitiesUnderMouse.begin(), entitiesUnderMouse.end(), [this](const MapEntityPtr& map_entity){ SelectEntity(map_entity); });
				}
				else if (ev.Control || moveSize > 5.0f)
				{
					m_LastDragMove = ev.PointerPosition;
					Vector2 offset = ev.PointerPosition - m_DragFrom;
					for (auto it = m_SelectedEntities.begin(), end = m_SelectedEntities.end(); it != end; ++it)
					{
						EditorMapEntity* editorEntity = dynamic_cast<EditorMapEntity*>(it->get());
						// Set the offset of the overlays
						std::for_each(editorEntity->selectionRenderables.begin(), editorEntity->selectionRenderables.end(), [&offset](const RenderablePtr& renderable) {
							renderable->SetOffset(offset);
						});
					}
				}
			}
			return;
		}
		else if (ev.Code == CL_KEY_SHIFT)
		{
			m_ShiftSelect = ev.ButtonPressed;
			return;
		}
		// Button released. Note that this is the global handler (for input received when
		//  the cursor is in-world AND over the GUI), in-world-only input is handled below.
		else if (ev.ButtonPressed == false)
		{
			switch (ev.Code)
			{
			case CL_KEY_Z:
				if (ev.Control)
					m_UndoManager.Undo();
				break;
			case CL_KEY_Y:
				if (ev.Control)
					m_UndoManager.Redo();
				break;
			}
		}

		// This will exit the fn if the cursor is over a GUI element, so the input handling below
		//  is for in-world interactions only
		Rocket::Core::Context *context = m_MainDocument->GetContext();
		for (int i = 0, num = context->GetNumDocuments(); i < num; ++i)
		{
			if (context->GetDocument(i)->IsPseudoClassSet("hover"))
				return;
		}

		if (ev.InputType == RawInput::Button)
		{
			if (ev.ButtonPressed == true)
			{
				switch (ev.Code)
				{
				case CL_KEY_LEFT:
					m_CamVelocity.x = -10;
					break;
				case CL_KEY_RIGHT:
					m_CamVelocity.x = 10;
					break;
				case CL_KEY_UP:
					m_CamVelocity.y = -10;
					break;
				case CL_KEY_DOWN:
					m_CamVelocity.y = 10;
					break;
				case CL_MOUSE_LEFT:
					m_ReceivedMouseDown = true;
					//if (m_ActiveTool == tool_move)
					{
						m_DragFrom = ev.PointerPosition;
						m_SelectionRectangle.right = m_SelectionRectangle.left = (float)ev.PointerPosition.x;
						m_SelectionRectangle.bottom = m_SelectionRectangle.top = (float)ev.PointerPosition.y;
					}
					break;
				}
			}
			else if (ev.ButtonPressed == false) // Button released
			{
				switch (ev.Code)
				{
				case 192: // `~ key
					{
						Rocket::Core::ElementDocument *consoleWindow = GUI::getSingleton().GetConsoleWindow();
						if (GUI::getSingleton().GetContext()->GetFocusElement() != consoleWindow)
							consoleWindow->Show();
					}
				case CL_KEY_LEFT:
				case CL_KEY_RIGHT:
					m_CamVelocity.x = 0;
					break;
				case CL_KEY_UP:
				case CL_KEY_DOWN:
					m_CamVelocity.y = 0;
					break;

				case CL_MOUSE_LEFT:
					// Only run the tool's left click command if the mouse was PRESSED outside a GUI
					//  window, as well as released outside one (getting here indicates that
					//  the mouse was at least /released/ outside a GUI window):
					if (m_ReceivedMouseDown)
					{
						// This conditional makes sure the move is only counted as a
						//  drag operation if it was probably intended to be - if the
						//  move tool isn't selected, small movements are probably unintentional
						if (m_Dragging && (m_ActiveTool == tool_move || (ev.PointerPosition - m_DragFrom).squared_length() > 1.0f))
						{
							if (!m_DragSelect)
							{
								// How much the pointer has moved since the mouse-button was pressed
								Vector2 offset = ev.PointerPosition - m_DragFrom;

								// This function will perform the actual drag action
								auto fn_move = [&offset](const MapEntityPtr& map_entity)
								{
									EditorMapEntity* editorEntity = dynamic_cast<EditorMapEntity*>(map_entity.get());
									editorEntity->entity->SetPosition(editorEntity->entity->GetPosition() + offset);
									// Set the offset of the overlays
									std::for_each(editorEntity->selectionRenderables.begin(), editorEntity->selectionRenderables.end(), [](const RenderablePtr& renderable) {
										renderable->SetOffset(Vector2::zero());
									});
								};

								// Create an undo-action for this drag action
								if (m_SelectedEntities.size() > 1)
								{
									MultiAction* multi_drag = new MultiAction();
									for (auto it = m_SelectedEntities.begin(), end = m_SelectedEntities.end(); it != end; ++it)
									{
										const EditorMapEntityPtr& editor_entity = boost::dynamic_pointer_cast<EditorMapEntity>(*it);
										if (editor_entity)
										{
											UndoableActionPtr move_action( new MoveAction(editor_entity, offset) );
											multi_drag->AddAction(move_action);
										}
										// Actually move the entity
										fn_move(*it);
									}
									m_UndoManager.Add( UndoableActionPtr(multi_drag) );
								}
								else if (m_SelectedEntities.size() == 1)
								{
									const EditorMapEntityPtr& editor_entity = boost::dynamic_pointer_cast<EditorMapEntity>(*m_SelectedEntities.begin());
									if (editor_entity)
										m_UndoManager.Add( UndoableActionPtr(new MoveAction(editor_entity, offset)) );
									// Actually move the entity
									fn_move(*m_SelectedEntities.begin());
								}
							}
						}
						else
						{
							processLeftClick(ev);
						}
						m_ReceivedMouseDown = false;
					}
					m_Dragging = false;
					m_DragSelect = false;
					break;
				case CL_MOUSE_RIGHT:
					processRightClick(ev);
					break;
				case CL_MOUSE_MIDDLE:
					// Show Radial Menu (the editor icons for each entity under the mouse displayed in a circle around the cursor)
					break;
				}
			} // Button released
		}
	}

	void translatePointerToWorld(Vector2::type* x, Vector2::type* y, Renderer* renderer, const ViewportPtr& view)
	{
		// Figure out where the pointer is within the world
		CL_Rectf area;
		renderer->CalculateScreenArea(area, view, true);
		*x += area.left; *y += area.top;
	}

	void translatePointerToWorld(Vector2* pointerPos, Renderer* renderer, const ViewportPtr& view)
	{
		translatePointerToWorld(&pointerPos->x, &pointerPos->y, renderer, view);
	}

	inline void Editor::processLeftClick(const RawInput &ev)
	{
		switch (m_ActiveTool)
		{
		case tool_place:
			if (!m_CurrentEntityType.empty() && m_EntityTypes.find(m_CurrentEntityType) != m_EntityTypes.end())
			{
				Vector2 position(ev.PointerPosition);

				CL_Rectf area;
				m_Renderer->CalculateScreenArea(area, m_Viewport, true);

				EditorMapEntityPtr mapEntity = CreateEntity(m_CurrentEntityType, "", m_PseudoEntityMode, area.left + position.x, area.top + position.y);
				if (mapEntity)
				{
					UndoableActionPtr action(new AddRemoveEntityAction(this, m_EntityFactory, m_EntityManager, mapEntity, true));
					addUndoAction(action);
				}
			}
			break;
		case tool_delete:
			break;
		case tool_move:
			// Toggle selection
			if (m_ActiveTool == tool_move && m_ShiftSelect)
			{
				MapEntityArray underMouse;
				GetEntitiesAt(underMouse, ev.PointerPosition, true);
				for (auto it = underMouse.begin(), end = underMouse.end(); it != end; ++it)
				{
					EditorMapEntity* editorEntity = dynamic_cast<EditorMapEntity*>(it->get());
					if (editorEntity->selected)
						DeselectEntity(editorEntity);
					else
						SelectEntity(editorEntity);
				}
			}
			break;
		}
	}

	inline void Editor::processRightClick(const RawInput &ev)
	{
		switch (m_ActiveTool)
		{
		case tool_place:
		case tool_delete:
		case tool_move:
			{
			// Find Entities under the cursor
			Vector2 worldPosition(ev.PointerPosition);
			translatePointerToWorld(&worldPosition, m_Renderer, m_Viewport);

			GameMapLoader::MapEntityArray entitiesUnderMouse;
			GetEntitiesAt(entitiesUnderMouse, worldPosition);
			ShowContextMenu(ev.PointerPosition, entitiesUnderMouse);
			}
			break;
		}
	}

	void Editor::ProcessEvent(Rocket::Core::Event& ev)
	{
		if (ev == "close") // A property dialog is being closed
		{
			for (EntityEditorDialogArray::iterator it = m_EntityDialogs.begin(), end = m_EntityDialogs.end(); it != end; ++it)
			{
				EntityEditorDialogPtr &dialog = *it;
				if (dialog->GetDocument() == ev.GetTargetElement())
				{
					m_EntityDialogs.erase(it);
					break;
				}
			} 
		}
	}

	void Editor::DisplayError(const std::string &title, const std::string &message)
	{
		SendToConsole(title + " error:" + message);
	}

	void Editor::clearCtxMenu(MenuItem *menu, Editor::MenuItemConnections &connections)
	{
		for (MenuItemConnections::iterator it = connections.begin(), end = connections.end(); it != end; ++it)
		{
			it->disconnect();
		}
		connections.clear();
		menu->RemoveAllChildren();
	}

	void Editor::ShowContextMenu(const Vector2i &position, const GameMapLoader::MapEntityArray &entities)
	{
		clearCtxMenu(m_PropertiesMenu, m_PropertiesMenuConnections);
		clearCtxMenu(m_EntitySelectionMenu, m_SelectionMenuConnections);

		MenuItem* item = new MenuItem("Deselect All", "deselect");
		m_SelectionMenuConnections.push_back(
			item->SignalClicked.connect( [this](const MenuItemEvent& ev) { DeselectAll(); m_RightClickMenu->Hide(); } ) );
		m_EntitySelectionMenu->AddChild(item);

		for (MapEntityArray::const_iterator it = entities.begin(), end = entities.end(); it != end; ++it)
		{
			const MapEntityPtr &mapEntity = *it;
			const EntityPtr &entity = mapEntity->entity;
			// Add an item for this entity to the Properties sub-menu
			MenuItem *item = new MenuItem(std::string(mapEntity->hasName ? entity->GetName() : "") + "(" + entity->GetType() + ")", entity->GetName());
			m_PropertiesMenu->AddChild(item);
			{
				boost::signals2::connection signalConnection = item->SignalClicked.connect( boost::bind(&Editor::ctxShowProperties, this, _1, mapEntity.get()) );
				m_PropertiesMenuConnections.push_back(signalConnection);
			}
			item->release();

			// Add an item for this entity to the Select sub-menu
			item = new MenuItem(std::string(mapEntity->hasName ? entity->GetName() : "") + "(" + entity->GetType() + ")", entity->GetName());
			m_EntitySelectionMenu->AddChild(item);
			{
				boost::signals2::connection signalConnection = item->SignalClicked.connect( boost::bind(&Editor::ctxSelectEntity, this, _1, mapEntity.get()));
				m_SelectionMenuConnections.push_back(signalConnection);
			}
			item->release();
		}

		m_RightClickMenu->Show(position.x, position.y);
	}

	void Editor::addUndoAction(const UndoableActionPtr &action)
	{
		m_UndoManager.Add(action);
		repopulateUndoMenu();
	}

	void Editor::repopulateUndoMenu()
	{
	}

	void Editor::ctxShowProperties(const MenuItemEvent &ev, MapEntity* map_entity)
	{
		ShowProperties(map_entity);
		m_RightClickMenu->Hide();
	}

	void Editor::ctxSelectEntity(const MenuItemEvent &ev, MapEntity* map_entity)
	{
		if (!m_ShiftSelect)
			DeselectAll();
		SelectEntity(map_entity);

		m_RightClickMenu->Hide();
	}

	void Editor::ShowProperties(const EntityPtr &entity)
	{
		for (auto it = m_PseudoEntities.begin(), end = m_PseudoEntities.end(); it != end; ++it)
		{
			const MapEntityPtr &gmEntity = *it;
			if (gmEntity->entity == entity)
			{
				ShowProperties(gmEntity);
				return;
			}
		}
		for (auto it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			const MapEntityPtr &gmEntity = *it;
			if (gmEntity->entity == entity)
			{
				ShowProperties(gmEntity);
				return;
			}
		}
	}

	void Editor::ShowProperties(const MapEntityPtr &entity)
	{
		EntityEditorDialogPtr dialog(new EntityEditorDialog(entity, &m_UndoManager));
		dialog->Show();
		dialog->GetDocument()->AddEventListener("close", this);
		m_EntityDialogs.push_back(dialog);
	}

	EditorMapEntityPtr Editor::CreateEntity(const std::string &type, const std::string &name, bool synced, float x, float y)
	{
		EditorMapEntityPtr gmEntity(new EditorMapEntity());
		gmEntity->entity = m_EntityFactory->InstanceEntity(type, name);
		if (!gmEntity->entity)
		{
			SendToConsole("Failed to create entity of type " + type);
			return gmEntity;
		}

		if (synced)
		{
			ObjectID id = m_IdStack.getFreeID();
			gmEntity->entity->SetID(id);
			m_InstanceSynchroniser->TakeID(id);
		}

		gmEntity->CreateEditorFixture();

		if (!name.empty())
		{
			gmEntity->entity->_setName(name);
			gmEntity->hasName = true;
		}
		else
			gmEntity->hasName = false;

		gmEntity->synced = synced;

		gmEntity->entity->SetPosition(Vector2(x, y));

		addMapEntity(gmEntity);

		// Record type usage
		m_UsedTypes.insert(type);

		return gmEntity;
	}

	//! The old point query
	class MapEntityPointQuery : public b2QueryCallback
	{
	public:
		MapEntityPointQuery(GameMapLoader::MapEntityArray *output_array, const b2Vec2& point)
		{
			m_Point = point;
			m_Entities = output_array;
		}

		bool ReportFixture(b2Fixture* fixture)
		{
			b2Body* body = fixture->GetBody();
			{
				bool inside = fixture->TestPoint(m_Point);
				if (inside)
				{
					Fixture *wrapper = static_cast<Fixture*>( fixture->GetUserData() );
					MapEntityFixtureUserData* userData = dynamic_cast<MapEntityFixtureUserData*>(wrapper->GetUserData().get());
					if (userData != NULL)
					{
						m_Entities->push_back(userData->map_entity);
					}
				}
			}

			// Continue the query.
			return true;
		}

		b2Vec2 m_Point;
		GameMapLoader::MapEntityArray* m_Entities;
	};

	//! Implements b2QueryCallback, returning all entities within an AABB that satisfy the passed function
	class MapEntityQuery : public b2QueryCallback
	{
	public:
		MapEntityQuery(GameMapLoader::MapEntityArray *output_array, std::function<bool (b2Fixture*, bool&)> test_fn)
			: m_Test(test_fn),
			m_Entities(output_array)
		{
		}

		bool ReportFixture(b2Fixture* fixture)
		{
			b2Body* body = fixture->GetBody();
			bool continue_query = true;
			if (!m_Test || m_Test(fixture, continue_query))
			{
				Fixture *wrapper = static_cast<Fixture*>( fixture->GetUserData() );
				MapEntityFixtureUserData* userData = dynamic_cast<MapEntityFixtureUserData*>(wrapper->GetUserData().get());
				if (userData != NULL)
				{
					m_Entities->push_back(userData->map_entity);
				}
			}
			// Continue if the test func. said to
			return continue_query;
		}

		std::function<bool (b2Fixture*, bool&)> m_Test;
		GameMapLoader::MapEntityArray* m_Entities;
	};

	void Editor::GetEntitiesAt(GameMapLoader::MapEntityArray &out, const Vector2 &position, bool screen_to_world)
	{
		b2Vec2 p(position.x, position.y);
		if (screen_to_world)
			translatePointerToWorld(&p.x, &p.y, m_Renderer, m_Viewport);
		p *= s_SimUnitsPerGameUnit;

		// Make a small box.
		b2AABB aabb;
		b2Vec2 d;
		d.Set(0.001f, 0.001f);
		aabb.lowerBound = p - d;
		aabb.upperBound = p + d;

		// Query the world for overlapping shapes, the lambda makes sure they actually hit the point
		MapEntityQuery callback(&out, [p](b2Fixture* fixture, bool&)->bool { return fixture->TestPoint(p); });

		PhysicalWorld::getSingleton().GetB2World()->QueryAABB(&callback, aabb);
	}

	void Editor::GetEntitiesOverlapping(GameMapLoader::MapEntityArray &out, const CL_Rectf &rectangle, bool convert_coords)
	{
		// Figure out where the rectangle is in the world
		Vector2 top_left(std::min(rectangle.left, rectangle.right), std::min(rectangle.top, rectangle.bottom));
		Vector2 bottom_right(std::max(rectangle.left, rectangle.right), std::max(rectangle.top, rectangle.bottom));
		if (convert_coords)
		{
			translatePointerToWorld(&top_left, m_Renderer, m_Viewport);
			translatePointerToWorld(&bottom_right, m_Renderer, m_Viewport);
		}
		// Make an AABB for the given rect
		b2AABB aabb;
		aabb.lowerBound.Set(top_left.x * s_SimUnitsPerGameUnit, top_left.y * s_SimUnitsPerGameUnit);
		aabb.upperBound.Set(bottom_right.x * s_SimUnitsPerGameUnit, bottom_right.y * s_SimUnitsPerGameUnit);

		// Query the world for overlapping shapes
		MapEntityQuery callback(&out, [](b2Fixture*, bool&)->bool { return true; });

		PhysicalWorld::getSingleton().GetB2World()->QueryAABB(&callback, aabb);
	}

	void Editor::AddEntity(const Editor::MapEntityPtr &map_entity)
	{
		addMapEntity(map_entity);
	}

	void removeFrom(Editor::MapEntityArray &container, const Editor::MapEntityPtr &map_entity)
	{
		for (Editor::MapEntityArray::iterator it = container.begin(), end = container.end(); it != end; ++it)
			if (*it == map_entity)
			{
				container.erase(it);
				break;
			}
	}

	void Editor::RemoveEntity(const Editor::MapEntityPtr &map_entity)
	{
		if (map_entity->entity->IsPseudoEntity())
			removeFrom(m_PseudoEntities, map_entity);
		else
			removeFrom(m_Entities, map_entity);

		DeselectEntity(map_entity);

		m_EntityManager->RemoveEntity(map_entity->entity);

		ScriptManager::getSingleton().GetEnginePtr()->GarbageCollect();
	}

	Editor::MapEntityPtr Editor::FindEntity(const EntityPtr& entity)
	{
		MapEntityArray* collection = nullptr;
		// Choose the collection that this entity will be part of (if it is present)
		if (entity->IsSyncedEntity())
			collection = &m_Entities;
		else
			collection = &m_PseudoEntities;
		// Find the MapEntity in the collection that points to the given entity
		auto _where = std::find_if(m_Entities.begin(), m_Entities.end(), [&entity](const MapEntityPtr& map_entity)->bool {
			return (map_entity->entity ? map_entity->entity == entity : false);
		});
		// If one was found, return it
		if (_where != m_Entities.end())
			return *_where;
		else
			return MapEntityPtr();
	}

	void Editor::SelectEntity(const Editor::MapEntityPtr &map_entity)
	{
		EditorMapEntityPtr editorEntity = boost::dynamic_pointer_cast<EditorMapEntity>( map_entity );
		// Mark the entity as selected
		auto result = m_SelectedEntities.insert(editorEntity);
		if (!result.second) // Don't reselect
			return;
		editorEntity->selected = true;
		// Add the GUI representation of the selection
		RenderableGeneratedSprite *selectionOverlay = new RenderableGeneratedSprite(m_SelectionOverlay);
		selectionOverlay->AddTag("select_overlay");
		selectionOverlay->AddTag("select_overlay_box");
		selectionOverlay->SetOrigin(origin_center);
		editorEntity->entity->AddRenderable(selectionOverlay); // Add the renderable to the entity
		// List the renderable in the editor entity (so it can be efficiently accessed - without searching every renderable)
		editorEntity->selectionRenderables.push_back(selectionOverlay);

		selectionOverlay = new RenderableGeneratedSprite(m_SelectionOverlay_Rotate);
		selectionOverlay->AddTag("select_overlay");
		selectionOverlay->AddTag("select_overlay_rotate");
		selectionOverlay->SetOrigin(origin_center);
		editorEntity->entity->AddRenderable(selectionOverlay);
		editorEntity->selectionRenderables.push_back(selectionOverlay);
	}

	void Editor::DeselectEntity(const Editor::MapEntityPtr &map_entity)
	{
		EditorMapEntityPtr editorEntity = boost::dynamic_pointer_cast<EditorMapEntity>( map_entity );
		auto _where = m_SelectedEntities.find(editorEntity);
		if (_where != m_SelectedEntities.end())
		{
			editorEntity->selected = false;
			editorEntity->entity->RemoveRenderablesWithTag("select_overlay");
			editorEntity->selectionRenderables.clear();
			m_SelectedEntities.erase(_where);
		}
	}

	void Editor::DeselectAll()
	{
		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [](const EditorMapEntityPtr &selectedEntity) {
			selectedEntity->entity->RemoveRenderablesWithTag("select_overlay");
			selectedEntity->selected = false; });
		m_SelectedEntities.clear();
	}

	void Editor::LookUpEntityType(StringVector &results, const std::string &search_term)
	{
		typedef std::pair<EntityTypeLookupSet::iterator, EntityTypeLookupSet::iterator> IterRange;
		IterRange range = m_EntityTypes.prefix_range(search_term);

		while (range.first != range.second)
		{
			results.push_back(*range.first);
			++range.first;
		}
	}

	void Editor::LookUpEntityType(const std::string &search_term)
	{
		StringVector results;
		LookUpEntityType(results, search_term);
		m_EditorDataSource->UpdateSuggestions(results);
	}

	const std::string & Editor::GetSuggestion(size_t index)
	{
		return m_EditorDataSource->GetSuggestion(index);
	}

	void Editor::SetDisplayActualSprites(bool enable)
	{
		SendToConsole("Editor::SetDisplayActualSprites() is not yet implemented. You should get on that.");
	}

	void Editor::SetActiveTool(EditorTool tool)
	{
		m_ActiveTool = tool;
	}

	void Editor::SetEntityType(const std::string &type)
	{
		m_CurrentEntityType = type;
	}

	void Editor::SetEntityMode(bool pseudo)
	{
		m_PseudoEntityMode = pseudo;
	}

	void Editor::AttachUndoMenu(ElementUndoMenu *element)
	{
		m_UndoManager.AttachListener(element, true);
	}

	void Editor::AttachRedoMenu(ElementUndoMenu *element)
	{
		m_UndoManager.AttachListener(element, false);
	}

	void Editor::Undo()
	{
		m_UndoManager.Undo();
	}

	void Editor::Redo()
	{
		m_UndoManager.Redo();
	}

	void Editor::Undo(unsigned int index)
	{
		m_UndoManager.Undo(index);
	}

	void Editor::Redo(unsigned int index)
	{
		m_UndoManager.Redo(index);
	}

	void Editor::Save(const std::string &filename)
	{
		CL_String dataFileName = CL_PathHelp::get_basename(filename) + ".entdata";
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		CL_IODevice out = vdir.open_file("Editor/" + dataFileName, CL_File::create_always, CL_File::access_write);

		serialiseEntityData(out);

		TiXmlDocument *doc = new TiXmlDocument();
		buildMapXml(doc);
		SaveXml_PhysFS(doc, "Editor/" + filename);
		delete doc;

		m_CurrentFilename = filename;
	}

	void Editor::Load(const std::string &filename)
	{
		CL_String dataFileName = CL_PathHelp::get_basename(filename) + ".entdata";
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		CL_IODevice in = vdir.open_file("Editor/" + dataFileName, CL_File::open_existing, CL_File::access_read);

		m_Entities.clear();
		m_PseudoEntities.clear();
		//m_PlainEntityArray.clear();
		m_EntityManager->Clear();

		SerialisedDataArray archetypes, entities;
		loadEntityData(in, archetypes, entities);

		EditorEntityDeserialiser deserialiserImpl;

		TiXmlDocument *doc = OpenXml_PhysFS("Editor/" + filename);
		try
		{
			parseMapXml(doc, archetypes, entities, deserialiserImpl);
		}
		catch (FileTypeException &ex)
		{
			DisplayError(ex.GetName(), ex.GetDescription());
		}
		delete doc;

		initialiseEntities(entities, deserialiserImpl);

		m_CurrentFilename = filename;
	}

	void Editor::Save()
	{
		if (!m_CurrentFilename.empty())
			Save(m_CurrentFilename);
	}

	void Editor::Load()
	{
		if (!m_CurrentFilename.empty())
			Load(m_CurrentFilename);
	}

	void Editor::Close()
	{
		m_Entities.clear();
		m_PseudoEntities.clear();
		//m_PlainEntityArray.clear();
		m_EntityManager->Clear();

		m_IdStack.freeAll();
	}

	void Editor::Compile(const std::string &filename)
	{
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		CL_IODevice out = vdir.open_file(filename.c_str(), CL_File::create_always, CL_File::access_write);

		Save();
		GameMapLoader::CompileMap(out, m_UsedTypes, m_Archetypes, m_PseudoEntities, m_Entities);
	}

	void Editor::SpawnEntities()
	{		
		ObjectID nextId = 1;
		for (MapEntityArray::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			MapEntityPtr &mapEntity = *it;
			if (mapEntity->synced)
				mapEntity->entity->SetID(nextId++);
			mapEntity->entity->Spawn();
		}
		m_InstanceSynchroniser->Reset(nextId);
	}

	void Editor_SelectEntity(asIScriptObject* entity, Editor* obj)
	{
		Editor::MapEntityPtr mapEntity = obj->FindEntity(ScriptedEntity::GetAppObject(entity));
		if (mapEntity)
			obj->SelectEntity(mapEntity);
	}

	void Editor::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSingletonType<Editor>("Editor", engine);

		r = engine->RegisterEnum("EditorTool"); FSN_ASSERT(r >= 0);
		r = engine->RegisterEnumValue("EditorTool", "place", tool_place); FSN_ASSERT(r >= 0);
		r = engine->RegisterEnumValue("EditorTool", "delete", tool_delete); FSN_ASSERT(r >= 0);
		r = engine->RegisterEnumValue("EditorTool", "move", tool_move); FSN_ASSERT(r >= 0);
		r = engine->RegisterEnumValue("EditorTool", "run_script", tool_move); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void setActiveTool(EditorTool)",
			asMETHOD(Editor, SetActiveTool), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void selectEntity(IEntity@)",
			asFUNCTION(Editor_SelectEntity), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void selectEntity(MapEntity@)",
			asMETHOD(Editor, SelectEntity), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void searchTypes(StringArray& out, string &in)",
			asMETHODPR(Editor, LookUpEntityType, (StringVector&, const std::string&), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void updateSuggestions(const string &in)",
			asMETHODPR(Editor, LookUpEntityType, (const std::string&), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"const string& getSuggestion(uint)",
			asMETHOD(Editor, GetSuggestion), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void setDisplayActualSprites(bool)",
			asMETHOD(Editor, SetDisplayActualSprites), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void setEntityType(const string &in)",
			asMETHOD(Editor, SetEntityType), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void setPlacePseudoEntities(bool)",
			asMETHOD(Editor, SetEntityMode), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void attachUndoMenu(ElementUndoMenu@)",
			asMETHOD(Editor, AttachUndoMenu), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void attachRedoMenu(ElementUndoMenu@)",
			asMETHOD(Editor, AttachRedoMenu), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void undo()",
			asMETHODPR(Editor, Undo, (void), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void redo()",
			asMETHODPR(Editor, Redo, (void), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void undo(uint)",
			asMETHODPR(Editor, Undo, (unsigned int), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void redo(uint)",
			asMETHODPR(Editor, Redo, (unsigned int), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void enable()",
			asMETHOD(Editor, Enable), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void startEditor()",
			asMETHOD(Editor, Start), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void stopEditor()",
			asMETHOD(Editor, Stop), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void save(const string &in)",
			asMETHODPR(Editor, Save, (const std::string &), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void load(const string &in)",
			asMETHODPR(Editor, Load, (const std::string &), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void save()",
			asMETHOD(Editor, Save), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void load()",
			asMETHOD(Editor, Load), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void close()",
			asMETHOD(Editor, Close), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void compile(const string &in)",
			asMETHODPR(Editor, Compile, (const std::string &), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

	void Editor::addMapEntity(const GameMapLoader::MapEntityPtr &map_entity)
	{
		if (map_entity->entity->IsPseudoEntity())
			m_PseudoEntities.push_back(map_entity);
		else
			m_Entities.push_back(map_entity);
		//m_PlainEntityArray.push_back(map_entity->entity);
		//m_Streamer->AddEntity(map_entity->entity);
		m_EntityManager->AddEntity(map_entity->entity);
	}

	void Editor::serialiseEntityData(CL_IODevice file)
	{
		// TODO:? the code that serialises entity data to a CL_IODevice could conceiveably be a public GameMapLoader method,
		//  since it already exists there, and this method would simply iterate over each entity and call that. However,
		//  since serialising/writing entity data is so simple I haven't at this time implemented that - much in the same
		//  way that you don't implement a class method for writing integers to std::cout.

		file.write_uint32(m_Archetypes.size());
		unsigned int dataIndex = 0;
		for (GameMapLoader::ArchetypeMap::iterator it = m_Archetypes.begin(), end = m_Archetypes.end(); it != end; ++it)
		{
			GameMapLoader::Archetype &archetype = it->second;
			// Update the data index in the editor's archetype info (this is written to the XML file)
			archetype.dataIndex = dataIndex++;

			// Write the state information
			file.write_uint32(archetype.packet.mask);
			file.write_string_a(archetype.packet.data.c_str());
		}

		file.write_uint32(m_PseudoEntities.size() + m_Entities.size());
		SerialisedData state;
		for (GameMapLoader::MapEntityArray::iterator it = m_PseudoEntities.begin(), end = m_PseudoEntities.end(); it != end; ++it)
		{
			const GameMapLoader::MapEntityPtr &gmEntity = *it;
			state.mask = gmEntity->stateMask ;
			gmEntity->entity->SerialiseState(state, true);

			file.write_uint32(state.mask);
			file.write_string_a(state.data);
		}
		for (GameMapLoader::MapEntityArray::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			const GameMapLoader::MapEntityPtr &gmEntity = *it;
			state.mask = gmEntity->stateMask;
			gmEntity->entity->SerialiseState(state, true);

			file.write_uint32(state.mask);
			file.write_string_a(state.data);
		}
	}

	void Editor::loadEntityData(CL_IODevice file, Editor::SerialisedDataArray &archetypes, Editor::SerialisedDataArray &entities)
	{
		SerialisedData state;

		unsigned int numArchetypes = file.read_uint32();
		archetypes.reserve(numArchetypes);
		for (unsigned int i = 0; i < numArchetypes; ++i)
		{
			state.mask = file.read_int32();
			CL_String8 stateString = file.read_string_a();
			state.data.assign(stateString.data(), stateString.length());
			archetypes.push_back(state);
		}

		unsigned int numEntityStates = file.read_uint32();
		entities.reserve(numEntityStates);
		for (unsigned int i = 0; i < numEntityStates; ++i)
		{
			state.mask = file.read_int32();
			CL_String8 stateString = file.read_string_a();
			state.data.assign(stateString.data(), stateString.length());
			entities.push_back(state);
		}
	}

	void Editor::buildMapXml(TiXmlDocument *document)
	{
		TiXmlDeclaration *decl = new TiXmlDeclaration( XML_STANDARD, "", "" );
		document->LinkEndChild(decl);

		TiXmlElement *root = new TiXmlElement("editor_data");
		document->LinkEndChild(root);

		//TiXmlElement *typesElm = new TiXmlElement("used_types");
		//root->LinkEndChild(typesElm);
		//TiXmlElement *elem;
		//for (StringSet::iterator it = m_UsedTypes.begin(), end = m_UsedTypes.end(); it != end; ++it)
		//{
		//	elem = new TiXmlElement("type");
		//	elem->SetAttribute("name", it->c_str());

		//	typesElm->LinkEndChild(elem);
		//}

		TiXmlElement *archetypesElm = new TiXmlElement("archetypes");
		root->LinkEndChild(archetypesElm);
		for (GameMapLoader::ArchetypeMap::iterator it = m_Archetypes.begin(), end = m_Archetypes.end(); it != end; ++it)
		{
			const GameMapLoader::Archetype &archetype = it->second;

			TiXmlElement *archetypeElm = new TiXmlElement("archetype");
			archetypeElm->SetAttribute("name", it->first);
			archetypeElm->SetAttribute("entity_type", archetype.entityTypename);
			archetypeElm->SetAttribute("data_index", archetype.dataIndex);
			archetypesElm->LinkEndChild(archetypeElm);
		}

		TiXmlElement *entities = new TiXmlElement("entities");
		root->LinkEndChild(entities);
		unsigned int dataIndex = 0;
		for (GameMapLoader::MapEntityArray::iterator it = m_PseudoEntities.begin(), end = m_PseudoEntities.end(); it != end; ++it)
		{
			const GameMapLoader::MapEntityPtr &gmEntity = *it;

			TiXmlElement *entityElm = new TiXmlElement("entity");
			if (gmEntity->hasName)
				entityElm->SetAttribute("name", gmEntity->entity->GetName());
			entityElm->SetAttribute("type", gmEntity->entity->GetType());
			if (!gmEntity->archetypeId.empty())
				entityElm->SetAttribute("archetype", gmEntity->archetypeId);
			entityElm->SetAttribute("data_index", dataIndex++);
			entities->LinkEndChild(entityElm);

			TiXmlElement *transform = new TiXmlElement("location");
			std::ostringstream positionStr; positionStr << gmEntity->entity->GetPosition().x << "," << gmEntity->entity->GetPosition().y;
			transform->SetAttribute("position", positionStr.str());
			transform->SetAttribute("angle", boost::lexical_cast<std::string>(gmEntity->entity->GetAngle()));
			entityElm->LinkEndChild(transform);
		}
		for (GameMapLoader::MapEntityArray::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			const GameMapLoader::MapEntityPtr &gmEntity = *it;

			TiXmlElement *entityElm = new TiXmlElement("entity");
			entityElm->SetAttribute("id", gmEntity->entity->GetID());
			if (gmEntity->hasName)
				entityElm->SetAttribute("name", gmEntity->entity->GetName());
			entityElm->SetAttribute("type", gmEntity->entity->GetType());
			if (!gmEntity->archetypeId.empty())
				entityElm->SetAttribute("archetype", gmEntity->archetypeId);
			entityElm->SetAttribute("data_index", dataIndex++);
			entities->LinkEndChild(entityElm);

			TiXmlElement *transform = new TiXmlElement("location");
			std::ostringstream positionStr; positionStr << gmEntity->entity->GetPosition().x << "," << gmEntity->entity->GetPosition().y;
			transform->SetAttribute("position", positionStr.str());
			transform->SetAttribute("angle", boost::lexical_cast<std::string>(gmEntity->entity->GetAngle()));
			entityElm->LinkEndChild(transform);
		}
	}

	void Editor::parseMapXml(TiXmlDocument *document, const Editor::SerialisedDataArray &archetypes, const Editor::SerialisedDataArray &entities, Editor::EditorEntityDeserialiser &deserialiser)
	{
		TiXmlElement *root = document->FirstChildElement();
		if (root->ValueStr() != "editor_data")
			FSN_EXCEPT(ExCode::FileType, "Editor::parseMapXml", "The given file is not a map file");

		TiXmlElement *child = root->FirstChildElement();
		bool parsedArchetypes = false;
		while (child != NULL)
		{
			if (child->ValueStr() == "archetypes")
			{
				parse_Archetypes(child, archetypes);
				parsedArchetypes = true;
			}
			else if (child->ValueStr() == "entities")
			{
				if (parsedArchetypes)
					parse_Entities(child, entities.size(), deserialiser);
				else
					FSN_EXCEPT(ExCode::FileType, "Editor::parseMapXml", "Map XML files must have the <archetypes> element before the <entities> element");
			}

			child = child->NextSiblingElement();
		}
	}

	bool getAttribute(std::string &out, TiXmlElement *element, const std::string &name, bool throw_on_failure = true)
	{
		const char *attribute = element->Attribute(name.c_str());
		if (attribute != NULL)
		{
			out = attribute;
			return true;
		}
		else if (throw_on_failure)
		{
			char name0 = std::tolower(name[0], std::locale());
			std::string indefinateArticle = (name0 == 'a' || name0 == 'e' || name0 == 'i' || name0 == 'o' || name0 == 'u') ? "a" : "an";
			std::string message = "<" + element->ValueStr() + "> elements must have " + indefinateArticle + " '" + name + "' attribute";
			FSN_EXCEPT(ExCode::FileType, "FusionEngine::getAttribute", message);
		}
		else
			return false;
	}

	std::string getAttribute(TiXmlElement *element, const std::string &name, bool throw_on_failure = true)
	{
		std::string value;
		getAttribute(value, element, name, throw_on_failure);
		return value;
	}

	void Editor::parse_Archetypes(TiXmlElement *element, const Editor::SerialisedDataArray &archetype_data)
	{
		TiXmlElement *child = element->FirstChildElement();
		std::string name;
		while (child != NULL)
		{
			getAttribute(name, child, "name");
			GameMapLoader::Archetype &archetype = m_Archetypes[name];

			getAttribute(archetype.entityTypename, child, "entity_type");

			const char *attribute = child->Attribute("data_index");
			if (attribute != NULL)
				archetype.dataIndex = boost::lexical_cast<unsigned int>(attribute);
			else
				archetype.dataIndex = archetype_data.size();

			if (archetype.dataIndex < archetype_data.size())
			{
				archetype.packet = archetype_data[archetype.dataIndex];
				// Record type usage
				m_UsedTypes.insert(archetype.entityTypename);
			}
			else
				SendToConsole("Reading map XML: The archetype '" + name + "' has an invalid index attribute");

			child = child->NextSiblingElement();
		}
	}

	template <typename T>
	void parse_vector(const std::string &value, T *x, T *y)
	{
		std::string::size_type d = value.find(",");
		if (d != std::string::npos)
		{
			std::string xstr = value.substr(0, d), ystr = value.substr(d+1);
			boost::trim(xstr); boost::trim(ystr);
			*x = boost::lexical_cast<T>(xstr);
			*y = boost::lexical_cast<T>(ystr);
		}
		else
		{
			*x = *y = boost::lexical_cast<T>(boost::trim_copy(value));
		}
	}

	void Editor::parse_Entities(TiXmlElement *element, unsigned int entity_data_count, Editor::EditorEntityDeserialiser &deserialiser)
	{
		EntityFactory *factory = m_EntityFactory;

		TiXmlElement *child = element->FirstChildElement();
		std::string name, type, idStr;
		// Resize the entities array to construct enough MapEntity objects
		m_Entities.reserve(entity_data_count);
		while (child != NULL)
		{
			unsigned int dataIndex = entity_data_count;
			{
				const char *attribute = child->Attribute("data_index");
				if (attribute != NULL)
					dataIndex = boost::lexical_cast<unsigned int>(attribute);
			}

			name.clear();

			getAttribute(type, child, "type");
			getAttribute(idStr, child, "id", false);
			getAttribute(name, child, "name", false);

			if (dataIndex >= entity_data_count)
			{
				SendToConsole("Reading map XML: The entity '" + type + ":" + (name.empty() ? std::string("default") : name) + "' has an invalid index attribute");
				child = child->NextSiblingElement();
				continue;
			}

			EditorMapEntityPtr gmEntity(new EditorMapEntity());

			gmEntity->dataIndex = dataIndex;

			if (name.empty())
			{
				name = "default";
				gmEntity->hasName = false;
			}

			gmEntity->entity = factory->InstanceEntity(type, name);
			if (gmEntity->entity)
			{
				m_UsedTypes.insert(type);

				// Check for archetype
				getAttribute(gmEntity->archetypeId, child, "archetype", false);

				// Create the fixture that allows the user to click on the entity
				gmEntity->CreateEditorFixture();

				EntityPtr &entity = gmEntity->entity;

				TiXmlElement *location = child->FirstChildElement();
				if (location->ValueStr() == "location")
				{
					std::string attrValue; getAttribute(attrValue, location, "position");
					Vector2 position;
					parse_vector(attrValue, &position.x, &position.y);
					entity->SetPosition(position);

					getAttribute(attrValue, location, "angle");
					float angle = boost::lexical_cast<float>(attrValue);
				}

				ObjectID id = 0;
				if (!idStr.empty())
					id = boost::lexical_cast<ObjectID>(idStr);

				entity->SetID(id);
				addMapEntity(gmEntity);
				// This will allow Entities that reference this Entity (by ID) in their SerialisedState data to find it
				deserialiser.ListEntity(entity);
			}

			child = child->NextSiblingElement();
		}
	}

	inline void Editor::initialiseEntities(
		const GameMapLoader::MapEntityArray::iterator &first, const GameMapLoader::MapEntityArray::iterator &last,
		const Editor::SerialisedDataArray &entity_data,
		const Editor::EditorEntityDeserialiser &deserialiser_impl)
	{
		EntityDeserialiser entityDeserialiser(&deserialiser_impl);

		for (GameMapLoader::MapEntityArray::iterator it = first; it != last; ++it)
		{
			const GameMapLoader::MapEntityPtr &gmEntity = *it;
			// Initialise with archetype data
			if (!gmEntity->archetypeId.empty())
			{
				GameMapLoader::ArchetypeMap::const_iterator _where = m_Archetypes.find(gmEntity->archetypeId);
				if (_where != m_Archetypes.end() && _where->second.entityTypename == gmEntity->entity->GetType())
					gmEntity->entity->DeserialiseState(_where->second.packet, true, entityDeserialiser);
			}
			// Init. with specific entity data
			gmEntity->entity->DeserialiseState(entity_data[gmEntity->dataIndex], true, entityDeserialiser);
		}
	}

	void Editor::initialiseEntities(const Editor::SerialisedDataArray &entity_data, const EditorEntityDeserialiser &deserialiser_impl)
	{
		initialiseEntities(m_PseudoEntities.begin(), m_PseudoEntities.end(), entity_data, deserialiser_impl);
		initialiseEntities(m_Entities.begin(), m_Entities.end(), entity_data, deserialiser_impl);
	}

}
