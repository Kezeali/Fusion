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

#ifndef Header_FusionEditor
#define Header_FusionEditor

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <EMP/Core/DataSource.h>
#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Controls/ElementFormControlSelect.h>

#include <containers/structured_set.hpp>

#include "FusionState.h"

#include "FusionContextMenu.h"
#include "FusionEditorUndo.h"
#include "FusionElementSelectableDataGrid.h"
#include "FusionElementUndoMenu.h"
#include "FusionGameMapLoader.h"
#include "FusionIDStack.h"
#include "FusionInputHandler.h"
#include "FusionScriptModule.h"
#include "FusionViewport.h"
#include "FusionXML.h"

namespace FusionEngine
{

	//! Provides lists of Entity types
	class EditorDataSource : public EMP::Core::DataSource
	{
	public:
		EditorDataSource();
		virtual ~EditorDataSource();

		void UpdateSuggestions(const StringVector &suggestions);
		const std::string &GetSuggestion(size_t index);

		void GetRow(EMP::Core::StringList& row, const EMP::Core::String& table, int row_index, const EMP::Core::StringList& columns);
		int GetNumRows(const EMP::Core::String& table);

	protected:
		StringVector m_Suggestions;
		const EntityArray *m_Entities;
	};

	class EntityEditorDialog;
	typedef std::tr1::shared_ptr<EntityEditorDialog> EntityEditorDialogPtr;

	//! Editor system (runs the map editor interface)
	class Editor : public System, public Rocket::Core::EventListener
	{
	public:
		//! Constructor
		Editor(InputManager *input, EntityFactory *entity_factory, Renderer *renderer, InstancingSynchroniser *instancing, PhysicalWorld *world, StreamingManager *streaming_manager, GameMapLoader *map_util, EntityManager *entity_manager);
		//! DTOR
		virtual ~Editor();

	public:
		typedef GameMapLoader::MapEntity MapEntity;
		typedef GameMapLoader::MapEntityPtr MapEntityPtr;
		typedef GameMapLoader::MapEntityArray MapEntityArray;

		//! Returns the name of this system ("editor")
		const std::string &GetName() const;

		//! Initialise
		bool Initialise();
		//! Clean up
		void CleanUp();
		//! Update
		void Update(float split);
		//! Draw
		void Draw();

		//! Starts the editor (doesn't stop the ontology, so you should do that first)
		void Start();
		//! Stops the editor
		void Stop();

		//! Sets the module that entity scripts are added to
		void SetEntityModule(const ModulePtr &module);
		//! Called when the entity module is rebuilt
		void OnBuildEntities(BuildModuleEvent &ev);

		//! Starts Editor mode
		/*!
		* If a game map is loaded, it will be cleared. If Initialise() hasn't been called yet, the
		* editor will be enabled when Initialise() is called.
		*/
		void Enable(bool enable = true);

		void OnRawInput(const RawInput &ev);

		void ProcessEvent(Rocket::Core::Event& ev);

		//! Shows a message box containing the given entities
		void ShowMessage(const std::string& title, const std::string& message, const MapEntityArray& entities, std::function<void (const MapEntityPtr&)> accept_callback);
		//! Opens a message box with an error message
		void DisplayError(const std::string &title, const std::string &message);
		void ShowContextMenu(const Vector2i &position, const MapEntityArray &entities);
		void ShowProperties(const EntityPtr &entity);
		void ShowProperties(const MapEntityPtr &entity);

		//! Creates a new map-entity
		EditorMapEntityPtr CreateEntity(const std::string &type, const std::string &name, bool pseudo, float x, float y);

		void GetEntitiesAt(MapEntityArray &out, const Vector2 &position, bool convert_to_world_coords = false);
		void GetEntitiesOverlapping(MapEntityArray &out, const CL_Rectf &rectangle, bool convert_to_world_coords = false);

		//! Adds the given map-Entity to the Editor
		void AddEntity(const MapEntityPtr &map_entity);
		//! Removes the given map-Entity from the Editor
		void RemoveEntity(const MapEntityPtr &map_entity);

		//! Looks for a MapEntity that points to the given Entity instance
		MapEntityPtr FindEntity(const EntityPtr& entity);

		void SelectEntity(const MapEntityPtr &map_entity);
		void DeselectEntity(const MapEntityPtr &map_entity);
		void DeselectAll();

		void selectEntity(const EditorMapEntityPtr &map_entity);
		void deselectEntity(const EditorMapEntityPtr &map_entity);

		// The following methods are primarily for scripting use
		//! Returns entity-tyes beginning with...
		void LookUpEntityType(StringVector &results, const std::string &search_term);
		//! Updates the EditorDataSource with the results of the look up on the given name
		void LookUpEntityType(const std::string &search_term);

		const std::string &GetSuggestion(size_t index);

		void SetDisplayActualSprites(bool enable);

		enum EditorTool
		{
			tool_place,
			tool_delete,
			tool_move
		};
		void SetActiveTool(EditorTool tool);

		void SetEntityType(const std::string &type);
		void SetEntityMode(bool pseudo);

		void AttachUndoMenu(ElementUndoMenu *element);
		void AttachRedoMenu(ElementUndoMenu *element);

		void Undo();
		void Redo();
		void Undo(unsigned int index);
		void Redo(unsigned int index);

		void StartEditor();
		void StopEditor();

		void Save(const std::string &filename);
		void Load(const std::string &filename);

		void Save();
		void Load();

		void Close();

		void Compile(const std::string &filename);

		//! Adds all Entities in the current map to the given entity manager
		/*!
		* \remarks
		* The manager will be cleared first, so you may want to save the game in
		* order to not annoy the user
		*/
		void SpawnEntities();

		static void Register(asIScriptEngine *engine);

	protected:
		class EditorEntityDeserialiser : public IEntityRepo
		{
		public:
			void ListEntity(const EntityPtr &entity);
			EntityPtr GetEntity(ObjectID id) const;
		private:
			typedef std::tr1::unordered_map<ObjectID, EntityPtr> EntityIdMap;
			EntityIdMap m_EntityMap;
		};

		Renderer *m_Renderer;
		StreamingManager *m_Streamer;
		InputManager *m_Input;
		PhysicalWorld *m_PhysicalWorld;
		EntityFactory *m_EntityFactory;
		InstancingSynchroniser *m_InstanceSynchroniser;
		EntityManager *m_EntityManager;
		GameMapLoader *m_MapUtil;

		boost::signals2::connection m_RawInputConnection;
		boost::signals2::connection m_EntityBuildConnection;

		ViewportPtr m_Viewport;
		CameraPtr m_Camera;

		Vector2 m_CamVelocity;

		bool m_ReceivedMouseDown;
		bool m_ShiftSelect;
		bool m_AltSelect;
		bool m_DragSelect;

		UndoableActionManager m_UndoManager;
		//ElementUndoMenu *m_UndoMenu;

		typedef std::vector<EntityEditorDialogPtr> EntityEditorDialogArray;
		EntityEditorDialogArray m_EntityDialogs;

		Rocket::Core::ElementDocument *m_MainDocument;
		//Rocket::Core::ElementDocument *m_EntityListDocument;

		ContextMenu *m_RightClickMenu;

		MenuItem *m_PropertiesMenu;
		MenuItem *m_EntitySelectionMenu;

		typedef std::vector<boost::signals2::connection> MenuItemConnections;
		MenuItemConnections m_PropertiesMenuConnections;
		MenuItemConnections m_SelectionMenuConnections;

		bool m_Enabled;

		EditorTool m_ActiveTool;
		bool m_Dragging;
		Vector2 m_DragFrom;
		CL_Rectf m_SelectionRectangle;
		// In order to optimise dragging by only moving the selection boxes when the
		//  offset has changed significantly, this is set to the current drag offset
		//  every time the selection boxes are moved
		Vector2 m_LastDragMove;

		CL_Sprite m_SelectionOverlay;
		CL_Sprite m_SelectionOverlay_Rotate;
		CL_Sprite m_SelectionOverlay_Resize;

		std::string m_CurrentFilename;

		std::string m_CurrentEntityType;
		bool m_PseudoEntityMode;

		typedef containers::structured_set<std::string> EntityTypeLookupSet;
		EntityTypeLookupSet m_EntityTypes;

		EditorDataSource *m_EditorDataSource;

		EntityArray m_PlainEntityArray;

		typedef std::set<EditorMapEntityPtr> EntitySet;
		EntitySet m_SelectedEntities;
		std::unordered_set<EditorMapEntity*> m_EntitiesWithinSelectionRectangle;

		// Map data (entities, etc.) - passed to map builder (GameMapLoader)
		StringSet m_UsedTypes;
		GameMapLoader::ArchetypeMap m_Archetypes;
		GameMapLoader::MapEntityArray m_PseudoEntities;
		GameMapLoader::MapEntityArray m_Entities;

		ObjectIDStack m_IdStack;

		inline void processLeftClick(const RawInput &ev);
		inline void processRightClick(const RawInput &ev);

		void generateSelectionOverlay();

		void addUndoAction(const UndoableActionPtr &action);
		void repopulateUndoMenu();

		void clearCtxMenu(MenuItem *menu, Editor::MenuItemConnections &connections);

		//! Called when a Properties menu-item is clicked
		void ctxShowProperties(const MenuItemEvent &ev, MapEntity *entity);

		//! Called when a Select menu-item is clicked
		void ctxSelectEntity(const MenuItemEvent &ev, MapEntity *entity);

		//! Updates the selection rectangle
		/*!
		* \param[in]
		* The position of the pointer, either on the screen or within the
		* game world (see next parameter)
		* \param[in] translate_to_world
		* If an on-screen position is given for the pointer, this should
		* be set to true so that the position can be translated to an
		* in-world point.
		*/
		void updateSelectionRectangle(const Vector2& pointer_position, bool translate_to_world);
		//! Moves the bottom right corner of the selection rectangle
		void updateSelectionRectangleByOffset(const Vector2& offset);

		//void markForSelection(const EditorMapEntity& map_entity, bool mark);

		//! Lists the given entity in the relevant containers
		void addMapEntity(const GameMapLoader::MapEntityPtr &entity);

		//! Serialises and saves the state data of all map entities to the given file
		void serialiseEntityData(CL_IODevice file);

		typedef std::vector<SerialisedData> SerialisedDataArray;
		//! Reads serialised entity data from the given file
		void loadEntityData(CL_IODevice file, SerialisedDataArray &archetypes, SerialisedDataArray &entities);

		//! Creates an XML document for the current map
		void buildMapXml(TiXmlDocument *document);
		//! Reads the given Map XML doc.
		void parseMapXml(TiXmlDocument *document, const SerialisedDataArray &archetypes, const SerialisedDataArray &entities, EditorEntityDeserialiser &translator);

		//! Reads the given <archetypes> element from an XML map doc
		void parse_Archetypes(TiXmlElement *element, const SerialisedDataArray &archetype_data);
		//! Reads the given <entities> element from an XML map doc.
		void parse_Entities(TiXmlElement *element, unsigned int entity_count, EditorEntityDeserialiser &translator);

		// Deserialises state data
		void initialiseEntities(const SerialisedDataArray &entity_data, const EditorEntityDeserialiser &translator);
		// To reduce code repitition
		inline void initialiseEntities(const GameMapLoader::MapEntityArray::iterator &first, const GameMapLoader::MapEntityArray::iterator &last, const Editor::SerialisedDataArray &data, const Editor::EditorEntityDeserialiser &deserialiser_impl);

	};

}

#endif
