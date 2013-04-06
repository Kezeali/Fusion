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

#ifndef H_FusionEditor
#define H_FusionEditor

#include "FusionPrerequisites.h"

#include "FusionEngineExtension.h"
#include "FusionResourceEditorFactory.h"

#include "FusionCellStreamTypes.h"
#include "FusionTypes.h"
#include "FusionVectorTypes.h"
#include "FusionResourcePointer.h"
#include "FusionViewport.h"

#include <angelscript.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/signals2/connection.hpp>
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <array>
#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>

namespace Rocket { namespace Core {
	class Context;
	class ElementDocument;
	class EventListener;
} }

void intrusive_ptr_add_ref(asIScriptFunction *ptr);
void intrusive_ptr_release(asIScriptFunction *ptr);

namespace FusionEngine
{

	class ISystemWorld;
	class AngelScriptWorld;
	class Box2DWorld;
	class CLRenderWorld;
	class Viewport;

	class ContextMenu;
	class MenuItem;

	class DialogListener;

	class DockedWindowManager;

	class WorldSaver;
	class SaveDataArchive;

	class EditorOverlay;
	class SelectionDrawer;

	class PreviewFormatter;

	class ResourceDatabase;
	class ResourceBrowserDataSource;

	class ResourceEditor;

	class ShapeTool;
	class EditorPolygonTool;
	class EditorRectangleTool;
	class EditorCircleTool;

	namespace Inspectors {
		class ComponentInspector;
	}

	class InspectorGenerator;

	class WindowDropTarget;

	class Editor : public EngineExtension, public ResourceEditorFactory
	{
	public:
		Editor(const std::vector<std::string> &args);
		virtual ~Editor();

		std::string GetName() const { return "editor"; }

		void CleanUp();

		void Activate();
		void Deactivate();
		bool IsActive() const { return m_Active; }

		bool IsEditor() const { return true; }

		void SetOptions(const ClientOptions& options);
		void SetDisplay(const clan::DisplayWindow& display);
		void SetComponentFactory(const std::shared_ptr<ComponentFactory>& factory) { m_ComponentFactory = factory; }
		void SetEntityInstantiator(const std::shared_ptr<EntityInstantiator>& instantiator) { m_EntityInstantiator = instantiator; }
		void SetEntityManager(const std::shared_ptr<EntityManager>& manager) { m_EntityManager = manager; }
		// TODO: ? abstract class MapLoader with Save and Load methods
		void SetMapLoader(const std::shared_ptr<RegionCellArchivist>& map_loader);
		void SetStreamingManager(const std::shared_ptr<StreamingManager>& manager) { m_StreamingManager = manager; }
		void SetWorldSaver(WorldSaver* saver) { m_Saver = saver; }
		void SetDataArchiver(const std::shared_ptr<SaveDataArchive>& archiver) { m_DataArchiver = archiver; }

		void OnWorldCreated(const std::shared_ptr<ISystemWorld>& world);

		void SetAngelScriptWorld(const std::shared_ptr<AngelScriptWorld>& asw);

		void Update(float time, float dt);

		std::vector<std::shared_ptr<RendererExtension>> MakeRendererExtensions() const;

		const std::shared_ptr<Viewport>& GetViewport() const { return m_Viewport; }
		Rocket::Core::Context* GetGUIContext() const { return m_GUIContext; }

		void Save();
		void Save(const std::string& name);		
		void Load();
		void Load(const std::string& name);

		void Compile(const std::string& name);

		bool DragEnter(const std::string &path);
		bool DragDrop(const std::string &path);

		bool IsResourceEditable(const std::string& file) const;

		virtual std::shared_ptr<ResourceEditor> StartResourceEditor(const std::string& path);

		virtual std::shared_ptr<ResourceEditor> StartResourceEditor(const std::string& path, const Vector2& offset);

		std::string GetResourceType(const std::string& path);

		EntityPtr CreateEntity(const std::string& transform_type, const Vector2& pos, float angle, bool synced, bool streaming);

		//! Move the camera so that the given entity is in view
		void GoToEntity(const EntityPtr& entity);

		void ForEachSelected(std::function<void (const EntityPtr&)> fn);

		void SelectEntity(const EntityPtr& entity);
		void DeselectEntity(const EntityPtr& entity);
		void DeselectAll();

		void SelectEntityWithID(const ObjectID id);
		void GoToEntityWithID(const ObjectID id);

		size_t GetNumSelected() const;
		
		void AddEntityToDelete(const EntityPtr& entity);

		void CreateArchetypeInstance(const std::string& archetype_name, const Vector2& pos, float angle, clan::Rectf selected_area, float grid_size);

		bool TranslateScreenToWorld(ViewportPtr viewport, float* x, float* y) const;
		bool TranslateScreenToWorld(float* x, float* y) const;
		Vector2 ReturnScreenToWorld(float x, float y) const;

		Vector2 GetMousePositionInWindow() const;
		std::pair<Vector2, ViewportPtr> GetPositionInWorldAndViewport(Vector2 pos_in_window) const;
		std::pair<Vector2, ViewportPtr> GetMousePositionInWorldAndViewport() const;
		ViewportPtr GetViewportUnderMouse() const;
		Vector2 GetMousePositionInWorld() const;

		typedef std::function<void (const std::string&)> FileBrowserOverrideResultFn_t;
		typedef std::function<void (const std::string&, const std::string&, const FileBrowserOverrideResultFn_t&)> FileBrowserOverrideFn_t;

		void SetFilebrowserOpenOverride(const FileBrowserOverrideFn_t& fn);

		void SetFilebrowserSaveOverride(const FileBrowserOverrideFn_t& fn);

		typedef std::function<std::shared_ptr<Inspectors::ComponentInspector> (void)> InspectorFactory;

		enum Tool
		{
			None,
			Polygon,
			Line,
			Rectangle,
			Elipse,
			NumTools
		};

	private:
		enum QueryType
		{
			General,
			Physical
		};

		bool m_Active;
		std::string m_OriginalSavePath;

		clan::DisplayWindow m_DisplayWindow;
		std::shared_ptr<AngelScriptWorld> m_AngelScriptWorld;
		std::shared_ptr<Box2DWorld> m_Box2DWorld;
		std::shared_ptr<CLRenderWorld> m_RenderWorld;

		std::shared_ptr<Camera> m_EditCam;
		std::shared_ptr<Viewport> m_Viewport;

		std::shared_ptr<RegionCellArchivist> m_MapLoader;
		std::shared_ptr<StreamingManager> m_StreamingManager;

		std::shared_ptr<ComponentFactory> m_ComponentFactory;
		std::shared_ptr<EntityInstantiator> m_EntityInstantiator;
		std::shared_ptr<EntityManager> m_EntityManager;

		int m_NextFactoryId;
		std::unordered_map<int, std::shared_ptr<boost::signals2::scoped_connection>> m_ArchetypeFactoryLoadConnections;

		WorldSaver* m_Saver;
		std::shared_ptr<SaveDataArchive> m_DataArchiver;

		float m_EditCamRange;

		std::vector<EntityPtr> m_NonStreamedEntities;

		clan::Slot m_KeyDownSlot;
		clan::Slot m_KeyUpSlot;
		clan::Slot m_MouseDownSlot;
		clan::Slot m_MouseUpSlot;
		clan::Slot m_MouseMoveSlot;
		clan::Slot m_WindowResizeSlot;

		bool m_RebuildScripts;
		bool m_CompileMap;
		bool m_SaveMap;
		bool m_LoadMap;

		Rocket::Core::Context* m_GUIContext;

		std::string m_SaveName;

		std::shared_ptr<DialogListener> m_SaveDialogListener;
		std::shared_ptr<DialogListener> m_OpenDialogListener;

		boost::intrusive_ptr<ContextMenu> m_RightClickMenu;
		boost::intrusive_ptr<MenuItem> m_PropertiesMenu;
		boost::intrusive_ptr<MenuItem> m_EntitySelectionMenu;

		std::shared_ptr<PreviewFormatter> m_PreviewFormatter;

		std::shared_ptr<DockedWindowManager> m_DockedWindows;

		std::unordered_map<std::string, InspectorFactory> m_InspectorTypes;

		boost::intrusive_ptr<asIScriptFunction> m_CreateEntityFn;

		std::shared_ptr<ResourceDatabase> m_ResourceDatabase;
		std::shared_ptr<ResourceBrowserDataSource> m_ResourceBrowserDataSource;

		std::map<std::string, std::shared_ptr<ResourceEditor>> m_ResourceEditors;
		
		boost::intrusive_ptr<Rocket::Core::ElementDocument> m_ResourceBrowser;

		boost::intrusive_ptr<Rocket::Core::ElementDocument> m_Background;

		Vector2 m_CamVelocity;

		bool m_ShiftSelect;
		bool m_AltSelect;
		clan::Rectf m_SelectionRectangle;
		std::set<EntityPtr> m_SelectedEntities;

		Vector2 m_DragFrom;
		bool m_ReceivedMouseDown;
		bool m_Dragging;

		Tool m_Tool;

		std::string m_DragData;
		std::shared_ptr<WindowDropTarget> m_DropTarget;

		std::shared_ptr<EditorOverlay> m_EditorOverlay;
		std::shared_ptr<SelectionDrawer> m_SelectionDrawer;

		std::shared_ptr<EditorPolygonTool> m_PolygonTool;
		std::shared_ptr<EditorRectangleTool> m_RectangleTool;
		std::shared_ptr<EditorCircleTool> m_CircleTool;

		std::array<std::shared_ptr<ShapeTool>, Tool::NumTools> m_ShapeTools;

		std::list<EntityPtr> m_ToDelete;

		std::queue<std::function<void (void)>> m_ActionQueue;

		FileBrowserOverrideFn_t m_OpenDialogOverride;
		FileBrowserOverrideFn_t m_SaveDialogOverride;

		void ShowSaveMapDialog();
		void ShowLoadMapDialog();

		void ShowSaveDialog(const std::string& title, const std::string& initial_path);
		void ShowLoadDialog(const std::string& title, const std::string& initial_path);

		void ToggleResourceBrowser();
		bool IsResourceBrowserVisible() const;
		void ShowResourceBrowser();
		void HideResourceBrowser();

		void BuildCreateEntityScript();

		void DeleteEntity(const EntityPtr& entity);

		void SaveEntities(OCellStream& file, const std::set<EntityPtr>& entities);
		std::vector<EntityPtr> LoadEntities(std::shared_ptr<ICellStream> file, const Vector2& offset, float base_angle);

		void CreatePrefab(const std::string& filename, const std::set<EntityPtr>& selected_entities);
		std::vector<EntityPtr> InstantiatePrefab(const std::string& filename, const Vector2& offset, float base_angle);

		void CopySelectedEntities();
		void PasteEntities(const Vector2& top_left, float base_angle = 0.f);

		void NudgeSelectedEntities(const Vector2& pixels);

		void QueueAction(const std::function<void (void)>& action);
		void ExecuteAction();

		void OnKeyDown(const clan::InputEvent& ev);
		void OnKeyUp(const clan::InputEvent& ev);
		void OnMouseDown(const clan::InputEvent& ev);
		void OnMouseUp(const clan::InputEvent& ev);
		void OnMouseMove(const clan::InputEvent& ev);

		void OnMouseDown_Selection(const clan::InputEvent& ev);
		void OnMouseUp_Selection(const clan::InputEvent& ev);

		void OnMouseMove_Move(const clan::InputEvent& ev);

		void OnWindowResize(int x, int y);

		void ShowContextMenu(const Vector2i& position, const std::set<EntityPtr>& entities);

		void UpdateSelectionRectangle(const Vector2& pointer_position, bool translate_position);

		void ForEachSelectedWithColours(std::function<void (const EntityPtr&, const clan::Colorf&)> fn);

		void DoWithArchetypeFactory(const std::string& archetype_name, std::function<void (const ResourcePointer<ArchetypeFactory>&)> fn);

		clan::Rectf GetBBOfSelected();

		void GetEntitiesOverlapping(std::vector<EntityPtr>& results, const clan::Rectf& area, const QueryType query_type);

		void InitInspectorGenerator(InspectorGenerator& generator, const std::function<void (void)>& close_callback);

		void CreatePropertiesWindow(const EntityPtr& entity, const std::function<void (void)>& close_callback = std::function<void (void)>());
		void CreatePropertiesWindow(const std::vector<EntityPtr>& entities, const std::function<void (void)>& close_callback = std::function<void (void)>());
		void CreatePropertiesWindowForSelected();

		void RegisterScriptType(asIScriptEngine* engine);

	};

}

#endif
