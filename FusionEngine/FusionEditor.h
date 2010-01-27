/*
  Copyright (c) 2009-2010 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
		
		
	File Author(s):

		Elliot Hayward

*/

#ifndef Header_FusionEngine_Editor
#define Header_FusionEngine_Editor

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionState.h"

#include "FusionIDStack.h"
#include "FusionViewport.h"
#include "FusionInputHandler.h"
#include "FusionGameMapLoader.h"
#include "FusionContextMenu.h"
#include "FusionElementSelectableDataGrid.h"
#include "FusionElementUndoMenu.h"
#include "FusionEditorUndo.h"

#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/ElementDocument.h>
#include <EMP/Core/DataSource.h>
#include <Rocket/Controls/ElementFormControlSelect.h>

#include <containers/structured_set.hpp>


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

	class PropertyEditorDialog;
	typedef std::tr1::shared_ptr<PropertyEditorDialog> PropertyEditorDialogPtr;

	//! Editor system (runs the map editor interface)
	class Editor : public System, public Rocket::Core::EventListener
	{
	public:
		//! Constructor
		Editor(InputManager *input, Renderer *renderer, EntityFactory *entity_factory, PhysicalWorld *world, StreamingManager *streaming_manager, GameMapLoader *map_util);
		virtual ~Editor();

	public:
		// TODO: use these typedefs
		typedef GameMapLoader::GameMapEntity MapEntity;
		typedef GameMapLoader::GameMapEntityPtr MapEntityPtr;
		typedef GameMapLoader::GameMapEntityArray MapEntityArray;

		const std::string &GetName() const;

		//! System Init implementation
		bool Initialise();

		void CleanUp();

		void Update(float split);

		void Draw();

		//! Starts Editor mode
		/*!
		* If a game map is loaded, it will be cleared. If Initialise() hasn't been called yet, the
		* editor will be enabled when Initialise() is called.
		*/
		void Enable(bool enable = true);

		void OnRawInput(const RawInput &ev);

		void ProcessEvent(Rocket::Core::Event& ev);

		void DisplayError(const std::string &title, const std::string &message);
		void ShowContextMenu(const Vector2i &position, const MapEntityArray &entities);
		void ShowProperties(const EntityPtr &entity);
		void ShowProperties(const MapEntityPtr &entity);

		MapEntityPtr CreateEntity(const std::string &type, const std::string &name, bool pseudo, float x, float y);

		void GetEntitiesAt(MapEntityArray &out, const Vector2 &position);

		void AddEntity(const GameMapLoader::GameMapEntityPtr &entity);
		void RemoveEntity(const GameMapLoader::GameMapEntityPtr &entity);

		// The following methods are primarily for scripting use
		//! Returns entity-tyes beginning with...
		void LookUpEntityType(StringVector &results, const std::string &search_term);
		//! Updates the EditorDataSource with the results of the look up on the given name
		void LookUpEntityType(const std::string &search_term);

		const std::string &GetSuggestion(size_t index);

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
		void SpawnEntities(EntityManager *manager);

		static void Register(asIScriptEngine *engine);

	protected:
		class EditorEntityDeserialiser : public EntityDeserialiseImpl
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
		//EntityManager *m_EntityManager;
		GameMapLoader *m_MapUtil;

		boost::signals2::connection m_RawInputConnection;

		ViewportPtr m_Viewport;
		CameraPtr m_Camera;

		Vector2 m_CamVelocity;

		UndoableActionManager m_UndoManager;
		//ElementUndoMenu *m_UndoMenu;

		typedef std::vector<PropertyEditorDialogPtr> PropertyDialogArray;
		PropertyDialogArray m_PropertyDialogs;

		Rocket::Core::ElementDocument *m_MainDocument;
		//Rocket::Core::ElementDocument *m_EntityListDocument;

		ContextMenu *m_RightClickMenu;
		MenuItem *m_PropertiesMenu;
		typedef std::vector<boost::signals2::connection> MenuItemConnections;
		MenuItemConnections m_PropertiesMenuConnections;

		bool m_Enabled;

		std::string m_CurrentFilename;

		std::string m_CurrentEntityType;
		bool m_PseudoEntityMode;

		typedef containers::structured_set<std::string> EntityTypeLookupSet;
		EntityTypeLookupSet m_EntityTypes;

		EditorDataSource *m_EditorDataSource;

		EntityArray m_PlainEntityArray;

		// Map data (entities, etc.) - passed to map creator ("GameMapLoader")
		StringSet m_UsedTypes;
		GameMapLoader::ArchetypeMap m_Archetypes;
		GameMapLoader::GameMapEntityArray m_PseudoEntities;
		GameMapLoader::GameMapEntityArray m_Entities;

		IDStack m_IdStack;

		void addUndoAction(const UndoableActionPtr &action);
		void repopulateUndoMenu();

		void showProperties(const MenuItemEvent &ev, const MapEntityPtr &entity);

		//! Lists the given entity in the relevant containers
		void addMapEntity(const GameMapLoader::GameMapEntityPtr &entity);

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
		inline void initialiseEntities(const GameMapLoader::GameMapEntityArray::iterator &first, const GameMapLoader::GameMapEntityArray::iterator &last, const Editor::SerialisedDataArray &data, const Editor::EditorEntityDeserialiser &deserialiser_impl);

	};

}

#endif
