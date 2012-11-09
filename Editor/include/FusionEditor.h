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

#include "FusionTypes.h"
#include "FusionVectorTypes.h"
#include "FusionResourcePointer.h"

#include <angelscript.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/signals2/connection.hpp>
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <array>
#include <functional>
#include <memory>
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

	class Editor : public EngineExtension
	{
	public:
		Editor(const std::vector<CL_String> &args);
		virtual ~Editor();

		std::string GetName() const { return "editor"; }

		void CleanUp();

		void Activate();
		void Deactivate();
		bool IsActive() const { return m_Active; }

		bool IsEditor() const { return true; }

		void SetOptions(const ClientOptions& options);
		void SetDisplay(const CL_DisplayWindow& display);
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

		bool IsResourceEditable(const std::string& file) const;

		void StartResourceEditor(const std::string& path);

		//! Move the camera so that the given entity is in view
		void GoToEntity(const EntityPtr& entity);

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

		CL_DisplayWindow m_DisplayWindow;
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

		CL_Slot m_KeyDownSlot;
		CL_Slot m_KeyUpSlot;
		CL_Slot m_MouseDownSlot;
		CL_Slot m_MouseUpSlot;
		CL_Slot m_MouseMoveSlot;

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

		Vector2 m_CamVelocity;

		bool m_ShiftSelect;
		bool m_AltSelect;
		CL_Rectf m_SelectionRectangle;
		std::set<EntityPtr> m_SelectedEntities;

		Vector2 m_DragFrom;
		bool m_ReceivedMouseDown;
		bool m_Dragging;

		Tool m_Tool;

		std::shared_ptr<EditorOverlay> m_EditorOverlay;
		std::shared_ptr<SelectionDrawer> m_SelectionDrawer;

		std::shared_ptr<EditorPolygonTool> m_PolygonTool;
		std::shared_ptr<EditorRectangleTool> m_RectangleTool;
		std::shared_ptr<EditorCircleTool> m_CircleTool;

		std::array<std::shared_ptr<ShapeTool>, Tool::NumTools> m_ShapeTools;

		std::list<EntityPtr> m_ToDelete;

		void ShowSaveDialog();
		void ShowLoadDialog();

		void ToggleResourceBrowser();
		bool IsResourceBrowserVisible() const;
		void ShowResourceBrowser();
		void HideResourceBrowser();

		void Save();
		void Load();

		void BuildCreateEntityScript();

		EntityPtr CreateEntity(const std::string& transform_type, const Vector2& pos, float angle, bool synced, bool streaming);
		void DeleteEntity(const EntityPtr& entity);
		void AddEntityToDelete(const EntityPtr& entity);

		void CopySelectedEntities();
		void PasteEntities(const Vector2& top_left, float base_angle = 0.f);

		void NudgeSelectedEntities(const Vector2& pixels);

		void OnKeyDown(const CL_InputEvent& ev, const CL_InputState& state);
		void OnKeyUp(const CL_InputEvent& ev, const CL_InputState& state);
		void OnMouseDown(const CL_InputEvent& ev, const CL_InputState& state);
		void OnMouseUp(const CL_InputEvent& ev, const CL_InputState& state);
		void OnMouseMove(const CL_InputEvent& ev, const CL_InputState& state);

		void OnMouseDown_Selection(const CL_InputEvent& ev);
		void OnMouseUp_Selection(const CL_InputEvent& ev);

		void OnMouseMove_Move(const CL_InputEvent& ev);

		void ShowContextMenu(const Vector2i& position, const std::set<EntityPtr>& entities);

		bool TranslateScreenToWorld(float* x, float* y) const;
		Vector2 ReturnScreenToWorld(float x, float y) const;
		void UpdateSelectionRectangle(const Vector2& pointer_position, bool translate_position);

		void SelectEntity(const EntityPtr& entity);
		void DeselectEntity(const EntityPtr& entity);
		void DeselectAll();

		size_t GetNumSelected() const;
		void ForEachSelected(std::function<bool (const EntityPtr&)> fn);
		void ForEachSelectedWithColours(std::function<bool (const EntityPtr&, const CL_Colorf&)> fn);

		void DoWithArchetypeFactory(const std::string& archetype_name, std::function<bool (const ResourcePointer<ArchetypeFactory>&)> fn);

		CL_Rectf GetBBOfSelected();

		void GetEntitiesOverlapping(std::vector<EntityPtr>& results, const CL_Rectf& area, const QueryType query_type);

		void InitInspectorGenerator(InspectorGenerator& generator, const std::function<void (void)>& close_callback);

		void CreatePropertiesWindow(const EntityPtr& entity, const std::function<void (void)>& close_callback = std::function<void (void)>());
		void CreatePropertiesWindow(const std::vector<EntityPtr>& entities, const std::function<void (void)>& close_callback = std::function<void (void)>());
		void CreatePropertiesWindowForSelected();

		void RegisterScriptType(asIScriptEngine* engine);

	};

}

#endif
